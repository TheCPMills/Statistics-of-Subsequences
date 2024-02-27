# Supporting document for Parallel.cpp
To set your desired parameters, see the USER PARAMETERS section at the top of Parallel.cpp.

**NOTE: Parallel.cpp consumes 2^(2*length -1) * 4 * 2 = 4^(length+1) bytes of memory. If you do not have enough RAM for the length parameter you set, it (being the program or possibly your entire system) will probably crash/run extremely slowly. Always verify you have enough RAM before running!**

Parallel.cpp does not directly follow the explanation in the paper. Namely, a few optimizations are made so that it runs faster. Note that Parallel.cpp does *not* implement any form of external memory reading/writing. That code is in a separate file. 

Most prominently, L_01 and L_10 have been combined into one loop. This way, elementwise maximums can be done locally instead of separately in F, which saves time. Additionally, given a string in L_01's range, we make the observation that the accesses for that element are related to the accesses for an element in L_10 (namely, element+powminus2) and use that fact to reuse some computations.

It is also noted how, for L_00, we can use the symmetric transformations described in Section 3.4.1 to our advantage. Since L_00 iterates only from [0, ..., 2^(2ℓ−2)), the first bit of a and b will always be 0. Thus, for a given x, the procedure always accesses the values at 4x, 4x + 1, 4x + 2, and 4x + 3. For x ≥ 2^(2ℓ−3), it will access pairs ≥ 2^(2ℓ−1), which fall outside the vector. Instead of straightforwardly transforming the pairs back to their symmetric position within the vector using Equation 3, we can instead observe that all that happens when doing this transformation for the second half of F_00 is that the elements it accesses are reversed. Thus, the sequence of values F_00 outputs is symmetric about its center (self-symmetric). So, instead of doing this symmetric transformation with Equation 3 and recomputing the value, we can instead just set v[2^(2ℓ−2)-1 - x] = v[x].

Additionally, as mentioned, instead of storing three vectors and performing a recurrence on them, we need only store two. The bound calculation also gets adjusted because of this.
Instead of calculating an R and E, in the binary case, we need only calculate the minimum elementwise change between the two vectors in the recurrence. The bound is then calculated as (2*min_change)/(1+min_change), as described by Lueker.