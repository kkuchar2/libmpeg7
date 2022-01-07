package com.mpeg7.network;

import java.nio.ByteOrder;

public class Util {
    public static int byteArrayToInt(byte[] rawDataSize, ByteOrder littleEndian) {
        return rawDataSize[3] & 0xFF |
                (rawDataSize[2] & 0xFF) << 8 |
                (rawDataSize[1] & 0xFF) << 16 |
                (rawDataSize[0] & 0xFF) << 24;
    }


    public static byte[] intToByteArray(int value) {
        return new byte[] {
                (byte) (value >>> 24),
                (byte) (value >>> 16),
                (byte) (value >>> 8),
                (byte) value};
    }
}