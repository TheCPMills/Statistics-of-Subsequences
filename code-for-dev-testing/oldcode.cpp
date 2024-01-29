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

// this went at the bottom of the new loop
//  cout << "nextloop\n";
//  for (uint64_t str = start + powminus2 / 2; str < end; str++) {
//      // loop 1
//      const uint64_t A = str & 0xAAAAAAAAAAAAAAAA;
//      const uint64_t TB = (str & 0x5555555555555555) & (powminus2 - 1);
//      uint64_t ATB0 = A | (TB << 2);
//      uint64_t ATB1 = ATB0 | 1;  // 0b1 <- the smallest bit in B is set to 1

//     const double loop1val = v[ATB0] + v[ATB1];
//     // cout << loop1val << endl;

//     ret[str] = 0.5 * std::max(ret[str], loop1val) + 2 * R;  //+2*R //TODO:ADD THIS
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