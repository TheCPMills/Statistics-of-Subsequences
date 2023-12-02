## Just a collection of TODOs
Moving some of the longer/misc TODOS out of code comments into an external document so that the code is cleaner.


### High Priority
// TODO: see if you can take advantage of complement symmetry (complement operator ~)
// TODO: time benchmarking for each part of FeasibleTriplet/F function


### Lower Priority
// make sure to pass by reference, not by value
// https://eigen.tuxfamily.org/dox/group__TopicPassingByValue.html
// TODO: look into using Ref's (or smart pointers?)

// Seems like they actually recommend using dynamic arrays for large sizes
// even if you know the size at compile time.
// https://eigen.tuxfamily.org/dox/group__TutorialMatrixClass.html

// TODO: figure out if this matters
// https://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html

// See https://eigen.tuxfamily.org/dox/TopicPreprocessorDirectives.html for flags to consider
// E.g., EIGEN_NO_DEBUG (currently set at top)
// May be possible to get around stack size issues with this. Unsure if good idea?

// TODO: see if -m64 compile flag matters
// TODO: check if can use https://eigen.tuxfamily.org/dox/TopicUsingIntelMKL.html

// in F_01
// TODO: see if combining into single loop, with second loop as str_new = str + powminus2 is faster
// TODO: maybe convert into bitset

