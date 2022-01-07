package com.mpeg7.network;

public abstract class BaseFrame {

    /**
     * Header size is 5 bytes:
     *   1 byte for command
     *   4 bytes for data size sent by server as unsigned int
     */
    protected static final int HEADER_SIZE = 5;

    public Command command;
    public byte[] data;

    protected BaseFrame(Command command, byte[] data) {
        this.command = command;
        this.data = data;
    }
}