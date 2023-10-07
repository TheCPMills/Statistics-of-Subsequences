import javax.imageio.*;
import java.awt.*;
import java.awt.image.*;
import java.io.*;

public class Part3 {
    public static void part3(int n, BufferedImage src) throws Exception {
        int size = 4096;
        BufferedImage img = new BufferedImage(size, size, BufferedImage.TYPE_INT_RGB);
        Graphics2D g = img.createGraphics();
        g.drawImage(src, 0, 0, size, size, null);
        g.dispose();

        // staircase black pixels from bottom left to top right
        int stride = size / (int) Math.pow(2, n - 1);
        final int thickness = 2;

        int x = -thickness / 2;
        int y = size + thickness / 2 - 1;

        int xMax = size + thickness / 2;
        int yMin = -thickness / 2 - 1;
        
        while (x < xMax && y > yMin) {
            drawBox(img, x, y, thickness, thickness, 0xffffffff);
            x += thickness;

            drawBox(img, x, y, stride - thickness, thickness, 0xffffffff); // draw horizontal
            x += stride - thickness;

            drawBox(img, x, y, thickness, thickness, 0xffffffff);
            y -= thickness;

            drawBox(img, x, y, thickness, stride - thickness, 0xffffffff); // draw vertical
            y -= stride - thickness;
        }

        ImageIO.write(img, "png", new File("LCS Matrices/res/matrix-mods/n=" + n + "-SecondQuadrant.png"));
    }

    private static void drawBox(BufferedImage img, int x, int y, int width, int height, int color) {
        for (int i = x; i < x + width; i++) {
            for (int j = y; j > y - height; j--) {
                if (i < 0 || i >= img.getWidth() || j < 0 || j >= img.getHeight()) {
                    continue;
                } else {
                    img.setRGB(i, j, color);
                }
            }
        }
    }
}
