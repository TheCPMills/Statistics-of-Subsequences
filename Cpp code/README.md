Constructs upper triangle of binary LCS matrix and writes it to a file. Change BITSTR_LEN at top to change the lengths of the bit string pairs.

This does so uses fast bitset C++ structures, so it runs very quickly. That is its main purposes.

The code and comments are very messy at the moment. You should really only be using this to quickly generate large LCS matrices, the other point of this file (which most of the comments pertain to) is now obsolete.