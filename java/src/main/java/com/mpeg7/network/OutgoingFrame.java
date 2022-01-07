package com.mpeg7.network;

import java.nio.ByteBuffer;

public class OutgoingFrame extends BaseFrame {

    public OutgoingFrame(Command command, byte[] data) {
        super(command, data);
    }

    public byte[] serialize() {

        byte cmd = command.getValue();
        byte[] dataSize = Util.intToByteArray(data.length);

        ByteBuffer bb = ByteBuffer.allocate(HEADER_SIZE + data.length);
        bb.put(cmd);
        bb.put(dataSize);
        bb.put(data);
        return bb.array();
    }

    public static OutgoingFrame of(Command cmd, byte[] data) {
        return new OutgoingFrame(cmd, data);
    }

    @Override
    public String toString() {
        return "{ command: " + command + ";"
                + " data_size: " + data.length + " bytes; }";
    }
}
