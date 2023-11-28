#include "ModelGenerator.h"
#include <windows.h>

int main(int argc, char const *argv[]) {
    ModelGenerator::generate(10, 10);
    return 0;
}

void ModelGenerator::generate(int n, int m) {
    // get current path
    string userDir = getenv("USERPROFILE"); // get user directory
    string filePath = userDir + "\\OneDrive\\Documents\\GitHub\\Statistics-of-Subsequences\\res\\models\\"; // get file path

    cout << filePath << endl;

    string objFileName = "model_" + to_string(n) + "x" + to_string(m) + ".obj";
    string mtlFileName = "model_" + to_string(n) + "x" + to_string(m) + ".mtl";

    ofstream objWriter;
    ofstream mtlWriter;

    // create obj file
    objWriter.open(filePath + objFileName);
    genObj(n, m, objWriter);
    objWriter.close();

    // create mtl file
    mtlWriter.open(filePath + mtlFileName);
    genMtl(n, m, mtlWriter);
}

void ModelGenerator::genObj(int n, int m, ofstream &objWriter) {
    objWriter << "mtllib model_" + to_string(n) + "x" + to_string(m) + ".mtl\n" << endl;

}

void ModelGenerator::genMtl(int n, int m, ofstream &mtlWriter) {
    return;
}