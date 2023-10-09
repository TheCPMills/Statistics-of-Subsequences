import java.util.*;
import javax.imageio.*;
import java.awt.Graphics2D;
import java.awt.image.*;
import java.io.*;

public class Part1 {
    public static void main(String[] args) throws Exception {
        int maxN = 10;
        FileWriter writer = new FileWriter(new File("res/matrix-mods/irregularities.txt"));
        part1(maxN, writer);
        writer.close();
    }

    private static void part1(int maxN, FileWriter writer) throws Exception {
        int[] colors = Gradient.generate(new int[]{0xfde724, 0x79d151, 0x29788e, 0x404387, 0x440154}, maxN + 1);
        int[] editedColors = Gradient.generate(new int[]{0xf75757, 0xf5a173, 0xf5d073, 0xdf73f5, 0x805ddf, 0x5f7ce6, 0x44cbe8}, maxN + 1);

        HashMap<Integer, Integer> decrementedColorsMap = new HashMap<>();
        for(int i = 0; i < colors.length - 1; i++) {
            decrementedColorsMap.put(0xff000000 + colors[i + 1], 0xff000000 + colors[i]);
        }

        HashMap<Integer, Integer> editedColorsMap = new HashMap<>();
        for(int i = 0; i < colors.length; i++) {
            editedColorsMap.put(0xff000000 + colors[i], 0xff000000 + editedColors[i]);
        }

        for (int n = 1; n <= maxN; n++) {
            BufferedImage img = new BufferedImage((int) Math.pow(2, n), (int) Math.pow(2, n), BufferedImage.TYPE_INT_RGB);
            List<String> strings = genStringSet(n);
			boolean[][] usingFirstCharOfB = new boolean[strings.size() / 2][strings.size() / 2];
            boolean[][] longestCommonSubsequenceOfTailsOfAAndBPresentAfterFirstOccurenceOfOneInA = new boolean[strings.size() / 2][strings.size() / 2];
            for(int i = 0; i < strings.size(); i++) {
                for(int j = 0; j < strings.size(); j++) {
                    int lcs = lcs_dynamic_programming(strings.get(i), strings.get(j));

					if (i < Math.pow(2, n - 1) && j < Math.pow(2, n - 1)) {
						usingFirstCharOfB[i][(int) Math.pow(2, n - 1) - j - 1] = usesFirstCharOfB(strings.get(i), strings.get(j));
                        longestCommonSubsequenceOfTailsOfAAndBPresentAfterFirstOccurenceOfOneInA[i][(int) Math.pow(2, n - 1) - j - 1] = LCSoToAaBPAFOoOiA(strings.get(i), strings.get(j));
					}

                    int color = colors[lcs];
                    img.setRGB(i, (int) Math.pow(2, n) - j - 1, color);
                }
            }

            int size = 4096;
            BufferedImage resized = new BufferedImage(size, size, BufferedImage.TYPE_INT_RGB);
            Graphics2D g = resized.createGraphics();
            g.drawImage(img, 0, 0, size, size, null);
            g.dispose();
            ImageIO.write(resized, "png", new File("res/matrix/n=" + n + ".png"));

            Part2.part2(n, img, decrementedColorsMap, editedColorsMap, usingFirstCharOfB, longestCommonSubsequenceOfTailsOfAAndBPresentAfterFirstOccurenceOfOneInA, writer);
        }
    }

    static int lcs_dynamic_programming(String s1, String s2) {
        int m = s1.length();
        int n = s2.length();
        int[][] dp = new int[m + 1][n + 1];

        for(int i = 0; i <= m; i++) {
            for(int j = 0; j <= n; j++) {
                if(i == 0 || j == 0) {
                    dp[i][j] = 0;
                } else if(s1.charAt(i - 1) == s2.charAt(j - 1)) {
                    dp[i][j] = 1+dp[i - 1][j - 1];
                } else {
                    dp[i][j] = Math.max(
                        dp[i - 1][j],
                        dp[i][j - 1]
                    );
                }
            }
        }

        return dp[m][n];
    }

    static List<String> genStringSet(int length) {
        // Generate all binary strings of length n
        List<String> strings = new ArrayList<>();
        for(int i=0; i<Math.pow(2, length); i++) {
            String s = Integer.toBinaryString(i);
            while(s.length() < length) {
                s = "0" + s;
            }
            strings.add(s);
        }
        return strings;
    }

    //===============================================================================

    static List<String> LcsHelper(String s1, String s2, int m, int n, int[][] dp) {
        // check if the end of either sequence is reached or not
        if (m == 0 || n == 0) {
            // create a list with one empty string and return
            return new ArrayList<>(Arrays.asList(""));
        }
        
        // check if the last character of s1 and s2 matches
        if (s1.charAt(m - 1) == s2.charAt(n - 1)) {
            // ignore the last characters of s1 and s2 and find all LCS of substring s1[0 … m-2], s2[0 … n-2] and store it in a list
            List<String> lcs = LcsHelper(s1, s2, m - 1, n - 1, dp);
        
            // append current character s1[m-1] or s2[n-1] to all LCS of substring s1[0 … m-2] and s2[0 … n-2]
            for (int i = 0;  i < lcs.size(); i++) {
                lcs.set(i, lcs.get(i) + s1.charAt(m - 1));
            }
            
            return lcs;
        }
        
        // we will reach here when the last character of s1 and s2 don't match
        
        // if a top cell of the current cell has more value than the left cell,
        // then ignore the current character of string s1 and find all LCS of substring s1[0 … m-2], s2[0 … n-1]
        if (dp[m - 1][n] > dp[m][n - 1]) {
            return LcsHelper(s1, s2, m - 1, n, dp);
        }
        
        // if a left cell of the current cell has more value than the top cell,
        // then ignore the current character of string s2 and find all LCS of substring s1[0 … m-1], s2[0 … n-2]
        if (dp[m][n - 1] > dp[m - 1][n]) {
            return LcsHelper(s1, s2, m, n - 1, dp);
        }
        
        // if the top cell has equal value to the left cell, then consider both characters
        List<String> top = LcsHelper(s1, s2, m - 1, n, dp);
        List<String> left = LcsHelper(s1, s2, m, n - 1, dp);

        // merge two lists and return
        List<String> resultList = new ArrayList<>(top);
        resultList.addAll(left);
        return resultList;
    }

    static Set<String> lcsSet(String s1, String s2) {
        // calculate length of s1 and s2
        int m = s1.length();
        int n = s2.length();

        // dp[i][j] stores the length of LCS of substring s1[0 … i-1] and s2[0 … j-1]
        int[][] dp = new int[m + 1][n + 1];
        
        // fill the lookup dp table in a bottom-up manner
        for (int i = 1; i < m + 1; i++) {
            for (int j = 1; j < n + 1; j++) {
                // check if the current character of s1 and s2 matches
                if (s1.charAt(i - 1) == s2.charAt(j - 1)) {
                    dp[i][j] = dp[i - 1][j - 1] + 1;
                } else { // otherwise, the current character of s1 and s2 don't match
                    dp[i][j] = Math.max(dp[i - 1][j], dp[i][j - 1]);
                }
            }
        }

        // find all the longest common sequences
        List<String> lcs = LcsHelper(s1, s2, m, n, dp);
        
        // since a list can contain duplicates, so remove duplicates using set and return unique LCS list
        return new HashSet<>(lcs);
    }

	static boolean containsSubsequence(String s, String subsequence) {
		int i = 0;
		int j = 0;
		while (i < s.length() && j < subsequence.length()) {
			if (s.charAt(i) == subsequence.charAt(j)) {
				j++;
			}
			i++;
		}
		return j == subsequence.length();
	}

	static boolean usesFirstCharOfB(String s1, String s2) {
		Set<String> lcsSet = lcsSet(s1, s2);

		String tailOfB = s2.substring(1);
		for (String lcs : lcsSet) {
			if (containsSubsequence(tailOfB, lcs)) {
				return false;
			}
		}
		return true;
	}

    static boolean LCSoToAaBPAFOoOiA(String s1, String s2) {
        String tailOfB = s2.substring(1);

        // find first index of 1 in s1
        int firstOneInA = s1.indexOf('1');

        // if there is a 1 in s1
        if (firstOneInA != -1) {
            // get the substring of s1 after the first 1
            String newA = s1.substring(firstOneInA + 1);

            // find all LCS of A and tailOfB
            Set<String> lcsSet = lcsSet(s1, tailOfB);
            for (String lcs : lcsSet) {
                // if the LCS is a subsequence of newA, then return false
                if (containsSubsequence(newA, lcs)) {
                    return true;
                }
            }
        }

        return false;
    }
}
