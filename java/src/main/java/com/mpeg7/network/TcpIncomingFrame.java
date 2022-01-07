package com.mpeg7.network;

import java.util.UUID;

public class TcpIncomingFrame extends BaseFrame {

    private long whenReceived;
    private long receivingTime;
    public UUID clientId;

    public TcpIncomingFrame(UUID clientId, Command command, byte[] data) {
        super(command, data);
        this.clientId = clientId;
    }

    public void setTimeWhenReceived(long time) {
        whenReceived = time;
    }

    public void setReceiveDuration(long receivingTime) {
        this.receivingTime = receivingTime;
    }

    @Override
    public String toString() {
        return " { command: " + command + ";"
                + " clientId: " + clientId +
                  " data_size: " + data.length + " bytes;" +
                  " when_received: " + whenReceived + ";" +
                  " receiving_time: " + receivingTime + " ms }";
    }
}