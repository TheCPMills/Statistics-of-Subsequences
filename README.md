## C++ Implementation of the Kiwi-Soto/Lueker Algorithm
The code for the implementation of the Kiwi-Soto/Lueker are held in this branch.
See their papers here:
(Kiwi and Soto) https://www.cambridge.org/core/journals/combinatorics-probability-and-computing/article/abs/on-a-speculated-relation-between-chvatalsankoff-constants-of-several-sequences/7982322390D3236DC7BC96E42855768A
(Lueker) https://dl.acm.org/doi/10.1145/1516512.1516519


As of December 1, 2023, the up-to-date versions are:
1. StaticLengthAlgo.cpp contains a guaranteed correct but slower implementation. This is not recommended for use.
2. SVNoCopy.cpp contains a version that uses only a single vector for the recurrence as outlined by Lueker. While results match Lueker's and thus suggest empirical correctness, we are undergoing the process of ensuring the final calculation of the lower bound is correct. This is the version recommended for use.
3. ExternalMem.cpp contains an in-progress version where vectors are written/read to/from external memory. As the length of the strings increase, it becomes impossible for the vectors to fit in RAM. As such, writing to external memory becomes necessary for calculating high bounds. This version is incomplete.
4. KiwiSotoAlgorithm.py contains a *much* slower version of the algorithm, implemented in python. This is the easiest version to understand and write in, so we occasionally use this for testing out new ideas (e.g. implementing new symmetries). This version is not recommended for use.

NOTE: the contents of eigen-3.4.0 are NOT written by us. This is the location of the Eigen 3 library (which has been imported directly into the repository, as recommended). Eigen 3 is licensed under MPL2. See Eigen 3 documentation here[https://eigen.tuxfamily.org/index]php?title=Main_Page and source code here[https://gitlab.com/libeigen/eigen].


We have not yet created a make file. To run, make sure you have a c++ compiler installed. Then, navigate to this directory, and run `g++ -O3 -I ./eigen-3.4.0/ ./SVNocpy.cpp -o SVNoCopy` and then run `./SVNoCopy` (or `./SVNoCopy.exe` on Windows).
