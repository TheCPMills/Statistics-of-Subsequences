// #define EIGEN_NO_DEBUG 1
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
using Eigen::ArrayXd;
using Eigen::Map;
using std::cout;
using std::endl;

#define length 3
#define NUM_THREADS 1
// Careful: ensure that NUM_THREADS divides 2^(2*length-1) (basically always will for l > 3 if power of 2)
#define CALC_EVERY_X_ITERATIONS 25

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
const uint64_t FILESIZE = powminus1 * sizeof(double);

template <typename Derived>
void printArray(const Eigen::ArrayBase<Derived> &arr) {
    for (int i = 0; i < arr.size(); i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}
void printArrayMap(const double *arr, const int len) {
    for (int i = 0; i < len; i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}

struct arrayAccessor {
    int mmapNum;
    uint64_t mmapIndex;
};

// TODO: convert to much better macro
arrayAccessor arrayPos(uint64_t val) {
    arrayAccessor a;

    a.mmapNum = val / (ONE_GB * 2);
    a.mmapIndex = val % ONE_GB * 2;
    return a;
}

// void loopUntilTolerance(double tol){
//     double prev = 0.0;
//     if (prev == 0.0 || tol-prev > )
// }

/* Normally, we find R with R = (v2 - v1).maxCoeff();
    However, this is slower than the multithreaded F functions if not parallelized!
    So instead we break that calculation up across threads.
    Additionally, E = std::max(0.0, (v2 + 2 * R - v1).maxCoeff()). We can use
    this function for that as well, and just add 2*R at the end. */
double subtract_and_find_max_parallel(const double *v1, const double *v2) {
    std::future<double> maxVals[NUM_THREADS];
    const uint64_t incr = powminus1 / NUM_THREADS;

    // Function to calculate the maximum coefficient in a particular (start...end) slice
    auto findMax = [v1, v2](uint64_t start, uint64_t end) {
        Map<const ArrayXd> vecv2(v2 + start, end - start);
        Map<const ArrayXd> vecv1(v1 + start, end - start);
        return (vecv2 - vecv1).maxCoeff();
    };

    // THIS FUNCTION SHOULDNT NEED AS MUCH CHANGING. CAN JUST SET EACH THREAD TO HANDLE THE MIN BETWEEN
    // 2GB AND (TOTAL SIZE)/NUM_THREADS, DOING MANY ITERATIONS AS NECESSARY
    // Set threads to calculate the max coef in their own smaller slices using above function
    for (int i = 0; i < NUM_THREADS; i++) {
        maxVals[i] = std::async(std::launch::async, findMax, incr * i, incr * (i + 1));
    }

    // Now calculate the global max
    double R = maxVals[0].get();  // .get() waits until the thread completes
    for (int i = 1; i < NUM_THREADS; i++) {
        double coef = maxVals[i].get();
        if (coef > R) {
            R = coef;
        }
    }
    return R;
}

/* This function is essentially the combined form of F_01_loop1 and F_01_loop2, with the elementwise maximum
 * between loop 1 and loop 2's values also being performed locally. This means we have two strings: the string
 * computed by iterating through loop 1 (str), and the string from loop 2 it needs to be compared to for
 * elementwise maximum (str3).
 * On top of that, another property is taken advantage of to reuse some computation.
 * If you add powminus2 to str to get str2, then some of the values computed for str2 (which is an element
 * normally computed in loop2) will be the same as those computed for str. So we reuse those values to calculate
 * str2's values here. We can then also notice that if we subtract powminus2 from str3 to get str4 (which we can
 * similarly reuse some parts of the computation to do), we get the string we need to do a maximum for str2
 * with. So we can use this to fill in another entry. So at every iteration we are computing 4 entries (reusing
 * computation where possible), and then reducing down to 2 entries (one at the start of array, one at the end)
 * by taking the max.
 * Visually:
 *     first half of arr           second half of arr           outside bounds of arr
 *                           val1                       val2
 * [                         |-->           |           <--]
 *
 *                           str                       str4 str2                     str3
 * [                         |-->           |           <--]-->          |           <--|
 * 0                       powm2         +powm3          powm1        +powm3         +powm2
 *
 *
 * A few other notes to understand this code:
 * - 0xAA... is the binary string 10101010..., meaning str & 0xAA... gives us A.
 * - Similarly, 0x55... is the binary string 01010101..., meaning str & 0x55... gives us B.
 * - Doing & (powminus2 - 1) zeros out the first bit of A and of B since (powminus2-1) is 0s until the 2nd
 * bit of str and 1s after. Similarly, doing & (powminus1 - 1) zeros out only the first bit of A.*/
void F_01_combined(const uint64_t start, const uint64_t end, const double *v, double *ret, const double R,
                   const int threadNum) {
    auto startTime = std::chrono::system_clock::now();
    for (uint64_t str = start; str < end; str++) {
        // save val for loop 2
        const uint64_t str2 = str + powminus2;
        // above will ALWAYS have the effect of setting biggest 1 to 0, and putting 1 to left
        const uint64_t str3 = powminus0 + powminus2 - 1 - str2;
        const uint64_t str4 = str3 - powminus2;
        // may be able to do each of these +'s and -'s as 2 bitwise ops? &, |, or ~?

        // Compute as in Loop 1 [str]
        // Keep A as is, remove the first bit of B and shift B left, and then set the new
        // last bit of B to either 0 or 1.
        const uint64_t A = str & 0xAAAAAAAAAAAAAAAA;
        const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
        const uint64_t ATB0 = A | (TB << 2);  // the smallest bit in B is implicitly set to 0
        const uint64_t ATB1 = ATB0 | 1;       // 0b01 <- the smallest bit in B is set to 1

        // Compute as in Loop 2 [str2]
        // Keep B as is, remove the first bit of A and shift A left, and then set the new
        // last bit of A to either 0 or 1.
        const uint64_t TA = A;
        // const uint64_t TA = (str2 & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);  //equivalent!
        const uint64_t B = TB;
        // const uint64_t B = str2 & 0x5555555555555555; //equivalent!
        const uint64_t TA0B = (TA << 2) | B;  // the smallest bit in A is implicitly set to 0
        const uint64_t TA1B = TA0B | 2;       // 0b10 <- the smallest bit in A is set to 1

        // These are no longer necessary
        // TA0B = std::min(TA0B, (powminus0 - 1) - TA0B);
        // TA1B = std::min(TA1B, (powminus0 - 1) - TA1B);

        // Compute as in Loop 2 [str3]
        const uint64_t TA3 = (str3 & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);
        const uint64_t B3 = str3 & 0x5555555555555555;
        uint64_t TA0B3 = (TA3 << 2) | B3;  // the smallest bit in A is implicitly set to 0
        // uint64_t TA1B3 = TA0B3 | 2;

        // these lines used to be:
        // TA0B3 = std::min(TA0B3, (powminus0 - 1) - TA0B3);
        // TA1B3 = std::min(TA1B3, (powminus0 - 1) - TA1B3);
        // but can equivalently be written as
        TA0B3 = (powminus0 - 1) - TA0B3;
        // TA1B3 = (powminus0 - 1) - TA1B3;
        // equivalent to taking the complement: (~TA1B3) & (0xFFFFFFFFFFFFFFFF >> 64 - 2 * length);
        // Below optimization is possible since TA1B3 always equals TA0B3 but with a 0 at end of B instead of 1;
        const uint64_t TA1B3 = TA0B3 & (0xFFFFFFFFFFFFFFFF - 2);

        const double loop1val = v[ATB0] + v[ATB1];
        const double loop2val = v[TA0B] + v[TA1B];
        const double loop2valcomp = v[TA0B3] + v[TA1B3];
        ret[str] = 0.5 * std::max(loop1val, loop2valcomp) + R;

        // Compute as in Loop 1 [str4]
        const uint64_t A_2 = TA3;
        const uint64_t TB_2 = B3;
        const uint64_t ATB0_2 = A_2 | (TB_2 << 2);  // the smallest bit in B is implicitly set to 0
        const uint64_t ATB1_2 = ATB0_2 | 1;         // 0b1 <- the smallest bit in B is set to 1
        const double loop1val2 = v[ATB0_2] + v[ATB1_2];
        ret[str4] = 0.5 * std::max(loop2val, loop1val2) + R;
        // TODO: figure out which values in the max are larger if possible
        // if (loop2val > loop1val2) {
        //     cout << "CASE 1\n";
        // }
        // if (loop2val < loop1val2) {
        //     cout << "CASE 2\n";
        // }
        // if (loop2val == loop1val2) {
        //     cout << "SAME\n";
        // }
        // if (loop1val > loop2valcomp) {
        //     cout << "           CASE 1\n";
        // }
        // if (loop1val < loop2valcomp) {
        //     cout << "           CASE 2\n";
        // }
        // if (loop1val == loop2valcomp) {
        //     cout << "           SAME\n";
        // }

        //  TODO: loop2val is symmetric USE THIS FACT!

        //(+R because +R in each v value, but then 0.5*result)
        // printf("AT %i:  %f %f %f %f   %i %i  %i %i  %i %i  %i %i\n", str - start, loop1val, loop2val, loop2valcomp,
        //        loop1val2, ATB0, ATB1, TA0B, TA1B, TA0B3, TA1B3, ATB0_2, ATB1_2);
        /*TODO: Figure out the very odd pattern: in bottom half of loop1val's and bottom half of loop2valcomp's vals, if
         * you iterate through the accessors for loop1val in order (i.e. 64 65 -> 66 67 -> 68 69 and so on) then their
         * corresponding loop1val match with the loop2valcomp that far from the bottom. So for l=4, 64 65 is the 0th
         * from the start (of second half of vals), so their corresponding loop1val is the same as the 0th from the
         * bottom loop2valcomp (corresponding to 127 125). 66 67 is 1 from the start so it matches with 1 from the
         * bottom. And so on.
         * Similarly, for top half of loop2valcomp's vals and top half of loop1val2's vals. Start at smallest accessors
         * for loop1val2's values and go in increasing order, working up from the bottom of the top half of
         * loop2valcomp's values as you go.*/

        // What if we tried indexing scheme where B is on the right, reversed (so it's 0.....A(B reversed))?
        // that way, we iterate by +2 (starting at 0, and at 1) to keep same nice property of not having to check first
        // bit. always same (always different for starting at 1) up until halfway, where it reverses to always different
        // (always same for starting at 1).
        //
        // const int64_t strTEST1 = str & () std::bitset<2 * length> aTEST(A);
    }
    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endTime - startTime;
    printf("Thread %i time taken: %f\n", threadNum, elapsed_seconds.count());
    // TODO: threads are taking a significantly variable amount of time. implement a pooling scheme.
}

void F_01(const double *v, double *ret, const double R) {
    const uint64_t start = powminus2;
    const uint64_t end = powminus2 + powminus3;
    std::thread threads[NUM_THREADS];
    const uint64_t incr = (end - start) / (NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] =
            std::thread(F_01_combined, start + incr * i, start + incr * (i + 1), std::cref(v), std::ref(ret), R, i);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    //  return ret;
}

void F_12(const double *v, double *ret) {
    auto loop = [v, ret](uint64_t start, uint64_t end) {
        for (uint64_t str = start; str < end; str++) {
            // const uint64_t TA = (str & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);
            // const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
            // uint64_t TA0TB0 = (TA << 2) | (TB << 2);
            const uint64_t TA0TB0 = (str & (powminus2 - 1)) << 2;  // equivalent to above 3 lines!
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

            ret[str] = 1 + .25 * (v[TA0TB0] + v[TA0TB1] + v[TA1TB0] + v[TA1TB1]);
            ret[powminus2 - 1 - str] = ret[str];  // array is self-symmetric!
            // TODO: see if can use this symmetry to reduce space needed
        }
    };
    std::thread threads[NUM_THREADS];
    const uint64_t start = 0;
    const uint64_t end = powminus3;
    const uint64_t incr = (end - start) / (NUM_THREADS);  // Careful to make sure NUM_THREADS is a divisor!
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(loop, start + incr * i, start + incr * (i + 1));
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    // return ret;
}

// (remember: any changes made in here have to be made in F_withplusR as well)
void F(const double *v1, const double *v2, double *ret) {
    // Computes f01 and f11, does their elementwise maximum, and places it into second half of ret
    auto start2 = std::chrono::system_clock::now();
    F_01(v1, ret, 0);
    auto end2 = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end2 - start2;
    cout << "Elapsed time F_01 (s): " << elapsed_seconds.count() << endl;

    //  Puts f_double into the first half of the ret vector
    start2 = std::chrono::system_clock::now();
    F_12(v2, ret);
    end2 = std::chrono::system_clock::now();
    elapsed_seconds = end2 - start2;
    cout << "Elapsed time F_12 (s): " << elapsed_seconds.count() << endl;
}

// Exactly like F, but saves memory as it can be fed v, v+R but use only one vector
void F_withplusR(const double R, const double *v2, double *ret) {
    // v2+R is normally fed to F_01. However, we can actually avoid doing so.

    // v2+R is NOT fed to F_01. Instead, since ret[] is always set to v[] + v[], we can just add 2*R at the end.
    F_01(v2, ret, R);
    /* While it would be nice to do this function first and then add 2*R to the vector, it would prevent us from reusing
    memory since F_12 requires half a vector, but F_01 temporarily requires a full vector (as currently implemented).
    v2 without R added*/
    F_12(v2, ret);
}

int openFile(std::string filepath) {
    // char filepath[] = "./mmapped.bin";
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

// TODO: CAN CLOSE FILE DESCRIPTOR AFTER INITING MAP?

// TODO: is this passing by value?
void closeFile(int fd, double *map) {
    if (munmap(map, FILESIZE) == -1) {
        perror("Error un-mmapping the file");
    }
    close(fd);
}

double *prepareMap(int fd) {
    double *map;
    /* Stretch file size to the size of the vector we need to store*/
    if (lseek(fd, FILESIZE - 1, SEEK_SET) == -1) {
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
    map = (double *)mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }
    return map;
}

// check this https://stackoverflow.com/questions/13024087/what-are-memory-mapped-page-and-anonymous-page

int fileStuff(int fd, double *map) {
    // read in from map to array
    Map<ArrayXd> mf(map, 4);
    cout << mf;

    // write to extern memory
    auto source = &mf(0);
    std::memcpy(map + (sizeof(double)) * 4, source, (sizeof(double)) * 4);

    // read in starting at 5th value
    Map<ArrayXd> mf2(map + (sizeof(double)) * 4, 4);
    mf2 = mf2 + 2;
    cout << mf2;
    return 0;
}

void initVectorMaps(double **vecmmaps, std::string fileprefix) {
    // create enough mmaps for one vector
    // GB = 1024^3 bytes
    // one vector is powminus1 * 8 bytes large
    if (FILESIZE < ONE_GB * 2) {
        cout << "NO MMAPING NEEDED PLS RECONSIDER, THIS WILL PROBABLY BREAK\n";
    }
    const int num_maps = FILESIZE / (ONE_GB * 2);
    for (int i = 0; i < num_maps; i++) {
        int fd = openFile((fileprefix + std::to_string(i) + ".bin").c_str());
        vecmmaps[i] = prepareMap(fd);  // todo: verify is copying by reference
    }
}

void FeasibleTriplet(int n) {
    // for now doing it with only 1 mmaped region per array. first see if that works (prob wont). if not switch to more.
    int fd_v1 = openFile("./mmappedv1.bin");
    // a pointer to the start of the mmaped region (I think)
    double *mapv1 = prepareMap(fd_v1);
    std::memset(mapv1, 0, FILESIZE);  // set all values to 0
    int fd_v2 = openFile("./mmappedv2.bin");
    double *mapv2 = prepareMap(fd_v2);

    // TODO: replace 4 with number of things to read
    // for (int i = 0; i < 10; i++) {
    // }
    // idk if actually reads into voltaile mem?
    Map<ArrayXd> vec(mapv1, 4);
    int v1mmaps[FILESIZE / (ONE_GB * 2)];

    //  ArrayXd v0 = ArrayXd::Zero(powminus1); // unneeded in single vec impl
    auto start = std::chrono::system_clock::now();
    // ArrayXd v1 = ArrayXd::Zero(powminus1);

    // ArrayXd u = ArrayXd::Zero(powminus1);  // u never used
    // ArrayXd v2(powminus1);  // we pass a pointer to this, and have it get filled in

    double r = 0;
    double e = 0;
    for (int i = 2; i < n + 1; i++) {
        // Writes new vector (v2) into v2
        auto start2 = std::chrono::system_clock::now();

        F(mapv1, mapv1, mapv2);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start2;
        cout << "Elapsed time F (s): " << elapsed_seconds.count() << endl;
        if (i % CALC_EVERY_X_ITERATIONS == 0 || i == n) {
            start2 = std::chrono::system_clock::now();
            // TODO: if this uses a vector of mem temporarily, store v2-v1 into v1?
            const double R = subtract_and_find_max_parallel(mapv1, mapv2);
            end = std::chrono::system_clock::now();
            elapsed_seconds = end - start2;
            cout << "Elapsed time (s) mc1: " << elapsed_seconds.count() << endl;
            //  Beyond this point, v1's values are no longer needed, so we reuse
            //  its memory for other computations.

            // NOTE: THE v1's HERE ARE REUSED MEMORY. v1's VALUES NOT USED, INSTEAD
            // v1 IS COMPLETELY OVERWRITTEN HERE.
            // Normally F(v2+R, v2, v0), but I think this ver saves an entire vector
            // of memory.
            // TODO: i think you can do this (calculate R and E) only every X iterations
            start2 = std::chrono::system_clock::now();
            F_withplusR(R, mapv2, mapv1);
            end = std::chrono::system_clock::now();
            elapsed_seconds = end - start2;
            cout << "Elapsed time FpR (s): " << elapsed_seconds.count() << endl;
            // TODO: INVESTIGATE WHY ABOVE IS SO MUCH FASTER THAN F??
            // except now it isnt being faster???

            // Idea: normally below line is ArrayXd W = v2 + 2 * R - v0;
            // We again reuse v1 to store W, and with single vector v0 is replaced by v1.
            start2 = std::chrono::system_clock::now();
            const double E = std::max(subtract_and_find_max_parallel(mapv1, mapv2) + 2 * R, 0.0);
            end = std::chrono::system_clock::now();
            elapsed_seconds = end - start2;
            cout << "Elapsed time mc2 (s): " << elapsed_seconds.count() << endl;

            // TODO: +2*R keeps getting added places... wonder if it's at all necessary to do,
            //  and if we can instead just do it at the end for R and E?

            // // FIXME: FIGURE OUT WHAT NEW IF STATEMENT NEEDS TO BE
            // if (R - E >= r - e) {
            //     // u = v2;  // u is never used
            //     r = R;
            //     e = E;
            // }
            r = R;
            e = E;

            // FIXME: Need to verify mathematically the correctness of this new calculation.
            if (PRINT_EVERY_ITER) {
                // (other quantities of potential interest)
                // cout << "At n=" << i << ": " << 2.0 * (r - e) << endl;
                // cout << "At n=" << i << ": " << 2.0 * (r - e) / (1 + (r - e)) << endl;
                // cout << "At n=" << i << ": " << 2.0 * (e - r) / (1 + (e - r)) << endl;
                // cout << "At n=" << i << ": " << 2.0 * (e - r) << endl;
                // cout << "R, E: " << R << " " << E << endl;
                // cout << (2.0 * r / (1 + r)) << endl;
                printf("At n=%i: %.9f\n", i, (2.0 * r / (1 + r)) + 2.0 * (r - e) / (1 + (r - e)));
                end = std::chrono::system_clock::now();
                elapsed_seconds = end - start;
                cout << "Elapsed time (s): " << elapsed_seconds.count() << endl;
            }

            // cout << r << " " << R << endl;
        }
        // Swap pointers of v1 and v2
        // v0 = v1;
        // v1 = v2;
        std::swap(mapv1, mapv2);
    }

    // return u, r, e
    // cout << "Result: " << 2.0 * (r - e) << endl;
    printf("Single Vec Result: %.9f\n", 2.0 * r / (1 + r));
    printf("(Alt more acc): %.9f\n", (2.0 * r / (1 + r)) + 2.0 * (r - e) / (1 + (r - e)));
    // TODO: compare to Lower
    //  cout << "v1" << endl;
    //  printArray(v1);
    //  cout << "v2" << endl;
    //  printArray(v2);
}

int main() {
    cout << "Starting with l = " << length << "..." << endl;
    auto start = std::chrono::system_clock::now();
    FeasibleTriplet(100);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    cout << "Elapsed time (s): " << elapsed_seconds.count() << endl;
    // TODO: check if eigen init parallel is something to do
    return 0;
}