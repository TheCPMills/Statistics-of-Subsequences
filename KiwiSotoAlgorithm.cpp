#include <Eigen/Dense>
#include <chrono>
#include <cmath>
#include <ctime>
#include <iostream>
using namespace std;
using Eigen::ArrayXd;

// TODO: in all array accesses, get rid of range checking
// https://eigen.tuxfamily.org/dox/group__QuickRefPage.html

// TODO: could maybe reduced computation by replacing length arugment with static variables
// or using constexpr's (e.g. for the pow variables)
// also, just declare things as constant in general
// make sure to pass by reference, not by value
// https://eigen.tuxfamily.org/dox/group__TopicPassingByValue.html

// TODO: define length at the top here (#define length NUMBER), remove length as argument anywhere
// already tested this, verified it does indeed reduce computation time
// if pow is static, then i think most (all?) the arrays can be static sized

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

ArrayXd F_01(const ArrayXd &v, int length) {
    ArrayXd ret = ArrayXd::Zero(pow(2, 2 * length));

    // TODO: Make these uint64_t
    uint32_t start = pow(2, 2 * length - 2);
    uint32_t end = pow(2, 2 * length - 1);

    // maybe convert into bitvec
    for (uint32_t str = start; str < end; str++) {
        uint32_t A = str & 0xAAAAAAAA;
        // the & is to just get rid of first character
        uint32_t TB = (str & 0x55555555) & ((1 << (2 * length - 2)) - 1);
        uint32_t ATB0 = A | (TB << 2);
        uint32_t ATB1 = ATB0 | 1;      // 0b1 <- the smallest bit in B is set to 1
        ret[str] = v[ATB0] + v[ATB1];  // if h(A) != h(B) and h(A) = z
    }

    // from halfway to 3/4 of way
    for (uint32_t str = end; str < 3 * start; str++) {
        uint32_t TA = (str & 0xAAAAAAAA) & ((1 << (2 * length - 1)) - 1);
        uint32_t B = str & 0x55555555;
        uint32_t TA0B = (TA << 2) | B;
        uint32_t TA1B = TA0B | 2;
        ret[str] = v[TA0B] + v[TA1B];  // if h(A) != h(B) and h(A) = z
    }
    return ret;
}

ArrayXd F_12(const ArrayXd &v, int length) {
    ArrayXd ret = ArrayXd::Zero(pow(2, 2 * length));
    for (uint32_t str = 0; str < int(pow(2, 2 * length - 2)); str++) {
        uint32_t TA = (str & 0xAAAAAAAA) & ((1 << (2 * length - 1)) - 1);
        uint32_t TB = (str & 0x55555555) & ((1 << (2 * length - 2)) - 1);
        uint32_t TA0TB0 = (TA << 2) | (TB << 2);
        uint32_t TA0TB1 = TA0TB0 | 0b1;
        uint32_t TA1TB0 = TA0TB0 | 0b10;
        uint32_t TA1TB1 = TA0TB0 | 0b11;
        ret[str] = v[TA0TB0] + v[TA0TB1] + v[TA1TB0] + v[TA1TB1];
    }
    return ret;
}

ArrayXd F(const ArrayXd &v1, const ArrayXd &v2, int length) {
    // try as bool?
    ArrayXd b(int(pow(2, 2 * length)));
    b << ArrayXd::Ones(pow(2, 2 * length - 2)), ArrayXd::Zero(pow(2, 2 * length - 1)),
        ArrayXd::Ones(pow(2, 2 * length - 2));

    ArrayXd f01 = F_01(v1, length);
    ArrayXd f11 = f01.reverse();
    ArrayXd f_double = F_12(v2, length);

    // TODO: return by reference, not value? is that a thing? (check other F's as well)
    return b + ((0.5 * f01) + (0.25 * f_double)).max((0.5 * f11) + (0.25 * f_double.reverse()));
}

void FeasibleTriplet(int length, int n) {
    // alternatively, 2 << (2*length) or maybe 1 <<
    ArrayXd v0 = ArrayXd::Zero(pow(2, 2 * length));
    ArrayXd v1 = ArrayXd::Zero(pow(2, 2 * length));

    ArrayXd u = ArrayXd::Zero(pow(2, 2 * length));  // is u ever used? get rid of it?
    double r = 0;
    double e = 0;
    for (int i = 2; i < n + 1; i++) {
        ArrayXd v2 = F(v1, v0, length);
        double R = (v2 - v1).maxCoeff();
        ArrayXd W = v2 + 2 * R - F(v2 + R, v2, length);
        double E = std::max(0.0, W.maxCoeff());
        if (R - E >= r - e) {
            u = v2;  // TODO: is u ever used? get rid of it?
            r = R;
            e = E;
        }
        // TODO: verify that these are copying reference, not value?
        v0 = v1;
        v1 = v2;
    }

    // return u, r, e
    cout << "Result: " << 2.0 * (r - e) << endl;
}

int main() {
    int strLen = 12;
    for (int i = 1; i < strLen + 1; i++) {
        auto start = std::chrono::system_clock::now();
        FeasibleTriplet(i, 100);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        cout << "Elapsed time: " << elapsed_seconds.count() << endl;
    }
    return 0;
}