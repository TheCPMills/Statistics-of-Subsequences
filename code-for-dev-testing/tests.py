if __name__ == '__main__':
    length = 4
    powminus0 = 2 ** (2*length)
    powminus1 = 2 ** (2*length-1)
    powminus2 = 2 ** (2*length-2)
    powminus3 = 2 ** (2*length-3)
    start = powminus2
    end = powminus1
    for str in range(start, end):
        A = str & 0xAAAAAAAAAAAAAAAA
        TB = (str & 0x5555555555555555) & (powminus2 - 1)
        ATB0 = A | (TB << 2)
        ATB1 = ATB0 | 1
        print(str, ": {0:0{1}b} -->  ".format(str, 2*length), "{0:0{1}b}, ".format(ATB0, 2*length), " {0:0{1}b} ".format(ATB1, 2*length))
    
    # first 1/4: 
    
    print("\n")
    start2 = powminus1 -powminus1
    end2 = powminus1 + powminus2 -powminus1
    for str2 in range(start2, end2):
        TA = (str2 & 0xAAAAAAAAAAAAAAAA) & (powminus1 - 1)
        B = str2 & 0x5555555555555555
        TA0B = (TA << 2) | B
        TA1B = TA0B | 2;       

        TA0B = min(TA0B, (powminus0 - 1) - TA0B)
        TA1B = min(TA1B, (powminus0 - 1) - TA1B)
        print(str2, ": {0:0{1}b} -->  ".format(str2, 2*length), "{0:0{1}b}, ".format(TA0B, 2*length), " {0:0{1}b} ".format(TA1B, 2*length))
