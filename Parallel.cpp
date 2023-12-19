#define EIGEN_NO_DEBUG 1
//  TODO: PUT ABOVE BACK

#include <Eigen/Dense>
#include <chrono>
#include <cmath>
#include <ctime>
#include <future>
#include <iostream>
#include <thread>
using Eigen::ArrayXd;
using std::cout;
using std::endl;

#define length 6
#define NUM_THREADS 4
// Careful: ensure that NUM_THREADS divides 2^(2*length-2) (basically always will for l > 3 if power of 2)

const bool PRINT_EVERY_ITER = true;

// equal to pow(2, 2 * length)
const uint64_t powminus0 = uint64_t(1) << (2 * length);
// equal to pow(2, 2 * length - 1)
const uint64_t powminus1 = uint64_t(1) << ((2 * length) - 1);
// equal to pow(2, 2 * length - 2)
const uint64_t powminus2 = uint64_t(1) << ((2 * length) - 2);

void printArray(const ArrayXd &arr) {
    for (int i = 0; i < arr.size(); i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}

// void loopUntilTolerance(double tol){
//     double prev = 0.0;
//     if (prev == 0.0 || tol-prev > )
// }

/* Normally, we find R with R = (v2 - v1).maxCoeff();
    However, this is slower than the multithreaded F functions if not parallelized!
    So instead we break that calculation up across threads.*/
double findR_parallel(const ArrayXd &v1, const ArrayXd &v2) {
    std::future<double> maxVals[NUM_THREADS];
    const uint64_t incr = powminus1 / NUM_THREADS;

    // Function to calculate the maximum coefficient in a particular (start...end) slice
    auto findMax = [](uint64_t start, uint64_t end, const ArrayXd &v1, const ArrayXd &v2) {
        return (v2(Eigen::seq(start, end - 1)) - v1(Eigen::seq(start, end - 1))).maxCoeff();
    };

    // Set threads to calculate the max coef in their own smaller slices
    for (int i = 0; i < NUM_THREADS; i++) {
        maxVals[i] = std::async(std::launch::async, findMax, incr * i, incr * (i + 1), std::cref(v1), std::cref(v2));
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

/*  Takes the elementwise maximum of f01 (first half of ret) and f11 (second half of ret), and
    fills the second half of the ret vector with the result (multiplied by 0.5). Afterward, first
    half of ret vector can be safely overwritten. Does so in a parallelized fashion, since this
    can be quite slow.
    Also adds 2*R. Can set R to 0 to not add anything (as in F).*/
void elementwise_max_parallel(ArrayXd &ret, const double R) {
    // start of second half of array is powminus2
    const uint64_t middle = powminus2;

    // TODO: benchmark if adding R always (but 0 half the time) is slower than only adding R
    // in a separate function. Probably no difference, but may as well check.
    auto elementwise_max = [R](uint64_t start, uint64_t end, ArrayXd &ret) {
        ret(Eigen::seq(middle + start, middle + end - 1)) =
            0.5 * ret(Eigen::seq(start, end - 1)).max(ret(Eigen::seq(middle + start, middle + end - 1))) + 2 * R;
    };

    std::thread threads[NUM_THREADS];
    const uint64_t incr = (powminus2) / (NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(elementwise_max, incr * i, incr * (i + 1), std::ref(ret));
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }
}

void F_01_loop1(const uint64_t start, const uint64_t end, const ArrayXd &v, ArrayXd &ret) {
    for (uint64_t str = start; str < end; str++) {
        // Take every other bit (starting at first position)
        const uint64_t A = str & 0xAAAAAAAAAAAAAAAA;
        // Take take every other bit (starting at second position)
        // The second & gets rid of the first bit
        const uint64_t TB = (str & 0x5555555555555555) & ((uint64_t(1) << (2 * length - 2)) - 1);
        uint64_t ATB0 = A | (TB << 2);
        uint64_t ATB1 = ATB0 | 1;  // 0b1 <- the smallest bit in B is set to 1

        // I wonder if assigning to new variables is faster?
        ATB0 = std::min(ATB0, (powminus0 - 1) - ATB0);
        ATB1 = std::min(ATB1, (powminus0 - 1) - ATB1);
        ret[str - powminus2] = v[ATB0] + v[ATB1];  // if h(A) != h(B) and h(A) = z
    }
}

void F_01_loop2(const uint64_t start, const uint64_t end, const ArrayXd &v, ArrayXd &ret) {
    for (uint64_t str = start; str < end; str++) {
        // Take every other bit (starting at first position)
        // The second & gets rid of the first bit
        const uint64_t TA = (str & 0xAAAAAAAAAAAAAAAA) & ((uint64_t(1) << (2 * length - 1)) - 1);
        // Take every other bit (starting at second position)
        const uint64_t B = str & 0x5555555555555555;
        uint64_t TA0B = (TA << 2) | B;
        uint64_t TA1B = TA0B | 2;

        // I wonder if assigning to new variables is faster?
        TA0B = std::min(TA0B, (powminus0 - 1) - TA0B);
        TA1B = std::min(TA1B, (powminus0 - 1) - TA1B);

        /* The below line is normally ret[str - powminus2] = v[TA0B] + v[TA1B];
        However, the values from this part get reversed afterwards. So, instead,
        we do the reversing here locally to avoid the need for that (it allows
        us to avoid needing to copy things when multithreading later in elementwise_max).
        (3*powminus2 -(str-powminus2) = powminus0-str) */
        ret[powminus0 - 1 - str] = v[TA0B] + v[TA1B];  // if h(A) != h(B) and h(A) = z
    }
}

void F_01(const ArrayXd &v, ArrayXd &ret) {
    const uint64_t start = powminus2;
    const uint64_t end = powminus1;
    std::thread threads[NUM_THREADS];
    const uint64_t incr = (end - start) / (NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(F_01_loop1, start + incr * i, start + incr * (i + 1), std::cref(v), std::ref(ret));
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(F_01_loop2, end + incr * i, end + incr * (i + 1), std::cref(v), std::ref(ret));
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    //  return ret;
}

void F_12(const ArrayXd &v, ArrayXd &ret) {
    // ret = ArrayXd::Zero(powminus2);
    std::thread threads[NUM_THREADS];
    const uint64_t start = 0;
    const uint64_t end = powminus2;
    const uint64_t incr = (end - start) / (NUM_THREADS);  // Careful to make sure NUM_THREADS is a divisor!
    auto loop = [](uint64_t start, uint64_t end, const ArrayXd &v, ArrayXd &ret) {
        for (uint64_t str = start; str < end; str++) {
            const uint64_t TA = (str & 0xAAAAAAAAAAAAAAAA) & ((uint64_t(1) << (2 * length - 1)) - 1);
            const uint64_t TB = (str & 0x5555555555555555) & ((uint64_t(1) << (2 * length - 2)) - 1);
            uint64_t TA0TB0 = (TA << 2) | (TB << 2);
            uint64_t TA0TB1 = TA0TB0 | 0b1;
            uint64_t TA1TB0 = TA0TB0 | 0b10;
            uint64_t TA1TB1 = TA0TB0 | 0b11;
            // There might be an argument to split this for loop into 4,
            // so that something something cache hits/optimization.
            // Probably not though.

            // I wonder if assigning to new variables is faster?
            TA0TB0 = std::min(TA0TB0, (powminus0 - 1) - TA0TB0);
            TA0TB1 = std::min(TA0TB1, (powminus0 - 1) - TA0TB1);
            TA1TB0 = std::min(TA1TB0, (powminus0 - 1) - TA1TB0);
            TA1TB1 = std::min(TA1TB1, (powminus0 - 1) - TA1TB1);
            ret[str] = 1 + .25 * (v[TA0TB0] + v[TA0TB1] + v[TA1TB0] + v[TA1TB1]);
            // TODO: figure out if doing 1+.25* is faster here (locally) or after at end
        }
    };
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(loop, start + incr * i, start + incr * (i + 1), std::cref(v), std::ref(ret));
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    // return ret;
}

// (remember: any changes made in here have to be made in F_withplusR as well)
void F(const ArrayXd &v1, const ArrayXd &v2, ArrayXd &ret) {
    // Places f01 and f11 back to back into the vector pointed to by ret
    auto start2 = std::chrono::system_clock::now();
    F_01(v1, ret);
    auto end2 = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end2 - start2;
    cout << "Elapsed time F1 (s): " << elapsed_seconds.count() << endl;

    start2 = std::chrono::system_clock::now();
    elementwise_max_parallel(ret, 0);
    end2 = std::chrono::system_clock::now();
    elapsed_seconds = end2 - start2;
    cout << "Elapsed time F2 (s): " << elapsed_seconds.count() << endl;

    //  Puts f_double into the first half of the ret vector
    start2 = std::chrono::system_clock::now();
    F_12(v2, ret);
    end2 = std::chrono::system_clock::now();
    elapsed_seconds = end2 - start2;
    cout << "Elapsed time F3 (s): " << elapsed_seconds.count() << endl;
}

// Exactly like F, but saves memory as it can be fed v, v+R but use only one vector
void F_withplusR(const double R, ArrayXd &v2, ArrayXd &ret) {
    // Places f01 and f11 back to back into the vector pointed to by ret
    // v2+R is normally fed to F_01. However, we can actually avoid doing so.
    F_01(v2, ret);

    // v2+R is NOT fed to F_01. Instead, since ret[] is always set to v[] + v[], we can just add 2*R at the end.
    elementwise_max_parallel(ret, R);
    /* While it would be nice to do this function first and then add 2*R to the vector, it would prevent us from reusing
    memory since F_12 requires half a vector, but F_01 temporarily requires a full vector (as currently implemented).
    v2 without R added*/
    F_12(v2, ret);
}

void FeasibleTriplet(int n) {
    //  ArrayXd v0 = ArrayXd::Zero(powminus1); // unneeded in single vec impl
    auto start = std::chrono::system_clock::now();
    ArrayXd v1 = ArrayXd::Zero(powminus1);

    // ArrayXd u = ArrayXd::Zero(powminus1);  // u never used
    double r = 0;
    double e = 0;
    for (int i = 2; i < n + 1; i++) {
        ArrayXd v2(powminus1);  // we pass a pointer to this, and have it get filled in
        // Writes new vector (v2) into v2
        auto start2 = std::chrono::system_clock::now();
        F(v1, v1, v2);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start2;
        cout << "Elapsed time F (s): " << elapsed_seconds.count() << endl;

        start2 = std::chrono::system_clock::now();
        // TODO: if this uses a vector of mem temporarily, store v2-v1 into v1?
        const double R = findR_parallel(v1, v2);
        end = std::chrono::system_clock::now();
        elapsed_seconds = end - start2;
        cout << "Elapsed time (s) mc1: " << elapsed_seconds.count() << endl;
        //  Beyond this point, v1's values are no longer needed, so we reuse
        //  its memory for other computations.

        // NOTE: THE v1's HERE ARE REUSED MEMORY. v1's VALUES NOT USED, INSTEAD
        // v1 IS COMPLETELY OVERWRITTEN HERE.
        // Normally F(v2+R, v2, v0), but I think this ver saves an entire vector
        // of memory at the cost of a small amount more computation.
        start2 = std::chrono::system_clock::now();
        F_withplusR(R, v2, v1);
        end = std::chrono::system_clock::now();
        elapsed_seconds = end - start2;
        cout << "Elapsed time FpR (s): " << elapsed_seconds.count() << endl;

        // Idea: normally below line is ArrayXd W = v2 + 2 * R - v0;
        // We again reuse v1 to store W, and with single vector v0 is replaced by v1.
        start2 = std::chrono::system_clock::now();
        v1 = v2 + 2 * R - v1;
        const double E = std::max(0.0, v1.maxCoeff());
        end = std::chrono::system_clock::now();
        elapsed_seconds = end - start2;
        cout << "Elapsed time mc2 (s): " << elapsed_seconds.count() << endl;
        // TODO: parallelize above operations

        // // FIXME: FIGURE OUT WHAT NEW IF STATEMENT NEEDS TO BE
        // if (R - E >= r - e) {
        //     // u = v2;  // u is never used
        //     r = R;
        //     e = E;
        // }
        r = R;
        e = E;
        // Swap pointers of v1 and v2
        // v0 = v1;
        // v1 = v2;
        std::swap(v1, v2);

        // FIXME: Need to verify mathematically the correctness of this new calculation.
        if (PRINT_EVERY_ITER) {
            // (other quantities of potential interest)
            // cout << "At n=" << i << ": " << 2.0 * (r - e) << endl;
            // cout << "At n=" << i << ": " << 2.0 * (r - e) / (1 + (r - e)) << endl;
            // cout << "At n=" << i << ": " << 2.0 * (e - r) / (1 + (e - r)) << endl;
            // cout << "At n=" << i << ": " << 2.0 * (e - r) << endl;
            // cout << "R, E: " << R << " " << E << endl;
            // cout << (2.0 * r / (1 + r)) << endl;
            cout << "At n=" << i << ": " << (2.0 * r / (1 + r)) + 2.0 * (r - e) / (1 + (r - e)) << endl;
            end = std::chrono::system_clock::now();
            elapsed_seconds = end - start;
            cout << "Elapsed time (s): " << elapsed_seconds.count() << endl;
        }

        // cout << r << " " << R << endl;
    }

    // return u, r, e
    // cout << "Result: " << 2.0 * (r - e) << endl;
    cout << "Single Vec Result: " << 2.0 * r / (1 + r) << endl;
}

int main() {
    cout << "Starting with l = " << length << "..." << endl;
    auto start = std::chrono::system_clock::now();
    FeasibleTriplet(200);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    cout << "Elapsed time (s): " << elapsed_seconds.count() << endl;

    return 0;
}