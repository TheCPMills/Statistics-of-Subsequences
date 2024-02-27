#include <Eigen/Dense>
#include <bitset>
#include <chrono>
#include <cmath>
#include <ctime>
#include <future>
#include <iostream>
#include <thread>
using Eigen::Array;
using std::cout;
using std::endl;

typedef Array<uint32_t, Eigen::Dynamic, 1> ArrayXui;

/*                  USER PARAMETERS
#########################################################*/
/* String length to use. Larger values give better bounds, but take ~4x longer and 4x more
 * space for every increase in length by 1.*/
#define length 12

/* Number of threads to use when running the code.*/
#define NUM_THREADS 3

/* Parameter is n, the maximum number of iterations to run for. However, it may
 * exit early if changes do not exceed a convergence tolerance.*/
#define MAX_ITERS 200

/* Calculate a bound every this many iterations. Larger values will make the code run faster, but with the caveat that
 * it may take more iterations to complete. We recommend setting to ~10 or 15 for best performance.*/
#define CALC_EVERY_X_ITERATIONS 1

/* Fixed point scale to use for fixed point arithmetic.
 * Note: it is best to set at 2^25 = 33554432 for length <= 18,
 * and 2^24 = 16777216 for length >= 19 (to avoid possibility of overflow).
 * However, for consistency with Lueker, we set it to 25000000.*/
#define FIXED_POINT_SCALE 25000000
/*######################################################*/
#define EIGEN_NO_DEBUG 1

// equal to pow(2, 2 * length)
const uint64_t powminus0 = uint64_t(1) << (2 * length);
// equal to pow(2, 2 * length - 1)
const uint64_t powminus1 = uint64_t(1) << ((2 * length) - 1);
// equal to pow(2, 2 * length - 2)
const uint64_t powminus2 = uint64_t(1) << ((2 * length) - 2);
// equal to pow(2, 2 * length - 3)
const uint64_t powminus3 = uint64_t(1) << ((2 * length) - 3);

template <typename Derived>
void printArray(const Eigen::ArrayBase<Derived> &arr) {
    for (int i = 0; i < arr.size(); i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}
double secondsSince(std::chrono::system_clock::time_point startTime) {
    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endTime - startTime;
    return elapsed_seconds.count();
}

/* Normally, we find min_change with min_change = (v2 - v1).minCoeff();
    However, this is slower than the multithreaded F functions if not parallelized!
    So instead we break that calculation up across threads. */
uint32_t subtract_and_find_min_parallel(const ArrayXui &v1, const ArrayXui &v2) {
    std::future<uint32_t> minVals[NUM_THREADS];

    // Function to calculate the maximum coefficient in a particular (start...end) slice
    auto findMin = [&v1, &v2](uint64_t start, uint64_t end) {
        return (v2(Eigen::seq(start, end - 1)) - v1(Eigen::seq(start, end - 1))).minCoeff();
    };

    // Set threads to calculate the max coef in their own smaller slices
    uint64_t prev_total = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        uint64_t total = ceil((i + 1) * double(powminus1) / NUM_THREADS);
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
}

/* This function is essentially the combined form of L_01 and L_10 with the elementwise maximum
 * between L_01 and L_10's values also being performed locally. This means we have two strings: the string
 * computed by iterating through L_01 (str), and the string from L_10 it needs to be compared to for
 * elementwise maximum (str3).
 * On top of that, another property is taken advantage of to reuse some computation.
 * If you add powminus2 to str to get str2, then some of the values computed for str2 (which is an element
 * normally computed in L_10) will be the same as those computed for str. So we reuse those values to calculate
 * str2's values here. We can then also notice that if we subtract powminus2 from str3 to get str4 (which we can
 * similarly reuse some parts of the computation to do), we get the string we need to do a maximum for str2
 * with. So we can use this to fill in another entry. So at every iteration we are computing 4 entries (reusing
 * computation where possible), and then reducing down to 2 entries (one at the start of array, one at the end)
 * by taking the max.
 * Visually:
 *     first half of ret           second half of ret           outside bounds of ret
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
 * - bit of str and 1s after. Similarly, doing & (powminus1 - 1) zeros out only the first bit of A.
 * - The T's stand for Tail. E.g., TB = Tail(B), ATB1 = (A | Tail(B)1), etc.*/
void L_combined(const uint64_t start, const uint64_t end, const ArrayXui &v, ArrayXui &ret) {
    for (uint64_t str = start; str < end; str++) {
        const uint64_t str2 = str + powminus2;
        // above will ALWAYS have the effect of setting biggest 1 to 0, and putting 1 to left
        const uint64_t str3 = powminus0 + powminus2 - 1 - str2;
        const uint64_t str4 = str3 - powminus2;

        // Compute as in L_01 [str]
        // Keep A as is, remove the first bit of B and shift B left, and then set the new
        // last bit of B to either 0 or 1.
        const uint64_t A = str & 0xAAAAAAAAAAAAAAAA;
        const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
        const uint64_t ATB0 = A | (TB << 2);  // the smallest bit in B is implicitly set to 0
        const uint64_t ATB1 = ATB0 | 1;       // 0b01 <- the smallest bit in B is set to 1

        // Compute as in L_10 [str2]
        // Keep B as is, remove the first bit of A and shift A left, and then set the new
        // last bit of A to either 0 or 1.
        const uint64_t TA = A;
        // const uint64_t TA = (str2 & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);  //equivalent!
        const uint64_t B = TB;
        // const uint64_t B = str2 & 0x5555555555555555; //equivalent!
        const uint64_t TA0B = (TA << 2) | B;  // the smallest bit in A is implicitly set to 0
        const uint64_t TA1B = TA0B | 2;       // 0b10 <- the smallest bit in A is set to 1

        // Compute as in L_10 [str3]
        const uint64_t TA_2 = (str3 & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);
        const uint64_t B_2 = str3 & 0x5555555555555555;
        uint64_t TA0B_2 = (TA_2 << 2) | B_2;  // the smallest bit in A is implicitly set to 0
        // uint64_t TA1B_2 = TA0B_2 | 2;

        // To reverse the position in the array according to Equation 3, we can do:
        TA0B_2 = (powminus0 - 1) - TA0B_2;
        // TA1B_2 = (powminus0 - 1) - TA1B_2;
        // equivalent to taking the complement: (~TA1B3) & (0xFFFFFFFFFFFFFFFF >> 64 - 2 * length);
        // Below tiny optimization is possible since TA1B3 always equals TA0B_2 but with a 0 at end of A instead of 1
        const uint64_t TA1B_2 = TA0B_2 & (0xFFFFFFFFFFFFFFFF - 2);

        // Compute as in L_01 [str4]
        const uint64_t A_2 = TA_2;                  // reuse computation!
        const uint64_t TB_2 = B_2;                  // reuse computation!
        const uint64_t ATB0_2 = A_2 | (TB_2 << 2);  // the smallest bit in B is implicitly set to 0
        const uint64_t ATB1_2 = ATB0_2 | 1;         // 0b1 <- the smallest bit in B is set to 1

        //
        // Ugly uint64_t casting to avoid temporary overflows
        const uint64_t val1_F01 = uint64_t(v[ATB0]) + uint64_t(v[ATB1]);
        const uint64_t val1_F10 = uint64_t(v[TA0B_2]) + uint64_t(v[TA1B_2]);
        ret[str] = std::max(val1_F01, val1_F10) >> 1;  // >> 1 equiv to divison by 2

        // Ugly uint64_t casting to avoid temporary overflows
        const uint64_t val2_F01 = uint64_t(v[ATB0_2]) + uint64_t(v[ATB1_2]);
        const uint64_t val2_F10 = uint64_t(v[TA0B]) + uint64_t(v[TA1B]);
        ret[str4] = std::max(val2_F01, val2_F10) >> 1;  // >> 1 equiv to division by 2
    }
}

/*A wrapper for the combined L_01 and L_10 functions. All it does is run the function in a parallel manner across
 * distinct slices of the values in the range [powminus2,...,powminus+powminus3).*/
void L_combined_wrapper(const ArrayXui &v, ArrayXui &ret) {
    const uint64_t start = powminus2;
    const uint64_t end = powminus2 + powminus3;
    std::thread threads[NUM_THREADS];
    uint64_t prev_total = start;
    for (int i = 0; i < NUM_THREADS; i++) {
        uint64_t total = start + ceil((i + 1) * double(end - start) / NUM_THREADS);
        threads[i] = std::thread(L_combined, prev_total, total, std::cref(v), std::ref(ret));
        prev_total = total;
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    //  return ret;
}

/* Straightforward parallelized implementation of L_00, taking advantage of
 * the symmetry described in the supporting markdown file. Fills in the first half of ret.*/
void L_00(const ArrayXui &v, ArrayXui &ret) {
    // Lambda function defines the loop
    auto loop = [&v, &ret](uint64_t start, uint64_t end) {
        for (uint64_t str = start; str < end; str++) {
            const uint64_t TA0TB0 = str << 2;
            const uint64_t TA0TB1 = TA0TB0 | 0b1;
            const uint64_t TA1TB0 = TA0TB0 | 0b10;
            const uint64_t TA1TB1 = TA0TB0 | 0b11;

            // These used to be necessary, but are really just asking if str >= powminus3.
            // But now that we're using the symmetry (see supporting document), this will
            // never be the case since str only reaches powminus3 -1.
            // TA0TB0 = std::min(TA0TB0, (powminus0 - 1) - TA0TB0);
            // TA0TB1 = std::min(TA0TB1, (powminus0 - 1) - TA0TB1);
            // TA1TB0 = std::min(TA1TB0, (powminus0 - 1) - TA1TB0);
            // TA1TB1 = std::min(TA1TB1, (powminus0 - 1) - TA1TB1);

            // Ugly uint64_t casting to avoid temporary overflows
            ret[str] = (FIXED_POINT_SCALE * 1) +
                       ((uint64_t(v[TA0TB0]) + uint64_t(v[TA0TB1]) + uint64_t(v[TA1TB0]) + uint64_t(v[TA1TB1])) >> 2);
            // >> 2 equiv to division by 4
            ret[powminus2 - 1 - str] = ret[str];  // array is self-symmetric!
        }
    };

    // Set threads to run the loop lambda on their own slices
    std::thread threads[NUM_THREADS];
    const uint64_t start = 0;
    const uint64_t end = powminus3;
    uint64_t prev_total = start;
    for (int i = 0; i < NUM_THREADS; i++) {
        uint64_t total = start + ceil((i + 1) * double(end - start) / NUM_THREADS);
        threads[i] = std::thread(loop, prev_total, total);
        prev_total = total;
    }
    for (int i = 0; i < NUM_THREADS; i++) {  // wait for each thread to finish
        threads[i].join();
    }

    // return ret;
}

void F(const ArrayXui &v1, ArrayXui &ret) {
    // Computes L_01 and L_10, does their elementwise maximum, and places it into second half of ret
    L_combined_wrapper(v1, ret);

    // Puts first half of the new vector as computed by L_00 into first half of ret
    L_00(v1, ret);
}

void FeasibleTriplet(int n) {
    auto start = std::chrono::system_clock::now();
    ArrayXui v1 = ArrayXui::Zero(powminus1);
    ArrayXui v2(powminus1);  // we pass a pointer to this, and have it get filled in

    uint32_t prev_min_change = 0;
    uint32_t min_change = 0;
    for (int i = 2; i < n + 1; i++) {
        auto start2 = std::chrono::system_clock::now();
        // Writes new vector (v2) into v2
        F(v1, v2);
        cout << "Elapsed time F (s): " << secondsSince(start2) << endl;

        if (i % CALC_EVERY_X_ITERATIONS == 0 || i == n) {
            min_change = subtract_and_find_min_parallel(v1, v2);

            // Convert fixed point arithmetic back to a decimal number
            double min_change_d = double(min_change) / FIXED_POINT_SCALE;

            printf("Bound at n=%i: %.9f. (min change = %i)\nElapsed time (s): %f\n", i,
                   2 * min_change_d / (1 + min_change_d), min_change, secondsSince(start));
            flush(cout);

            // Check if the change falls under the tolerance. If it does, exit.
            if (min_change - prev_min_change < 3 && min_change - prev_min_change != 0) {
                cout << "Minimum changes no longer exceeding tolerance, convergence reached! Quitting...\n";
                break;
            }
            prev_min_change = min_change;
        }
        // Swap pointers of v1 and v2
        std::swap(v1, v2);
    }

    double min_change_d = double(min_change) / FIXED_POINT_SCALE;
    printf("FINAL RESULT: %.9f.\n", 2 * min_change_d / (1 + min_change_d));
}

int main() {
    cout << "Starting with l = " << length << "..." << endl;
    auto start = std::chrono::system_clock::now();

    FeasibleTriplet(MAX_ITERS);

    cout << "Elapsed time (s): " << secondsSince(start) << endl;
    return 0;
}