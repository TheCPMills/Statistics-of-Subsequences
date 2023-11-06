import itertools
import math
import time

import numpy as np

import visualization


def gen_bitstrs(len):
    return [''.join(i) for i in itertools.product('01', repeat=len)]


# Create lcs matrix (this is slow, Cpp version is preferred for large matrices)
def lcs_len(s1, s2):
    m, n = len(s1), len(s2)
    L = [[0 for x in range(n + 1)] for x in range(m + 1)]
    # L = np.zeros((m+1, n+1))
    for i in range(m + 1):
        for j in range(n + 1):
            if i == 0 or j == 0:
                L[i][j] = 0
            elif s1[i - 1] == s2[j - 1]:
                L[i][j] = L[i - 1][j - 1] + 1
            else:
                L[i][j] = max(L[i - 1][j], L[i][j - 1])
    return L[m][n]


def populateMatrix(bitstrs, strlen, vis=True):
    a = np.zeros((strlen, strlen))
    # this is very inefficient
    for i in range(strlen):
        for j in range(i + 1):
            a[i][j] = lcs_len(bitstrs[i], bitstrs[j])

    # matrix is symmetric across main diagonal, so fill in upper triangle and mirror across
    # this gives a roughly 2x speedup
    a = a + a.T - np.diag(np.diag(a))

    print("Matrix complete!")
    print("Mean:", a.mean())
    print("Mean/n", a.mean() / math.log2(strlen))
    print("Time taken:", time.time() - start_time)
    print(np.linalg.matrix_rank(a, hermitian=True))
    q = a.T @ a
    print(np.linalg.matrix_rank(q))
    if vis:
        visualization.visMatrix(a)
    print(a)
    visualization.plot3d(a.flatten(), math.pow(2, strLen))
    return np.linalg.matrix_rank(a, hermitian=True)


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    start_time = time.time()
    strLen = 3
    print(gen_bitstrs(strLen))
    populateMatrix(gen_bitstrs(strLen), pow(2, strLen))

    # lens = []
    # ranks = []
    # for i in range(1, 11):
    #     lens.append(i)
    #     ranks.append(populateMatrix(gen_bitstrs(i), pow(2, i), vis=False))
    #
    # visualization.rankPlot(lens, ranks)

    # mean for 10: 0.6978439331054688
    # fileName = "C:\\Users\\dunca\\Computer Science\\WPI Classes\\MQP\\C++ Code\\LCSgrid11.txt"
    # loadedMatrix = np.loadtxt(open(fileName, "rb"), delimiter=",")  # dtype=np.uint8
    # loadedMatrix = loadedMatrix + loadedMatrix.T - np.diag(np.diag(loadedMatrix))
    # print("Mean/n (loaded)", loadedMatrix.mean() / math.log2(loadedMatrix.shape[0]))
    # visualization.visMatrix(loadedMatrix)
