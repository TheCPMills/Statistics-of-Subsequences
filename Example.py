import numpy as np


# l = 2 string length
# d = 2 (2-string comparison)
# Sigma = 2 (binary alphabet)

# string pair ordering (lexicographical):
# 00 00, 00 01, 00 10, 00 11, 01 00, 01 01, 01 10, 01 11,
# 10 00, 10 01, 10 10, 10 11, 11 00, 11 01, 11 10, 11 11


def F01(v):
    return np.array([0, 0, v[0] + v[1], v[2] + v[3], 0, 0, v[4] + v[5], v[6] + v[7],
                     v[0] + v[4], v[1] + v[5], 0, 0, v[8] + v[12], v[9] + v[13], 0, 0])


def F02(v):
    return np.array([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, v[0] + v[1] + v[4] + v[5], v[2] + v[3] + v[6] + v[7],
                     0, 0, v[8] + v[9] + v[12] + v[13], v[10] + v[11] + v[14] + v[15]])


def F11(v):
    return np.array([0, 0, v[2] + v[6], v[3] + v[7], 0, 0, v[10] + v[14], v[11] + v[15],
                     v[8] + v[9], v[10] + v[11], 0, 0, v[12] + v[13], v[14] + v[15], 0, 0])


def F12(v):
    return np.array([v[0] + v[1] + v[4] + v[5], v[2] + v[3] + v[6] + v[7], 0, 0,
                     v[8] + v[9] + v[12] + v[13], v[10] + v[11] + v[14] + v[15], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])

def F0(vA, vB):
    f01 = F01(vA)
    f02 = F02(vB)
    res = 0.5 * f01 + 0.25 * f02
    return res


def F1(vA, vB):
    f11 = F11(vA)
    f12 = F12(vB)
    res = 0.5 * f11 + 0.25 * f12
    return res


def F(vA, vB):
    f0 = F0(vA, vB)
    f1 = F1(vA, vB)
    return np.array([1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1]) + np.maximum(f0, f1)


def feasible_triplet(n):
    v_0 = np.zeros(16)
    v_1 = np.zeros(16)

    (u, r, e) = (np.zeros(16), 0, 0)

    for i in range(2, n + 1):
        v_2 = F(v_1, v_0)
        R = np.max(v_2 - v_1)
        W = v_2 + np.full(16, 2 * R) - F(v_2 + np.full(16, R), v_2)
        E = max(0, np.max(W))
        if R - E >= r - e:
            (u, r, e) = (v_2, R, E)

        v_0 = v_1
        v_1 = v_2

    return u, r, e


# result is pretty bad lower bound because this uses a very low value for l (2)
if __name__ == '__main__':
    _u, _r, _e = feasible_triplet(1000)
    print(f"({_u}, {_r}, {_e})")
    print(2 * (_r - _e))
