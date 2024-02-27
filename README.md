## C++ Implementation of the Binary Kiwi-Soto/Lueker Algorithm
The code for the implementation of the Binary Case Kiwi-Soto/Lueker algorithm are held in this branch.
See their papers here:

(Kiwi and Soto) https://www.cambridge.org/core/journals/combinatorics-probability-and-computing/article/abs/on-a-speculated-relation-between-chvatalsankoff-constants-of-several-sequences/7982322390D3236DC7BC96E42855768A

(Lueker) https://dl.acm.org/doi/10.1145/1516512.1516519


As of Jan 29, 2024, the up-to-date versions are:
1. Parallel.cpp contains a parallelized version that works only off of RAM. It is by far the fastest one, but does not work once arrays become too large to fit in RAM. For additional explanations of Parallel.cpp, see its supporting markdown document.
2. BetterMemNew.cpp contains an in-progress version where vectors are written/read to/from external memory in sequential chunks. As the length of the strings increase, it becomes impossible for the vectors to fit in RAM. As such, writing to external memory becomes necessary for calculating high bounds. Doing this in sequential order (which is much faster than non-sequentially) is non-trivial, however. This version is in progress, but working. It currently has no guardrails against incorrect parameters, so be careful to use sensible values and make sure sizes and numbers of threads are powers of 2. It will only work on Linux machines.
3. KiwiSotoAlgorithm.py contains a *much* slower version of the algorithm, implemented in python. This is the easiest version to understand and write in, so we occasionally use this for testing out new ideas (e.g. implementing new symmetries). This version is not recommended for use.

NOTE: the contents of eigen-3.4.0 are NOT written by us. This is the location of the Eigen 3 library (which has been imported directly into the repository, as recommended). Eigen 3 is licensed under MPL2. See Eigen 3 documentation [here](https://eigen.tuxfamily.org/index) and source code [here](https://gitlab.com/libeigen/eigen).


We have not yet created a Makefile. To run, make sure you have a C++ compiler installed. Then, clone this repository, navigate to this directory, and run `g++ -O3 -I -pthread ./eigen-3.4.0/ ./Parallel.cpp -o Parallel` and then run `./Parallel` (or `./Parallel.exe` on Windows).
**NOTE: Versions that read and write from external memory work only on Linux systems.**
