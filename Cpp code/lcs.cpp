#include <math.h>

#include <bitset>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
using namespace std;
// using namespace std::chrono;

#define BITSTR_LEN 15
#define B_L BITSTR_LEN
#define BITSTR_LEN_POW (1 << BITSTR_LEN)
#define B_L_P BITSTR_LEN_POW
#define BITSTR_LEN_TIMES2 BITSTR_LEN * 2
#define HASH_KEY_SIZE (8 + (BITSTR_LEN * 2))

// TODO: benchmark struct of 2 vecs vs 2 fixed size arrays with wasted space
// or no struct, but one vec (or array?) and using pointer arithmetic
// if assignment via = copies for vector, then what I do rn is extremely inefficient
struct colRow {
    std::vector<uint8_t> col;
    std::vector<uint8_t> row;
};

// Careful, 28? bits is the max len we can represent with this (ulong is 64 bits)
struct Comparer {
    bool operator()(const bitset<HASH_KEY_SIZE>& b1, const bitset<HASH_KEY_SIZE>& b2) const {
        return b1.to_ulong() < b2.to_ulong();
    }
};

// static map<bitset<BITSTR_LEN * 4>, tuple<uint8_t[], uint8_t[]>> hashMap;
static std::unordered_map<bitset<HASH_KEY_SIZE>, colRow> hashMap;

// Generate all bit strings (as an array of bitsets) of length BITSTR_LEN (there are 2^BITSTR_LEN of them)
// TODO: BENCHMARK DOING THIS INITIALLY VS ONLY WHEN INSERTING/ACCESSING
// bitset<BITSTR_LEN * 2>* genBitstrs() {
//     static bitset<BITSTR_LEN * 2> bitstrs[BITSTR_LEN_POW];
//     const int n = BITSTR_LEN;
//     for (int i = 0; i < (1 << n); i++) {
//         bitset<BITSTR_LEN * 2> b(i);
//         bitset<BITSTR_LEN * 2> leftpad(BITSTR_LEN_POW - 1);
//         b = (leftpad << BITSTR_LEN) | b;
//         bitstrs[i] = b;
//     }
//     return bitstrs;
// }
bitset<BITSTR_LEN>* genBitstrs() {
    static bitset<BITSTR_LEN> bitstrs[BITSTR_LEN_POW];
    const int n = BITSTR_LEN;
    for (int i = 0; i < (1 << n); i++) {
        bitstrs[i] = bitset<BITSTR_LEN>(i);
    }
    return bitstrs;
}

// TODO: investigate small-alphabet speedups (e.g. Method of Four Russians), for naive AND for grid algo
// TODO: turn this into a block-lcs finder with a better dynamic programming algorithm which is way more optimized
// further, it could probably be broken into threads
uint8_t findLCS(bitset<BITSTR_LEN> S1, bitset<BITSTR_LEN> S2) {
    const int m2 = BITSTR_LEN;
    const int n2 = BITSTR_LEN;
    uint8_t LCS_table[m2 + 1][n2 + 1];  // Be careful, this means maximum representable bitstr_len is 127.

    // Building the mtrix in bottom-up way
    // TODO: benchmark == vs XOR
    // TODO: is this matrix symmetric? if so, need only compute half of it I think? (and init matrix to 0s)
    for (int i = 0; i <= m2; i++) {
        for (int j = 0; j <= n2; j++) {
            if (i == 0 || j == 0)  // TODO: this if statement could probably be optimized out, but a shift to block-lcs
                                   // would make this optimization unnecessary I think
                LCS_table[i][j] = 0;
            else if (S1[i - 1] == S2[j - 1])
                LCS_table[i][j] = LCS_table[i - 1][j - 1] + 1;
            else
                LCS_table[i][j] = max(LCS_table[i - 1][j], LCS_table[i][j - 1]);
        }
    }
    return LCS_table[m2][n2];
}

// TODO: also check to see if squares at iteration i+1 are related to squares at iteration i

// Perhaps a block-lcs finder would work as follows:
// (Compute bitstr matrix for bitstrs of length 1)
// Starting at length 2, find the LCS matrix for all bitstr pairs

// intialize hashmap for every string pair (/2, avoiding dupes, can optimize to /4 and better)
// or maybe make it an array, indexed by bitstring val (but u have to do bitshfting since strings grow to the right
// since least significant digit is to the right, wait nvm array doesnt work I think)
// each entry is a (strlen+1)x(strlen+1) 2d array of 0s then, you
// gradually fill up the matrices
// maybe do like this:
// you've initialized everything
// (sort bitstrings in particular way?)
// (maybe have a custom hashing function, that hashes with 3 inputs (2strs and current len?), hashing function may be
// key)
// also intialize the 4 (3) 1-bit matrices
// for i = 2 through strlen
//      for j = 0 through (2^i) -1
//          for k = 0 through (2^i) -1
//              for every j, k pair (considering only leftmost i digits): update matrix
//
// updating matrix is as follows:
// find (MUST BE CONSTANT TIME) the matrix corresponding to leftmost i-1 digits
// calculate the update: need only calculate right-most column and bottom-most row
// potential calculation optimization: half the time, you know you're just adding 1 to every row/col (since your new
// pairs will have both 1s or both 0s at the end)

// ooh wait a minute, I think you only care about the last row and column of each matrix
// potentially very big optimization for space

// TODO: investigate uint_fast8_t
//  this is probabily inefficient. optimize.
//  (+8 could really be instead computed as (rounded up) log2 of BITSTR_LEN)
//  i think log2 can actually be done using bitwise ops online, so can be compile time
// TODO: benchmark bitshifting left, then bitshifting right vs ANDing with bitstring of 1s shifted right
// CAREFUL: this ASSUMES your bitstrings you pass have ONE MORE BIT than the bitstrings you are accessing
// use noShift for printing to a file, or access outside the main algorithm
const colRow& accessHashMap(const bitset<BITSTR_LEN>& s1, const bitset<BITSTR_LEN>& s2, uint8_t numBits) {
    bitset<HASH_KEY_SIZE> leftpad = bitset<HASH_KEY_SIZE>(numBits) << (BITSTR_LEN * 2);
    // bitset<HASH_KEY_SIZE> b1 =
    //     bitset<HASH_KEY_SIZE>(((s1 << (BITSTR_LEN - numBits)) >> (BITSTR_LEN - numBits)).to_ulong()) << (BITSTR_LEN);
    // bitset<HASH_KEY_SIZE> b2(((s2 << (BITSTR_LEN - numBits)) >> ((BITSTR_LEN - numBits)))
    //                              .to_ulong());  // NEED TO FIGURE OUT IF LEFT OR RIGHT SHIFT FOR b1, b2
    bitset<HASH_KEY_SIZE> b1 = bitset<HASH_KEY_SIZE>((s1 >> 1).to_ulong()) << (BITSTR_LEN);
    bitset<HASH_KEY_SIZE> b2 = bitset<HASH_KEY_SIZE>((s2 >> 1).to_ulong());
    // cout << (leftpad | b1 | b2) << "ACS" << endl << flush;
    return hashMap[leftpad | b1 | b2];
}

//"effective modern c++" as a resource

// this is probably inefficient. optimize.
// in two ways: might not be passing pointer to col, row arrays (idk), and the actual putting in hashmap
// also might not be passing pointer to the bitsets either
void insertIntoHashMap(const bitset<BITSTR_LEN>& s1, const bitset<BITSTR_LEN>& s2, uint8_t numBits,
                       const colRow& colrow) {
    bitset<HASH_KEY_SIZE> leftpad = bitset<HASH_KEY_SIZE>(numBits) << (BITSTR_LEN * 2);
    bitset<HASH_KEY_SIZE> b1 = bitset<HASH_KEY_SIZE>(s1.to_ulong()) << (BITSTR_LEN);
    bitset<HASH_KEY_SIZE> b2(s2.to_ulong());
    // cout << (leftpad | b1 | b2) << "INS" << endl << flush;
    hashMap[leftpad | b1 | b2] = colrow;
    // TODO: to do the halving and quartering of the matrix optimizations, I think you can do it in here:
    //   (along with shortening the loops so as not to repeat, ofc)
    //   swap b1 and b2, and insert colrow with row and col swapped
    //   perform NOT on b1 and b2, insert with colrow unchanged
    //   perform NOT on b1 and b2, swap them, and insert colrow with row and col swapped
}

void initHashMap() {
    // TODO: ensure shifting just puts 0s and doesn't copy 1s
    // TODO: benchmark dynamic bitset vs static bitset using BITSTR_LEN padding 1 bits at the start which are shifted
    // to keep track of length (so that e.g. 000 (padded: 111000) when considering leftmost 2 bits (->100000) != 000
    // (->000000) when considering all 3 bits) taking two bitstrs of len*2, shifting each, and then concatenating them

    // TODO: instead of tuple, do array of size 2 of uint8_t arrays?
    // or just struct ig
    // map<bitset<BITSTR_LEN * 4>, tuple<uint8_t[], uint8_t[]>> hashMap;
    std::vector<uint8_t> col00;
    col00.reserve(1);
    col00.push_back(1);
    std::vector<uint8_t> row00;
    row00.reserve(1);
    row00.push_back(1);
    std::vector<uint8_t> col01;
    col01.reserve(1);
    col01.push_back(0);
    std::vector<uint8_t> row01;
    row01.reserve(1);
    row01.push_back(0);
    std::vector<uint8_t> col10;
    col10.reserve(1);
    col10.push_back(0);
    std::vector<uint8_t> row10;
    row10.reserve(1);
    row10.push_back(0);
    std::vector<uint8_t> col11;
    col11.reserve(1);
    col11.push_back(1);
    std::vector<uint8_t> row11;
    row11.reserve(1);
    row11.push_back(1);

    bitset<BITSTR_LEN> b0(0);
    bitset<BITSTR_LEN> b1(1);
    insertIntoHashMap(b0, b0, 1, colRow{col00, row00});
    insertIntoHashMap(b0, b1, 1, colRow{col01, row01});
    insertIntoHashMap(b1, b0, 1, colRow{col10, row10});  // eventually won't be necessary?
    insertIntoHashMap(b1, b1, 1, colRow{col11, row11});  // might also eventually become unnecessary
    // map<bitset<BITSTR_LEN * 4>, colRow> hashMap;
}

// I wonder if there's additional optimizations you can make, knowing that e.g. lcs(1101, 0101) = lcs(0010, 1010)
// ^(wait that might be why u can cut the half matrix into a quarter matrix)
// or optimizations because more broadly you don't care about any individual lcs, just the total

// TODO: might be able to make big O(n) optimization by only caring about the longest common subsequences (with the most
// amount of the string leftover) of previous subsequences that ended in 1 or in 0. idea is that should it really be
// O(2n) to compute new longest common subsequence when given matrix for (numBits -1)? Need to think about this.
// (you can only increase subseq len by 1 by adding a bit to end of both, which is why this might work)

// new data structure idea: each bitstr pair is hashed and carries only one row and one column (right-most row,
// bottom-most column)
// when you need to calculate something new, you just hash on the leftmost k bits of the strings in the pair to get
// the relevant row and column should probably use map() or some optimized way of operating on everything at once
// (esp for just adding 1) could potentially delete (free mem) everything of len < i -2 once you reach i
void calcColRow(const bitset<BITSTR_LEN>& s1, const bitset<BITSTR_LEN>& s2, uint8_t numBits) {
    // should probably do n = numBits -1, and use that instead of numBits to avoid having to subtract 1 each time
    // CAREFUL ABOUT RUNNING INTO ISSUES WITH BITSET IMPLICITLY CHANGING SIZE, IDK IF A PROBLEM BUT MAY BE
    // take only leftmost numBits
    //

    // retrieve hashmap[(s1(LeftBits-1), s2(LeftBits-1))]
    // cout << endl << s1 << ", " << s2 << " with numBits: " << unsigned(numBits) << endl << flush;

    auto [prevCol, prevRow] = accessHashMap(s1, s2, numBits - 1);

    // TODO: DECIDE BETWEEN VECTOR OR DYNAMICALLY ALLOCATED ARRAY OR SMARTPOINTER
    // newCol
    // int* L = new int[mid];
    // delete[] L; //DONT FORGET TO DELETE?
    // TODO: BENCHMARK BETWEEN THESE TWO TYPES OF VEC
    // https://stackoverflow.com/questions/11134497/constant-sized-vector
    colRow colrow;
    // std::vector<uint8_t> col;
    colrow.col.reserve(numBits);
    // std::vector<uint8_t> row;
    colrow.row.reserve(numBits);

    // FIXME: YOU WERE WORKING ON
    //  you had some orders reversed. You need to determine:
    //  should strings be parsed from left to right, or right to left
    //  should s1 be the string corresponding to the row in the table, or to the column
    //  Depending on those decisions, figure out how each part should be written.
    //  s1 probably col, s2 probably row
    //       to update col: accessed s1 element stays fixed, accessed s2 element changes
    //       to update row: accessed s2 element stays fixed, accessed s1 element changes
    //
    // if I parse strings from left to right (s[strlen] -> s[0])
    //      fixed s1 el is s[numBits -1], s2 is (numBits -1 ) -> 0 (wait nvm I think backwards)
    //      fixed s2 el is s[numBits -1], s1 is (numBits -1) -> 0 (wait nvm I think backwards)
    //
    // when query hashmap, need to query:
    //  shift bits left to remove first bit (that we consider), then shift back
    //  wait no shouldn't it be shift right 1 and that's it

    // uint8_t* col = new uint8_t[numBits];
    // uint8_t* row = new uint8_t[numBits];
    // need to figure out if there's a row of 0s at the top and left
    // need to fiugre out if loops to numBits, numBits-1, or numBits-2, and if starts at i=1
    // array is numBits long, but we calculate first and last element outside loop
    // need to figure out if this is backwards
    if (s1[0] == s2[numBits - 1])
        colrow.col.push_back(1);  // 0 + 1
    else
        colrow.col.push_back(prevCol[0]);  // max(prevCol[0], 0)
    if (s1[numBits - 1] == s2[0])
        colrow.row.push_back(1);  // 0 + 1
    else
        colrow.row.push_back(prevRow[0]);  // max(0, prevRow[0])

    // NOTE: everything is basically being done in reverse
    for (int i = numBits - 2; i > 0; --i) {
        // slightly differently outside of loop benchmark: doing these in the same for loop, vs doing these in a
        // separate for (there may be memory/os optimization differences that could be significant enough to matter)
        if (s1[0] == s2[i]) {
            // needs to be numBits - i
            // possible optimization: hav cols/rows be reversed so that subtracting isnt necessary
            // only better if can still use push_back
            colrow.col.push_back(prevCol[numBits - 1 - i - 1] + 1);  // col[i] =
        } else
            colrow.col.push_back(max(prevCol[numBits - 1 - i], colrow.col[numBits - 1 - i - 1]));  // col[i] =
        if (s1[i] == s2[0])
            colrow.row.push_back(prevRow[numBits - 1 - i - 1] + 1);  // row[i] =
        else
            colrow.row.push_back(max(prevRow[numBits - 1 - i], colrow.row[numBits - 1 - i - 1]));  // row[i] =
    }

    // fill in the last entry for row and col (it depends on both row and col, so must be done separately from
    // above)
    if (s1[0] == s2[0]) {
        colrow.row.push_back(prevRow[(numBits - 1) - 1] + 1);  // row[numBits -1] =
        colrow.col.push_back(colrow.row[numBits - 1]);         // col[numBits -1] =
    } else {
        colrow.row.push_back(max(colrow.row[(numBits - 1) - 1], colrow.col[(numBits - 1) - 1]));  // row[numBits -1] =
        colrow.col.push_back(colrow.row[numBits - 1]);                                            // col[numBits -1] =
    }
    // cout << unsigned(col[numBits - 1]) << ", " << unsigned(row[numBits - 1]) << endl << flush;
    // cout << unsigned(prevCol[numBits - 2]) << ", " << unsigned(prevRow[numBits - 2]) << endl << flush;
    // if (numBits == 4) {
    //     for (int i = 0; i < numBits; i++) {
    //         cout << unsigned(col[i]);
    //     }
    //     cout << ", ";
    //     for (int i = 0; i < numBits; i++) {
    //         cout << unsigned(row[i]);
    //     }
    //     cout << endl;
    //     for (int i = 0; i < numBits - 1; i++) {
    //         cout << unsigned(prevCol[i]);
    //     }
    //     cout << ", ";
    //     for (int i = 0; i < numBits - 1; i++) {
    //         cout << unsigned(prevRow[i]);
    //     }
    //     cout << endl;
    // }
    // FIXME: do NOT instantiate a new colRow here, where it has to copy the values. Instead, you should be
    // instantiating the colRow at the beginning and filling in its values as you go.

    // FIXME: THERE'S A TON OF PLACES WHERE YOU PASS VECTOR (OR ARRAY OR STRUCT) INSTEAD OF A REFERENCE TO IT, BUT
    // REFERENCE IS WAY FASTER https://godbolt.org/z/rfajEh5o4
    // Or maybe wrap things in std::move()
    // ALSO, I THINK THIS SAME ISSUE MAY APPLY TO PASSING BITSETS.
    insertIntoHashMap(s1, s2, numBits, colrow);

    // calculate numBits - 1 col elements (based off col)
    // calculate numBits - 1 row elements (based off row)
    // calculate numBits'th col and row element (they are the same)
    // set the col[numBits -1] element to row[numBits -1]
}
// probably need struct containing col and row?

// TODO: investigate: does reversing both string order matter to LCS

void gridLCS(const bitset<BITSTR_LEN> bitstrs[BITSTR_LEN_POW]) {
    auto start = chrono::high_resolution_clock::now();
    // given array of every bitstr of len BITSTR_LEN

    // init hashmap (experiment with custom fast-hashes)
    // construct row and col for every pair of i-length bitstrings, starting at 2
    for (int i = 2; i <= BITSTR_LEN; i++) {
        // for every pair of i-length bitstrings
        for (int j = 0; j < (1 << i); j++) {  // TODO: can cut in half (quarter?)
            for (int k = 0; k < (1 << i); k++) {
                calcColRow(bitstrs[j], bitstrs[k], i);
            }
        }
        // cutting in half/quartering might be more complicated. I think it requires more complicated transposing right
        // here, since once you get to next level (i) you DO care about everything from previous I think. Which means
        // transposing the row/col entries for everything, idk if it's mathematically valid? Need to think about it.
        // Maybe it means swapping row and col? if it's just that simple, might be able to still do it computationally
        // efficiently.
    }
    auto stop = chrono::high_resolution_clock::now();
    cout << endl << (chrono::duration_cast<chrono::microseconds>(stop - start).count() * 1.0) / (10.0e+5) << endl;
}
// change all num++ to ++num?

// possible optimizations for above thing? https://en.wikipedia.org/wiki/Fenwick_tree

void populateMatrix(const bitset<BITSTR_LEN> bitstrs[]) {  // bitstrs len is 2^BITSTR_LEN
    auto start = chrono::high_resolution_clock::now();
    const int numstrs = BITSTR_LEN_POW;  // 2^BITSTR_LEN
    static uint8_t a[numstrs][numstrs];  // static to allocate on heap, otherwise we run into stack being too small
    for (int i = 0; i < numstrs; i++) {  // only fill in the upper triangle (TODO: can optimize more!)
        for (int j = 0; j <= i; j++) {
            a[i][j] = findLCS(bitstrs[i], bitstrs[j]);
        }
    }
    auto stop = chrono::high_resolution_clock::now();
    cout << endl << (chrono::duration_cast<chrono::microseconds>(stop - start).count() * 1.0) / (10.0e+5) << endl;
    // for (int i = 0; i < BITSTR_LEN_POW; i++) {
    //     cout << endl;
    //     for (int j = 0; j <= i; j++) {
    //         cout << a[i][j] << " ";
    //     }
    //     for (int k = i + 1; k < BITSTR_LEN_POW; k++) {
    //         cout << "0 ";
    //     }
    // }

    // for (int i = 0; i < numstrs; i++) {  // only fill in the upper triangle (TODO: can optimize more!)
    //     cout << bitstrs[i] << ", ";
    // }
    // cout << endl;
    // for (int i = 0; i < numstrs; i++) {  // only fill in the upper triangle (TODO: can optimize more!)
    //     cout << bitstrs[i] << endl;
    // }

    // This file writing is wondrously inefficient, but I am guessing that computation time >>> file writing time
    // so it doensn't really matter. However, if space becomes an issue, this can be improved a very large amount
    // by just writing uint8's to a file, and when parsing it, knowing to read one byte at a time
    cout << "Writing to file...\n";
    ofstream OutputFile("LCSgrid" + std::to_string(BITSTR_LEN) + ".txt");
    // Write to the file
    // OutputFile << BITSTR_LEN << endl;
    for (int i = 0; i < BITSTR_LEN_POW; i++) {
        for (int j = 0; j <= i; j++) {
            OutputFile << to_string(a[i][j]);
            if (j < BITSTR_LEN_POW - 1) {
                OutputFile << ",";
            }
        }
        for (int k = i + 1; k < BITSTR_LEN_POW; k++) {
            OutputFile << 0;
            if (k < BITSTR_LEN_POW - 1) {
                OutputFile << ",";
            }
        }
        OutputFile << endl;
    }

    // Close the file
    OutputFile.close();
    cout << "File writing complete!\n";
}

// if you NOT both bitstrs you get same bitstr
// is there a pattern if you reverse both bitstrs?
//  bitset<BITSTR_LEN> reverseBS(std::bitset<BITSTR_LEN> b) {
//      bitset<BITSTR_LEN> a;
//      for (std::size_t i = 0; i < BITSTR_LEN; i++) {
//          a[i] = b[BITSTR_LEN - (i + 1)];
//      }
//      return a;
//  }

colRow accessHashMapNoShift(bitset<BITSTR_LEN> s1, bitset<BITSTR_LEN> s2, uint8_t numBits) {
    bitset<HASH_KEY_SIZE> leftpad = bitset<HASH_KEY_SIZE>(numBits) << (BITSTR_LEN * 2);
    // bitset<HASH_KEY_SIZE> b1 =
    //     bitset<HASH_KEY_SIZE>(((s1 << (BITSTR_LEN - numBits)) >> (BITSTR_LEN - numBits)).to_ulong()) << (BITSTR_LEN);
    // bitset<HASH_KEY_SIZE> b2(((s2 << (BITSTR_LEN - numBits)) >> ((BITSTR_LEN - numBits)))
    //                              .to_ulong());  // NEED TO FIGURE OUT IF LEFT OR RIGHT SHIFT FOR b1, b2
    bitset<HASH_KEY_SIZE> b1 = bitset<HASH_KEY_SIZE>(s1.to_ulong()) << (BITSTR_LEN);
    bitset<HASH_KEY_SIZE> b2 = bitset<HASH_KEY_SIZE>(s2.to_ulong());
    // cout << (leftpad | b1 | b2) << "ACS" << endl << flush;
    return hashMap[leftpad | b1 | b2];
}
void readHashMapToFile(const bitset<BITSTR_LEN> bitstrs[]) {
    cout << "Writing to file (hashmap)...\n";
    ofstream OutputFile("LCSgrid" + std::to_string(BITSTR_LEN) + "GRID.txt");
    // Write to the file
    // OutputFile << BITSTR_LEN << endl;
    for (int i = 0; i < BITSTR_LEN_POW; i++) {
        for (int j = 0; j < BITSTR_LEN_POW; j++) {
            OutputFile << to_string(accessHashMapNoShift(bitstrs[i], bitstrs[j], BITSTR_LEN).col[BITSTR_LEN - 1]);
            // cout << bitstrs[i] << ", " << bitstrs[j] << flush << endl;
            if (j < BITSTR_LEN_POW - 1) {
                OutputFile << ",";
            }
        }
        // for (int k = i + 1; k < BITSTR_LEN_POW; k++) {
        //     OutputFile << 0;
        //     if (k < BITSTR_LEN_POW - 1) {
        //         OutputFile << ",";
        //     }
        // }
        OutputFile << endl;
    }

    // Close the file
    OutputFile.close();
    cout << "File writing complete (hashmap)!\n";
}

int main() {
    const bitset<BITSTR_LEN>* bitstrs = genBitstrs();  //
    // for (int i = 0; i < BITSTR_LEN_POW; i++) {
    //     cout << bitstrs[i] << endl;
    // }
    cout << "About to begin populating matrix";
    populateMatrix(bitstrs);

    // Below is an asymptotically faster algorithm, but much slower to start.
    // It is not worth running.
    // initHashMap();
    // gridLCS(bitstrs);
    // readHashMapToFile(bitstrs);
}
