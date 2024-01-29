#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <Eigen/Dense>
#include <chrono>
#include <cmath>
#include <ctime>
#include <iostream>
using namespace std;
using Eigen::Array;
using Eigen::ArrayXd;
using Eigen::ArrayXi;
using Eigen::Map;

// This file is for figuring out how mmap a file, and read it to eigen

// Try these flags for faster? https://stackoverflow.com/questions/8056984/speeding-up-file-i-o-mmap-vs-read?rq=4
// https://stackoverflow.com/questions/55379852/fast-file-reading-in-c-comparison-of-different-strategies-with-mmap-and-std

#define FILEPATH "./mmapped.bin"
#define NUMINTS (10)
#define FILESIZE uint64_t(NUMINTS * sizeof(double))
int fileStuff() {
    int i;
    int fd;
    int result;
    double *map; /* mmapped array of int's */

    /* Open a file for writing.
     *  - Creating the file if it doesn't exist.
     *  - Truncating it to 0 size if it already exists. (not really needed) [NOTE: PROBABLY HARMFUL FOR US?]
     *
     * Note: "O_WRONLY" mode is not sufficient when mmaping.
     */
    fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    /* Stretch the file size to the size of the (mmapped) array of ints
     */
    result = lseek(fd, FILESIZE - 1, SEEK_SET);
    if (result == -1) {
        close(fd);
        perror("Error calling lseek() to 'stretch' the file");
        exit(EXIT_FAILURE);
    }

    /* Something needs to be written at the end of the file to
     * have the file actually have the new size.
     * Just writing an empty string at the current file position will do.
     *
     * Note:
     *  - The current position in the file is at the end of the stretched
     *    file due to the call to lseek().
     *  - An empty string is actually a single '\0' character, so a zero-byte
     *    will be written at the last byte of the file.
     */
    result = write(fd, "", 1);
    if (result != 1) {
        close(fd);
        perror("Error writing last byte of the file");
        exit(EXIT_FAILURE);
    }

    /* Now the file is ready to be mmapped.
     */
    // TODO: probably want to set as bianry
    map = (double *)mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }

    /* Now write int's to the file as if it were memory (an array of ints).
     */
    for (i = 1; i <= NUMINTS; ++i) {
        map[i] = 20.23 * i;
    }

    Map<ArrayXd> mf(map, 4);
    cout << mf;

    auto source = &mf(0);
    std::memcpy(map + (sizeof(double)) * 4, source, (sizeof(double)) * 4);

    Map<ArrayXd> mf2(map + (sizeof(double)) * 4, 4);
    mf2 = mf2 + 2;
    cout << mf2;

    /* Don't forget to free the mmapped memory
     */
    if (munmap(map, FILESIZE) == -1) {
        perror("Error un-mmapping the file");
        /* Decide here whether to close(fd) and exit() or not. Depends... */
    }

    /* Un-mmaping doesn't close the file, so we still need to do that.
     */
    close(fd);
    return 0;
}

int main() {
    fileStuff();
    return 0;
}