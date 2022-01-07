package com.mpeg7.bridge;

public class libmpeg7 {
    static {
        System.loadLibrary("opencv_core");
        System.loadLibrary("mpeg7");
    }
    public native String extractDescriptor(int desType, String imgPath, String[] params);
    public native String extractDescriptorFromData(int desType, byte[] data, String[] params);
    public native String calculateDistance(String xml1, String xml2, String[] params);
}