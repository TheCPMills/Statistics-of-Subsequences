#define EIGEN_NO_DEBUG 1
// TODO: PUT ABOVE BACK

#include <Eigen/Dense>
#include <chrono>
#include <cmath>
#include <ctime>
#include <iostream>
using namespace std;
using Eigen::Array;
using Eigen::ArrayXd;

#define length 14

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

void F_01(const ArrayXd &v, ArrayXd &ret) {
    // cout << "Has memory here" << endl;
    // cout.flush();
    // ret = ArrayXd::Zero(powminus1);  // technically not necessary I think
    // cout << "Ran out of memory before here" << endl;
    // cout.flush();

    const uint64_t start = powminus2;
    const uint64_t end = powminus1;

    // from 1/4 to 1/2 of way through
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
        ret[str - start] = v[ATB0] + v[ATB1];  // if h(A) != h(B) and h(A) = z
    }

    // from halfway to 3/4 of way
    for (uint64_t str = end; str < 3 * start; str++) {
        // Take take every other bit (starting at first position)
        // The second & gets rid of the first bit
        const uint64_t TA = (str & 0xAAAAAAAAAAAAAAAA) & ((uint64_t(1) << (2 * length - 1)) - 1);
        // Take every other bit (starting at second position)
        const uint64_t B = str & 0x5555555555555555;
        uint64_t TA0B = (TA << 2) | B;
        uint64_t TA1B = TA0B | 2;

        // I wonder if assigning to new variables is faster?
        TA0B = std::min(TA0B, (powminus0 - 1) - TA0B);
        TA1B = std::min(TA1B, (powminus0 - 1) - TA1B);

        ret[str - start] = v[TA0B] + v[TA1B];  // if h(A) != h(B) and h(A) = z
    }
    //  return ret;
}

void F_12(const ArrayXd &v, ArrayXd &ret) {
    // ret = ArrayXd::Zero(powminus2);
    for (uint64_t str = 0; str < powminus2; str++) {
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
    // return ret;
}

// (remember: any changes made in here have to be made in F_withplusR as well)
void F(const ArrayXd &v1, const ArrayXd &v2, ArrayXd &ret) {
    // Places f01 and f11 back to back into the vector pointed to by ret
    F_01(v1, ret);

    /* This line does the following:
    Takes the elementwise maximum of f01 (first half of ret) and f11 (second half of ret, iterated in
    reverse order), and fills the second half of the ret vector with the result (multiplied by 0.5). Afterward, first
    half of ret vector can be safely overwritten. The .eval() is necessary to prevent aliasing issues. */
    ret(Eigen::seq(powminus2, powminus1 - 1)) =
        0.5 * (ret(Eigen::seq(0, powminus2 - 1)).max(ret(Eigen::seq(powminus1 - 1, powminus2, -1))).eval());

    // Puts f_double into the first half of the ret vector
    F_12(v2, ret);
}

// Exactly like F, but saves memory as it can be fed v, v+R but use only one vector
void F_withplusR(double R, ArrayXd &v2, ArrayXd &ret) {
    // Places f01 and f11 back to back into the vector pointed to by ret
    // v2+R is normally fed to F_01. However, we can actually avoid doing so.
    F_01(v2, ret);

    // v2+R is NOT fed to F_01. Instead, since ret[] is always set to v[] + v[], we can just add 2*R at the end.
    ret(Eigen::seq(powminus2, powminus1 - 1)) =
        0.5 * (ret(Eigen::seq(0, powminus2 - 1)).max(ret(Eigen::seq(powminus1 - 1, powminus2, -1))).eval()) + 2 * R;

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
        F(v1, v1, v2);

        // TODO: if this uses a vector of mem temporarily, store v2-v1 into v1?
        double R = (v2 - v1).maxCoeff();
        // Beyond this point, v1's values are no longer needed, so we reuse
        // its memory for other computations.

        // NOTE: THE v1's HERE ARE REUSED MEMORY. v1's VALUES NOT USED, INSTEAD
        // v1 IS COMPLETELY OVERWRITTEN HERE.
        // Normally F(v2+R, v2, v0), but I think this ver saves an entire vector
        // of memory at the cost of a small amount more computation.
        F_withplusR(R, v2, v1);

        // Idea: normally below line is ArrayXd W = v2 + 2 * R - v0;
        // We again reuse v1 to store W, and with single vector v0 is replaced by v1.
        v1 = v2 + 2 * R - v1;
        double E = std::max(0.0, v1.maxCoeff());

        // // FIXME: FIGURE OUT WHAT NEW IF STATEMENT NEEDS TO BE
        // if (R - E >= r - e) {
        //     // u = v2;  // u is never used
        //     r = R;
        //     e = E;
        // }
        r = R;
        e = E;
        // TODO: Make these copy by reference, not value (swap pointers)
        // v0 = v1;
        v1 = v2;

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
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
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