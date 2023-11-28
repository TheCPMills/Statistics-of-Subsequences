#ifndef MODEL_GENERATOR_H
#define MODEL_GENERATOR_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

class ModelGenerator {
    private:
        static void genObj(int, int, ofstream&);
        static void genMtl(int, int, ofstream&);

    public:
        static void generate(int, int);
};

#endif
