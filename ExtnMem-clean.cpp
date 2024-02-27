#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <bitset>
#include <chrono>
#include <cmath>
#include <cstring>
#include <ctime>
#include <future>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
using std::cout;
using std::endl;

// 1024^3 bytes, GB as in Gibibyte, not Gigabyte
#define ONE_GB uint64_t(1073741824)

/*                  USER PARAMETERS
#########################################################
 * Make sure to follow all instructions here. There are currently no guardrails
 * against incorrect parameters.*/

/* Length of the strings. For this verion, do not set < 5.*/
#define length 13

/* To write to/from external memory, we create a number of files for each vector. This parameter determines the size of
 * each individual file, in bytes.
 * - MUST be a divisor of TOTAL_VEC_SIZE. This means it must be a power of 2, and <= TOTAL_VEC_SIZE.
 * - Must be > TOTAL_VEC_SIZE/512 (i.e., TOTAL_VEC_SIZE/256 or larger). This is because most systems have a hard limit
 * of 1024 open files. If you manually adjust that limit yourself in system preferences, you are able to set this
 * parameter to <= TOTAL_VEC_SIZE/512.
 * - For larger vectors, i.e. length >18, we've found that using more files (up to 256 per vector) is generally a good
 * idea. Computers don't usually like single files that are hundreds of GB large.*/
#define SINGLE_FILE_SIZE (2 * ONE_GB / (4 * 4 * 4 * 4 * 4))

/* This parameter determines how deep the recursion goes. How much RAM the program uses depends only on this parameter
 * and the length parameter.
 * - The program uses (5/4) * 4 *2^(2*length) / 2^(STOP_DEPTH) = (5/4) * LOOP_CHUNK_SIZE bytes of RAM.
 * - There's basically no reason to ever set RECURSE_STOP_DEPTH lower than 2, since at 0 and 1 it's strictly worse than
 * the RAM-only version (it uses the same or more RAM, but is way slower).*/
#define RECURSE_STOP_DEPTH 5

/* Number of threads to use.*/
#define NUM_THREADS 1

/* Calculate a bound every this many iterations. Larger values will make the code run faster, but with the caveat that
 * it may take more iterations to complete. We recommend setting to ~10 or 15 for best performance.*/
#define CALC_EVERY_X_ITERATIONS 10

/* Fixed point scale to use for fixed point arithmetic.
 * Note: it is best to set at 2^25 = 33,554,432 for length <= 18,
 * and 2^24 = 16,777,216 for length >= 19 (to avoid possibility of overflow).
 * However, for consistency with Lueker, we set it to 25000000.*/
#define FIXED_POINT_SCALE 25000000
/* Remove the files generated after running?*/
#define REMOVE_FILES true
/* Print how long each copy to/from external memory took?*/
#define PRINT_MEM_COPIES false

// TODO: allow NUM_THREADS to be anything

/*######################################################*/

// equal to pow(2, 2 * length)
const uint64_t powminus0 = uint64_t(1) << (2 * length);
// equal to pow(2, 2 * length - 1)
const uint64_t powminus1 = uint64_t(1) << ((2 * length) - 1);
// equal to pow(2, 2 * length - 2)
const uint64_t powminus2 = uint64_t(1) << ((2 * length) - 2);
// equal to pow(2, 2 * length - 3)
const uint64_t powminus3 = uint64_t(1) << ((2 * length) - 3);

// Define the size of the file (in bytes) needed to store a single full vector
const uint64_t TOTAL_VEC_SIZE = (powminus1 * sizeof(uint32_t));
#define SINGLE_FILE_ENTRIES (SINGLE_FILE_SIZE / sizeof(uint32_t))
#define LOOP_CHUNK_SIZE ((powminus0 * sizeof(uint32_t)) / (1 << RECURSE_STOP_DEPTH))

// Convenience functions
void printArrayMap(const uint32_t *arr, const int len) {
    cout << "Printing arraymap of given size " << len << "\n";
    for (int i = 0; i < len; i++) {
        cout << arr[i] << " ";
    }
    cout << "\n";
}
void printFileMap(uint32_t **arr) {
    for (uint64_t i = 0; i < TOTAL_VEC_SIZE / SINGLE_FILE_SIZE; i++) {
        for (uint64_t j = 0; j < SINGLE_FILE_ENTRIES; j++) {
            cout << arr[i][j] << " ";
        }
    }
    cout << "\n";
}
double secondsSince(std::chrono::system_clock::time_point startTime) {
    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endTime - startTime;
    return elapsed_seconds.count();
}

/*#######################################################################################
 SUPPORTING CODE FOR READING/WRITING FILES. NEEDED, BUT MOSTLY JUST TECHNICAL.
 IT IS NOT NECESSARY TO UNDERSTAND THE INS AND OUTS OF THESE PARTS. ALL THEY DO IS ALLOW FOR
 READING AND WRITING TO EXTERNAL FILES IN PARALLEL, BUT ARE NOT THE RECURSION ITSELF.*/

/* Open a file for reading / writing (w+)
 *  - In binary mode (b)
 *  - Creating the file if it doesn't exist (w)
 *  - Failing if the file already exists as a safety measure (x),
 * and return its file descriptor.*/
int openFile(std::string filepath) {
    FILE *fp = fopen(filepath.c_str(), "wb+x");
    if (fp == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    int fd = fileno(fp);

    // posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
    return fd;
}

/* Create all the files for one vector, and write their file descriptors (fd's) to vecfds*/
void initVectorFDs(int *vecfds, std::string fileprefix, const bool zeroInit) {
    if (TOTAL_VEC_SIZE < SINGLE_FILE_SIZE) {
        cout << "NO MMAPING NEEDED PLEASE RECONSIDER, THIS WILL PROBABLY BREAK\n";
    }
    if (TOTAL_VEC_SIZE % SINGLE_FILE_SIZE != 0) {
        cout << "Error: SINGLE_FILE_SIZE must divide TOTAL_VEC_SIZE!";
    }

    const int num_maps = TOTAL_VEC_SIZE / SINGLE_FILE_SIZE;
    unsigned char *zeros = 0;
    if (zeroInit) {  // only need to zero initialize v1
        zeros = (unsigned char *)std::calloc(SINGLE_FILE_SIZE, sizeof(char));
    }

    for (int i = 0; i < num_maps; i++) {
        int fd = openFile((fileprefix + "-" + std::to_string(i) + ".bin").c_str());
        vecfds[i] = fd;
        if (zeroInit) {
            // Zero initialize (memset) in a parallel manner
            auto zerofile = [&zeros](int fd, uint64_t num_bytes, uint64_t offset) -> void {
                if (pwrite(fd, zeros, num_bytes, offset) == -1) {
                    perror("Error zeroing out the files");
                }
            };
            std::thread threads[NUM_THREADS];
            uint64_t prev_total = 0;
            for (int j = 0; j < NUM_THREADS; j++) {
                uint64_t total = ceil((j + 1) * double(SINGLE_FILE_SIZE) / NUM_THREADS);
                threads[j] = std::thread(zerofile, vecfds[i], prev_total, total);
                prev_total = total;
            }
            for (int j = 0; j < NUM_THREADS; j++) {
                threads[j].join();
            }
        }

        // Print progress (setting up files can take a while for large length)
        if (num_maps >= 10) {
            if (i % (num_maps / 10) == 0) {
                printf("Done making %i of %i files...\n", i, num_maps);
                std::flush(cout);
            }
        }
    }
    if (zeroInit) {
        std::free(zeros);
    }
}

/* Copy memory (size = num_elements*sizeof(uint32_t)) either from the external memory files to
 * the memory map, or from the memory map to the external memory files.*/
void copy_mem(uint32_t *memmap, int *vecfds, uint64_t num_elements, uint64_t read_offset, bool file_to_mem) {
    auto startTime = std::chrono::system_clock::now();

    // Lambda function for parallel copying to/from external memory
    auto parallel_copy = [&file_to_mem](uint32_t *memmap, int filemap, const uint64_t intra_file_offset,
                                        const uint64_t num_els) -> void {
        std::thread threads[NUM_THREADS];
        uint64_t prev_total = 0;
        for (int i = 0; i < NUM_THREADS; i++) {
            uint64_t total = ceil((i + 1) * num_els / double(NUM_THREADS));
            if (file_to_mem) {
                threads[i] = std::thread(pread, filemap, memmap + prev_total, (total - prev_total) * sizeof(uint32_t),
                                         intra_file_offset * sizeof(uint32_t) + prev_total * sizeof(uint32_t));
            } else {
                threads[i] = std::thread(pwrite, filemap, memmap + prev_total, (total - prev_total) * sizeof(uint32_t),
                                         intra_file_offset * sizeof(uint32_t) + prev_total * sizeof(uint32_t));
            }
            prev_total = total;
        }
        for (int i = 0; i < NUM_THREADS; i++) {
            threads[i].join();
        }
    };

    // If you are reading/writing more than (or equal) amount of elements than are in a single file:
    // Read/write all the elements, one file at a time.
    for (uint64_t i = 0; i < num_elements / SINGLE_FILE_ENTRIES; i++) {
        uint64_t index = (read_offset / SINGLE_FILE_ENTRIES) + i;
        if (index * SINGLE_FILE_SIZE > TOTAL_VEC_SIZE) {
            cout << "There is likely a mistake in copy_mem\n";
        }
        const uint64_t num_els = SINGLE_FILE_ENTRIES;
        parallel_copy(memmap + SINGLE_FILE_ENTRIES * i, vecfds[index], 0, num_els);
    }
    // If you are reading/writing less elements than are in a single file:
    // Read/write all the elements, offsetting within the file.
    if (num_elements < SINGLE_FILE_ENTRIES) {
        uint64_t index = (read_offset / SINGLE_FILE_ENTRIES) + 0;
        const uint64_t intra_file_offset = read_offset % SINGLE_FILE_ENTRIES;
        const uint64_t num_els = num_elements;
        parallel_copy(memmap, vecfds[index], intra_file_offset, num_els);
    }
    if (PRINT_MEM_COPIES) {
        if (file_to_mem) {
            cout << "Copy (file to mem) with " << num_elements << " elements time taken: " << secondsSince(startTime)
                 << "\n";
        } else {
            cout << "Copy (mem to file) with " << num_elements << " elements time taken: " << secondsSince(startTime)
                 << "\n";
        }
    }
}

uint32_t *make_ram_map(uint64_t numbytes) {
    // For large length, if your system supports it, put back MAP_HUGETLB flag!
    uint32_t *readmap = (uint32_t *)mmap(NULL, numbytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (readmap == MAP_FAILED) {
        perror("Error mmapping the file (in make_ram_map)");
        exit(EXIT_FAILURE);
    }
    return readmap;
}

/* SUPPORTING CODE FOR READING/WRITING FILES ENDS HERE.
#######################################################################################
#######################################################################################*/

/* Find the minimum change between two vectors.
 * This is slower than the multithreaded F functions if not parallelized!
 * So we break that calculation up across threads. */
uint32_t subtract_and_find_min_parallel(uint32_t *readmap, int *fdsv1, int *fdsv2) {
    const uint64_t half_map_size = (LOOP_CHUNK_SIZE / sizeof(uint32_t)) / 2;
    // Lambda function to make threads to find min in largest chunk that can fit in readmap
    auto find_min_par = [&readmap, &half_map_size]() {
        std::future<uint32_t> minVals[NUM_THREADS];
        // const uint64_t incr = half_map_size / NUM_THREADS;

        // Function to calculate the minimum coefficient in a particular (start...end) slice
        auto findMin = [&readmap, &half_map_size](uint64_t start, uint64_t end) {
            uint32_t minCoef = 0 - 1;
            for (uint64_t str = start; str < end; str++) {
                minCoef = std::min(minCoef, readmap[str + half_map_size] - readmap[str]);
            }
            return minCoef;
        };

        // Set threads to calculate the max coef in their own smaller slices using above function
        // for (int i = 0; i < NUM_THREADS; i++) {
        //     minVals[i] = std::async(std::launch::async, findMin, incr * i, incr * (i + 1));
        // }
        uint64_t prev_total = 0;
        for (int i = 0; i < NUM_THREADS; i++) {
            uint64_t total = ceil((i + 1) * double(half_map_size) / NUM_THREADS);
            minVals[i] = std::async(std::launch::async, findMin, prev_total, total);
            prev_total = total;
        }

        // Now calculate the global max
        uint32_t min_change = minVals[0].get();  // .get() waits until the thread completes
        for (int i = 1; i < NUM_THREADS; i++) {
            uint32_t coef = minVals[i].get();
            if (coef < min_change) {
                min_change = coef;
            }
        }
        return min_change;
    };

    uint32_t min_change = 0 - 1;  // set to largest value possible

    // Loop through, calculating the min change for the vectors for as large of a chunk
    // as will fit in readmap at a time.
    for (uint64_t i = 0; i < (2 * TOTAL_VEC_SIZE / LOOP_CHUNK_SIZE); i++) {
        uint64_t offset = i * (LOOP_CHUNK_SIZE / sizeof(uint32_t)) / 2;
        // put v1 into first half of readmap
        copy_mem(readmap, fdsv1, half_map_size, offset, true);
        // put v2 into second half of readmap
        copy_mem(readmap + half_map_size, fdsv2, half_map_size, offset, true);
        min_change = std::min(min_change, find_min_par());
    }
    return min_change;
}

/* Combined version of L_01 and L_10. As in the simpler case where no external memory is necessary, we again combine
 * L_10 and L_01 into one loop and do their elementwise maximum locally instead of in F. It's much more convoluted to
 * define a recursion that allows for computation reuse, so we do not do that here (memory I/O dominates compute time
 * anyways).*/
void L_combined(const uint64_t str_start, const uint64_t str_end, const uint64_t index_offset,
                const uint64_t index_offset_rev, const uint32_t *v, uint32_t *ret) {
    // Lambda function that implements L_01 and corresponding L_10 for the given [start,...,end) slice
    auto loop = [&v, &ret, &index_offset, &index_offset_rev, &str_start](uint64_t start, uint64_t end) {
        for (uint64_t str = start; str < end; str++) {
            // F_01 *********************************************************
            // Take every other bit (starting at first position)
            const uint64_t A = str & 0xAAAAAAAAAAAAAAAA;
            // Take every other bit (starting at second position)
            // The second & gets rid of the first bit
            const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
            uint64_t ATB0 = A | (TB << 2);  // smallest bit in B implicitly set to 0
            uint64_t ATB1 = ATB0 | 1;       // 0b1 <- the smallest bit in B is set to 1

            // Correct for the fact that we can't hold the entire vector at once
            ATB0 = ATB0 - index_offset;
            ATB1 = ATB1 - index_offset;

            // F_10 ********************************************************
            const uint64_t str_r = powminus0 - 1 - str;
            const uint64_t TA = (str_r & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);
            const uint64_t B = str_r & 0x5555555555555555;
            uint64_t TA0B = (TA << 2) | B;  // smallest bit in A implicitly set to 0
            uint64_t TA1B = TA0B | 2;       // 0b10 <- the smallest bit in A is set to 1
            // Transform back into the vector according to Equation 3
            TA0B = std::min(TA0B, (powminus0 - 1) - TA0B) - index_offset_rev;
            TA1B = std::min(TA1B, (powminus0 - 1) - TA1B) - index_offset_rev;

            // Take their maximum, and divide it by 2
            // A lot of ugly reassignment to avoid temporary overflows when adding
            ret[str - str_start] =
                uint32_t(std::max(uint64_t(v[ATB0]) + uint64_t(v[ATB1]), uint64_t(v[TA0B]) + uint64_t(v[TA1B])) >> 1);
        }
    };

    // Set threads to compute L_combined in their own start...end slices
    std::thread threads[NUM_THREADS];
    uint64_t prev_total = str_start;
    for (int i = 0; i < NUM_THREADS; i++) {
        uint64_t total = str_start + ceil((i + 1) * double(str_end - str_start) / NUM_THREADS);
        threads[i] = std::thread(loop, prev_total, total);
        prev_total = total;
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }
}

/* Implements L_00 as described in the paper, writing values into ret.*/
void L_00(const uint64_t str_start, const uint64_t str_end, uint32_t *v, uint32_t *ret) {
    const uint64_t arr_bound = 2 * (str_end - str_start) - 1;
    const uint64_t offset = str_start << 2;
    auto loop = [&v, &ret, &arr_bound, &offset, &str_start](uint64_t start, uint64_t end) {
        auto startTime = std::chrono::system_clock::now();
        for (uint64_t str = start; str < end; str++) {
            const uint64_t TA0TB0 = (str << 2) - offset;  // equivalent to above 3 lines!
            const uint64_t TA0TB1 = TA0TB0 | 0b1;
            const uint64_t TA1TB0 = TA0TB0 | 0b10;
            const uint64_t TA1TB1 = TA0TB0 | 0b11;

            // These used to be necessary, but are really just asking if str >= powminus3.
            // But now that we're using the symmetry, this will never be the case since str
            // only reaches powminus3 -1. (See other supporting document)
            // TA0TB0 = std::min(TA0TB0, (powminus0 - 1) - TA0TB0);
            // TA0TB1 = std::min(TA0TB1, (powminus0 - 1) - TA0TB1);
            // TA1TB0 = std::min(TA1TB0, (powminus0 - 1) - TA1TB0);
            // TA1TB1 = std::min(TA1TB1, (powminus0 - 1) - TA1TB1);

            // A lot of ugly reassignment to avoid temporary overflows when adding
            ret[str - str_start] =
                1 * FIXED_POINT_SCALE +
                uint32_t(uint64_t(v[TA0TB0]) + uint64_t(v[TA0TB1]) + uint64_t(v[TA1TB0]) + uint64_t(v[TA1TB1]) >> 2);
            ret[arr_bound - (str - str_start)] = ret[str - str_start];  // array is self-symmetric!
        }
    };
    std::thread threads[NUM_THREADS];
    uint64_t prev_total = str_start;
    for (int i = 0; i < NUM_THREADS; i++) {
        uint64_t total = str_start + ceil((i + 1) * double(str_end - str_start) / NUM_THREADS);
        threads[i] = std::thread(loop, prev_total, total);
        prev_total = total;
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }
}

/* Given a string pair represented as an integer, return the string pair that it will be max'd with,
 * and indicate if that pair is in a block of strings to be iterated in reverse order.*/
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

/* This is the recursive driver ('wrapper') that runs the algorithm, determining what values of v1 to read in and what
 * new values in v2 those values allows us to calculate. Please refer to the supporting document for this code before
 * trying to read it closely.*/
void recursive_driver(int *fdsv1, int *fdsv2, uint32_t *readmap, uint32_t *writemap) {
    /* This lambda supports the actual recursion. It's used to find, given a range of strings pairs (i.e. starting
     * string pair and number of string pairs) iterated on by L_01, what range of values will need to be iterated
     * through for L_10 (given only as the start of those ranges, the size of each range is determined from context) in
     * order to perform the elementwise maximums. There is either one range or two ranges of values in L_10 that needs
     * to get iterated through.*/
    auto inv_access_ranges_of = [](uint64_t start_str, uint64_t num_strs) -> std::pair<uint64_t, uint64_t> {
        std::pair<uint64_t, bool> idx_and_is_reverse = inv_idx_of(start_str);
        uint64_t TA0B = idx_and_is_reverse.first;
        bool reverse = idx_and_is_reverse.second;

        // If reversed (access was >= powm1), then accessing will be in reverse
        // So subtract num of strings so that TA0B...TA0B+num_strs is still accurate
        if (reverse) {
            // Don't have to split in two
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

    // This lambda performs the actual recursion
    std::function<void(uint64_t[2], uint64_t, int, int)> recurse_Lcombined;
    recurse_Lcombined = [&fdsv1, &fdsv2, &readmap, &writemap, &recurse_Lcombined, &inv_access_ranges_of](
                            uint64_t offsets[2], uint64_t index_offset, int depth, int stage) -> void {
        // num_strs = powminus(2+depth)
        uint64_t num_strs = uint64_t(1) << (2 * (length)-2 - depth);
        if (depth < RECURSE_STOP_DEPTH) {
            // Two-part recursion as described in supporting document
            switch (stage) {
                case 1: {
                    uint64_t offsets_next[2] = {offsets[0], offsets[0] + num_strs * 2 / 4};
                    recurse_Lcombined(offsets_next, index_offset, depth + 1, 2);

                    offsets_next[0] = offsets[0] + num_strs / 4;
                    offsets_next[1] = offsets[0] + num_strs * 3 / 4;
                    recurse_Lcombined(offsets_next, index_offset + 2 * num_strs / 2, depth + 1, 2);
                    break;
                }
                case 2: {
                    uint64_t offsets_next[2] = {offsets[0], 0};
                    recurse_Lcombined(offsets_next, index_offset, depth + 1, 1);

                    offsets_next[0] = offsets[1];
                    recurse_Lcombined(offsets_next, index_offset + 2 * num_strs / 2, depth + 1, 1);
                    break;
                }
            }

        } else {  // Now, we do actual the file I/O and L functions for the range given by the recursion
            uint64_t start_i = index_offset;
            uint64_t end_i = start_i + 2 * num_strs;

            // load in arr[start_i] thru arr[end_i]
            uint64_t num_bytes = 2 * num_strs * sizeof(uint32_t);
            uint64_t num_bytes_write = num_strs * sizeof(uint32_t);
            copy_mem(readmap, fdsv1, num_strs * 2, start_i, true);

            uint64_t default_arr[2] = {0, 0};
            if (stage == 1) {  // if only need to iterate over one range of sequential values for L_01
                uint64_t start = powminus2 + offsets[0];
                uint64_t end = start + num_strs;
                std::pair<uint64_t, uint64_t> access_ranges = inv_access_ranges_of(start, num_strs);

                if (access_ranges.first ==
                    access_ranges.second) {  // if need to iterate over only one range of sequential values for L_10
                    // Read in the values, do L_combined, write out the new values in writemap
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, fdsv1, 2 * num_strs, reverse_start_i, true);
                    L_combined(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);
                    copy_mem(writemap, fdsv2, num_strs, start, false);

                } else {  // need to iterate over two ranges of sequential values for L_10
                    // Read in the values for first range, do L_combined for first range
                    end = start + num_strs / 2;
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, fdsv1, num_strs, reverse_start_i, true);
                    L_combined(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);

                    // Read in the values for the second range, do L_combined for second range
                    start = end;
                    end = start + num_strs / 2;
                    reverse_start_i = access_ranges.second;
                    copy_mem(readmap + 2 * num_strs, fdsv1, num_strs, reverse_start_i, true);
                    L_combined(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap + num_strs / 2);

                    // Write out the new values in writemap
                    copy_mem(writemap, fdsv2, num_strs, start - num_strs / 2, false);
                }

            } else {  // stage == 2
                      // To use the memory that was read in sequentially, need to iterate over two ranges of str
                      // Further, we need to check if the indices we need to access for reverse part of each of these
                      // two ranges is broken into one or two.

                // FOR THE FIRST RANGE NEEDED FOR L_01
                uint64_t start = powminus2 + offsets[0];
                uint64_t end = start + num_strs / 2;
                std::pair<uint64_t, uint64_t> access_ranges = inv_access_ranges_of(start, num_strs / 2);
                if (access_ranges.first ==
                    access_ranges.second) {  // if need to iterate over only one range of sequential values for L_10
                    // Read in the values, do L_combined, write out the new values in writemap
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, fdsv1, num_strs, reverse_start_i, true);
                    L_combined(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);
                    copy_mem(writemap, fdsv2, num_strs / 2, start, false);

                } else {  // need to iterate over two ranges of sequential values for L_10
                    // Read in the values for first range, do L_combined for first range
                    end = start + num_strs / 4;
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, fdsv1, num_strs / 2, reverse_start_i, true);
                    L_combined(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);

                    // Read in the values for second range, do L_combined for second range
                    start = end;
                    end = start + num_strs / 4;
                    reverse_start_i = access_ranges.second;
                    copy_mem(readmap + 2 * num_strs, fdsv1, num_strs / 2, reverse_start_i, true);
                    L_combined(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap + num_strs / 4);

                    // Write out the new values in writemap
                    copy_mem(writemap, fdsv2, num_strs / 2, start - num_strs / 4, false);
                }

                // FOR THE SECOND RANGE NEEDED FOR L_01
                start = powminus2 + offsets[1];
                end = start + num_strs / 2;
                access_ranges = inv_access_ranges_of(start, num_strs / 2);
                if (access_ranges.first ==
                    access_ranges.second) {  // if need to iterate over only one range of sequential values for L_10
                    // Read in the values, do L_combined, write out the new values in writemap
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, fdsv1, num_strs, reverse_start_i, true);
                    L_combined(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);
                    copy_mem(writemap, fdsv2, num_strs / 2, start, false);

                } else {  // need to iterate over two ranges of sequential values for L_10
                    // Read in the values for first range, do L_combined for first range
                    end = start + num_strs / 4;
                    uint64_t reverse_start_i = access_ranges.first;
                    copy_mem(readmap + 2 * num_strs, fdsv1, num_strs / 2, reverse_start_i, true);
                    L_combined(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);

                    // Read in the values for second range, do L_combined for second range
                    start = end;
                    end = start + num_strs / 4;
                    reverse_start_i = access_ranges.second;
                    copy_mem(readmap + 2 * num_strs, fdsv1, num_strs / 2, reverse_start_i, true);
                    L_combined(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap + num_strs / 4);

                    // Write out the new values in writemap
                    copy_mem(writemap, fdsv2, num_strs / 2, start - num_strs / 4, false);
                }
            }

            // RUNNING L_00
            // Now, we can run L_00 with no additional file reading using the first half of readmap!
            const uint64_t loop2_start = (start_i >> 2);
            const uint64_t loop2_end = loop2_start + num_strs / 2;
            // Writes num_strs * 2 (twice as much) strs, in two blocks that are contiguous w.r.t. themselves, but not
            // necessarily each other, with the blocks separated at the middle
            L_00(loop2_start, loop2_end, readmap, writemap);

            // then do two write chunks:
            // write out first half of write array, using the first contiguous block
            copy_mem(writemap, fdsv2, num_strs / 2, loop2_start, false);
            // write out second half of write array, using the second contiguous block
            copy_mem(writemap + num_strs / 2, fdsv2, num_strs / 2, powminus2 - loop2_end, false);
            return;
        }
    };
    uint64_t begin_arr[2] = {0, 0};
    recurse_Lcombined(begin_arr, 0, 0, 1);
    cout << "\n";
}

void FeasibleTriplet(int n) {
    auto start = std::chrono::system_clock::now();
    int v1fds[TOTAL_VEC_SIZE / SINGLE_FILE_SIZE];  // contains file descriptors of the files that comprise v1
    initVectorFDs(v1fds, "./filev1", true);
    printf("Files for v1 prepared in %f seconds\n", secondsSince(start));
    int v2fds[TOTAL_VEC_SIZE / SINGLE_FILE_SIZE];  // contains file descriptors of the files that comprise v2
    initVectorFDs(v2fds, "./filev2", false);
    printf("Files prepared in %f seconds\n", secondsSince(start));
    std::flush(cout);

    // Create two mmaps: one to read from, one to write to
    // Map to read from is 4x as large as map to write to
    uint32_t *readmap = make_ram_map(LOOP_CHUNK_SIZE);
    uint32_t *writemap = make_ram_map(LOOP_CHUNK_SIZE / 4);

    double min_change_d = 0;
    long long int prev_min_change = 0;

    for (int i = 2; i < n + 1; i++) {
        auto start2 = std::chrono::system_clock::now();
        // Writes new vector into v2
        recursive_driver(v1fds, v2fds, readmap, writemap);
        cout << "Elapsed time F (s): " << secondsSince(start2) << "\n";
        std::flush(cout);
        fflush(NULL);  // flush ALL file buffers

        if (i % CALC_EVERY_X_ITERATIONS == 0 || i == n) {
            // Calculate min_change used in bound
            const uint32_t min_change = subtract_and_find_min_parallel(readmap, v1fds, v2fds);
            min_change_d = double(min_change) / FIXED_POINT_SCALE;

            // Calculate bound
            printf("At n=%i: %.9f.\nElapsed time (s): %f\n", i, 2 * min_change_d / (1 + min_change_d),
                   secondsSince(start));
            std::flush(cout);
            if (min_change - prev_min_change < (CALC_EVERY_X_ITERATIONS * 2) && min_change - prev_min_change != 0) {
                cout << "Minimum changes no longer exceeding tolerance, convergence reached! Quitting...\n";
                break;
            }
            prev_min_change = min_change;

        } else {
            printf("Elapsed time (s) at n=%i: %f\n", i, secondsSince(start));
            std::flush(cout);
        }

        // Swap pointers of v1 and v2
        std::swap(v1fds, v2fds);
    }

    printf("Final lower bound: %.9f\n", 2.0 * min_change_d / (1 + min_change_d));

    cout << "Performing cleanup..." << endl;
    if (REMOVE_FILES) {
        const int num_maps = TOTAL_VEC_SIZE / SINGLE_FILE_SIZE;
        for (int i = 0; i < num_maps; i++) {
            std::remove(("./filev1-" + std::to_string(i) + ".bin").c_str());
            std::remove(("./filev2-" + std::to_string(i) + ".bin").c_str());
        }
    }
    fcloseall();
}

int main() {
    cout << "Starting with l = " << length << "...\n";
    cout << "Vector size in bytes (entries): " << TOTAL_VEC_SIZE << " (" << (TOTAL_VEC_SIZE / sizeof(uint32_t))
         << ")\n";
    cout << "Map size in bytes: " << SINGLE_FILE_SIZE << " (" << TOTAL_VEC_SIZE / SINGLE_FILE_SIZE << " maps)\n";
    cout << "Subdivided chunk size in bytes (entries), STOP_DEPTH: " << LOOP_CHUNK_SIZE << " ("
         << (LOOP_CHUNK_SIZE / sizeof(uint32_t)) << "), " << RECURSE_STOP_DEPTH << "\n";
    cout << "Threads: " << NUM_THREADS << " (+1)\n";
    std::flush(cout);

    auto start = std::chrono::system_clock::now();
    FeasibleTriplet(100);
    cout << "Elapsed time (s): " << secondsSince(start) << "\n";
    return 0;
}
