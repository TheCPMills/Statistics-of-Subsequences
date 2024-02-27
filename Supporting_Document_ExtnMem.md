# Supporting document for ExtnMem.cpp
To set your desired parameters, see the USER PARAMETERS section at the top of Binary-ExtnMem.cpp. Make sure to set valid parameters!

The main difference between the implementation here and how it is described in the paper is the recursion for sequential memory access. The recursions in the paper assume that L_01 and L_10 are computed independently of each other. However, this would necessitate a lot of extra file I/O to do their elementwise maximum. So instead, we define a recursion that works for both at once (and tack on L_00 for free at the end).

First, note how the recursion is defined in the paper. It means we can only split up into chunks of 4^(recurse depth). Ideally, we would like as fine grain control as possible, i.e., 2^(recurse depth).

To do so, we can somewhat fake it. Split the recursion into two cases:
```
recurse(offsets, index_offset, depth, case):
	num_strs = powminus(2 + 2*depth)
	if (depth < stop_depth):
        if case == 1: 
            offsets_next = {offsets[0], offsets[0] + num_strs * 2 / 4};
            recurse(offsets_next, index_offset, depth + 1, 2);

            offsets_next = {offsets[0] + num_strs / 4, offsets[0] + num_strs * 3 / 4}
            recurse(offsets_next, index_offset + 2 * num_strs / 2, depth + 1, 2);
            
        if case == 2: 
            offsets_next = {offsets[0], 0};
            recurse(offsets_next, index_offset, depth + 1, 1);

            offsets_next = {offsets[1], 0};
            recurseF(offsets_next, index_offset + 2 * num_strs / 2, depth + 1, 1);
```
To start the recursion, we call it with `recurse({0, 0}, 0, 0, 1)`.
What we've done here is split it up into two cases so that each time, we only have to read in one sequential block of memory, but we may have to iterate over two (possibly non-contiguous) sequential ranges of values (which the recursion stores the starts of) to do so.
This gives us finer grain control over the size of the RAM buffer we hold.

But this still only works for L_01. We want to make it work for L_01 and L_10 at the same time (see other Binary Case supporting document). To do this, we define a function that, when given a string pair (encoded as an integer), tells us what string pair it gets max'd with. Let's assume we call this function only with the first string pair of a sequential block for L_01. We can then use the returned pair as a starting point for figuring out what values need to be read in for L_10. Of course, many of the values for L_10 are read in reverse, so we need to determine if we need to read things in 'reverse' from memory (reversed copying is much slower than forward sequential copying, so we instead offset the string backward, read it into RAM sequentially in the forward direction, and access the RAM values in reverse).

Note that we are *not* guaranteed that the values we must read in for L_10 to correspond with our L_01 block lie in one sequential block. While we can define a similar recursion for L_10 that guarantees sequential memory access, it does not exactly align with L_01. However, we *are* guaranteed that we can sequentially access memory within at most two sequential blocks (intuitively, the recursion for L_10 is offset by 1 bit from L_01).

So, with all that said, here's the overview of what happens:

    Perform the recursion, exiting in either case 1 or case 2.
    Load in v[index_offset] through v[index_offset+2*num_strs] into first half of readmap.
    If exit in case 1:
        This means we need only iterate over 1 sequential range of values to access memory sequentially for L_01.
        Now check if this means if we to access one or two sequential blocks of memory for L_10.
        If one:
            Load in the block of memory for L_10 into the second half of readmap.
            Perform L_combined on the range of values for L_10 and for L_01.
            Write out the writemap.
        If two:
            Load in the first block of memory for L_10 into the third quarter of readmap.
            Perform L_combined on the first half of the range of values for L_01, and the first range of values for L_10.
            Load in the second block of memory for L_10 into the last quarter of readmap.
            Perform L_combined on the second half of the range of values for L_01, and the second range of values for L_10. 

            Write out the writemap.

    If exit in case 2:
        This means we need to iterate over 2 sequential ranges of values to access memory sequentially for L_01.
        For the first sequential range of values for L_01:
            Perform steps as in case 1 for this sequential range of values
        For the second sequential range of values for L_01:
            Perform steps as in case 1 for this sequential range of values
    
Notice here that we can also calculate F_00 without any extra reading from memory. Before all of these cases, we read in the range of values `v[index_offset] through v[index_offset+2*num_strs]`. These are the values we read in to perform the accesses for F_01, so in total they will span the entire [0,...,powminus1) range. Thus, to calculate F_00, we need not do any extra file reading. We can simply reuse the values stored in the first half of readmap (the sequential range `v[index_offset] through v[index_offset+2*num_strs]`) and use them to calculate F_00 (exploiting the symmetry described by Equation 3 to calculate two values at once, see other Binary Case supporting document for explanation).