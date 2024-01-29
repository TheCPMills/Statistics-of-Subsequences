std::function<void(uint64_t, uint64_t, int)> recurseF_01_loop1;
recurseF_01_loop1 = [&v, &ret, &recurseF_01_loop1](uint64_t offset, uint64_t index_offset, int depth) -> void {
    // num_strs = powminus(2+2*depth)
    uint64_t num_strs = uint64_t(1) << (2 * (length - depth) - 2);
    if (depth < STOP_AT) {
        recurseF_01_loop1(offset, index_offset, depth + 1);
        recurseF_01_loop1(offset + num_strs / 4, index_offset + 2 * num_strs * 2 / 4, depth + 1);
        recurseF_01_loop1(offset + num_strs * 2 / 4, index_offset + 2 * num_strs / 4, depth + 1);
        recurseF_01_loop1(offset + num_strs * 3 / 4, index_offset + 2 * num_strs * 3 / 4, depth + 1);
    } else {
        uint64_t start_i = index_offset;
        uint64_t end_i = start_i + 2 * num_strs;
        // load in arr[start_i] thru arr[end_i]
        // copy_mem(ret, v, num_strs * sizeof(double));

        uint64_t start = powminus2 + offset;
        uint64_t end = start + num_strs;
        // F_01_loop1(start, end, v, ret);

        // write out result
        printf("Str start, end: %i, %i.  Index start, end: %i, %i\n", start, end, start_i, end_i);
        return;
    }
};

recurseF_01_loop1(0, 0, 0);
cout << endl;

std::function<void(uint64_t, uint64_t, int)> recurseF_01_loop2;
recurseF_01_loop2 = [&v, &ret, &recurseF_01_loop2](uint64_t offset, uint64_t index_offset, int depth) -> void {
    // num_strs = powminus(2+3*depth)
    uint64_t num_strs = uint64_t(1) << (2 * length - (3 * depth) - 2);
    if (depth < STOP_AT) {
        recurseF_01_loop2(offset, index_offset, depth + 1);
        recurseF_01_loop2(offset + num_strs * 1 / 8, index_offset + 2 * num_strs * 2 / 8, depth + 1);
        recurseF_01_loop2(offset + num_strs * 2 / 8, index_offset + 2 * num_strs / 8, depth + 1);
        recurseF_01_loop2(offset + num_strs * 3 / 8, index_offset + 2 * num_strs * 3 / 8, depth + 1);
        recurseF_01_loop2(offset + num_strs * 4 / 8, index_offset + 2 * num_strs * 7 / 8, depth + 1);
        recurseF_01_loop2(offset + num_strs * 5 / 8, index_offset + 2 * num_strs * 5 / 8, depth + 1);
        recurseF_01_loop2(offset + num_strs * 6 / 8, index_offset + 2 * num_strs * 6 / 8, depth + 1);
        recurseF_01_loop2(offset + num_strs * 7 / 8, index_offset + 2 * num_strs * 4 / 8, depth + 1);
    } else {
        uint64_t start_i = index_offset;
        uint64_t end_i = start_i + 2 * num_strs;
        // load in arr[start_i] thru arr[end_i]
        // copy_mem(ret, v, num_strs * sizeof(double));

        uint64_t start = powminus1 + offset;
        uint64_t end = start + num_strs;
        // F_01_loop1(start, end, v, ret);
        printf("Str start, end: %i, %i.  Index start, end: %i, %i\n", start, end, start_i, end_i);
        return;
    }
};
recurseF_01_loop2(0, 0, 0);
cout << endl;

std::function<void(uint64_t[2], uint64_t, int, int, bool)> recurseF_01_loop2_3part;
recurseF_01_loop2_3part = [&v, &ret, &recurseF_01_loop2_3part](uint64_t offsets[2], uint64_t index_offset, int depth,
                                                               int stage, bool reverse) -> void {
    // num_strs = powminus(2+depth)
    uint64_t num_strs = uint64_t(1) << (2 * (length)-2 - depth);
    if (depth < STOP_AT) {
        switch (stage) {
            case 1: {
                uint64_t offsets_next[2] = {offsets[0], 0};
                recurseF_01_loop2_3part(offsets_next, index_offset, depth + 1, 2, false);

                offsets_next[0] = offsets[0] + num_strs * 4 / 8;
                recurseF_01_loop2_3part(offsets_next, index_offset + 2 * num_strs * 4 / 8, depth + 1, 2, true);
                break;
            }
            case 2: {
                uint64_t offsets_next[2] = {offsets[0], offsets[0] + num_strs * 2 / 4};
                if (reverse) {
                    recurseF_01_loop2_3part(offsets_next, index_offset + 2 * num_strs * 2 / 4, depth + 1, 3, true);
                } else {
                    recurseF_01_loop2_3part(offsets_next, index_offset, depth + 1, 3, false);
                }

                offsets_next[0] = offsets[0] + num_strs * 1 / 4;
                offsets_next[1] = offsets[0] + num_strs * 3 / 4;
                if (reverse) {
                    recurseF_01_loop2_3part(offsets_next, index_offset, depth + 1, 3, true);
                } else {
                    recurseF_01_loop2_3part(offsets_next, index_offset + 2 * num_strs * 2 / 4, depth + 1, 3, false);
                }
                break;
            }
            case 3: {
                uint64_t offsets_next[2] = {offsets[0], 0};
                if (reverse) {
                    recurseF_01_loop2_3part(offsets_next, index_offset + 2 * num_strs * 1 / 2, depth + 1, 1, false);
                } else {
                    recurseF_01_loop2_3part(offsets_next, index_offset, depth + 1, 1, false);
                }

                offsets_next[0] = offsets[1];
                if (reverse) {
                    recurseF_01_loop2_3part(offsets_next, index_offset, depth + 1, 1, false);
                } else {
                    recurseF_01_loop2_3part(offsets_next, index_offset + 2 * num_strs * 1 / 2, depth + 1, 1, false);
                }
                break;
            }
        }

    } else {
        uint64_t start_i = index_offset;
        uint64_t end_i = start_i + 2 * num_strs;
        // load in arr[start_i] thru arr[end_i]
        // copy_mem(ret, v, num_strs * sizeof(double));

        if (stage == 1) {
            uint64_t start = powminus2 + powminus2 + offsets[0];
            uint64_t end = start + num_strs;
            printf("Str start, end: %i, %i.  Index start, end: %i, %i\n", start, end, start_i, end_i);

        } else if (stage == 2) {
            uint64_t start = powminus2 + powminus2 + offsets[0];
            uint64_t end = start + num_strs;
            printf("Str start, end: %i, %i.  Index start, end: %i, %i\n", start, end, start_i, end_i);
        } else {  // stage == 3
            uint64_t start = powminus2 + powminus2 + offsets[0];
            uint64_t end = start + num_strs / 2;
            printf("Str start, end: %i, %i.  Index start, end: %i, %i\n", start, end, start_i, end_i);
            start = powminus2 + powminus2 + offsets[1];
            end = start + num_strs / 2;
            printf("Str start, end: %i, %i.  Index start, end: %i, %i\n", start, end, start_i, end_i);
        }

        return;
    }
};
uint64_t begin_arr2[2] = {0, 0};
recurseF_01_loop2_3part(begin_arr2, 0, 0, 1, false);
cout << endl;

std::function<void(uint64_t, uint64_t, int)> recurseF_01_loop2;
recurseF_01_loop2 = [&v, &ret, &recurseF_01_loop2](uint64_t offset, uint64_t index_offset, int depth) -> void {
    // num_strs = powminus(2+3*depth)
    uint64_t num_strs = uint64_t(1) << (2 * length - (3 * depth) - 2);
    if (depth < STOP_AT) {
        recurseF_01_loop2(offset, index_offset, depth + 1);
        recurseF_01_loop2(offset + num_strs * 1 / 8, index_offset + 2 * num_strs * 2 / 8, depth + 1);
        recurseF_01_loop2(offset + num_strs * 2 / 8, index_offset + 2 * num_strs / 8, depth + 1);
        recurseF_01_loop2(offset + num_strs * 3 / 8, index_offset + 2 * num_strs * 3 / 8, depth + 1);
        recurseF_01_loop2(offset + num_strs * 4 / 8, index_offset + 2 * num_strs * 7 / 8, depth + 1);
        recurseF_01_loop2(offset + num_strs * 5 / 8, index_offset + 2 * num_strs * 5 / 8, depth + 1);
        recurseF_01_loop2(offset + num_strs * 6 / 8, index_offset + 2 * num_strs * 6 / 8, depth + 1);
        recurseF_01_loop2(offset + num_strs * 7 / 8, index_offset + 2 * num_strs * 4 / 8, depth + 1);
    } else {
        uint64_t start_i = index_offset;
        uint64_t end_i = start_i + 2 * num_strs;
        // load in arr[start_i] thru arr[end_i]
        // copy_mem(ret, v, num_strs * sizeof(double));

        uint64_t start = powminus1 + offset;
        uint64_t end = start + num_strs;
        // F_01_loop1(start, end, v, ret);
        printf("Str start, end: %i, %i.  Index start, end: %i, %i\n", start, end, start_i, end_i);
        return;
    }
};
recurseF_01_loop2(0, 0, 0);
cout << endl;

std::function<uint64_t(uint64_t, uint64_t, uint64_t[2], uint64_t, int, int, bool)> recurseF_01_loop2_3part;
recurseF_01_loop2_3part = [&recurseF_01_loop2_3part](uint64_t target_str, uint64_t target_num_strs, uint64_t offsets[2],
                                                     uint64_t index_offset, int depth, int stage,
                                                     bool reverse) -> uint64_t {
    // num_strs = powminus(2+depth)
    uint64_t num_strs = uint64_t(1) << (2 * (length)-2 - depth);
    if (num_strs > target_num_strs) {
        switch (stage) {
            case 1: {
                if (target_str < offsets[0] + num_strs * 4 / 8) {
                    uint64_t offsets_next[2] = {offsets[0], 0};
                    return recurseF_01_loop2_3part(target_str, target_num_strs, offsets_next, index_offset, depth + 1,
                                                   2, false);
                } else {
                    uint64_t offsets_next[2] = {offsets[0] + num_strs * 4 / 8, 0};
                    return recurseF_01_loop2_3part(target_str, target_num_strs, offsets_next,
                                                   index_offset + 2 * num_strs * 4 / 8, depth + 1, 2, true);
                }
                break;
            }
            case 2: {
                if (((target_str >= offsets[0] + num_strs * 2 / 4) && (target_str < offsets[0] + num_strs * 3 / 4)) ||
                    target_str < offsets[0] + num_strs * 1 / 4) {
                    uint64_t offsets_next[2] = {offsets[0], offsets[0] + num_strs * 2 / 4};
                    if (reverse) {
                        return recurseF_01_loop2_3part(target_str, target_num_strs, offsets_next,
                                                       index_offset + 2 * num_strs * 2 / 4, depth + 1, 3, true);
                    } else {
                        return recurseF_01_loop2_3part(target_str, target_num_strs, offsets_next, index_offset,
                                                       depth + 1, 3, false);
                    }
                } else {
                    uint64_t offsets_next[2] = {offsets[0] + num_strs * 1 / 4, offsets[0] + num_strs * 3 / 4};
                    if (reverse) {
                        return recurseF_01_loop2_3part(target_str, target_num_strs, offsets_next, index_offset,
                                                       depth + 1, 3, true);
                    } else {
                        return recurseF_01_loop2_3part(target_str, target_num_strs, offsets_next,
                                                       index_offset + 2 * num_strs * 2 / 4, depth + 1, 3, false);
                    }
                }
                break;
            }
            case 3: {
                if (target_str < offsets[1]) {
                    uint64_t offsets_next[2] = {offsets[0], 0};
                    if (reverse) {
                        return recurseF_01_loop2_3part(target_str, target_num_strs, offsets_next,
                                                       index_offset + 2 * num_strs * 1 / 2, depth + 1, 1, false);
                    } else {
                        return recurseF_01_loop2_3part(target_str, target_num_strs, offsets_next, index_offset,
                                                       depth + 1, 1, false);
                    }
                } else {
                    uint64_t offsets_next[2] = {offsets[1], 0};
                    if (reverse) {
                        return recurseF_01_loop2_3part(target_str, target_num_strs, offsets_next, index_offset,
                                                       depth + 1, 1, false);
                    } else {
                        return recurseF_01_loop2_3part(target_str, target_num_strs, offsets_next,
                                                       index_offset + 2 * num_strs * 1 / 2, depth + 1, 1, false);
                    }
                }
                break;
            }
            default: {
                return 0;
            }
        }

    } else {
        uint64_t start_i = index_offset;
        uint64_t end_i = start_i + 2 * num_strs;
        // load in arr[start_i] thru arr[end_i]
        // printf("[REV] Str start, end: %i, %i.  Index start, end: %i, %i\n", target_str + powminus1,
        //        target_str + num_strs + powminus1, start_i, end_i);
        cout << "IN RECURSE: " << index_offset << " " << stage << endl;
        return start_i;
    }
};

// uint64_t reverse_start_i =
//                 recurseF_01_loop2_3part(powminus1 - end, num_strs / 2, 0, default_arr, 0, 1, false).first;
//             cout << "writing f01 about to start " << num_strs << " " << start << " " << end << " "
//                  << reverse_start_i << endl;
//             std::flush(cout);
//             copy_mem(readmap + 2 * num_strs, filemapsv1, num_strs, reverse_start_i, true);
//             cout << "writing f01 2 about to start" << endl;
//             std::flush(cout);
//             // THE FOURTH ARGUMENT (reverse_index) IS PROBABLY WRONG
//             F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 2, readmap, writemap);
//             cout << "writing f01 3 about to start" << endl;
//             std::flush(cout);
//             copy_mem(writemap, filemapsv2, num_strs / 2, start, false);
//             // printf("Str start, end: %i, %i.  Index start, end: %i, %i\n", start, end, start_i, end_i);
//             cout << "writing f01 p1 done" << endl;
//             std::flush(cout);

//             // DO SECOND HALF
//             start = powminus2 + offsets[1];
//             end = start + num_strs / 2;
//             // feed the recurse searcher so it recurses down until it finds target_str, and returns the indexes to
//             // load in for that
//             reverse_start_i =
//                 recurseF_01_loop2_3part(powminus1 - end, num_strs / 2, 0, default_arr, 0, 1, false).second;
//             copy_mem(readmap + 3 * num_strs, filemapsv1, num_strs, reverse_start_i, true);
//             F_01_LOOP(start, end, start_i, reverse_start_i - num_strs * 3, readmap, writemap);
//             copy_mem(writemap, filemapsv2, num_strs / 2, start, false);
//             cout << "writing f01 (pt 2) " << num_strs << " " << start << " " << end << " " << reverse_start_i
//                  << endl;
//             std::flush(cout);

//             // printf("Str start, end: %i, %i.  Index start, end: %i, %i\n", start, end, start_i, end_i);