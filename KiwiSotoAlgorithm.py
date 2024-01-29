import numpy as np


# Conceptually, for strings x1x2...xl and y1y2...yl, the ordering used is the lexicographical ordering of
# x1y1x2y2...xlyl
# Note: binary operations below rely on 32-bit constants. This also means that program can only currently calculate up
# to length 16 without overflow. Easy to change, but be careful of overflow
def FeasibleTriplet(length, n):
    v0 = np.zeros(2 ** (2 * length - 1))
    v1 = np.zeros(2 ** (2 * length - 1))

    (u, r, e) = (np.zeros(2 ** (2 * length - 1)), 0, 0)

    for i in range(2, n + 1):
        v2 = F(v1, v1, length) # I CHANGED v0 to v1 IN SECOND LOC
        R = np.max(v2 - v1)
        W = v2 + 2 * R - F(v2 + R, v2, length)
        E = max(0, np.max(W))

        if R - E >= r - e:
            (u, r, e) = (v2, R, E)
        agh = np.min(v2-v1)

        v0 = v1
        v1 = v2
        # print(v1)
        # print(v2)
        # print(R, E)
        # r = R
        # e = E
        #print("at ",i,":  R:  ",(R*25000000),"  E: ",(E*25000000), ((E-2*R)*25000000) )
        print("agh",2*agh/(1+agh))
        print("at ",i,":  R:  ",R,"  E: ",E )
        print("E:", E*25000000, "   R:", R*25000000)
        print(2*(R-E))
        EmR = 2*R-E
        print(2*EmR/(1+EmR))
        # print(2.0 * r / (1 + r))
        # print(2.0 * (r) / (1 + (r-e)))
        # print(2.0 * (r) / (1 + (r-e)))
        # print(r-e)
    print(v0)
    print(v1)
    return u, r, e


# note: b is 1 where v1 and v2 start with the same character and 0 otherwise
def F(v1, v2, length):
    [f01, f11] = np.split(F_01(v1, length), 2)
    f11 = np.flip(f11)
    f_double = F_12(v2, length)


    # b + max
    b_combined = np.concatenate((1 + 0.25 * f_double, 0.5 * np.maximum(f01, f11)))
    return b_combined


def F_01(v, length):
    #print("NEW ITER")
    ret = np.zeros(2 ** (2 * length - 1))

    start = 2 ** (2 * length - 2)
    # range2 is where h(A) != z and h(B) = z

    # st & 0xAAAAAAAA = only even bits of st, corresponding to bits in A
    # st & 0x55555555 = only odd bits of st, corresponding to bits in B

    # range1 is the range of ordered string pairs (A, B) where h(A) = z and h(B) != z
    for st in range(start, 2 * start):
        # For st = (A, B),
        # v[(A, T(B)0)] + v[(A, T(B)1)]
        A = st & 0xAAAAAAAA
        TB = (st & 0x55555555) & ((1 << (2 * length - 2)) - 1)
        ATB0 = A | (TB << 2)
        ATB1 = ATB0 | 0b1  # 0b1 <- the smallest bit in B is set to 1

        # Transform indices to symmetrical position if they would be in right half of vector v
        ATB0 = min(ATB0, (2 ** (2 * length) - 1) - ATB0)
        ATB1 = min(ATB1, (2 ** (2 * length) - 1) - ATB1)
        ret[st - start] = v[ATB0] + v[ATB1]  # if h(A) != h(B) and h(A) = z
        #print(st-start)

    for st in range(2 * start, 3 * start):
        # For st = (A, B),
        # v[(T(A)0, B)] + v[(T(A)1, B)]
        TA = (st & 0xAAAAAAAA) & ((1 << (2 * length - 1)) - 1)
        B = st & 0x55555555
        TA0B = (TA << 2) | B
        TA1B = TA0B | 0b10  # 0b10 <- the smallest bit in A is set to 1
        
        # Transform indices to symmetrical position if they would be in right half of vector v
        TA0B = min(TA0B, (2 ** (2 * length) - 1) - TA0B)
        TA1B = min(TA1B, (2 ** (2 * length) - 1) - TA1B)
        ret[st - start] = v[TA0B] + v[TA1B]  # if h(A) != h(B) and h(B) = z
        #print(st-start)

    return ret


def F_12(v, length):
    ret = np.zeros(2 ** (2 * length - 2))

    for st in range(0, 2 ** (2 * length - 2)):
        # For st = (A, B),
        # v[(T(A)0, T(B)0)] + v[(T(A)0, T(B)1)] + v[(T(A)1, T(B)0)] + v[(T(A)1, T(B)1)]
        TA = (st & 0xAAAAAAAA) & ((1 << (2 * length - 1)) - 1)
        TB = (st & 0x55555555) & ((1 << (2 * length - 2)) - 1)
        TA0TB0 = (TA << 2) | (TB << 2)
        TA0TB1 = TA0TB0 | 0b1
        TA1TB0 = TA0TB0 | 0b10
        TA1TB1 = TA0TB0 | 0b11

        # Transform indices to symmetrical position if they would be in right half of vector v
        TA0TB0 = min(TA0TB0, (2 ** (2 * length) - 1) - TA0TB0)
        TA0TB1 = min(TA0TB1, (2 ** (2 * length) - 1) - TA0TB1)
        TA1TB0 = min(TA1TB0, (2 ** (2 * length) - 1) - TA1TB0)
        TA1TB1 = min(TA1TB1, (2 ** (2 * length) - 1) - TA1TB1)
        ret[st] = v[TA0TB0] + v[TA0TB1] + v[TA1TB0] + v[TA1TB1]  # if h(A) = h(B) = z

    return ret


def main():
    for i in range(1, 2):
        (v, r, e) = FeasibleTriplet(4, 40)
        print(2*(r - e))
        print(2.0 * r / (1 + r))


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
