import java.awt.image.*;
import java.util.*;
import java.io.*;

public class Part2 {
    public static void part2(int n, BufferedImage src, HashMap<Integer, Integer> decrementedColorsMap, HashMap<Integer, Integer> editedColorsMap, boolean[][] usingFirstCharOfB, boolean[][] longestCommonSubsequenceOfAAndTailOfBPresentAfterFirstOccurenceOfOneInA, FileWriter writer) throws Exception {
        BufferedImage img = new BufferedImage(src.getWidth() / 2, src.getHeight() / 2, BufferedImage.TYPE_INT_RGB);

        double gamma = 3.0;

        // proposition 4
        for (int i = 0; i < src.getWidth() / 2; i++) {
            for (int j = 0; j < src.getHeight() / 2; j++) {
                int topColor = editedColorsMap.get(src.getRGB(i, j));
                int bottomColor;

                if (usingFirstCharOfB[i][j] && !longestCommonSubsequenceOfAAndTailOfBPresentAfterFirstOccurenceOfOneInA[i][j]) {
                    bottomColor = editedColorsMap.get(decrementedColorsMap.get(src.getRGB(i, (int) (j + Math.pow(2, n - 1))))); // proposition 5
                } else {
                    bottomColor = editedColorsMap.get(src.getRGB(i, (int) (j + Math.pow(2, n - 1)))); // proposition 4
                }
                
                int red = (int) Math.abs(((topColor & 0xff0000) >> 16) - ((bottomColor & 0xff0000) >> 16));
                int green = (int) Math.abs(((topColor & 0x00ff00) >> 8) - ((bottomColor & 0x00ff00) >> 8));
                int blue = (int) Math.abs((topColor & 0x0000ff) - (bottomColor & 0x0000ff));

                double sRed = red / 255.0;
                double sGreen = green / 255.0;
                double sBlue = blue / 255.0;

                sRed = Math.pow(sRed, 1 / gamma);
                sGreen = Math.pow(sGreen, 1 / gamma);
                sBlue = Math.pow(sBlue, 1 / gamma);

                red = (int) (sRed * 255);
                green = (int) (sGreen * 255);
                blue = (int) (sBlue * 255);

                int pixelColor = (red << 16) + (green << 8) + blue;
                img.setRGB(i, j, (int) pixelColor);
            }
        }

        Part3.part3(n, img);
        Part4.part4(n, img, writer);
    }
}
