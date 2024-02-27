# C++ Implementation of the Feasible Triplet Algorithm

This code is an implementation of the Feasible Triplet Algorithm as defined in the paper "Improved Lower Bounds for the Chvátal-Sankoff
Constants", written by George T. Heineman, Chase Miller, Daniel Reichman, Andrew Salls, Gábor Sárközy, and Duncan Soiffer. We include a brief summary below, but for a further explanation of the code, please read this paper.

The Chvatal-Sankoff constants $\gamma_{\sigma, d}$ are a set of constants such that 

$\gamma_{\sigma, d} = \lim\limits_{n \to \infty} \frac{E[LCS_{\sigma, d}]}{n}$

where $LCS_{\sigma, d}$ is the $d$-LCS for $d$ random strings, where each string is of length $n$ and each character in the string is one of $\sigma$ possible characters, chosen with equal chances for each character randomly. Essentially, this constant describes the expected length of the LCS when compared to the original strings as long as the strings are long enough.

The goal of this code is to calculate a lower bound on the constants, since their exact value is currently unknown.

# How to run
All code necessary should be included in this repository. First, download the repository. Open ParallelMulti.cpp and adjust the parameters at the top of the file. Keep in mind that this algorithm is highly exponential, creating and modifying multiple $alphabet\_size^{string\_count*length}$ vectors of doubles. Be careful not to set alphabet_size, string_count, or length too large.

To compile the program, open terminal where ParallelMulti.cpp is saved, and run

`g++ -O3 -pthread -I EIGEN_PATH ./ParallelMulti.cpp -o ParallelMulti`

where EIGEN_PATH is the path to the Eigen C++ library, version 3.4.0. If you cloned this repository off GitHub, the path should be ./eigen-3.4.0/

This compilation command assumes you have g++ installed, but a slightly modified command should work with gcc or any other standard C++ compiler.

This should have created the file ParallelMulti in the same folder where ParallelMulti.cpp is saved. With the terminal still in this folder, run

`./ParallelMulti`

to run the algorithm.

# Further code explanation
Below is an explanation of some properties that are relied on in the implementation:

## Ordering of vectors
All of the vectors in the program implicity use a specific ordering. We can convert between an array of string_count strings and an index for the vectors using the functions stringsToInt and intToStrings. We now define the ordering:

Given strings with characters $a_1a_2\dots a_l$, $b_1b_2\dots b_l$, $\dots$, $\sigma_1\sigma_2\dots \sigma_l$, where $l$ = length, and $\sigma$ is the string_count-th string, we convert each character to a number between 0 (inclusive) and alphabet_size (exclusive). We then combine those numbers into the base alphabet_size number

$(a_1b_1\dots \sigma_1)(a_2b_2\dots \sigma_2)\dots(a_lb_l\dots \sigma_l)$

Finally, this number is converted into a base-10 number that can be stored in C++. The resulting integer is the index that is associated with the strings $a, b, \dots, \sigma$.

## F_b_step, F_b_equals_1, and b
One important part of the Feasible Triplet algorithm is the $b$ vector. For every index in $b$, $b$ has a value of 1 if the strings associated with that index all start with the same character, and 0 otherwise.

Under the ordering we defined, the strings that all start with the same characters appear contiguously in our vectors when we increment through the indices in order. Consider the base alphabet-size number format defined above. When we increment the index, this is equivalent to adding one to this number in base alphabet_size. The strings start with the same character if the first string_count characters in the number are all the same. One of these numbers only changes every $alphabet\_size^{string\_count*(length - 1)}$ increments, which we call F_b_step.

The first F_b_step indices all start with 0. To determine how many steps must be taken to reach the next range of indices that all start with 1 (in general to go from all indices starting with $x$ to starting with $x + 1$), we observe that the number of necessary steps can be modelled by a finite geometric series with $d$ terms. We can then use the formula for a finite geometric series to know the exact number of steps necessary, which we call F_b_equals_1.

These constants get used in the line (index / F_b_step) % F_b_equals_1 == 0

index / F_b_step determines which step (starting at 0) index is located in, and then % F_b_equals_1 == 0 checks that the step is a multiple of the number of steps necessary to be in the range of indices that all start with the same character.

# Credits
This code is based on the definition of the Feasible Triplet algorithm developed by Kiwi and Soto. See their paper at https://www.cambridge.org/core/journals/combinatorics-probability-and-computing/article/abs/on-a-speculated-relation-between-chvatalsankoff-constants-of-several-sequences/7982322390D3236DC7BC96E42855768A

Learn more about Eigen at https://eigen.tuxfamily.org/index.php?title=Main_Page