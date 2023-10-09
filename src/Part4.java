import java.awt.image.*;
import java.io.*;

public class Part4 {
    public static void part4(int n, BufferedImage src, FileWriter writer) throws Exception {
        int width = src.getWidth();
        int height = src.getHeight();

        boolean irregularityPresent = false;
        writer.write("======== n = " + n + " ========\n");
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                // if the pixel is not black
                if (src.getRGB(i, j) != 0xff000000) {
                    // write the pixel's coordinates to the file in binary
                    String s1 = Integer.toBinaryString(i);
                    String s2 = Integer.toBinaryString(height - j - 1);

                    // pad the strings with 0s
                    while (s1.length() < n) {
                        s1 = "0" + s1;
                    }
                    while (s2.length() < n) {
                        s2 = "0" + s2;
                    }

                    writer.write(s1 + " " + s2 + "\n");
                    irregularityPresent = true;
                }
            }
        }
        if (!irregularityPresent) {
            writer.write("No irregularities found.\n");
        }
        writer.write("\n");
    }
}