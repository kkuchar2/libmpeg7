package com.mpeg7.network;

import java.nio.ByteBuffer;
import java.util.UUID;

public class Packet {

    public long receiveStartTime;

    public UUID clientId;
    public byte command;
    public int dataSize;
    public ByteBuffer data;

    public BaseFrame deserialize() {
        return Command.of(command).map(command
            -> new TcpIncomingFrame(clientId, command, data.array()))
                .orElseGet(()
            -> new TcpIncomingFrame(clientId, Command.UNKNOWN, data.array()));
    }
}