#define EIGEN_NO_DEBUG 1
//   TODO: PUT ABOVE BACK

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
using Eigen::ArrayXd;
using Eigen::Map;
using std::cout;
using std::endl;

#define length 6
#define NUM_THREADS 1
// Careful: ensure that NUM_THREADS divides 2^(2*length-1) (basically always will for l > 3 if power of 2)
#define CALC_EVERY_X_ITERATIONS 1

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
const uint64_t TOTAL_VEC_SIZE = powminus1 * sizeof(double);
//(2 * ONE_GB)
#define SINGLE_MAP_SIZE (8 * 32)
#define SINGLE_MAP_ENTRIES (SINGLE_MAP_SIZE / sizeof(double))

#define STOP_AT 0
#define LOOP_CHUNK_SIZE ((powminus0 * sizeof(double)) / (1 << STOP_AT))

void recursive_driver(double **filemapsv1, double **filemapsv2, double *readmap, double *writemap);

template <typename Derived>
void printArray(const Eigen::ArrayBase<Derived> &arr) {
    for (int i = 0; i < arr.size(); i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}
void printArrayMap(const double *arr, const int len) {
    cout << "Printing arraymap of given size " << len << endl;
    for (int i = 0; i < len; i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}

void printFileMap(double **arr) {
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
    return fd;
}
// https://stackoverflow.com/questions/21119617/does-mmap-or-malloc-allocate-ram
//  TODO: CAN CLOSE FILE DESCRIPTOR AFTER INITING MAP?

// TODO: is this passing by value?
void closeFile(int fd, double *map) {
    if (munmap(map, TOTAL_VEC_SIZE) == -1) {
        perror("Error un-mmapping the file");
    }
    close(fd);
}

double *prepareMap(int fd) {
    double *map;
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
    // msync or something may be necessary
    map = (double *)mmap(NULL, SINGLE_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return map;
}

// check this https://stackoverflow.com/questions/13024087/what-are-memory-mapped-page-and-anonymous-page

// Create enough (shared, anonymous) mmaps for one vector
void initVectorMaps(double **vecmmaps, std::string fileprefix, const bool zeroInit) {
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
            // TODO: parallelize
            std::memset(vecmmaps[i], 0, SINGLE_MAP_SIZE);
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

void copy_mem(double *memmap, double **filemaps, uint64_t elements, uint64_t read_offset, bool file_to_mem) {
    // printArrayMap(memmap, LOOP_CHUNK_SIZE / (4 * sizeof(bytes)));
    //  TODO: parallelize
    for (uint64_t i = 0; i < elements / SINGLE_MAP_ENTRIES; i++) {
        uint64_t index = (read_offset / SINGLE_MAP_ENTRIES) + i;
        if (index * SINGLE_MAP_SIZE > TOTAL_VEC_SIZE) {
            cout << "There is likely a mistake in copy_mem\n";
        }
        // cout << i << " " << index << endl;
        // std::flush(cout);
        if (file_to_mem) {
            std::memcpy((void *)(memmap + SINGLE_MAP_ENTRIES * i), (void *)filemaps[index], SINGLE_MAP_SIZE);
        } else {
            std::memcpy((void *)filemaps[index], (void *)(memmap + SINGLE_MAP_ENTRIES * i), SINGLE_MAP_SIZE);
        }
    }
    // TODO: can also optimize tthis so that it can do things in reverse, to avoid needing to store/write
    //  half of F_12
}
/* Normally, we find R with R = (v2 - v1).maxCoeff();
    However, this is slower than the multithreaded F functions if not parallelized!
    So instead we break that calculation up across threads.
    Additionally, E = std::max(0.0, (v2 + 2 * R - v1).maxCoeff()). We can use
    this function for that as well, and just add 2*R at the end. */
double subtract_and_find_min_parallel(double *readmap, double **v1, double **v2) {
    const uint64_t half_map_size = (LOOP_CHUNK_SIZE / sizeof(double)) / 2;
    auto find_min_par = [&readmap, &half_map_size]() {
        std::future<double> minVals[NUM_THREADS];
        const uint64_t incr = half_map_size / NUM_THREADS;

        // Function to calculate the maximum coefficient in a particular (start...end) slice
        auto findMin = [&readmap, &half_map_size](uint64_t start, uint64_t end) {
            double minCoef = INFINITY;
            for (uint64_t str = start; str < end; str++) {
                minCoef = std::min(minCoef, readmap[str + half_map_size] - readmap[str]);
            }
            return minCoef;
        };

        // THIS FUNCTION SHOULDNT NEED AS MUCH CHANGING. CAN JUST SET EACH THREAD TO HANDLE THE MIN BETWEEN
        // 2GB AND (TOTAL SIZE)/NUM_THREADS, DOING MANY ITERATIONS AS NECESSARY
        // Set threads to calculate the max coef in their own smaller slices using above function
        for (int i = 0; i < NUM_THREADS; i++) {
            minVals[i] = std::async(std::launch::async, findMin, incr * i, incr * (i + 1));
        }

        // Now calculate the global max
        double R = minVals[0].get();  // .get() waits until the thread completes
        for (int i = 1; i < NUM_THREADS; i++) {
            double coef = minVals[i].get();
            if (coef < R) {
                R = coef;
            }
        }
        return R;
    };
    // TODO: optimization where you use the fact tthat the last values from ehre are read in (potentially you can set it
    // up so that itt has nice values in there), so you dont have to read in as many things at next iterattion of F_01
    double R = INFINITY;
    // this can be improved by not waiting for threads. instad check constanttly for ready tthreads
    for (uint64_t i = 0; i < (2 * TOTAL_VEC_SIZE / LOOP_CHUNK_SIZE); i++) {
        uint64_t offset = i * (LOOP_CHUNK_SIZE / sizeof(double)) / 2;
        // put v1 into first half of readmap
        copy_mem(readmap, v1, half_map_size, offset, true);
        // put v2 into second half of readmap
        copy_mem(readmap + half_map_size, v2, half_map_size, offset, true);
        R = std::min(R, find_min_par());
    }
    return R;
}

void F_01_LOOP(const uint64_t start, const uint64_t end, const uint64_t index_offset, const uint64_t index_offset_rev,
               const double *v, double *ret) {
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
        // cout << str << ": " << ATB0 + index_offset << " " << ATB1 + index_offset << " " << TA0B + index_offset_rev
        //      << " " << TA1B + index_offset_rev << "             " << ATB0 << " " << ATB1 << " " << TA0B << " " <<
        //      TA1B
        //      << endl;
        // cout << v[ATB0] << " " << v[ATB1] << " " << v[TA0B] << " " << v[TA1B] << endl;
        // cout << index_offset << " " << index_offset_rev << endl;

        // cout << str << ": " << ATB0 << " " << ATB1 << " " << TA0B << " " << TA1B << endl;
        //" " << v[TA0B] << " " <<
        //  v[TA0B]<< endl; cout << ATB0 << " " << ATB1 << " " << TA0B + index_offset_rev << " " << TA1B +
        //  index_offset_rev << endl;

        ret[str - start] = std::max(v[ATB0] + v[ATB1], v[TA0B] + v[TA1B]) / 2;  // if h(A) != h(B) and h(A) = z
        // cout << (str - start) << ", " << ret[str - start] << endl;

        // cout << ATB0 << " " << ATB1 << " " << v[ATB0] << " " << v[ATB1] << endl;
    }
}

void F_12_LOOP(const uint64_t str_start, const uint64_t str_end, double *v, double *ret) {
    const uint64_t arr_bound = 2 * (str_end - str_start) - 1;
    const uint64_t offset = str_start << 2;
    auto loop = [&v, &ret, &arr_bound, &offset](uint64_t start, uint64_t end) {
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

            // cout << str - start << " " << arr_bound - str << endl;
            // TODO: may be able to optimize these memory accesses
            ret[str - start] = 1 + .25 * (v[TA0TB0] + v[TA0TB1] + v[TA1TB0] + v[TA1TB1]);
            ret[arr_bound - (str - start)] = ret[str - start];  // array is self-symmetric!
            // cout << "F12: " << (str - start) << " " << arr_bound - (str - start) << " " << ret[str - start] << endl;

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

// todo: look into posix_inform

void recursive_driver(double **filemapsv1, double **filemapsv2, double *readmap, double *writemap) {
    // THIS RECURSION MIGHT NOT BE CORRECT. MAYBE HAVE TO (IN DEPTH=0) CHECK FIRST BIT OF A, AND THAT
    // DETERMINES WHETHER IT GETS REVERSED GOING FORWARD.
    std::function<std::pair<uint64_t, uint64_t>(uint64_t, uint64_t, uint64_t, uint64_t[2], int, int, bool)>
        recurseF_01_loop2_3part;
    recurseF_01_loop2_3part = [&recurseF_01_loop2_3part](uint64_t target_str, uint64_t target_num_strs, uint64_t offset,
                                                         uint64_t index_offsets[2], int depth, int stage,
                                                         bool reverse) -> std::pair<uint64_t, uint64_t> {
        // num_strs = powminus(2+depth)
        uint64_t num_strs = uint64_t(1) << (2 * (length)-2 - depth);
        if (num_strs > target_num_strs) {
            switch (stage) {
                case 1: {
                    if (target_str < offset + num_strs * 4 / 8) {
                        uint64_t index_offsets_next[2] = {index_offsets[0], 0};
                        return recurseF_01_loop2_3part(target_str, target_num_strs, offset, index_offsets_next,
                                                       depth + 1, 2, false);
                    } else {
                        uint64_t index_offsets_next[2] = {index_offsets[0] + 2 * num_strs * 4 / 8, 0};
                        return recurseF_01_loop2_3part(target_str, target_num_strs, offset + num_strs * 4 / 8,
                                                       index_offsets_next, depth + 1, 2, true);
                    }
                    break;
                }
                case 2: {
                    if (target_str < offset + num_strs * 2 / 4) {
                        if (reverse) {
                            uint64_t index_offsets_next[2] = {index_offsets[0] + 2 * num_strs * 3 / 4,
                                                              index_offsets[0] + 2 * num_strs * 1 / 4};
                            return recurseF_01_loop2_3part(target_str, target_num_strs, offset, index_offsets_next,
                                                           depth + 1, 3, true);
                        } else {
                            uint64_t index_offsets_next[2] = {index_offsets[0],
                                                              index_offsets[0] + 2 * num_strs * 2 / 4};
                            return recurseF_01_loop2_3part(target_str, target_num_strs, offset, index_offsets_next,
                                                           depth + 1, 3, false);
                        }
                    } else {
                        if (reverse) {
                            uint64_t index_offsets_next[2] = {index_offsets[0] + 2 * num_strs * 2 / 4,
                                                              index_offsets[0]};
                            return recurseF_01_loop2_3part(target_str, target_num_strs, offset + num_strs * 2 / 4,
                                                           index_offsets_next, depth + 1, 3, true);
                        } else {
                            uint64_t index_offsets_next[2] = {index_offsets[0] + 2 * num_strs * 1 / 4,
                                                              index_offsets[0] + 2 * num_strs * 3 / 4};
                            return recurseF_01_loop2_3part(target_str, target_num_strs, offset + num_strs * 2 / 4,
                                                           index_offsets_next, depth + 1, 3, false);
                        }
                    }
                    break;
                }
                case 3: {
                    if (target_str < offset + num_strs / 2) {
                        if (reverse) {
                            uint64_t index_offsets_next[2] = {index_offsets[0], 0};
                            return recurseF_01_loop2_3part(target_str, target_num_strs, offset, index_offsets_next,
                                                           depth + 1, 1, false);
                        } else {
                            uint64_t index_offsets_next[2] = {index_offsets[0], 0};
                            return recurseF_01_loop2_3part(target_str, target_num_strs, offset, index_offsets_next,
                                                           depth + 1, 1, false);
                        }
                    } else {
                        if (reverse) {
                            uint64_t index_offsets_next[2] = {index_offsets[1], 0};
                            return recurseF_01_loop2_3part(target_str, target_num_strs, offset + num_strs / 2,
                                                           index_offsets_next, depth + 1, 1, false);
                        } else {
                            uint64_t index_offsets_next[2] = {index_offsets[1], 0};
                            return recurseF_01_loop2_3part(target_str, target_num_strs, offset + num_strs / 2,
                                                           index_offsets_next, depth + 1, 1, false);
                        }
                    }
                    break;
                }
                default: {
                    std::pair<uint64_t, uint64_t> index(0, 0);
                    return index;
                }
            }

        } else {
            if (stage == 3) {
                std::pair<uint64_t, uint64_t> index(index_offsets[0], index_offsets[1]);
                return index;
                // return two indices
            } else {
                std::pair<uint64_t, uint64_t> index(index_offsets[0], index_offsets[0]);
                return index;
                // return one index
            }
            // uint64_t start_i = index_offset;
            // uint64_t end_i = start_i + 2 * num_strs;
            // // load in arr[start_i] thru arr[end_i]
            // // printf("[REV] Str start, end: %i, %i.  Index start, end: %i, %i\n", target_str + powminus1,
            // //        target_str + num_strs + powminus1, start_i, end_i);
            // cout << "IN RECURSE: " << index_offset << " " << stage << endl;
            // return start_i;
        }
    };

    std::function<void(uint64_t[2], uint64_t, int, int)> recurseF_01_loop1_2part;
    recurseF_01_loop1_2part = [&filemapsv1, &filemapsv2, &readmap, &writemap, &recurseF_01_loop1_2part,
                               &recurseF_01_loop2_3part](uint64_t offsets[2], uint64_t index_offset, int depth,
                                                         int stage) -> void {
        // num_strs = powminus(2+depth)
        uint64_t num_strs = uint64_t(1) << (2 * (length)-2 - depth);
        if (depth < STOP_AT) {
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
            uint64_t num_bytes = 2 * num_strs * sizeof(double);
            uint64_t num_bytes_write = num_strs * sizeof(double);
            copy_mem(readmap, filemapsv1, num_strs * 2, start_i, true);
            uint64_t default_arr[2] = {0, 0};

            // TODO: REUSE AS MUCH MEM AS POSSIBLE BY OVERLAPPING
            if (stage == 1) {
                uint64_t start = powminus2 + offsets[0];
                uint64_t end = start + num_strs;
                std::pair<uint64_t, uint64_t> access_ranges =
                    recurseF_01_loop2_3part(powminus1 - end, num_strs, 0, default_arr, 0, 1, false);
                cout << "Stage 1 " << access_ranges.first << " " << access_ranges.second << endl;

                if (access_ranges.first == access_ranges.second) {
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, 2 * num_strs, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);
                    copy_mem(writemap, filemapsv2, num_strs, start, false);

                    //                     cout << "writing f01 about to start " << num_strs << " " << start << " " <<
                    //                     end << " "
                    //      << reverse_start_i << endl;
                    // std::flush(cout);

                } else {  // need to access in two ranges
                    end = start + num_strs / 2;
                    uint64_t reverse_start_i = access_ranges.second;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);

                    start = end;
                    end = start + num_strs / 2;
                    reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap + num_strs / 2);

                    copy_mem(writemap, filemapsv2, num_strs, start - num_strs / 2, false);
                }

            } else {  // stage == 2
                      // To read in the memory sequentially, need to iterate over two ranges of str
                      // Further, we don't know if the index we need to access for reverse part of each of these two
                      // ranges is broken into one or two.

                uint64_t start = powminus2 + offsets[0];
                cout << "start: " << start << endl;
                uint64_t end = start + num_strs / 2;
                std::pair<uint64_t, uint64_t> access_ranges =
                    recurseF_01_loop2_3part(powminus1 - end, num_strs / 2, 0, default_arr, 0, 1, false);
                cout << "Stage 2 half 1 " << access_ranges.first << " " << access_ranges.second << endl;

                if (access_ranges.first == access_ranges.second) {
                    cout << "Acc ranges equal" << endl;
                    std::flush(cout);
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);
                    copy_mem(writemap, filemapsv2, num_strs / 2, start, false);

                    //                     cout << "writing f01 about to start " << num_strs << " " << start << " " <<
                    //                     end << " "
                    //      << reverse_start_i << endl;
                    // std::flush(cout);

                } else {  // need to access in two ranges
                    // FIXME: debugging process should start by looking at the reverse_start_i and making sure they are
                    // correct
                    cout << "Acc ranges NOT equal" << endl;
                    std::flush(cout);
                    end = start + num_strs / 4;
                    uint64_t reverse_start_i = access_ranges.second;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs / 2, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);
                    cout << start << " " << reverse_start_i << endl;

                    start = end;
                    end = start + num_strs / 4;
                    reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs / 2, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap + num_strs / 4);

                    copy_mem(writemap, filemapsv2, num_strs / 2, start - num_strs / 4, false);
                    cout << start << " " << reverse_start_i << endl;
                }

                // DO SECOND HALF
                start = powminus2 + offsets[1];
                end = start + num_strs / 2;
                // feed the recurse searcher so it recurses down until it finds target_str, and returns the indexes to
                // load in for that
                access_ranges = recurseF_01_loop2_3part(powminus1 - end, num_strs / 2, 0, default_arr, 0, 1, false);
                cout << "Stage 2 half 2 " << access_ranges.first << " " << access_ranges.second << endl;

                if (access_ranges.first == access_ranges.second) {
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);
                    copy_mem(writemap, filemapsv2, num_strs / 2, start, false);

                    //                     cout << "writing f01 about to start " << num_strs << " " << start << " " <<
                    //                     end << " "
                    //      << reverse_start_i << endl;
                    // std::flush(cout);

                } else {  // need to access in two ranges
                    cout << "Acc ranges NOT equal (half2)" << endl;
                    std::flush(cout);
                    end = start + num_strs / 4;
                    // REMEMBER: YOU SWAPPED .SECOND WITH .FIRST, UNSURE WHICH SHOULD BE WHICH
                    uint64_t reverse_start_i = access_ranges.second;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs / 2, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);
                    cout << start << " " << reverse_start_i << endl;

                    start = end;
                    end = start + num_strs / 4;
                    reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs / 2, reverse_start_i, true);
                    F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap + num_strs / 4);

                    copy_mem(writemap, filemapsv2, num_strs / 2, start - num_strs / 4, false);
                    cout << start << " " << reverse_start_i << endl;
                }
            }
            // printFileMap(filemapsv1, TOTAL_VEC_SIZE / SINGLE_MAP_SIZE);
            // cout << "FIRST22 PRINT DONE\n";
            // printFileMap(filemapsv2);
            // cout << "FIRST PRINT DONE\n";
            // printArrayMap(readmap, LOOP_CHUNK_SIZE / sizeof(double));
            // cout << "FIRST PRINT DONE2222222\n";

            // LOOP F_12
            const uint64_t loop2_start = (start_i >> 2);
            const uint64_t loop2_end = loop2_start + num_strs / 2;
            // writes num_strs / 2 (twice as much) strs
            F_12_LOOP(loop2_start, loop2_end, readmap, writemap);
            // printArrayMap(writemap, LOOP_CHUNK_SIZE / (4 * sizeof(double)));
            cout << "WRITE MAP ABOVE\n";

            // then do two write chunks:
            // write first half of write array
            // cout << loop2_start << " " << loop2_end << " " << (num_bytes_write / 2) << " "
            //      << (LOOP_CHUNK_SIZE / (4 * 2)) << endl;
            copy_mem(writemap, filemapsv2, num_strs / 2, loop2_start, false);
            // cout << "writing AAAAAAA done" << endl;
            // std::flush(cout);
            // write second half of write array
            // printFileMap(filemapsv2);
            // cout << endl;
            // printArrayMap(writemap, LOOP_CHUNK_SIZE / (4 * sizeof(double)));
            // cout << loop2_end << " " << powminus2 - loop2_end << endl;
            copy_mem(writemap + num_strs / 2, filemapsv2, num_strs / 2, powminus2 - loop2_end, false);
            // printArrayMap(writemap, LOOP_CHUNK_SIZE / (4 * sizeof(double)));
            // cout << "writing BBBBBBBB done" << endl;
            // std::flush(cout);
            // printFileMap(filemapsv2);

            // cout << "V2 ABOVE\n";
            return;
        }
    };
    uint64_t begin_arr[2] = {0, 0};
    recurseF_01_loop1_2part(begin_arr, 0, 0, 1);
    cout << endl;
}

double *make_ram_map(uint64_t numbytes) {
    // TODO: put back MAP_HUGE_TLB FLAG
    double *readmap = (double *)mmap(NULL, numbytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (readmap == MAP_FAILED) {
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }
    return readmap;
}

void FeasibleTriplet(int n) {
    auto start = std::chrono::system_clock::now();
    double *v1maps[TOTAL_VEC_SIZE / SINGLE_MAP_SIZE];  // contains pointers to the maps that comprise v1
    initVectorMaps(v1maps, "./mmappedv1", true);
    double *v2maps[TOTAL_VEC_SIZE / SINGLE_MAP_SIZE];  // contains pointers to the maps that comprise v2
    initVectorMaps(v2maps, "./mmappedv2", false);

    // printf("Maps prepared in %f seconds\n", secondsSince(start));

    // Create two mmaps: one to read from, one to write to
    // map to read from should be 4x as large as map to write to
    double *readmap = make_ram_map(LOOP_CHUNK_SIZE);
    double *writemap = make_ram_map(LOOP_CHUNK_SIZE / 4);

    double r = 0;
    double e = 0;

    for (int i = 2; i < n + 1; i++) {
        // Writes new vector (v2) into v2
        auto start2 = std::chrono::system_clock::now();
        recursive_driver(v1maps, v2maps, readmap, writemap);
        printFileMap(v2maps);

        cout << "Elapsed time F (s): " << secondsSince(start2) << endl;
        if (i % CALC_EVERY_X_ITERATIONS == 0 || i == n) {
            const double R = subtract_and_find_min_parallel(readmap, v1maps, v2maps);

            r = R;
            cout << R << endl;
            if (PRINT_EVERY_ITER) {
                printf("At n=%i: %.9f\n", i, 2 * r / (1 + r));
                cout << "Elapsed time (s): " << secondsSince(start) << endl;
            }

        } else if (PRINT_EVERY_ITER) {
            printf("Elapsed time (s) at n=%i: %f\n", i, secondsSince(start));
        }
        // Swap pointers of v1 and v2
        std::swap(v1maps, v2maps);
    }

    // return u, r, e
    // cout << "Result: " << 2.0 * (r - e) << endl;
    printf("Single Vec Result: %.9f\n", 2.0 * r / (1 + r));

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
    cout << "Vector size in bytes (entries): " << TOTAL_VEC_SIZE << " (" << (TOTAL_VEC_SIZE / sizeof(double)) << ")\n";
    cout << "Map size in bytes: " << SINGLE_MAP_SIZE << " (" << TOTAL_VEC_SIZE / SINGLE_MAP_SIZE << " maps)\n";
    cout << "Subdivided chunk size in bytes (entries): " << LOOP_CHUNK_SIZE << " ("
         << (LOOP_CHUNK_SIZE / sizeof(double)) << ")\n";
    cout << "Threads: " << NUM_THREADS << " (+1)" << endl;
    auto start = std::chrono::system_clock::now();
    FeasibleTriplet(2);
    cout << "Elapsed time (s): " << secondsSince(start) << endl;
    // TODO: check if eigen init parallel is something to do
    return 0;
}