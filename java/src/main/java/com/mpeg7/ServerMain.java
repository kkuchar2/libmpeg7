package com.mpeg7;

import com.mpeg7.network.Command;
import com.mpeg7.network.OutgoingFrame;
import com.mpeg7.network.server.TcpServer;

import java.io.IOException;

public class ServerMain {

    public static void main(String[] args) throws IOException {
        TcpServer server = new TcpServer(5000);
        server.start();
        server.onFrameReceived.addObserver(frame -> {



            if (frame.command == Command.DESCRIBE_IMAGE) {

                System.out.println("<----- Received frame: " + frame);

                server.enqueue(OutgoingFrame.of(Command.DESCRIBE_IMAGE_RESULT, "ABCD".getBytes()), frame.clientId);
            }
        });
    }
}
