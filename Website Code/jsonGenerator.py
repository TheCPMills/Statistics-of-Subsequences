def genStringSet(length):
    # Generate all binary strings of length n
    if length == 0:
        return ['']
    else:
        return ['0' + s for s in genStringSet(length - 1)] + ['1' + s for s in genStringSet(length - 1)]
      
def LcsHelper(text1, text2, m, n, dp):
    # check if the end of either sequence is reached or not
    if m == 0 or n == 0:
        # create a list with one empty string and return
        return [""]

    # check if the last character of text1 and text2 matches
    if text1[m - 1] == text2[n - 1]:

        # ignore the last characters of text1 and text2 and find all LCS of substring text1[0 … m-2], text2[0 … n-2] and store it in a list
        lcs = LcsHelper(text1, text2, m - 1, n - 1, dp)

        # append current character text1[m-1] or text2[n-1] to all LCS of substring text1[0 … m-2] and text2[0 … n-2]
        for i in range(len(lcs)):
            lcs[i] = lcs[i] + (text1[m - 1])

        return lcs

    # we will reach here when the last character of text1 and text2 don't match

    # if a top cell of the current cell has more value than the left cell,
    # then ignore the current character of string text1 and find all LCS of substring text1[0 … m-2], text2[0 … n-1]
    if dp[m - 1][n] > dp[m][n - 1]:
        return LcsHelper(text1, text2, m - 1, n, dp)

    # if a left cell of the current cell has more value than the top cell,
    # then ignore the current character of string text2 and find all LCS of substring text1[0 … m-1], text2[0 … n-2]
    if dp[m][n - 1] > dp[m - 1][n]:
        return LcsHelper(text1, text2, m, n - 1, dp)

    # if the top cell has equal value to the left cell, then consider both characters
    top = LcsHelper(text1, text2, m - 1, n, dp)
    left = LcsHelper(text1, text2, m, n - 1, dp)

    # merge two lists and return
    return top + left


def lcsSet(text1, text2):
    # calculate length of text1 and text2
    m, n = len(text1), len(text2)

    # dp[i][j] stores the length of LCS of substring text1[0 … i-1] and text2[0 … j-1]
    dp = [[0 for x in range(n + 1)] for y in range(m + 1)]

    # fill the lookup dp table in a bottom-up manner
    for i in range(1, m + 1):
        for j in range(1, n + 1):

            # check if the current character of text1 and text2 matches
            if text1[i - 1] == text2[j - 1]:
                dp[i][j] = dp[i - 1][j - 1] + 1

            # otherwise, the current character of text1 and text2 don't match
            else:
                dp[i][j] = max(dp[i - 1][j], dp[i][j - 1])

    # find all the longest common sequences
    lcs = LcsHelper(text1, text2, m, n, dp)

    # since a list can contain duplicates, so remove duplicates using set and return unique LCS list
    return list(set(lcs))

#===============================================================================

def generate(n, m):
    # generate all binary strings of length n and m
    nStrings = genStringSet(n)
    mStrings = genStringSet(m)

    # find all LCSs of each pair of strings
    occurrences = {}
    for nString in nStrings:
        for mString in mStrings:
            lcs = lcsSet(nString, mString)
            for l in lcs:
                if l in occurrences:
                    occurrences[l] += 1
                else:
                    occurrences[l] = 1
    
    # sort occurrences by key length
    occurrences = {k: v for k, v in sorted(occurrences.items(), key=lambda item: len(item[0]))}

    # create JSON file with each key-value pair as an array
    with open('Website Code/res/files/' + str(n) + 'x' + str(m) + '.json', 'w') as f:
        f.write("{\n\t\"stringOccurrences\": [\n")
        for key in occurrences:
            f.write("\t\t[\"" + key + "\", " + str(occurrences[key]) + "],\n")
        f.seek(f.tell() - 3, 0) # remove last comma
        f.write("\n\t]\n}")
        f.close()

for i in range(1, 11):
    for j in range(i, 11):
        generate(i, j)