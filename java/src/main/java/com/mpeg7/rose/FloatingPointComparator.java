package com.mpeg7.rose;

public class FloatingPointComparator {

    private static final float EPSILON_FLOAT = 0.000001f;
    private static final double EPSILON_DOUBLE = 0.000001f;

    public static boolean approximately(float a, float b) {

        float absA = Math.abs(a);
        float absB = Math.abs(b);
        float diff = Math.abs(a - b);

        if (a == b) {
            return true;
        }

        if (a == 0.0f || b == 0 || diff < EPSILON_FLOAT) {
            return diff < EPSILON_FLOAT;
        }

        return diff / (absA + absB) < EPSILON_FLOAT;
    }

    public static boolean approximately(double a, double b) {

        double absA = Math.abs(a);
        double absB = Math.abs(b);
        double diff = Math.abs(a - b);

        if (a == b) {
            return true;
        }

        if (a == 0.0f || b == 0 || diff < EPSILON_DOUBLE) {
            return diff < EPSILON_DOUBLE;
        }

        return diff / (absA + absB) < EPSILON_DOUBLE;
    }
}
