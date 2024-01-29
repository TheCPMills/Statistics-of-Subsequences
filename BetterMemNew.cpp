#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <Eigen/Dense>
#include <bitset>
#include <chrono>
#include <cmath>
#include <ctime>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
using std::cout;
using std::endl;

#define length 20
#define NUM_THREADS 32
// Careful: ensure that NUM_THREADS divides 2^(2*length-1) (basically always will for l > 3 if power of 2)
#define CALC_EVERY_X_ITERATIONS 15

#define FIXED_POINT_SCALE 25000000

// 1024^3 bytes
#define ONE_GB uint64_t(1073741824)

const bool PRINT_EVERY_ITER = true;

// equal to pow(2, 2 * length)
const uint64_t powminus0 = uint64_t(1) << (2 * length);
// equal to pow(2, 2 * length - 1)
const uint64_t powminus1 = uint64_t(1) << ((2 * length) - 1);
// equal to pow(2, 2 * length - 2)
const uint64_t powminus2 = uint64_t(1) << ((2 * length) - 2);
// equal to pow(2, 2 * length - 3)
const uint64_t powminus3 = uint64_t(1) << ((2 * length) - 3);

// define the size of the file (in bytes) needed to store a single full vector
const uint64_t TOTAL_VEC_SIZE = (powminus1 * sizeof(uint32_t));
#define SINGLE_MAP_SIZE (2 * ONE_GB)
#define SINGLE_MAP_ENTRIES (SINGLE_MAP_SIZE / sizeof(uint32_t))

#define RECURSE_STOP_DEPTH 6
#define LOOP_CHUNK_SIZE ((powminus0 * sizeof(uint32_t)) / (1 << RECURSE_STOP_DEPTH))

void recursive_driver(uint32_t **filemapsv1, uint32_t **filemapsv2, uint32_t *readmap, uint32_t *writemap);

template <typename Derived>
void printArray(const Eigen::ArrayBase<Derived> &arr) {
    for (int i = 0; i < arr.size(); i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}
void printArrayMap(const uint32_t *arr, const int len) {
    cout << "Printing arraymap of given size " << len << endl;
    for (int i = 0; i < len; i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}
void printFileMap(uint32_t **arr) {
    for (int i = 0; i < TOTAL_VEC_SIZE / SINGLE_MAP_SIZE; i++) {
        for (int j = 0; j < SINGLE_MAP_ENTRIES; j++) {
            cout << arr[i][j] << " ";
        }
    }
    cout << endl;
}

double secondsSince(std::chrono::system_clock::time_point startTime) {
    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endTime - startTime;
    return elapsed_seconds.count();
}

int openFile(std::string filepath) {
    /* Open a file for writing.
     *  - Creating the file if it doesn't exist.
     *  - Truncating it to 0 size if it already exists. (not really needed) [NOTE: PROBABLY HARMFUL FOR US?]
     *
     * Note: "O_WRONLY" mode is not sufficient when mmaping.
     */
    int fd = open(filepath.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
    return fd;
}
// https://stackoverflow.com/questions/21119617/does-mmap-or-malloc-allocate-ram
//  TODO: CAN CLOSE FILE DESCRIPTOR AFTER INITING MAP?

// TODO: is this passing by value?
void closeFile(int fd, uint32_t *map) {
    if (munmap(map, TOTAL_VEC_SIZE) == -1) {
        perror("Error un-mmapping the file");
    }
    close(fd);
}

uint32_t *prepareMap(int fd) {
    uint32_t *map;
    /* Stretch file size to the size of the vector we need to store*/
    if (lseek(fd, TOTAL_VEC_SIZE - 1, SEEK_SET) == -1) {
        close(fd);
        perror("Error calling lseek() to 'stretch' the file");
        exit(EXIT_FAILURE);
    }

    /* Write something at end of file so file actually has the new size.
     * Current position is at end of stretched file due to lseek() call.
     * Write a single zero-byte (or anything, really) as last byte of file.
     */
    if (write(fd, "", 1) != 1) {
        close(fd);
        perror("Error writing last byte of the file");
        exit(EXIT_FAILURE);
    }

    /* Now file is ready to be mmapped.*/
    // TODO: probably want to set as bianry
    // TODO: INVESTIGATE MMAP FLAGS
    map = (uint32_t *)mmap(NULL, SINGLE_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the file (in prepareMap)");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return map;
}

// check this https://stackoverflow.com/questions/13024087/what-are-memory-mapped-page-and-anonymous-page

// Create enough (shared, anonymous) mmaps for one vector
void initVectorMaps(uint32_t **vecmmaps, std::string fileprefix, const bool zeroInit) {
    if (TOTAL_VEC_SIZE < SINGLE_MAP_SIZE) {
        cout << "NO MMAPING NEEDED PLS RECONSIDER, THIS WILL PROBABLY BREAK\n";
    }
    if (TOTAL_VEC_SIZE % SINGLE_MAP_SIZE != 0) {
        cout << "Error: SINGLE_MAP_SIZE must divide TOTAL_VEC_SIZE!";
    }
    const int num_maps = TOTAL_VEC_SIZE / SINGLE_MAP_SIZE;
    for (int i = 0; i < num_maps; i++) {
        // Names probably not actually be necessary for anon maps
        int fd = openFile((fileprefix + "-" + std::to_string(i) + ".bin").c_str());
        vecmmaps[i] = prepareMap(fd);  // todo: verify is copying by reference
        if (zeroInit) {
            // Zero initialize (memset) in a parallel manner
            auto setmem = [](uint32_t *vecmap, uint64_t num_bytes) -> void { std::memset(vecmap, 0, num_bytes); };
            std::thread threads[NUM_THREADS];
            const uint64_t incr =
                (SINGLE_MAP_SIZE) / (sizeof(uint32_t) * NUM_THREADS);  // Careful to make sure NUM_THREADS is a divisor!
            for (int j = 0; j < NUM_THREADS; j++) {
                threads[j] = std::thread(setmem, vecmmaps[i] + incr * j, incr * sizeof(uint32_t));
            }
            for (int j = 0; j < NUM_THREADS; j++) {
                threads[j].join();
            }
        }
        if (num_maps >= 10) {
            if (i % (num_maps / 10) == 0) {
                printf("Done making %i of %i maps...\n", i, num_maps);
            }
        }
    }
}

// UNUSED?
const char *map_file(const char *fname, uint64_t bytes) {
    int fd = open(fname, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(-1);
    }

    const char *addr = static_cast<const char *>(mmap(NULL, bytes, PROT_READ, MAP_PRIVATE, fd, 0u));
    if (addr == MAP_FAILED) {
        perror("Error mapping file");
        exit(-1);
    }

    // TODO close fd at some point in time, call munmap(...)
    return addr;
}

void copy_mem(uint32_t *memmap, uint32_t **filemaps, uint64_t elements, uint64_t read_offset, bool file_to_mem) {
    auto parallel_copy = [&file_to_mem](uint32_t *memmap, uint32_t *filemap) -> void {
        std::thread threads[NUM_THREADS];
        const uint64_t incr = (SINGLE_MAP_ENTRIES) / (NUM_THREADS);  // Careful to make sure NUM_THREADS is a divisor!
        for (int i = 0; i < NUM_THREADS; i++) {
            if (file_to_mem) {
                threads[i] =
                    std::thread(std::memcpy, memmap + incr * i, filemap + incr * i, SINGLE_MAP_SIZE / NUM_THREADS);
            } else {
                threads[i] =
                    std::thread(std::memcpy, filemap + incr * i, memmap + incr * i, SINGLE_MAP_SIZE / NUM_THREADS);
            }
        }
        for (int i = 0; i < NUM_THREADS; i++) {
            threads[i].join();
        }
    };
    //  TODO: parallelize
    for (uint64_t i = 0; i < elements / SINGLE_MAP_ENTRIES; i++) {
        uint64_t index = (read_offset / SINGLE_MAP_ENTRIES) + i;
        if (index * SINGLE_MAP_SIZE > TOTAL_VEC_SIZE) {
            cout << "There is likely a mistake in copy_mem\n";
        }
        parallel_copy(memmap + SINGLE_MAP_ENTRIES * i, filemaps[index]);
    }
    // TODO: can also optimize tthis so that it can do things in reverse, to avoid needing to store/write
    //  half of F_12
}
/* Normally, we find E with E = (v2 - v1).minCoeff();
    However, this is slower than the multithreaded F functions if not parallelized!
    So instead we break that calculation up across threads. */
uint32_t subtract_and_find_min_parallel(uint32_t *readmap, uint32_t **v1, uint32_t **v2) {
    const uint64_t half_map_size = (LOOP_CHUNK_SIZE / sizeof(uint32_t)) / 2;
    // Lambda function to make threads to find min in largest chunk that can fit in readmap
    auto find_min_par = [&readmap, &half_map_size]() {
        std::future<uint32_t> minVals[NUM_THREADS];
        const uint64_t incr = half_map_size / NUM_THREADS;

        // Function to calculate the maximum coefficient in a particular (start...end) slice
        auto findMin = [&readmap, &half_map_size](uint64_t start, uint64_t end) {
            uint32_t minCoef = 0 - 1;
            for (uint64_t str = start; str < end; str++) {
                minCoef = std::min(minCoef, readmap[str + half_map_size] - readmap[str]);
            }
            return minCoef;
        };

        // Set threads to calculate the max coef in their own smaller slices using above function
        for (int i = 0; i < NUM_THREADS; i++) {
            minVals[i] = std::async(std::launch::async, findMin, incr * i, incr * (i + 1));
        }

        // Now calculate the global max
        uint32_t E = minVals[0].get();  // .get() waits until the thread completes
        for (int i = 1; i < NUM_THREADS; i++) {
            uint32_t coef = minVals[i].get();
            if (coef < E) {
                E = coef;
            }
        }
        return E;
    };
    // TODO: optimization where you use the fact tthat the last values from ehre are read in (potentially you can set it
    // up so that itt has nice values in there), so you dont have to read in as many things at next iterattion of F_01
    uint32_t E = 0 - 1;
    // this can be improved by not waiting for threads. instad check constanttly for ready tthreads
    for (uint64_t i = 0; i < (2 * TOTAL_VEC_SIZE / LOOP_CHUNK_SIZE); i++) {
        uint64_t offset = i * (LOOP_CHUNK_SIZE / sizeof(uint32_t)) / 2;
        // put v1 into first half of readmap
        copy_mem(readmap, v1, half_map_size, offset, true);
        // put v2 into second half of readmap
        copy_mem(readmap + half_map_size, v2, half_map_size, offset, true);
        E = std::min(E, find_min_par());
    }
    return E;
}

void F_01_LOOP(const uint64_t str_start, const uint64_t str_end, const uint64_t index_offset,
               const uint64_t index_offset_rev, const uint32_t *v, uint32_t *ret) {
    auto loop = [&v, &ret, &index_offset, &index_offset_rev, &str_start](uint64_t start, uint64_t end) {
        for (uint64_t str = start; str < end; str++) {
            // Take every other bit (starting at first position)
            const uint64_t A = str & 0xAAAAAAAAAAAAAAAA;
            // Take take every other bit (starting at second position)
            // The second & gets rid of the first bit
            const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
            uint64_t ATB0 = A | (TB << 2);
            uint64_t ATB1 = ATB0 | 1;  // 0b1 <- the smallest bit in B is set to 1

            ATB0 = ATB0 - index_offset;
            ATB1 = ATB1 - index_offset;

            // LOOP 2
            const uint64_t str_r = powminus0 - 1 - str;
            const uint64_t TA = (str_r & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);
            // Take every other bit (starting at second position)
            const uint64_t B = str_r & 0x5555555555555555;
            uint64_t TA0B = (TA << 2) | B;
            uint64_t TA1B = TA0B | 2;

            TA0B = std::min(TA0B, (powminus0 - 1) - TA0B) - index_offset_rev;
            TA1B = std::min(TA1B, (powminus0 - 1) - TA1B) - index_offset_rev;

            // A lot of ugly reassignment to avoid 'temporary' overflows when adding
            ret[str - str_start] = uint32_t(std::max(uint64_t(v[ATB0]) + uint64_t(v[ATB1]),
                                                     uint64_t(v[TA0B]) + uint64_t(v[TA1B])) >>
                                            1);  // if h(A) != h(B) and h(A) = z
        }
    };
    std::thread threads[NUM_THREADS];
    const uint64_t incr = (str_end - str_start) / (NUM_THREADS);  // Careful to make sure NUM_THREADS is a divisor!
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(loop, str_start + incr * i, str_start + incr * (i + 1));
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }
}

void F_12_LOOP(const uint64_t str_start, const uint64_t str_end, uint32_t *v, uint32_t *ret) {
    const uint64_t arr_bound = 2 * (str_end - str_start) - 1;
    const uint64_t offset = str_start << 2;
    auto loop = [&v, &ret, &arr_bound, &offset, &str_start](uint64_t start, uint64_t end) {
        for (uint64_t str = start; str < end; str++) {
            // const uint64_t TA = (str & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);
            // const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
            // uint64_t TA0TB0 = (TA << 2) | (TB << 2);
            const uint64_t TA0TB0 = ((str & (powminus2 - 1)) << 2) - offset;  // equivalent to above 3 lines!
            const uint64_t TA0TB1 = TA0TB0 | 0b1;
            const uint64_t TA1TB0 = TA0TB0 | 0b10;
            const uint64_t TA1TB1 = TA0TB0 | 0b11;

            // These used to be necessary, but are really just asking if str >= powminus3.
            // But now that we're using the symmetry, this will never be the case since str
            // only reaches powminus3 -1.
            // TA0TB0 = std::min(TA0TB0, (powminus0 - 1) - TA0TB0);
            // TA0TB1 = std::min(TA0TB1, (powminus0 - 1) - TA0TB1);
            // TA1TB0 = std::min(TA1TB0, (powminus0 - 1) - TA1TB0);
            // TA1TB1 = std::min(TA1TB1, (powminus0 - 1) - TA1TB1);

            // TODO: may be able to optimize these memory accesses
            // A lot of ugly reassignment to avoid 'temporary' overflows when adding
            ret[str - str_start] =
                1 * FIXED_POINT_SCALE +
                uint32_t(uint64_t(v[TA0TB0]) + uint64_t(v[TA0TB1]) + uint64_t(v[TA1TB0]) + uint64_t(v[TA1TB1]) >> 2);
            ret[arr_bound - (str - str_start)] = ret[str - str_start];  // array is self-symmetric!
            // TODO: see if can use this symmetry to reduce space needed
        }
    };
    std::thread threads[NUM_THREADS];
    const uint64_t incr = (str_end - str_start) / (NUM_THREADS);  // Careful to make sure NUM_THREADS is a divisor!
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(loop, str_start + incr * i, str_start + incr * (i + 1));
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    // return ret;
}

// todo: look into posix_fadvise

std::pair<uint64_t, bool> inv_idx_of(uint64_t start_str) {
    uint64_t TA = (start_str & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);
    uint64_t B = start_str & 0x5555555555555555;
    uint64_t TA0B = (TA << 2) | B;
    // TA1B = TA0B | 2
    bool reverse = false;
    if (TA0B >= powminus1) {  // invert to symmetric location within array
        reverse = true;
        TA0B = (powminus0 - 1) - TA0B;
        // TA1B = (powminus0 -1) - TA1B
    }
    std::pair<uint64_t, bool> index(TA0B, reverse);
    return index;
}

void recursive_driver(uint32_t **filemapsv1, uint32_t **filemapsv2, uint32_t *readmap, uint32_t *writemap) {
    // NUM_STRS IS LIKELY MISMATCHED WITH THE ACCESS CALLER
    auto inv_access_ranges_of = [](uint64_t start_str, uint64_t num_strs) -> std::pair<uint64_t, uint64_t> {
        std::pair<uint64_t, bool> idx_and_is_reverse = inv_idx_of(start_str);
        uint64_t TA0B = idx_and_is_reverse.first;
        bool reverse = idx_and_is_reverse.second;

        // If reversed (access was >= powm1), then accessing will be in reverse
        // So subtract num of strings so that TA0B...TA0B+num_strs is still accurate
        if (reverse) {
            if (int(log2(num_strs)) % 2 == 1) TA0B -= 2 * num_strs - 1;
            // Will have to be split in two
            else
                TA0B -= num_strs - 1;
        }
        const uint64_t acc1 = TA0B;
        uint64_t acc2 = TA0B;

        // If has to be split in two
        if (int(log2(num_strs)) % 2 == 0) {
            uint64_t str2 = start_str + num_strs / 2;
            idx_and_is_reverse = inv_idx_of(str2);
            TA0B = idx_and_is_reverse.first;
            reverse = idx_and_is_reverse.second;

            if (reverse) TA0B -= num_strs - 1;
            acc2 = TA0B;
        }

        std::pair<uint64_t, uint64_t> range_starts(acc1, acc2);
        return range_starts;
    };

    std::function<void(uint64_t[2], uint64_t, int, int)> recurseF_01_loop1_2part;
    recurseF_01_loop1_2part = [&filemapsv1, &filemapsv2, &readmap, &writemap, &recurseF_01_loop1_2part,
                               &inv_access_ranges_of](uint64_t offsets[2], uint64_t index_offset, int depth,
                                                      int stage) -> void {
        // num_strs = powminus(2+depth)
        uint64_t num_strs = uint64_t(1) << (2 * (length)-2 - depth);
        if (depth < RECURSE_STOP_DEPTH) {
            switch (stage) {
                case 1: {
                    uint64_t offsets_next[2] = {offsets[0], offsets[0] + num_strs * 2 / 4};
                    recurseF_01_loop1_2part(offsets_next, index_offset, depth + 1, 2);

                    offsets_next[0] = offsets[0] + num_strs / 4;
                    offsets_next[1] = offsets[0] + num_strs * 3 / 4;
                    recurseF_01_loop1_2part(offsets_next, index_offset + 2 * num_strs / 2, depth + 1, 2);
                    break;
                }
                case 2: {
                    uint64_t offsets_next[2] = {offsets[0], 0};
                    recurseF_01_loop1_2part(offsets_next, index_offset, depth + 1, 1);

                    offsets_next[0] = offsets[1];
                    recurseF_01_loop1_2part(offsets_next, index_offset + 2 * num_strs / 2, depth + 1, 1);
                    break;
                }
            }

        } else {
            uint64_t start_i = index_offset;
            uint64_t end_i = start_i + 2 * num_strs;
            // load in arr[start_i] thru arr[end_i]
            uint64_t num_bytes = 2 * num_strs * sizeof(uint32_t);
            uint64_t num_bytes_write = num_strs * sizeof(uint32_t);
            copy_mem(readmap, filemapsv1, num_strs * 2, start_i, true);
            uint64_t default_arr[2] = {0, 0};

            // TODO: REUSE AS MUCH MEM AS POSSIBLE BY OVERLAPPING
            if (stage == 1) {
                uint64_t start = powminus2 + offsets[0];
                uint64_t end = start + num_strs;
                std::pair<uint64_t, uint64_t> access_ranges = inv_access_ranges_of(start, num_strs);
                if (access_ranges.first == access_ranges.second) {
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, 2 * num_strs, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);
                    copy_mem(writemap, filemapsv2, num_strs, start, false);

                } else {  // need to access in two ranges
                    end = start + num_strs / 2;
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);

                    start = end;
                    end = start + num_strs / 2;
                    reverse_start_i = access_ranges.second;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap + num_strs / 2);

                    copy_mem(writemap, filemapsv2, num_strs, start - num_strs / 2, false);
                }

            } else {  // stage == 2
                      // To read in the memory sequentially, need to iterate over two ranges of str
                      // Further, we don't know if the index we need to access for reverse part of each of these two
                      // ranges is broken into one or two.
                uint64_t start = powminus2 + offsets[0];
                uint64_t end = start + num_strs / 2;
                std::pair<uint64_t, uint64_t> access_ranges = inv_access_ranges_of(start, num_strs / 2);
                if (access_ranges.first == access_ranges.second) {
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);
                    copy_mem(writemap, filemapsv2, num_strs / 2, start, false);

                } else {  // need to access in two ranges
                    end = start + num_strs / 4;
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs / 2, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);

                    start = end;
                    end = start + num_strs / 4;
                    reverse_start_i = access_ranges.second;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs / 2, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap + num_strs / 4);

                    copy_mem(writemap, filemapsv2, num_strs / 2, start - num_strs / 4, false);
                }

                // DO SECOND HALF
                start = powminus2 + offsets[1];
                end = start + num_strs / 2;
                access_ranges = inv_access_ranges_of(start, num_strs / 2);
                if (access_ranges.first == access_ranges.second) {
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);
                    copy_mem(writemap, filemapsv2, num_strs / 2, start, false);

                } else {  // need to access in two ranges
                    end = start + num_strs / 4;
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs / 2, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);

                    start = end;
                    end = start + num_strs / 4;
                    reverse_start_i = access_ranges.second;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs / 2, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap + num_strs / 4);

                    copy_mem(writemap, filemapsv2, num_strs / 2, start - num_strs / 4, false);
                }
            }

            // LOOP F_12
            const uint64_t loop2_start = (start_i >> 2);
            const uint64_t loop2_end = loop2_start + num_strs / 2;
            // writes num_strs * 2 (twice as much) strs, in two blocks that are contiguous w.r.t. themselves, but not
            // necessarily each other
            F_12_LOOP(loop2_start, loop2_end, readmap, writemap);
            // then do two write chunks:
            // write first half of write array
            copy_mem(writemap, filemapsv2, num_strs / 2, loop2_start, false);
            // write second half of write array
            copy_mem(writemap + num_strs / 2, filemapsv2, num_strs / 2, powminus2 - loop2_end, false);

            return;
        }
    };
    uint64_t begin_arr[2] = {0, 0};
    recurseF_01_loop1_2part(begin_arr, 0, 0, 1);
    cout << endl;
}

uint32_t *make_ram_map(uint64_t numbytes) {
    // TODO: put back MAP_HUGETLB FLAG
    uint32_t *readmap = (uint32_t *)mmap(NULL, numbytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (readmap == MAP_FAILED) {
        perror("Error mmapping the file (in make_ram_map)");
        exit(EXIT_FAILURE);
    }
    return readmap;
}

void FeasibleTriplet(int n) {
    auto start = std::chrono::system_clock::now();
    uint32_t *v1maps[TOTAL_VEC_SIZE / SINGLE_MAP_SIZE];  // contains pointers to the maps that comprise v1
    initVectorMaps(v1maps, "./mmappedv1", true);
    printf("Maps for v1 prepared in %f seconds\n", secondsSince(start));
    uint32_t *v2maps[TOTAL_VEC_SIZE / SINGLE_MAP_SIZE];  // contains pointers to the maps that comprise v2
    initVectorMaps(v2maps, "./mmappedv2", false);

    printf("Maps prepared in %f seconds\n", secondsSince(start));
    std::flush(cout);
    // Create two mmaps: one to read from, one to write to
    // Map to read from is 4x as large as map to write to
    uint32_t *readmap = make_ram_map(LOOP_CHUNK_SIZE);
    uint32_t *writemap = make_ram_map(LOOP_CHUNK_SIZE / 4);

    double e = 0;
    long long int prev_min_change = 0;

    for (int i = 2; i < n + 1; i++) {
        auto start2 = std::chrono::system_clock::now();
        // Writes new vector into v2
        recursive_driver(v1maps, v2maps, readmap, writemap);
        cout << "Elapsed time F (s): " << secondsSince(start2) << endl;

        if (i % CALC_EVERY_X_ITERATIONS == 0 || i == n) {
            const uint32_t min_change = subtract_and_find_min_parallel(readmap, v1maps, v2maps);
            e = double(min_change) / FIXED_POINT_SCALE;
            cout << min_change << endl;
            printf("At n=%i: %.9f.\nElapsed time (s): %f\n", i, 2 * e / (1 + e), secondsSince(start));

            if (min_change - prev_min_change < 3 && min_change - prev_min_change != 0) {
                cout << "Minimum changes no longer exceeding tolerance, convergence reached! Quitting...\n";
                break;
            }
            prev_min_change = min_change;

        } else if (PRINT_EVERY_ITER) {
            printf("Elapsed time (s) at n=%i: %f\n", i, secondsSince(start));
        }
        // Swap pointers of v1 and v2
        std::swap(v1maps, v2maps);
    }

    // return u, r, e
    // cout << "Result: " << 2.0 * (r - e) << endl;
    printf("Final lower bound: %.9f\n", 2.0 * e / (1 + e));

    cout << "Performing cleanup...\n";
    const int num_maps = TOTAL_VEC_SIZE / SINGLE_MAP_SIZE;
    for (int i = 0; i < num_maps; i++) {
        munmap(v1maps[i], SINGLE_MAP_SIZE);
        munmap(v2maps[i], SINGLE_MAP_SIZE);
        std::remove(("./mmappedv1-" + std::to_string(i) + ".bin").c_str());
        std::remove(("./mmappedv2-" + std::to_string(i) + ".bin").c_str());
    }
}

int main() {
    cout << "Starting with l = " << length << "..." << endl;
    cout << "Vector size in bytes (entries): " << TOTAL_VEC_SIZE << " (" << (TOTAL_VEC_SIZE / sizeof(uint32_t))
         << ")\n";
    cout << "Map size in bytes: " << SINGLE_MAP_SIZE << " (" << TOTAL_VEC_SIZE / SINGLE_MAP_SIZE << " maps)\n";
    cout << "Subdivided chunk size in bytes (entries), STOP_DEPTH: " << LOOP_CHUNK_SIZE << " ("
         << (LOOP_CHUNK_SIZE / sizeof(uint32_t)) << "), " << RECURSE_STOP_DEPTH << endl;
    cout << "Threads: " << NUM_THREADS << " (+1)" << endl;
    auto start = std::chrono::system_clock::now();
    FeasibleTriplet(200);
    cout << "Elapsed time (s): " << secondsSince(start) << endl;
    // TODO: check if eigen init parallel is something to do
    return 0;
}

// STOP_DEPTH = 6
// request 90GB mem
// MAP_SIZE = 2GB *2 (i put at 2GB/2 for l=17)
// length = 20
// CALC_EVERY 15