public class Gradient {
    public static int[] generate(int[] stops, int steps) throws Exception {
        int[] colors = new int[steps];

        for (int i = 0; i < steps; i++) {
            colors[i] = getPercent(stops, (double) i / (steps - 1));
        }

        return colors;
    }

    private static int interpolate(int start, int end, double percent) {
        int red = (int) ((start >> 16) * (1 - percent) + (end >> 16) * percent);
        int green = (int) (((start >> 8) & 0xff) * (1 - percent) + ((end >> 8) & 0xff) * percent);
        int blue = (int) ((start & 0xff) * (1 - percent) + (end & 0xff) * percent);
        return (red << 16) + (green << 8) + blue;
    }

    private static int getPercent(int[] stops, double percent) {
        double step = 1.0 / (stops.length - 1);
        for (int i = 0; i < stops.length - 1; i++) {
            // if percent in between stops i and i + 1
            if (percent >= step * i && percent <= step * (i + 1)) {
                return interpolate(stops[i], stops[i + 1], (percent - step * i) / step);
            }
        }
        return -1;
    }
}
