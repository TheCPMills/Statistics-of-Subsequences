import java.util.*;
import javax.imageio.*;
import java.awt.Graphics2D;
import java.awt.image.*;
import java.io.*;

public class Part1 {
    public static void main(String[] args) throws Exception {
        int maxN = 10;
        part1(maxN);
    }

    private static void part1(int maxN) throws Exception {
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
            for(int i = 0; i < strings.size(); i++) {
                for(int j = 0; j < strings.size(); j++) {
                    int lcs = lcs_dynamic_programming(strings.get(i), strings.get(j));
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

            Part2.part2(n, img, decrementedColorsMap, editedColorsMap);
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
}
