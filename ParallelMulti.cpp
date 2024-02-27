#include <Eigen/Dense>
#include <bitset>
#include <chrono>
#include <cmath>
#include <ctime>
#include <future>
#include <iostream>
#include <string>
#include <thread>
using Eigen::ArrayXd;

// ===================================== Feasible Triplet Parameters and Constants =====================================

// ======= PARAMETERS =======

// Equivalent to l in the Feasible Triplet algorithm. Raising improves the Chvatal-Sankoff constant for the specified string_count and alphabet_size
#define length 1
// Equivalent to d in the Feasible Triplet algorithm.
#define string_count 2
//Equivalent to sigma in the Feasible Triplet algorithm
#define alphabet_size 11

/*
 * The maximum number of iterations to run the algorithm for before ending. If you do not know how many iterations are required, set this value to a high number and rely on going below TOLERANCE to stop the algorithm.
 */
#define MAX_ITERATIONS 1000

/*
 * The number of threads to use.
 */
#define NUM_THREADS 8

/*
 * Calculates the lower bound only every X iterations. Greatly decreases the overall runtime, but may result in a slightly higher number of total iterations.
*/
#define CALC_EVERY_X_ITERATIONS 10

/*
 * Minimum change from previous best feasible triplets's R and E to the current feasible triplet's R and E.
 * Will terminate the algorithm prior to MAX_ITERATIONS if the change is below this tolerance.
 */
#define TOLERANCE 0.000000005

/*
 * Whether intermediate steps, such as the runtimes for specific iterations of FeasibleTriplet and values of R and E prior to convergence, should be displayed.
 * If false, will only display the initial parameters, final lower bound value, and total runtime.
*/
const bool PRINT_INTERMEDIATE_STEPS = true;

/*
* The digits to use for converting between integers and strings. There must be at least string_count characters, and any additional characters are ignored.
*/
const std::string base_digits = "0123456789ABCDEF";

// ======= CONSTANTS =======

/*
 * Constant that represents every possible string_count tuple of strings, where every string is of length length and use an alphabet of size alphabet_size.
 * This is the size of every vector in the algorithm, and is caches to avoid wasting time recalculating it
 */
const uint64_t powminus0 = pow(alphabet_size, string_count* length);

/*
 * Constant that represents the maximum number of contiguous strings that all start with the same string_count number of characters under our defined ordering (see README).
 * e.g., the number of strings of the form (1..., 0..., 2...), where every group of strings has string_count strings, and each of those strings are of length length and use
 * an alphabet of size alphabet_size.
 */
const uint64_t F_b_step = pow(alphabet_size, string_count*(length - 1));

// F_b_equals_1 is the number of starting character combinations between (0, 0, ...) and (1, 1, ...), which is the same
// length for any (x, x, ...) and (x + 1, x + 1, ...)
/*
 * The number of starting character combinations for string_count strings before each string starts with (1..., 1..., 1..., ...) under our defined ordering (see README).
 * e.g., for pairs of binary strings, the starting characters are (0..., 0...), (0..., 1...), (1..., 0...), (1..., 1...), so F_b_equals_1 = 3.
 */
const uint64_t F_b_equals_1 = ((1 - pow(alphabet_size, string_count)) / (1 - alphabet_size));

// ============================================ Feasible Triplet Functions ===========================================

/*
 * Helper function that converts the difference between start and end time into a time period in seconds.
 *
 * Takes as parameters
 * startTime: The start time to evaluate the number of seconds since
 * 
 * Returns the difference between the current time and the provided start time, in seconds
 */
double secondsSince(std::chrono::system_clock::time_point startTime) {
    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endTime - startTime;
    return elapsed_seconds.count();
}

/*
 * Helper function that parallelizes the action of subtracting one vector from another and finding the maximum value in the resulting array.
 * Used in the calculations of R and E.
 * 
 * Takes as parameters:
 * v1: The first vector (subtracted from v2)
 * v2: The second vector
 * 
 * Returns the largest element in v2 - v1
 */
double subtractAndFindMaxParallel(const ArrayXd& v1, const ArrayXd& v2) {
    std::future<double> maxVals[NUM_THREADS];

    // Anonymous function that us used by each thread to calculate the maximum coefficient in a particular (start...end) slice
    auto findMax = [&v1, &v2](uint64_t start, uint64_t end) {
        return (v2(Eigen::seq(start, end - 1)) - v1(Eigen::seq(start, end - 1))).maxCoeff();
    };

    // Set threads to calculate the maximum element after subtracting for their own smaller slices
    uint64_t prev_total = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        uint64_t total = ceil((i + 1) * double(powminus0) / NUM_THREADS);
        maxVals[i] = std::async(std::launch::async, findMax, prev_total, total);
        prev_total = total;
    }

    // Calculate the global maximum from all threads
    double R = maxVals[0].get();  // .get() waits until the thread completes
    for (int i = 1; i < NUM_THREADS; i++) {
        double coef = maxVals[i].get();
        if (coef > R) {
            R = coef;
        }
    }

    return R;
}

/*
 * Converts from a tuple of strings to an index using our specified ordering (see README).
 * Additionally, applies a specific variation to the provided strings.
 * 
 * Takes as parameters
 * initial: an array containing the unvariated tuple of strings
 * shouldVariate: A boolean array equal to true at the indices of variating that do not start with z, and false everywhere else
 * variateValue: An integer that gets mapped to a specific variation of the strings in initial
 */
uint64_t stringsToInt(std::string initial[], bool shouldVariate[], int variateValue) {
    uint64_t output = 0;

    //By indexing by string length first and then by string, we recreate the order of the characters of the strings when interleaved
    for (int l = 0; l < length; l++) {
        for (int d = 0; d < string_count; d++) {
            // Thinking of output as the characters in the string, this shifts the characters left
            output *= alphabet_size;

            if (shouldVariate[d]) {
                /*
                 * If not at the end of the string, index as if the characters were shifted left one space. This is equivalent to removing the first character.
                 * If at the end of the string, use variateValue to determine the character to append to the end.
                 */
                if (l == length - 1) {
                    /*
                     * Treats variateValue as a base alphabet_size number, where each digit represents what character to use for one of the strings that is being variated
                     * Order doesn't matter since the order is consistent, and variateValue will take on every possible variation
                     */
                    output += variateValue % alphabet_size;
                    variateValue /= alphabet_size;
                } else { 
                    // Acting as if characters are shifted left one space
                    output += base_digits.find(initial[d][l + 1]);
                }
            } else {
                // If should not variate, just return the regular character at the index
                output += base_digits.find(initial[d][l]);
            }
        }
    }

    return output;
}

/*
 * Equivalent to the variate function defined in the Feasible Triplet algorithm.
 *
 * Takes as parameters
 * v: A singular vector containing one of the last string_count iteration results of the Feasible Triplet algorithm, with the specific vector being determined by Fz
 * variating: The set of initial strings which should be variated
 * numNz: The number of strings in variating that do not start with z (where z is determined by Fz)
 * shouldVariate: A boolean array equal to true at the indices of variating that do not start with z, and false everywhere else
 * 
 * Returns the sum of v indexed at every possible variation
 */
double variate(const ArrayXd& v, std::string variating[], int numNz, bool shouldVariate[]) {
    double output = 0.0;

    /*
     * There are exactly alphabet_size ^ numNz ways to variate numNz strings for an alphabet of size alphabet_size. 
     * Thus, as long as we have a way to uniquely map every integer up to alphabet_size ^ numNz to a unique variation, we can generate every variation this way.
     */
    for (uint64_t variateValue = 0; variateValue < pow(alphabet_size, numNz); variateValue++) {
        // Convert the strings back to an integer index, applying the specific variation determined by variateValue
        uint64_t index = stringsToInt(variating, shouldVariate, variateValue);
        output += v[index];
    }

    return output;
}

/*
 * Converts from an index to a tuple of strings using our specified ordering (see README).
 * Does this by repeatedly taking the remainder to get the end digit of the index in base string_count.
 * 
 * Takes as parameters
 * index: The index to convert to a tuple of strings
 * ret: An array that gets updated with the output strings. Assumes that the strings have been initialized to be able to hold at least string_count characters each.
 */
void intToStrings(uint64_t index, std::string ret[]) {
    for (int i = string_count * length - 1; i >= 0; i--) {
        // Order of strings in ret doesn't matter, adds characters from end to front
        ret[i % string_count][i / string_count] = base_digits[index % alphabet_size];
        index /= alphabet_size; //Automatically rounds down
    }
}

/*
 * The equivalent of the Fz function(s) from the Feasible Triplet algorithm.
 *
 * Takes as parameters
 * z: The specific character (represented as an integer) that Fz is checking for
 * v: an array of vectors containing the results of the last string_count iterations of the Feasible Triplet algorithm
 * index: The specific index out the output vector that is being calculated
 * 
 * Returns the value of Fz at the specified index
 */
double Fz(int z, const ArrayXd v[], uint64_t index) {
    // Initialize the tuple of strings that is being variated
    std::string indices[string_count];
    for (int i = 0; i < string_count; i++) {
        indices[i].insert(0, length, ' ');
    }

    // Use the defined ordering to convert index to a tuple of strings
    intToStrings(index, indices);

    // Count how many strings in indices do not start with the character z, and record which indices need to be variated
    int numNz = 0;
    bool NzPos[string_count];
    for (int i = 0; i < string_count; i++) {
        bool zDigit = indices[i][0] != base_digits[z];
        numNz += zDigit; //true = 1, false = 0, so adds 1 only if string does not start with z
        NzPos[i] = zDigit;
    }

    // If all of the strings start with z, then Fz = 0
    if (numNz == 0) 
    {
        return 0;
    }

    return pow(alphabet_size, -numNz) * variate(v[string_count - numNz], indices, numNz, NzPos);
}

/*
 * The equivalent of the F function from the Feasible Triplet algorithm.
 *
 * Takes as parameters
 * v: an array of vectors containing the results of the last string_count iterations of the Feasible Triplet algorithm
 * ret: An output vector that gets updated with the results of the F function
 */
void F(const ArrayXd v[], ArrayXd& ret) {
    auto start2 = std::chrono::system_clock::now();

    // An anonymous function that partitions the calculations of F across threads
    auto F_lambda = [&v, &ret](uint64_t start, uint64_t end) {
        double calculated;

        for (uint64_t index = start; index < end; index++) {
            calculated = 0.0;
            //Obtain the max across all Fz
            for (int s = 0; s < alphabet_size; s++) {
                calculated = std::max(Fz(s, v, index), calculated);
            }
            
            //Add 1 if the strings represented by index all start with the same character
            if ((index / F_b_step) % F_b_equals_1 == 0) {
                calculated += 1;
            }

            ret[index] = calculated;
        }
    };

    std::thread threads[NUM_THREADS];
    // Set threads to run F in their own smaller slices
    uint64_t prev_total = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        uint64_t total = ceil((i + 1) * double(powminus0) / NUM_THREADS);
        threads[i] = std::thread(F_lambda, prev_total, total);
        prev_total = total;
    }

    // Now wait for each thread to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    if(PRINT_INTERMEDIATE_STEPS) {        
        std::cout << "Elapsed time F (s): " << secondsSince(start2) << std::endl;
    }
}

/*
 * Represents the second time F is called in the Feasible Triplet algorithm.
 *
 * Takes as parameters
 * R: The value R calculated in the Feasible Triplet algorithm
 * vNew: The result of the first call to F
 * ret: An output vector that gets updated with the results of the second call to F
 */
void F_withplusR(const double R, const ArrayXd& vNew, ArrayXd& ret) {
    ArrayXd vR[string_count];

    for (int i = 0; i < string_count; i++) {
        // Order gets reversed within Fz due to the way v is ordered, so we construct the arguments to this call of F in reverse order to the order defined in the algorithm
        vR[i] = vNew + i * R;
    }

    F(vR, ret);
}

/*
* The FeasibleTriplet algorithm, equivalent to the algorithm defined in the paper. Since we don't need the first element in the triplet (u, r, e), we do not save it in this program.
*
* Takes as parameters
* length: The length of the strings used in calculations
* string_count: The number of strings in each calculation
* alphabet_size: The number of different characters that the strings can use
* n: The maximum number of iterations to run. 
*
* Normaly terminates after the change per iteration drops below TOLERANCE, but will terminate after n iterations if that is reached first.
*/
void FeasibleTriplet(int n) {
    auto start = std::chrono::system_clock::now();
    // Initialize an array of vectors that stores previous results. Starts filled with 0s
    ArrayXd v[string_count]; 

    for (int i = 0; i < string_count; i++) {
        v[i] = ArrayXd::Zero(powminus0);
    }

    // Initialize a vector that stores the newest result. Also starts filled with 0s
    ArrayXd vNew(powminus0);

    double r = 0;
    double e = 0;
    double prevr = 0;
    double preve = 0;
    for (int i = string_count; i < n + 1; i++) {
        if(PRINT_INTERMEDIATE_STEPS) {
            std::cout << "ITERATION " << i - string_count + 1 << std::endl;
        }

        // Calculate F and store result in vNew
        auto start2 = std::chrono::system_clock::now();
        F(v, vNew);
        if(PRINT_INTERMEDIATE_STEPS) {
            std::cout << "Elapsed time F (s): " << secondsSince(start2) << std::endl;
        }

        // Because string_count * (R - E), the approximation of a Chvatal-Sankoff constant, converges, we don't need to check the value of R - E every iteration.
        if (i % CALC_EVERY_X_ITERATIONS == 0 || i == n) {
            // Calculate the value of R
            start2 = std::chrono::system_clock::now();
            const double R = subtractAndFindMaxParallel(v[string_count - 1], vNew);
            if(PRINT_INTERMEDIATE_STEPS) {
                std::cout << "Elapsed time (s) mc1: " << secondsSince(start2) << std::endl;
            }

            /*
             * Beyond this point, we never use v[0]. As such, we can store calculations in v[0] rather then creating a new vector to store the results.
             * Calculate the equivalent of the W vector in the feasible triplet algorithm, and store the result in v[0].
             */
            start2 = std::chrono::system_clock::now();
            F_withplusR(R, vNew, v[0]);
            if(PRINT_INTERMEDIATE_STEPS) {
                std::cout << "Elapsed time FpR (s): " << secondsSince(start2) << std::endl;
            }

            /* Calculate the value of E. Because string_count * R is alwaus a non-negative value,
             * it does not effect the maximum value of v[0] and vNew, so it can be added after calculating the maximum.
             */
            start2 = std::chrono::system_clock::now();
            const double E = std::max(subtractAndFindMaxParallel(v[0], vNew) + string_count * R, 0.0);
            if(PRINT_INTERMEDIATE_STEPS) {
                std::cout << "Elapsed time mc2 (s): " << secondsSince(start2) << std::endl;
            }

            /*
             * The feasible triplet is only better if it improves the lower bound. The lower bound approximation is given by string_count * (R - E),
             * so if R - E is larger then the last feasible triplet saved (r - e), then it must be better, and we should save R and E instead.
             */
            if (R - E >= r - e) {
                r = R;
                e = E;
            }
            if(PRINT_INTERMEDIATE_STEPS) {
                printf("Result (iteration %d): %.9f\n", i - string_count + 1, string_count * (r - e));
                std::cout << "Calculated R-E: R = " << R << ", E = " << E << std::endl;
            }

            /*
             * As the difference between feasible triplets grows small, the change in the lower bound also grows small.
             * There is little point to continuing to run the algorithm once the difference becomes small enough, so when the difference drops below the specified TOLERANCE, the algorithm terminates early.
             */
            if ((r - e) - (prevr - preve) <= TOLERANCE && (prevr != r && preve != e)) {
                printf("Change under min tolerance, quitting...\n");
                break;
            }
            prevr = r;
            preve = e;
        }

        // Shifts every vector in v up by 1, replacing the last vector with vNew and discarding the values in v[0].
        std::swap(v[0], vNew);
        for (int i = 0; i < string_count - 1; i++) {
            std::swap(v[i], v[i + 1]);
        }
    }

    // Display the final approximation of the constant
    printf("Final Result: %.9f\n", string_count * (r - e));
}

/**
 * Runs FeasibleTriplet with the paramters defined at the top of the file.
 */
int main() {
    printf("Starting with l = %d, d = %d, sigma = %d...\n", length, string_count, alphabet_size);
    auto start = std::chrono::system_clock::now();
    FeasibleTriplet(MAX_ITERATIONS);
    std::cout << "Elapsed time (s): " << secondsSince(start) << std::endl;

    return 0;
}