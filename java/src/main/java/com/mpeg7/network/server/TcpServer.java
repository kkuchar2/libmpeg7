package com.mpeg7.network.server;

import com.mpeg7.network.*;
import com.mpeg7.rose.Event;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.*;
import java.util.concurrent.ConcurrentLinkedQueue;

public class TcpServer extends Thread {

    private final int port;

    private ServerSocket serverSocket;


    // How big are chunks of bytes that we use to read data of packet
    private static final int DATA_READ_CHUNK_BYTES = 8192;

    private final Queue<TcpIncomingFrame> framesToProcess = new ConcurrentLinkedQueue<>();

    private int clientCount = 0;

    public Event<TcpIncomingFrame> onFrameReceived = new Event<>();

    // Queues to store incoming and outgoing frames

    private Map<UUID, ClientServiceThread> threadMap = new HashMap<UUID, ClientServiceThread>();

    public TcpServer(int port) {
        System.out.println("Starting server on port: " + port);
        this.port = port;
        new TcpServer.TcpProcessingThread().start();
    }

    public class TcpProcessingThread extends Thread {
        @Override
        public void run() {
            while (true) {
                while (!framesToProcess.isEmpty()) {
                    TcpIncomingFrame frame = framesToProcess.poll();
                    if (frame != null) {
                        // Notify all subscribers
                        onFrameReceived.emit(frame);
                    }
                }
            }
        }
    }

    public class ClientServiceThread extends Thread {

        private final Queue<OutgoingFrame> framesToSend = new ConcurrentLinkedQueue<>();

        private final Socket connectionSocket;
        private final UUID clientId;
        private DataInputStream inputStream;
        private DataOutputStream outputStream;

        private long lastTimePingSent = 0;

        // Current reading packet that we're in
        private Packet packet = new Packet();

        // Should thread run
        private boolean running = true;

        // Current reading state that we're in
        private ReadingState state = ReadingState.COMMAND;


        public ClientServiceThread(Socket connectionSocket, UUID clientId) {
            this.connectionSocket = connectionSocket;
            this.clientId = clientId;
            System.out.println("New client connected: " + clientId);
        }

        long lastTimePongReceived = System.currentTimeMillis();

        @Override
        public void run() {
            try {
                try {
                    inputStream = new DataInputStream(new BufferedInputStream(connectionSocket.getInputStream()));
                    outputStream = new DataOutputStream(new BufferedOutputStream(connectionSocket.getOutputStream()));

                    while (running) {
                        receive(inputStream);
                        send(outputStream);
                        checkClientAvailability();
                    }
                }
                catch (IOException e) {
                    System.out.println("Client disconnected");
                    if (!connectionSocket.isClosed()) {
                        connectionSocket.close();
                    }

                    if (!connectionSocket.isClosed()) {
                        inputStream.close();
                        outputStream.close();
                    }
                }
            }
            catch (IOException e) {
                e.printStackTrace();
            }
        }

        public void enqueue(OutgoingFrame frame) {
            framesToSend.add(frame);
        }

        private void receive(DataInputStream inputStream) throws IOException {

            // How many bytes are available to read
            int available = inputStream.available();
            // Check in what state of reading we're in and act accordingly

            if (state == ReadingState.COMMAND && available >= 1) {
                // New packet started receiving - register start time
                packet.receiveStartTime = System.currentTimeMillis();
                packet.clientId = clientId;
                // Read command byte
                packet.command = inputStream.readByte();
                state = ReadingState.DATA_SIZE;
            }
            else if (state == ReadingState.DATA_SIZE && available >= 4) {
                // Read data size
                byte[] rawDataSize = new byte[4];
                inputStream.readFully(rawDataSize);
                packet.dataSize = Util.byteArrayToInt(rawDataSize, ByteOrder.LITTLE_ENDIAN);
                packet.data = ByteBuffer.allocate(packet.dataSize);
                state = ReadingState.DATA;
            }
            else if (state == ReadingState.DATA) {

                // Read data by chunks
                int remaining = packet.data.remaining();

                if (remaining > 0) {
                    if (remaining >= DATA_READ_CHUNK_BYTES && available >= DATA_READ_CHUNK_BYTES) {
                        byte[] chunk = new byte[DATA_READ_CHUNK_BYTES];
                        inputStream.readFully(chunk);
                        packet.data.put(chunk);
                    }
                    else if (remaining < DATA_READ_CHUNK_BYTES && available >= remaining) {
                        byte[] chunk = new byte[remaining];
                        inputStream.readFully(chunk);
                        packet.data.put(chunk);
                    }
                }
                else {
                    handleFullPacketReceived();
                    return;
                }

                if (packet.data.remaining() == 0) {
                    handleFullPacketReceived();
                }
            }
        }

        /**
         * Deserialize currently read packet to TcpIncomingFrame and store in incoming frames queue.
         */
        private void handleFullPacketReceived() {

            TcpIncomingFrame frame = (TcpIncomingFrame) packet.deserialize();
            frame.setTimeWhenReceived(System.currentTimeMillis());
            frame.setReceiveDuration(calculatePacketReceiveTime());
            framesToProcess.add(frame);
            onFrameReceived(frame);
            resetReceivingState();
        }

        private void checkClientAvailability() {
            long diff = System.currentTimeMillis() - lastTimePingSent;

            if (diff > 1000) {
                lastTimePingSent = System.currentTimeMillis();
                enqueue(OutgoingFrame.of(Command.PING, new byte[] {}));
            }
        }

        /**
         * Internal additional method handling received frame.
         * It's separate from processing thread to check server ping time properly
         * (synced with receiving thread)
         */
        private void onFrameReceived(TcpIncomingFrame frame) {
            if (frame.command == Command.PING) {
                enqueue(OutgoingFrame.of(Command.PONG, new byte[] {}));
            }
            else if (frame.command == Command.PONG) {
                lastTimePongReceived = System.currentTimeMillis();
            }
        }

        /**
         * Get frames from outgoing queue, serialize them and send
         */
        private void send(DataOutputStream outputStream) throws IOException {

            while (!framesToSend.isEmpty()) {
                OutgoingFrame frame = framesToSend.poll();

                if (frame.command != Command.PING && frame.command != Command.PONG) {
                    System.out.println("------> Sending frame: " + frame);
                }
                outputStream.write(frame.serialize());
            }

            outputStream.flush();
        }


        /**
         * Calculate time that took packet to receive from first packet byte received time
         */
        private long calculatePacketReceiveTime() {
            long receiveStartTime = packet.receiveStartTime;
            long currentTime = System.currentTimeMillis();
            return currentTime - receiveStartTime;
        }

        /**
         * Reset receiving state - receiving will start at command byte
         */
        private void resetReceivingState() {
            packet = new Packet();
            state = ReadingState.COMMAND;
        }

        /**
         * Enqueue frame to be sent in future
         */
    }

    public void enqueue(OutgoingFrame frame, UUID clientId) {
        threadMap.get(clientId).enqueue(frame);
    }

    @Override
    public void run() {

        try {
            serverSocket = new ServerSocket(this.port);

            while (true) {
                Socket newSocket = serverSocket.accept();
                UUID clientId = UUID.randomUUID();
                ClientServiceThread newThread = new ClientServiceThread(newSocket, clientId);
                threadMap.put(clientId, newThread);
                newThread.start();
            }
        }
        catch (IOException e) {
            e.printStackTrace();
        }
    }
}