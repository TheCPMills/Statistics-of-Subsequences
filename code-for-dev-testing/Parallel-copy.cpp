#define EIGEN_NO_DEBUG 1
//   TODO: PUT ABOVE BACK

#include <Eigen/Dense>
#include <bitset>
#include <chrono>
#include <cmath>
#include <ctime>
#include <future>
#include <iostream>
#include <thread>
using std::cout;
using std::endl;

#define length 5
#define NUM_THREADS 1
// Careful: ensure that NUM_THREADS divides 2^(2*length-1) (basically always will for l > 3 if power of 2)
#define FIXED_POINT_SCALE 25000000
#define TOL 3
#define CALC_EVERY_X_ITERATIONS 1

#define PRINT_ELAPSED_TIMES false

#define PRINT_EVERY_ITER true

// equal to pow(2, 2 * length)
const uint64_t powminus0 = uint64_t(1) << (2 * length);
// equal to pow(2, 2 * length - 1)
const uint64_t powminus1 = uint64_t(1) << ((2 * length) - 1);
// equal to pow(2, 2 * length - 2)
const uint64_t powminus2 = uint64_t(1) << ((2 * length) - 2);
// equal to pow(2, 2 * length - 3)
const uint64_t powminus3 = uint64_t(1) << ((2 * length) - 3);

typedef Eigen::Array<uint32_t, Eigen::Dynamic, 1> ArrayXui;

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

void printSecondsSince(std::string name, std::chrono::system_clock::time_point startTime) {
    if (PRINT_ELAPSED_TIMES) {
        const double seconds = secondsSince(startTime);
        cout << "Elapsed time " << name << " (s): " << seconds << endl;
    }
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
uint32_t subtract_and_find_min_parallel(const ArrayXui &v1, const ArrayXui &v2) {
    std::future<uint32_t> maxVals[NUM_THREADS];
    const uint64_t incr = powminus1 / NUM_THREADS;

    // Function to calculate the maximum coefficient in a particular (start...end) slice
    auto findMax = [&v1, &v2](uint64_t start, uint64_t end) {
        uint32_t minCoef = (0 - 1);
        for (uint64_t str = start; str < end; str++) {
            minCoef = std::min(minCoef, v2[str] - v1[str]);
            if (long(v2[str]) - long(v1[str]) < 0) {
                cout << long(v2[str]) - long(v1[str]) << endl;
                cout << v2[str] << "  " << v1[str] << endl;
                exit(0);
            }
        }
        return minCoef;
        // return (v2(Eigen::seq(start, end - 1)) - v1(Eigen::seq(start, end - 1))).minCoeff();
    };

    // Set threads to calculate the max coef in their own smaller slices
    for (int i = 0; i < NUM_THREADS; i++) {
        maxVals[i] = std::async(std::launch::async, findMax, incr * i, incr * (i + 1));
    }

    // Now calculate the global max
    uint32_t R = maxVals[0].get();  // .get() waits until the thread completes
    for (int i = 1; i < NUM_THREADS; i++) {
        uint32_t coef = maxVals[i].get();
        if (coef < R) {
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
 * ret[...]
 * 4
 *
 *
 * A few other notes to understand this code:
 * - 0xAA... is the binary string 10101010..., meaning str & 0xAA... gives us A.
 * - Similarly, 0x55... is the binary string 01010101..., meaning str & 0x55... gives us B.
 * - Doing & (powminus2 - 1) zeros out the first bit of A and of B since (powminus2-1) is 0s until the 2nd
 * bit of str and 1s after. Similarly, doing & (powminus1 - 1) zeros out only the first bit of A.*/
void F_01_combined(const uint64_t start, const uint64_t end, const ArrayXui &v, ArrayXui &ret, const double R,
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
        // 0100
        // 1011
        // 0001
        // 1001 TA0B3
        // 1011 TA1B3

        // these lines used to be:
        // TA0B3 = std::min(TA0B3, (powminus0 - 1) - TA0B3);
        // TA1B3 = std::min(TA1B3, (powminus0 - 1) - TA1B3);
        // but can equivalently be written as
        TA0B3 = (powminus0 - 1) - TA0B3;
        // TA1B3 = (powminus0 - 1) - TA1B3;
        // equivalent to taking the complement: (~TA1B3) & (0xFFFFFFFFFFFFFFFF >> 64 - 2 * length);
        // Below optimization is possible since TA1B3 always equals TA0B3 but with a 0 at end of A instead of 1;
        const uint64_t TA1B3 = TA0B3 & (0xFFFFFFFFFFFFFFFF - 2);

        const uint32_t loop1val = (v[ATB0] >> 1) + (v[ATB1] >> 1);
        const uint32_t loop2val = (v[TA0B] >> 1) + (v[TA1B] >> 1);
        const uint32_t loop2valcomp = (v[TA0B3] >> 1) + (v[TA1B3] >> 1);
        ret[str] = std::max(loop1val, loop2valcomp);  //+R

        // Compute as in Loop 1 [str4]
        const uint64_t A_2 = TA3;
        const uint64_t TB_2 = B3;
        const uint64_t ATB0_2 = A_2 | (TB_2 << 2);  // the smallest bit in B is implicitly set to 0
        const uint64_t ATB1_2 = ATB0_2 | 1;         // 0b1 <- the smallest bit in B is set to 1
        const uint32_t loop1val2 =
            (v[ATB0_2] >> 1) + (v[ATB1_2] >> 1);    // TODO: IMPLEMENT FIXED POINT ARITHMETIC OVERFLOWS ERRS BY TYPE
                                                    // CONVERSION INSTTEAD OF A BUNCH OF LOCAL OPERATIONS
        ret[str4] = std::max(loop2val, loop1val2);  //+R
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

        // TODO: I wonder if it's possible to look at (in case of mismatching bits) check if next two bits also don't
        // match (e.g. 10 and 01) and then say it's 1 + (...) since over those two bits you get exactly 1 match
    }
    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endTime - startTime;
    // printf("Thread %i time taken: %f\n", threadNum, elapsed_seconds.count());
    //  TODO: threads are taking a significantly variable amount of time. implement a pooling scheme.
}

void F_01(const ArrayXui &v, ArrayXui &ret, const double R) {
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

void F_12(const ArrayXui &v, ArrayXui &ret) {
    auto loop = [&v, &ret](uint64_t start, uint64_t end) {
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
            uint32_t aaa = ((v[TA0TB0] + v[TA0TB1] + v[TA1TB0] + v[TA1TB1]) & 3) >> 2;
            ret[str] = (1 * FIXED_POINT_SCALE) +
                       ((v[TA0TB0] >> 2) + (v[TA0TB1] >> 2) + (v[TA1TB0] >> 2) + (v[TA1TB1] >> 2) + aaa);
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
void F(const ArrayXui &v1, const ArrayXui &v2, ArrayXui &ret) {
    // Computes f01 and f11, does their elementwise maximum, and places it into second half of ret
    auto startTime = std::chrono::system_clock::now();
    F_01(v1, ret, 0);
    printSecondsSince("F_01", startTime);

    //  Puts f_double into the first half of the ret vector
    startTime = std::chrono::system_clock::now();
    F_12(v2, ret);
    printSecondsSince("F_12", startTime);
}

// Exactly like F, but saves memory as it can be fed v, v+R but use only one vector
void F_withplusR(const double R, const ArrayXui &v2, ArrayXui &ret) {
    // v2+R is normally fed to F_01. However, we can actually avoid doing so.

    // v2+R is NOT fed to F_01. Instead, since ret[] is always set to v[] + v[], we can just add 2*R at the end.
    F_01(v2, ret, R);
    /* While it would be nice to do this function first and then add 2*R to the vector, it would prevent us from reusing
    memory since F_12 requires half a vector, but F_01 temporarily requires a full vector (as currently implemented).
    v2 without R added*/
    F_12(v2, ret);
}

void FeasibleTriplet(int n) {
    //  ArrayXui v0 = ArrayXui::Zero(powminus1); // unneeded in single vec impl
    auto start = std::chrono::system_clock::now();
    ArrayXui v1 = ArrayXui::Zero(powminus1);

    // ArrayXui u = ArrayXui::Zero(powminus1);  // u never used
    ArrayXui v2(powminus1);  // we pass a pointer to this, and have it get filled in

    double r = 0;
    double e = 0;
    for (int i = 2; i < n + 1; i++) {
        // Writes new vector (v2) into v2
        auto start2 = std::chrono::system_clock::now();
        F(v1, v1, v2);
        printSecondsSince("F   ", start2);
        if (i % CALC_EVERY_X_ITERATIONS == 0 || i == n) {
            start2 = std::chrono::system_clock::now();
            // TODO: if this uses a vector of mem temporarily, store v2-v1 into v1?
            const uint32_t minChange = subtract_and_find_min_parallel(v1, v2);
            printSecondsSince("Max1", start2);
            cout << v1[24] << "  " << v2[24] << endl;

            //  Beyond this point, v1's values are no longer needed, so we reuse
            //  its memory for other computations.

            // NOTE: THE v1's HERE ARE REUSED MEMORY. v1's VALUES NOT USED, INSTEAD
            // v1 IS COMPLETELY OVERWRITTEN HERE.
            // Normally F(v2+R, v2, v0), but I think this ver saves an entire vector
            // of memory.
            // TODO: i think you can do this (calculate R and E) only every X iterations
            // start2 = std::chrono::system_clock::now();
            // F_withplusR(R, v2, v1);
            // printSecondsSince("F+R ", start2);
            // TODO: INVESTIGATE WHY ABOVE IS SO MUCH FASTER THAN F??
            // except now it isnt being faster???

            // Idea: normally below line is ArrayXui W = v2 + 2 * R - v0;
            // We again reuse v1 to store W, and with single vector v0 is replaced by v1.
            // start2 = std::chrono::system_clock::now();
            // const int E = -subtract_and_find_max_parallel(v1, v2);
            // const double E2 = std::max(subtract_and_find_max_parallel(v1, v2) + 2 * R, 0.0);

            // printSecondsSince("Max2", start2);

            // TODO: +2*R keeps getting added places... wonder if it's at all necessary to do,
            //  and if we can instead just do it at the end for R and E?

            // // FIXME: FIGURE OUT WHAT NEW IF STATEMENT NEEDS TO BE
            // if (R - E >= r - e) {
            //     // u = v2;  // u is never used
            //     r = R;
            //     e = E;
            // }
            // cout << "R: " << R << "   E: " << E << endl;
            cout << minChange << endl;
            // r = float(R) / FIXED_POINT_SCALE;
            e = double(minChange) / FIXED_POINT_SCALE;

            // FIXME: Need to verify mathematically the correctness of this new calculation.
            if (PRINT_EVERY_ITER) {
                // (other quantities of potential interest)
                // cout << "At n=" << i << ": " << 2.0 * (r - e) << endl;
                // cout << "At n=" << i << ": " << 2.0 * (r - e) / (1 + (r - e)) << endl;
                // cout << "At n=" << i << ": " << 2.0 * (e - r) / (1 + (e - r)) << endl;
                // cout << "At n=" << i << ": " << 2.0 * (e - r) << endl;
                // cout << "R, E: " << R << " " << E << endl;
                // cout << (2.0 * r / (1 + r)) << endl;
                // printf("At n=%i: %.9f\n", i, (2.0 * r / (1 + r)) + 2.0 * (r - e) / (1 + (r - e)));
                printf("At n=%i: %.8f\n", i, 2.0 * e / (1 + e));

                printSecondsSince("(Total)", start);
            }
            // This isn't just to save on computation. Past this point, fixed-point arithmetic can't be trusted! There's
            // just too much error.
            // if (R - E < TOL) {
            //     break;
            // }

            // cout << r << " " << R << endl;
        } else {
            printf("Iteration n=%i complete\n");
        }
        // Swap pointers of v1 and v2
        // v0 = v1;
        // v1 = v2;
        std::swap(v1, v2);
    }

    // return u, r, e
    // cout << "Result: " << 2.0 * (r - e) << endl;
    // printf("Single Vec Result: %.7f\n", 2.0 * r / (1 + r));
    printf("Single Vec Result (New): %.7f\n", 2.0 * e / (1 + e));
    // printf("Single Vec Result: %.7f\n", 2.0 * (r - e) / (1 + (r - e)));
    // printf("(Alt more acc): %.7f\n", (2.0 * r / (1 + r)) + 2.0 * (r - e) / (1 + (r - e)));
    // TODO: compare to Lower
    //  cout << "v1" << endl;
    //  printArray(v1);
    //  cout << "v2" << endl;
    //  printArray(v2);
}

int main() {
    cout << "Starting with l = " << length << "..." << endl;
    auto start = std::chrono::system_clock::now();
    FeasibleTriplet(50);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    cout << "Elapsed time (s): " << elapsed_seconds.count() << endl;
    // TODO: check if eigen init parallel is something to do
    ArrayXui aaa = ArrayXui(2);
    aaa[0] = 5;
    aaa[1] = -9;
    cout << "\n" << aaa.maxCoeff() << endl;
    return 0;
}