import numpy as np


def FeasibleTriplet(length, n):
    v = np.zeros((n, 2 ** (2 * length)))

    (u, r, e) = (v[0], 0, 0)

    for i in range(2, n + 1):
        v_i = F(v[i - 1], v[i - 2], length)
        R = max(v_i - v[i - 1])
        W = v_i + 2 * R - F(v_i + R, v_i, length)
        E = max(0, max(W))

        if R - E >= r - e:
            (u, r, e) = (v_i, R, E)

    return u, r, e


# note: b contains 2^2l entries, and is 1 where v1 and v2 start with the same character and 0 otherwise
def F(v1, v2, length):
    return np.concatenate((np.ones(2**(2*length - 2)), np.zeros(2**(2*length - 1)), np.ones(2**(2*length - 2)))) \
           + np.maximum(F_z(0, v1, v2, length), F_z(1, v1, v2, length))


def F_z(z, v1, v2, length):
    return 0.5 * F_z1(z, v1, length) + 0.25 * F_z2(z, v2, length)


def F_z1(z, v, length):
    ret = np.zeros(2 ** (2 * length))

    # NOTE: This determines the lexicographical order of the 2-tuple A. Currently assuming that for a tuple (A, B),
    # that B increments before A. Swapping the z == 0 to z == 1 implies the opposite, that A increments before B
    if z == 0:
        range1 = range(2 ** (2 * length - 2), 2 ** (2 * length - 1))
        range2 = range(2 ** (2 * length - 1), 3 * (2 ** (2 * length - 2)))
    else:  # z == 1
        range2 = range(2 ** (2 * length - 2), 2 ** (2 * length - 1))
        range1 = range(2 ** (2 * length - 1), 3 * (2 ** (2 * length - 2)))

    # st & 0xAAAAAAAA = only even bits of st, corresponding to bits in A
    # st & 0x55555555 = only odd bits of st, corresponding to bits in B
    for st in range1:
        # For st = (A, B),
        # v[(A, T(B)0)] + v[(A, T(B)1)]
        A = st & 0xAAAAAAAA
        TB = (st & 0x55555555) & (1 << (length - 1))
        ATB0 = A | (TB << 2)
        ATB1 = ATB0 | 0b1  # 0b1 <- the smallest bit in B is set to 1
        ret[st] = v[ATB0] + v[ATB1]  # if h(A) != h(B) and h(A) = z

    for st in range2:
        # For st = (A, B),
        # v[(T(A)0, B)] + v[(T(A)1, B)]
        TA = (st & 0xAAAAAAAA) & (1 << (length - 1))
        B = st & 0x55555555
        TA0B = (TA << 2) | B
        TA1B = TA0B | 0b10  # 0b10 <- the smallest bit in A is set to 1
        ret[st] = v[TA0B] + v[TA1B]  # if h(A) != h(B) and h(B) = z

    return ret


def F_z2(z, v, length):
    ret = np.zeros(2 ** (2 * length))

    if z == 0:
        rangeF = range(0, 2 ** (2 * length - 2))
    else:  # z == 1
        rangeF = range(3 * (2 ** (2 * length - 2)), 2 ** (2 * length))

    for st in rangeF:
        # For st = (A, B),
        # v[(T(A)0, T(B)0)] + v[(T(A)0, T(B)1)] + v[(T(A)1, T(B)0)] + v[(T(A)1, T(B)1)]
        TA = (st & 0xAAAAAAAA) & (1 << (length - 1))
        TB = (st & 0x55555555) & (1 << (length - 1))
        TA0TB0 = (TA << 2) | (TB << 2)
        TA0TB1 = TA0TB0 | 0b1
        TA1TB0 = TA0TB0 | 0b10
        TA1TB1 = TA0TB0 | 0b11
        ret[st] = v[TA0TB0] + v[TA0TB1] + v[TA1TB0] + v[TA1TB1]  # if h(A) = h(B) = z

    return ret


if __name__ == '__main__':
    (v, r, e) = FeasibleTriplet(6, 40)
    print((v, r, e))
