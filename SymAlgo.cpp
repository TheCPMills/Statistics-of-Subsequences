// #define EIGEN_NO_DEBUG 1

#include <Eigen/Dense>
#include <chrono>
#include <cmath>
#include <ctime>
#include <iostream>
using namespace std;
using Eigen::Array;
using Eigen::ArrayXd;

#define length 8

const bool PRINT_EVERY_ITER = false;

// equal to pow(2, 2 * length)
const uint64_t powminus0 = uint64_t(1) << (2 * length);
// equal to pow(2, 2 * length - 1)
const uint64_t powminus1 = uint64_t(1) << ((2 * length) - 1);
// equal to pow(2, 2 * length - 2)
const uint64_t powminus2 = uint64_t(1) << ((2 * length) - 2);

// TODO: in all array accesses, get rid of range checking
// https://eigen.tuxfamily.org/dox/group__QuickRefPage.html
// (what's difference between vec[i] and vec(i)?)

// make sure to pass by reference, not by value
// https://eigen.tuxfamily.org/dox/group__TopicPassingByValue.html
// TODO: look into using Ref's

// TODO: define length at the top here (#define length NUMBER), remove length as argument anywhere
// already tested this, verified it does indeed reduce computation time
// if pow is static, then i think most (all?) the arrays can be static sized

// Seems like they actually recommend using dynamic arrays for large sizes
// even if you know the size at compile time.
// https://eigen.tuxfamily.org/dox/group__TutorialMatrixClass.html

// TODO: figure out if this matters
// https://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html

// See https://eigen.tuxfamily.org/dox/TopicPreprocessorDirectives.html for flags to consider
// E.g., EIGEN_NO_DEBUG (currently set at top, but idk if I'm using it right?)
// May be possible to get around stack size issues with this. Unsure if good idea?

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
    ret = ArrayXd::Zero(powminus1);

    // TODO: Make these uint64_t
    const uint64_t start = powminus2;
    const uint64_t end = powminus1;

    // maybe convert into bitvec
    for (uint64_t str = start; str < end; str++) {
        const uint64_t A = str & 0xAAAAAAAA;
        // the & is to just get rid of first character
        const uint64_t TB = (str & 0x55555555) & ((uint64_t(1) << (2 * length - 2)) - 1);
        uint64_t ATB0 = A | (TB << 2);
        uint64_t ATB1 = ATB0 | 1;  // 0b1 <- the smallest bit in B is set to 1

        // I wonder if assigning to new variables is faster?
        ATB0 = std::min(ATB0, (powminus0 - 1) - ATB0);
        ATB1 = std::min(ATB1, (powminus0 - 1) - ATB1);
        ret[str - start] = v[ATB0] + v[ATB1];  // if h(A) != h(B) and h(A) = z
    }

    // from halfway to 3/4 of way
    for (uint64_t str = end; str < 3 * start; str++) {
        const uint64_t TA = (str & 0xAAAAAAAA) & ((uint64_t(1) << (2 * length - 1)) - 1);
        const uint64_t B = str & 0x55555555;
        uint64_t TA0B = (TA << 2) | B;
        uint64_t TA1B = TA0B | 2;

        // I wonder if assigning to new variables is faster?
        TA0B = std::min(TA0B, (powminus0 - 1) - TA0B);
        TA1B = std::min(TA1B, (powminus0 - 1) - TA1B);

        ret[str - start] = v[TA0B] + v[TA1B];  // if h(A) != h(B) and h(A) = z
    }
    // return ret;
}

void F_12(const ArrayXd &v, ArrayXd &ret) {
    ret = ArrayXd::Zero(powminus2);
    for (uint64_t str = 0; str < powminus2; str++) {
        const uint64_t TA = (str & 0xAAAAAAAA) & ((uint64_t(1) << (2 * length - 1)) - 1);
        const uint64_t TB = (str & 0x55555555) & ((uint64_t(1) << (2 * length - 2)) - 1);
        uint64_t TA0TB0 = (TA << 2) | (TB << 2);
        uint64_t TA0TB1 = TA0TB0 | 0b1;
        uint64_t TA1TB0 = TA0TB0 | 0b10;
        uint64_t TA1TB1 = TA0TB0 | 0b11;
        // There might be an argument to split this for loop into 4,
        // so that something something cache hits/optimization.

        // I wonder if assigning to new variables is faster?
        TA0TB0 = std::min(TA0TB0, (powminus0 - 1) - TA0TB0);
        TA0TB1 = std::min(TA0TB1, (powminus0 - 1) - TA0TB1);
        TA1TB0 = std::min(TA1TB0, (powminus0 - 1) - TA1TB0);
        TA1TB1 = std::min(TA1TB1, (powminus0 - 1) - TA1TB1);
        ret[str] = v[TA0TB0] + v[TA0TB1] + v[TA1TB0] + v[TA1TB1];
    }
    // return ret;
}

void F(const ArrayXd &v1, const ArrayXd &v2, ArrayXd &ret) {
    // try a potential optimization? perform 0.5* locally?
    ArrayXd f01f11;
    F_01(v1, f01f11);  // contains both f01 and f11 back to back
    // reverses f11
    f01f11(Eigen::seq(powminus2, powminus1 - 1)).reverseInPlace();

    ArrayXd f_double;
    F_12(v2, f_double);

    // The .eval() is necessary to prevent aliasing issues.
    f01f11 = 0.5 * (f01f11(Eigen::seq(0, powminus2 - 1)).max(f01f11(Eigen::seq(powminus2, powminus1 - 1))).eval());

    // Combine f_double and f01f11
    // TODO: figure out how to assign this without doing any (or very minimal) copying
    // because I am unsure if this copies the values to ret or just uses pointer
    ret << (1 + 0.25 * f_double), f01f11;
}

void FeasibleTriplet(int n) {
    ArrayXd v0 = ArrayXd::Zero(powminus1);
    cout << "snarf" << endl;
    ArrayXd v1 = ArrayXd::Zero(powminus1);
    cout << "snarf" << endl;

    // ArrayXd u = ArrayXd::Zero(powminus1);  // is u ever used? get rid of it?
    double r = 0;
    double e = 0;
    for (int i = 2; i < n + 1; i++) {
        ArrayXd v2(powminus1);
        cout << "snarf" << endl;
        F(v1, v0, v2);
        cout << "snarf" << endl;
        double R = (v2 - v1).maxCoeff();

        // ArrayXd retF(powminus1);
        // NOTE: THE v0's HERE ARE REUSED MEMORY. v0's VALUES NOT USED.
        F(v2 + R, v2, v0);
        ArrayXd W = v2 + 2 * R - v0;
        double E = std::max(0.0, W.maxCoeff());
        if (R - E >= r - e) {
            // u = v2;  // TODO: is u ever used? get rid of it?
            r = R;
            e = E;
        }
        // TODO: verify that these are copying reference, not value?
        v0 = v1;
        v1 = v2;

        if (PRINT_EVERY_ITER) {
            cout << "At n=" << i << ": " << 2.0 * (r - e) << endl;
        }
    }

    // return u, r, e
    cout << "Result: " << 2.0 * (r - e) << endl;
}

int main() {
    auto start = std::chrono::system_clock::now();
    FeasibleTriplet(100);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    cout << "Elapsed time: " << elapsed_seconds.count() << endl;

    return 0;
}