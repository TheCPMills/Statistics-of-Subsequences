import itertools
import time

import numpy as np


# Conceptually, for strings x1x2...xl and y1y2...yl, the ordering used is the lexicographical ordering of
# x1y1x2y2...xlyl
# Note: binary operations below rely on 32-bit constants. This also means that program can only currently calculate up
# to length 16 without overflow. Easy to change, but be careful of overflow
def FeasibleTriplet(length, sigma, d, iters):
    v = [np.zeros(sigma ** (d * length)) for _ in range(d)]
    (u, r, e) = (np.zeros(sigma ** (d * length)), 0, 0)

    for i in range(d, iters + 1):
        v.append(F(v[::-1], sigma, d, length))
        R = np.max(v[-1] - v[-2])
        W = v[-1] + d * R - F([v[-1] + (d - i - 1) * R for i in range(d)], sigma, d, length)
        E = max(0, np.max(W))

        if R - E >= r - e:
            (u, r, e) = (v[-1], R, E)
        del v[0]

    return u, r, e


# note: b is 1 where all v start with the same character and 0 otherwise
def F(v, sigma, d, length):
    b = np.zeros(sigma ** (d * length))
    for i in range(0, sigma ** d + 1, sigma * (sigma + 1) + 1):
        b[i * sigma ** (d * (length - 1)):(i + 1) * sigma ** (d * (length - 1))] = 1

    Fz = [F_z(z, v, sigma, d, length) for z in range(sigma)]
    return b + np.maximum.reduce(Fz)


def F_z(z, v, sigma, d, length):
    output = np.zeros(sigma ** (d * length))

    for i in range(sigma ** (d * length)):
        strings = intToStrings(i, sigma, d, length)
        Nz = [i for i in range(d) if strings[i][0] != str(z)]

        # output defaults to 0, so do nothing if Nz == 0
        if len(Nz) != 0:
            temp = variate(Nz, strings, v[len(Nz) - 1], sigma, length)
            output[i] = 1 / (sigma ** len(Nz)) * temp

    return output


def intToStrings(value, sigma, d, length):
    baseValue = np.base_repr(value, sigma).zfill(d * length)
    outputs = ["" for _ in range(d)]
    for i in range(len(baseValue)):
        outputs[i % d] += baseValue[i]

    return [st.zfill(length) for st in outputs]


def stringsToInt(values, sigma, length):
    outputStr = "".join([st[i] for i in range(length) for st in values])
    return int(outputStr, sigma)


def variate(Nz, strings, v, sigma, length):
    output = 0

    for c in itertools.product("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[:sigma], repeat=len(Nz)):
        variation = strings.copy()
        for j in range(len(Nz)):
            variation[Nz[j]] = variation[Nz[j]][1:] + c[j]
        output += v[stringsToInt(variation, sigma, length)]

    return output


def main():
    sigma = 2
    d = 3
    l = 7
    iters = 100
    start = time.time_ns()
    (v, r, e) = FeasibleTriplet(l, sigma, d, iters)
    end = time.time_ns()
    print(f"σ={sigma}, d={d}, l={l}, iterations={iters}")
    print(d * (r - e))
    print(f"Runtime: {(end - start) / 1000000}")


# Python is so annoying sometimes
if __name__ == '__main__':
    main()
