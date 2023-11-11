import numpy as np


# Conceptually, for strings x1x2...xl and y1y2...yl, the ordering used is the lexicographical ordering of
# x1y1x2y2...xlyl
# Note: binary operations below rely on 32-bit constants. This also means that program can only currently calculate up
# to length 16 without overflow. Easy to change, but be careful of overflow
def FeasibleTriplet(length, n):
    v0 = np.zeros(2 ** (2 * length))
    v1 = np.zeros(2 ** (2 * length))

    (u, r, e) = (np.zeros(2 ** (2 * length)), 0, 0)

    for i in range(2, n + 1):
        v2 = F(v1, v0, length)
        R = np.max(v2 - v1)
        W = v2 + 2 * R - F(v2 + R, v2, length)
        E = max(0, np.max(W))

        if R - E >= r - e:
            (u, r, e) = (v2, R, E)
        v0 = v1
        v1 = v2

    return u, r, e


# note: b contains 2^2l entries, and is 1 where v1 and v2 start with the same character and 0 otherwise
def F(v1, v2, length):
    b = np.concatenate((np.ones(2**(2*length - 2)), np.zeros(2**(2*length - 1)), np.ones(2**(2*length - 2))))

    f01 = F_z1(0, v1, length)
    f11 = F_z1(1, v1, length)
    f_double = F_z2(0, v2, length)
    
    return b + np.maximum(0.5 * f01 + 0.25 * f_double, 0.5 * f11 + 0.25 * np.flip(f_double))


def F_z(z, v1, v2, length):
    return 0.5 * F_z1(z, v1, length) + 0.25 * F_z2(z, v2, length)


def F_z1(z, v, length):
    ret = np.zeros(2 ** (2 * length))

    # range1 is the range of ordered string pairs (A, B) where h(A) = z and h(B) != z
    # range2 is where h(A) != z and h(B) = z
    if z == 0:
        range1 = range(2 ** (2 * length - 2), 2 ** (2 * length - 1))
        range2 = range(2 ** (2 * length - 1), 3 * (2 ** (2 * length - 2)))
    else:  # z == 1
        range2 = range(2 ** (2 * length - 2), 2 ** (2 * length - 1))
        range1 = range(2 ** (2 * length - 1), 3 * (2 ** (2 * length - 2)))

    # st & 0xAAAAAAAA = only even bits of st, corresponding to bits in A
    # st & 0x55555555 = only odd bits of st, corresponding to bits in B
    # Technically can do st | TB or st | TA instead of A | TB or B | TA, but less clear
    for st in range1:
        # For st = (A, B),
        # v[(A, T(B)0)] + v[(A, T(B)1)]
        A = st & 0xAAAAAAAA
        TB = (st & 0x55555555) & ((1 << (2 * length - 2)) - 1)
        ATB0 = A | (TB << 2)
        ATB1 = ATB0 | 0b1  # 0b1 <- the smallest bit in B is set to 1
        ret[st] = v[ATB0] + v[ATB1]  # if h(A) != h(B) and h(A) = z

    for st in range2:
        # For st = (A, B),
        # v[(T(A)0, B)] + v[(T(A)1, B)]
        TA = (st & 0xAAAAAAAA) & ((1 << (2 * length - 1)) - 1)
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
        TA = (st & 0xAAAAAAAA) & ((1 << (2 * length - 1)) - 1)
        TB = (st & 0x55555555) & ((1 << (2 * length - 2)) - 1)
        TA0TB0 = (TA << 2) | (TB << 2)
        TA0TB1 = TA0TB0 | 0b1
        TA1TB0 = TA0TB0 | 0b10
        TA1TB1 = TA0TB0 | 0b11
        ret[st] = v[TA0TB0] + v[TA0TB1] + v[TA1TB0] + v[TA1TB1]  # if h(A) = h(B) = z

    return ret


def main():
    for i in range(1, 11):
        (v, r, e) = FeasibleTriplet(i, 100)
        print(2*(r - e))


# Python is so annoying sometimes
# l = 1 to 10, n = 100
# 0.6666666666666643
# 0.7272727272727195
# 0.7479224376731253
# 0.7585767077281673
# 0.7654469749208062
# 0.770273875304099
# 0.7739750979416158
# 0.7768606643965228
# 0.7792593255657607
# 0.7812812337791541
if __name__ == '__main__':
    main()
