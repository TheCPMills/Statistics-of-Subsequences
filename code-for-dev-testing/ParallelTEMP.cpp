// #define EIGEN_NO_DEBUG 1
//   TODO: PUT ABOVE BACK

#include <Eigen/Dense>
#include <bitset>
#include <chrono>
#include <cmath>
#include <ctime>
#include <future>
#include <iostream>
#include <thread>
using Eigen::ArrayXd;
using std::cout;
using std::endl;

#define length 3
#define NUM_THREADS 1
// Careful: ensure that NUM_THREADS divides 2^(2*length-1) (basically always will for l > 3 if power of 2)

const bool PRINT_EVERY_ITER = true;

// equal to pow(2, 2 * length)
const uint64_t powminus0 = uint64_t(1) << (2 * length);
// equal to pow(2, 2 * length - 1)
const uint64_t powminus1 = uint64_t(1) << ((2 * length) - 1);
// equal to pow(2, 2 * length - 2)
const uint64_t powminus2 = uint64_t(1) << ((2 * length) - 2);

template <typename Derived>
void printArray(const Eigen::ArrayBase<Derived> &arr) {
    for (int i = 0; i < arr.size(); i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}

// void loopUntilTolerance(double tol){
//     double prev = 0.0;
//     if (prev == 0.0 || tol-prev > )
// }

// TODO: consider reversing order of strings so that no &'ing needs to happen, you just bitshift right
//  instead of left? (may not work for iterating through strings. need to figure out.)

/* Normally, we find R with R = (v2 - v1).maxCoeff();
    However, this is slower than the multithreaded F functions if not parallelized!
    So instead we break that calculation up across threads.
    Additionally, E = std::max(0.0, (v2 + 2 * R - v1).maxCoeff()). We can use
    this function for that as well, and just add 2*R at the end. */
double subtract_and_find_max_parallel(const ArrayXd &v1, const ArrayXd &v2) {
    std::future<double> maxVals[NUM_THREADS];
    const uint64_t incr = powminus1 / NUM_THREADS;

    // TODO: see if writing own function to do this insttead of Eigen is faster
    // Function to calculate the maximum coefficient in a particular (start...end) slice
    auto findMax = [&v1, &v2](uint64_t start, uint64_t end) {
        return (v2(Eigen::seq(start, end - 1)) - v1(Eigen::seq(start, end - 1))).maxCoeff();
    };

    // Set threads to calculate the max coef in their own smaller slices
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

void F_combined_loop(const uint64_t start, const uint64_t end, const ArrayXd &v, ArrayXd &ret) {
    for (uint64_t str = start; str < end; str++) {
        // save val for loop 2
        const uint64_t str2 =
            str + powminus2;  // this will ALWAYS have the effect of setting biggest 1 to 0, and putting 1 to left
        const uint64_t str3 = powminus0 + powminus2 - 1 - str2;  // initially forgot the +/-powminus2
        const uint64_t str4 = powminus0 - 1 - str2;
        // loop 1
        // Take every other bit (starting at first position)
        const uint64_t A = str & 0xAAAAAAAAAAAAAAAA;
        // Take take every other bit (starting at second position)
        // The second & gets rid of the first bit
        const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
        uint64_t ATB0 = A | (TB << 2);
        uint64_t ATB1 = ATB0 | 1;  // 0b1 <- the smallest bit in B is set to 1
        std::bitset<2 * length> atb0_before(ATB0);
        std::bitset<2 * length> atb1_before(ATB1);

        // I wonder if assigning to new variables is faster?
        // TODO: THESE ARE UNNECESSARY!
        ATB0 = std::min(ATB0, (powminus0 - 1) - ATB0);
        ATB1 = std::min(ATB1, (powminus0 - 1) - ATB1);
        // ret[str - powminus2] = v[ATB0] + v[ATB1];  // if h(A) != h(B) and h(A) = z

        // loop 2
        const uint64_t TA = (str2 & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);  //
        // Take every other bit (starting at second position)
        const uint64_t B = str2 & 0x5555555555555555;
        uint64_t TA0B = (TA << 2) | B;
        uint64_t TA1B = TA0B | 2;
        std::bitset<2 * length> ta0b_before(TA0B);
        std::bitset<2 * length> ta1b_before(TA1B);

        // TODO: may be clever way of figuring out these mins ahead of time
        // I wonder if assigning to new variables is faster?
        TA0B = std::min(TA0B, (powminus0 - 1) - TA0B);
        TA1B = std::min(TA1B, (powminus0 - 1) - TA1B);

        /* The below line is normally ret[str - powminus2] = v[TA0B] + v[TA1B];
        However, the values from this part get reversed afterwards. So, instead,
        we do the reversing here locally to avoid the need for that (it allows
        us to avoid needing to copy things when multithreading later in elementwise_max).
        (3*powminus2 -(str-powminus2) = powminus0-str) */
        // ret[powminus0 - 1 - str2] = v[TA0B] + v[TA1B];  // if h(A) != h(B) and h(A) = z

        // loop 3 (NEW, str3 is complement of str2 I THINK and is what gets cmprd to str)
        const uint64_t TA3 = (str3 & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);  //
        // Take every other bit (starting at second position)
        const uint64_t B3 = str3 & 0x5555555555555555;
        uint64_t TA0B3 = (TA3 << 2) | B3;
        uint64_t TA1B3 = TA0B3 | 2;
        std::bitset<2 * length> ta0b_before3(TA0B3);
        std::bitset<2 * length> ta1b_before3(TA1B3);

        // TODO: may be clever way of figuring out these mins ahead of time
        // I wonder if assigning to new variables is faster?
        TA0B3 = std::min(TA0B3, (powminus0 - 1) - TA0B3);
        TA1B3 = std::min(TA1B3, (powminus0 - 1) - TA1B3);

        // loop 1
        // Take every other bit (starting at first position)
        const uint64_t A4 = str4 & 0xAAAAAAAAAAAAAAAA;
        // Take take every other bit (starting at second position)
        // The second & gets rid of the first bit
        const uint64_t TB4 = (str4 & 0x5555555555555555) & (powminus2 - 1);
        uint64_t ATB04 = A4 | (TB4 << 2);
        uint64_t ATB14 = ATB04 | 1;  // 0b1 <- the smallest bit in B is set to 1
        std::bitset<2 * length> atb0_before4(ATB04);
        std::bitset<2 * length> atb1_before4(ATB14);

        std::bitset<2 * length> s(str);
        std::bitset<2 * length> s2(str2);
        std::bitset<2 * length> s3(str3);
        std::bitset<2 * length> a(A);
        std::bitset<2 * length> b2(B);
        std::bitset<2 * length> b3(B3);
        std::bitset<2 * length> a4(A4);
        std::bitset<2 * length> tb(TB);
        std::bitset<2 * length> ta2(TA);
        std::bitset<2 * length> ta3(TA3);
        std::bitset<2 * length> tb4(TB4);
        // A = TA2
        // B2 = TB

        std::bitset<2 * length> atb0_after(ATB0);
        std::bitset<2 * length> atb1_afer(ATB1);
        std::bitset<2 * length> ta0b_after(TA0B);
        std::bitset<2 * length> ta1b_after(TA1B);

        std::bitset<2 * length> ta0b_after3(TA0B3);
        std::bitset<2 * length> ta1b_after3(TA1B3);

        // TA0B3 = TA1B (both after)
        // TA1B3 = TA0B (both after)
        //  you get same relationship for the before, except that they are complemented (e.g. TA1B3 = ~TA0B)
        cout << s << " " << s2 << " (" << s3 << "),  " << a << " " << b2 << " (" << b3 << ") (" << a4 << ") ,  " << tb
             << " " << ta2 << " (" << ta3 << ") (" << tb4 << ") ,  " << atb0_after << " " << atb1_afer << " "
             << ta0b_after << " " << ta1b_after << " (" << ta0b_after3 << " " << ta1b_after3 << ") (" << atb0_before4
             << " " << atb1_before4 << ") ,  " << atb0_before << " " << atb1_before << " " << ta0b_before << " "
             << ta1b_before << " (" << ta0b_before3 << " " << ta1b_before3 << "),  " << endl;

        // should i be looking at this, or the inverse?
    }
}

void WIPLOOP(const ArrayXd &v, ArrayXd &ret, const double R) {
    const uint64_t start = powminus2;
    const uint64_t end = powminus1;
    for (uint64_t str = start; str < end; str++) {
        // save val for loop 2
        const uint64_t str2 =
            str + powminus2;  // this will ALWAYS have the effect of setting biggest 1 to 0, and putting 1 to left
        const uint64_t str3 = powminus0 + powminus2 - 1 - str2;  // NOTE: WORKS WITH BOTH - OR + powminus2
        // loop 1
        // Take every other bit (starting at first position)
        const uint64_t A = str & 0xAAAAAAAAAAAAAAAA;
        // Take take every other bit (starting at second position)
        // The second & gets rid of the first bit
        const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
        uint64_t ATB0 = A | (TB << 2);
        uint64_t ATB1 = ATB0 | 1;  // 0b1 <- the smallest bit in B is set to 1

        // ret[str - powminus2] = v[ATB0] + v[ATB1];  // if h(A) != h(B) and h(A) = z
        // will have to change this to go into second half of array

        // loop 2
        const uint64_t TA = A;
        // const uint64_t TA = (str2 & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);  //equivalent!

        // Take every other bit (starting at second position)
        const uint64_t B = TB;
        // const uint64_t B = str2 & 0x5555555555555555; //equivalent!
        uint64_t TA0B = (TA << 2) | B;
        uint64_t TA1B = TA0B | 2;

        // TODO: may be clever way of figuring out these mins ahead of time
        // I wonder if assigning to new variables is faster?
        TA0B = std::min(TA0B, (powminus0 - 1) - TA0B);
        TA1B = std::min(TA1B, (powminus0 - 1) - TA1B);

        /* The below line is normally ret[str - powminus2] = v[TA0B] + v[TA1B];
        However, the values from this part get reversed afterwards. So, instead,
        we do the reversing here locally to avoid the need for that (it allows
        us to avoid needing to copy things when multithreading later in elementwise_max).
        (3*powminus2 -(str-powminus2) = powminus0-str) */
        // ret[powminus0 - 1 - str2] = v[TA0B] + v[TA1B];  // if h(A) != h(B) and h(A) = z

        // loop 3 (NEW, str3 is complement of str2 I THINK and is what gets cmprd to str)
        const uint64_t TA3 = (str3 & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);  // equivalent!

        // Take every other bit (starting at second position)
        const uint64_t B3 = str3 & 0x5555555555555555;  // equivalent!
        uint64_t TA0B3 = (TA3 << 2) | B3;
        uint64_t TA1B3 = TA0B3 | 2;

        // TODO: may be clever way of figuring out these mins ahead of time
        // I wonder if assigning to new variables is faster?
        // i think these can both be checked by 1 if statement at the same time
        // (if TA0B3 > powminus1 -1 )
        TA0B3 = std::min(TA0B3, (powminus0 - 1) - TA0B3);
        TA1B3 = std::min(TA1B3, (powminus0 - 1) - TA1B3);

        // idea: computing the part of the array with no need for reversing, so that one value is being set
        // also computing entry as if starting from beginning of loop2
        //(also)
        // wait a minute: there may be other parts of loop1 that are symmetric/similar as in loop2

        // A = TA2
        // B2 = TB

        // TA0B3 = TA1B (both after)
        // TA1B3 = TA0B (both after)
        //  you get same relationship for the before, except that they are complemented (e.g. TA1B3 = ~TA0B)
        // NEVERMIND. I DIDNT HAVE -POWMINUS2 IN THERE.
        // CHECK FOR PATTERNS NOW. BUT PROBABLY ARENT ANY.
        // NEW PLAN: STILL DO REVERSAL LOCALLY (STR3).
        // ALSO STILL COMPUTE STR2.
        // actually, plan is pretty much unchanged. just that str3 cant be calc'd with str2.
        // First though, check to see if there's any computation that can be reused related to this new str3.
        // like, str3-powminus2 or something, idk.

        // TODO: figure ou if can increment by values of 2, and reuse parts of computation (can't reuse everything, but
        // maybe first parts)
        const double loop1val = v[ATB0] + v[ATB1];
        const double loop2val = v[TA0B] + v[TA1B];
        const double loop2valcomp = v[TA0B3] + v[TA1B3];
        ret[str] = 0.5 * std::max(loop1val, loop2valcomp) + 2 * R;  //+2*R //TODO:ADD THIS

        // can maybe optimize above to be two bitwise ops: ~ and &

        //  first test if above works
        //  next, test if this works:
        //  do this current loop only half as much, with below line
        //  ret[powminus1-1-str2]  = loop2val;
        //  then, in a separate loop that loops through the remaining half,
        //  compute only loop1val, and set ret[str] = 0.5* std::max(loop1val, ret[str]);
        cout << str << " " << str2 << " " << str3 << " " << loop2valcomp << endl;
    }
}

void WIPLOOP_new(const ArrayXd &v, ArrayXd &ret, const double R) {
    const uint64_t start = powminus2;
    const uint64_t end = powminus1;
    for (uint64_t str = start; str < start + powminus2 / 2; str++) {
        // save val for loop 2
        const uint64_t str2 =
            str + powminus2;  // this will ALWAYS have the effect of setting biggest 1 to 0, and putting 1 to left
        const uint64_t str3 = powminus0 + powminus2 - 1 - str2;
        // maybe be able to do this subtraction as 2 bitwise ops? & and ~?

        // I think str3-powminus2 shares a lot with str?
        // we want to place starting at powminus2 in the array, ending at powminus1 -1
        // str starts at powminus2  (goes until powminus1 -1 ||| powminus2+powminus3 -1)
        // str2 starts at powminus1 (goes until powminus1+powminus2 -1 ||| powminus1+powminus3 -1)
        // str3 starts at powminus1 - 1 + powminus2 (goes DOWN until powminus1 ||| powminus1+powminus3)
        // str4 = str3-powminus2
        // str4 starts at powminus1 -1 (goes DOWN until powminus2 ||| powminus2+powminus3)
        // const uint64_t str4 = str3 - powminus2;

        //  loop 1
        //  Take every other bit (starting at first position)
        const uint64_t A = str & 0xAAAAAAAAAAAAAAAA;
        // Take every other bit (starting at second position)
        // The second & gets rid of the first bit
        const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
        uint64_t ATB0 = A | (TB << 2);
        uint64_t ATB1 = ATB0 | 1;  // 0b1 <- the smallest bit in B is set to 1

        // loop 2
        const uint64_t TA = A;
        // const uint64_t TA = (str2 & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);  //equivalent!
        // Take every other bit (starting at second position)
        const uint64_t B = TB;
        // const uint64_t B = str2 & 0x5555555555555555; //equivalent!
        uint64_t TA0B = (TA << 2) | B;
        uint64_t TA1B = TA0B | 2;

        // TODO: may be clever way of figuring out these mins ahead of time
        TA0B = std::min(TA0B, (powminus0 - 1) - TA0B);
        TA1B = std::min(TA1B, (powminus0 - 1) - TA1B);

        // loop 3 (NEW, str3 is complement of str2 I THINK and is what gets cmprd to str)
        const uint64_t TA3 = (str3 & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);  // equivalent!

        // Take every other bit (starting at second position)
        const uint64_t B3 = str3 & 0x5555555555555555;  // equivalent!
        uint64_t TA0B3 = (TA3 << 2) | B3;
        uint64_t TA1B3 = TA0B3 | 2;
        // TODO: there should be a way of doing HALF the calculations, since +2 does nothing?

        TA0B3 = std::min(TA0B3, (powminus0 - 1) - TA0B3);
        TA1B3 = std::min(TA1B3, (powminus0 - 1) - TA1B3);

        const double loop1val = v[ATB0] + v[ATB1];
        const double loop2val = v[TA0B] + v[TA1B];
        const double loop2valcomp = v[TA0B3] + v[TA1B3];
        ret[str] = 0.5 * std::max(loop1val, loop2valcomp) + 2 * R;  //+2*R //TODO:ADD THIS

        ret[powminus0 - 1 - str2] = loop2val;

        cout << loop1val << " " << loop2val << " " << loop2valcomp << " " << endl;
        // TODO: loop2val is symmetric USE THIS FACT!
    }
    cout << "nextloop\n";
    for (uint64_t str = start + powminus2 / 2; str < end; str++) {
        // loop 1
        const uint64_t A = str & 0xAAAAAAAAAAAAAAAA;
        const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
        uint64_t ATB0 = A | (TB << 2);
        uint64_t ATB1 = ATB0 | 1;  // 0b1 <- the smallest bit in B is set to 1

        const double loop1val = v[ATB0] + v[ATB1];
        cout << loop1val << endl;

        ret[str] = 0.5 * std::max(ret[str], loop1val) + 2 * R;  //+2*R //TODO:ADD THIS
    }
}

// void WIPLOOP2(const ArrayXd &v, ArrayXd &ret, const double R) {
//     const uint64_t start = powminus2 + powminus2 / 2;
//     const uint64_t end = powminus1;
//     for (uint64_t str = start + powminus2 / 2; str < end; str++) {
//         // loop 1
//         const uint64_t A = str & 0xAAAAAAAAAAAAAAAA;
//         const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
//         uint64_t ATB0 = A | (TB << 2);
//         uint64_t ATB1 = ATB0 | 1;  // 0b1 <- the smallest bit in B is set to 1
//
//         const double loop1val = v[ATB0] + v[ATB1];
//         ret[str] = 0.5 * std::max(ret[str], loop1val) + 2 * R;  //+2*R //TODO:ADD THIS
//     }
// }

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
    // Function to perform the elementwise maximum as described above on a (end-start)-length chunk
    auto elementwise_max = [&ret, R](uint64_t start, uint64_t end) {
        ret(Eigen::seq(middle + start, middle + end - 1)) =
            0.5 * ret(Eigen::seq(start, end - 1)).max(ret(Eigen::seq(middle + start, middle + end - 1))) + 2 * R;
    };

    // Set threads to calculate the elementwise max as described above in their own chunk
    std::thread threads[NUM_THREADS];
    const uint64_t incr = (powminus2) / (NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(elementwise_max, incr * i, incr * (i + 1));
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
        const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
        uint64_t ATB0 = A | (TB << 2);
        uint64_t ATB1 = ATB0 | 1;  // 0b1 <- the smallest bit in B is set to 1

        // I wonder if assigning to new variables is faster?
        ATB0 = std::min(ATB0, (powminus0 - 1) - ATB0);
        ATB1 = std::min(ATB1, (powminus0 - 1) - ATB1);
        ret[str - powminus2] = v[ATB0] + v[ATB1];  // if h(A) != h(B) and h(A) = z
        // cout << ATB0 << " " << ATB1 << " " << v[ATB0] << " " << v[ATB1] << endl;
    }
}

void F_01_loop2(const uint64_t start, const uint64_t end, const ArrayXd &v, ArrayXd &ret) {
    for (uint64_t str = start; str < end; str++) {
        // TODO: see if this can be simplified at all
        //  (inc. shifting right instead of left to maybe not need one of the lines)
        // I believe an operation CAN be removed

        // Take every other bit (starting at first position)
        // The second & gets rid of the first bit
        const uint64_t TA = (str & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);
        // Take every other bit (starting at second position)
        const uint64_t B = str & 0x5555555555555555;
        uint64_t TA0B = (TA << 2) | B;
        uint64_t TA1B = TA0B | 2;

        // TODO: may be clever way of figuring out these mins ahead of time
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
    // TODO: FIGURE OUT IF TAKING ELEMENTWISE MAX CAN BE COMBINED INTO THIS
    // BY DOING THOSE STEPS LOCALLY
    const uint64_t start = powminus2;
    const uint64_t end = powminus1;
    // F_combined_loop(start, end, v, ret);
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
    auto loop = [&v, &ret](uint64_t start, uint64_t end) {
        for (uint64_t str = start; str < end; str++) {
            // const uint64_t TA = (str & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1);
            // const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
            // uint64_t TA0TB0 = (TA << 2) | (TB << 2);
            uint64_t TA0TB0 = (str & (powminus2 - 1)) << 2;  // equivalent to above 3 lines!
            uint64_t TA0TB1 = TA0TB0 | 0b1;
            uint64_t TA1TB0 = TA0TB0 | 0b10;
            uint64_t TA1TB1 = TA0TB0 | 0b11;
            // There might be an argument to split this for loop into 4,
            // so that something something cache hits/optimization.
            // Probably not though.

            // TODO: there may be a clever way of figuring out these mins AHEAD of time (check if any unnecessary)
            // like, it's really just asking if it's > powminus1
            // I wonder if assigning to new variables is faster?
            TA0TB0 = std::min(TA0TB0, (powminus0 - 1) - TA0TB0);
            TA0TB1 = std::min(TA0TB1, (powminus0 - 1) - TA0TB1);
            TA1TB0 = std::min(TA1TB0, (powminus0 - 1) - TA1TB0);
            TA1TB1 = std::min(TA1TB1, (powminus0 - 1) - TA1TB1);
            ret[str] = 1 + .25 * (v[TA0TB0] + v[TA0TB1] + v[TA1TB0] + v[TA1TB1]);
            // TODO: figure out if doing 1+.25* is faster here (locally) or after at end
            // TODO: implement the symmetry
        }
    };
    std::thread threads[NUM_THREADS];
    const uint64_t start = 0;
    const uint64_t end = powminus2;
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
void F(const ArrayXd &v1, const ArrayXd &v2, ArrayXd &ret) {
    // Places f01 and f11 back to back into the vector pointed to by ret
    // printArray(v1);
    // cout << "V1 ABOVE\n";
    // ArrayXd retTEST = ret;
    // printArray(retTEST);
    // WIPLOOP(v1, ret, 0);
    // F_combined_loop(powminus2, powminus1, v1, ret);
    WIPLOOP_new(v1, ret, 0);
    //  printArray(retTEST);
    //   printArray(v1);
    //   cout << "V1 ABOVE AFTER WIP\n";
    auto start2 = std::chrono::system_clock::now();
    // F_01(v1, ret);
    auto end2 = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end2 - start2;
    cout << "Elapsed time F1 (s): " << elapsed_seconds.count() << endl;
    // printArray(ret);
    //  printArray(v1);
    //  cout << "V1 ABOVE AFTER F_01\n";
    start2 = std::chrono::system_clock::now();
    // elementwise_max_parallel(ret, 0);
    end2 = std::chrono::system_clock::now();
    elapsed_seconds = end2 - start2;
    cout << "Elapsed time F2 (s): " << elapsed_seconds.count() << endl;
    // printArray(ret);

    //  Puts f_double into the first half of the ret vector
    start2 = std::chrono::system_clock::now();
    F_12(v2, ret);
    end2 = std::chrono::system_clock::now();
    elapsed_seconds = end2 - start2;
    cout << "Elapsed time F3 (s): " << elapsed_seconds.count() << endl;
    // F_12(v2, retTEST);
    //  printArray(ret.cwiseEqual(retTEST));
    //  printArray(ret);
    //  printArray(retTEST);
}

// Exactly like F, but saves memory as it can be fed v, v+R but use only one vector
void F_withplusR(const double R, ArrayXd &v2, ArrayXd &ret) {
    // Places f01 and f11 back to back into the vector pointed to by ret
    // v2+R is normally fed to F_01. However, we can actually avoid doing so.
    // F_01(v2, ret);

    // v2+R is NOT fed to F_01. Instead, since ret[] is always set to v[] + v[], we can just add 2*R at the end.
    // elementwise_max_parallel(ret, R);
    // WIPLOOP(v2, ret, R);
    WIPLOOP_new(v2, ret, R);
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
    ArrayXd v2(powminus1);  // we pass a pointer to this, and have it get filled in

    double r = 0;
    double e = 0;
    for (int i = 2; i < n + 1; i++) {
        // Writes new vector (v2) into v2
        auto start2 = std::chrono::system_clock::now();
        F(v1, v1, v2);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start2;
        cout << "Elapsed time F (s): " << elapsed_seconds.count() << endl;

        start2 = std::chrono::system_clock::now();
        // TODO: if this uses a vector of mem temporarily, store v2-v1 into v1?
        const double R = subtract_and_find_max_parallel(v1, v2);
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
        // TODO: INVESTIGATE WHY ABOVE IS SO MUCH FASTER THAN F??
        // except now it isnt being faster???

        // Idea: normally below line is ArrayXd W = v2 + 2 * R - v0;
        // We again reuse v1 to store W, and with single vector v0 is replaced by v1.
        start2 = std::chrono::system_clock::now();
        const double E = std::max(subtract_and_find_max_parallel(v1, v2) + 2 * R, 0.0);
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
    cout << "(Alt more acc): " << (2.0 * r / (1 + r)) + 2.0 * (r - e) / (1 + (r - e)) << endl;
    // cout << "v1" << endl;
    // printArray(v1);
    // cout << "v2" << endl;
    // printArray(v2);
}

int main() {
    cout << "Starting with l = " << length << "..." << endl;
    auto start = std::chrono::system_clock::now();
    FeasibleTriplet(10);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    cout << "Elapsed time (s): " << elapsed_seconds.count() << endl;
    // TODO: check if eigen init parallel is something to do
    return 0;
}