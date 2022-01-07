package com.mpeg7.network.client;

import com.mpeg7.network.*;
import com.mpeg7.rose.Event;
import com.mpeg7.rose.SimpleEvent;

import java.io.*;
import java.net.ConnectException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Objects;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

public class TcpClient extends Thread {

    // How often we should try to connect
    private static final int CONNECTION_RETRY_PERIOD_MS = 500;

    // How long to wait for ping from server
    private static final int PACKET_RECEIVE_TIME_LIMIT_MS = 20000;

    // How big are chunks of bytes that we use to read data of packet
    private static final int DATA_READ_CHUNK_BYTES = 8192;

    // How long we wait for server to ping us
    private static final int SERVER_PING_TIMEOUT_MS = 3000;

    private Socket socket;

    // Input and output streams for socket
    private DataInputStream inputStream;
    private DataOutputStream outputStream;

    // Current reading state that we're in
    private ReadingState state = ReadingState.COMMAND;

    // Current reading packet that we're in
    private Packet packet = new Packet();

    // Queues to store incoming and outgoing frames
    private final Queue<OutgoingFrame> framesToSend = new ConcurrentLinkedQueue<>();
    private final Queue<TcpIncomingFrame> framesToProcess = new ConcurrentLinkedQueue<>();

    // Observable notifying its observers about new received frame
    public Event<TcpIncomingFrame> onFrameReceived = new Event<>();

    private final String host;
    private final int port;

    // Should thread run
    private boolean running = true;

    // Is client connected
    private boolean connected = false;

    // Tracking ping frequency from server
    private boolean firstPingReceived = false;
    private long lastTimePingSent = 0;

    public SimpleEvent onConnected = new SimpleEvent();

    public TcpClient(String host, int port) {
        this.host = host;
        this.port = port;
        new TcpProcessingThread().start();
    }

    @Override
    public void run() {

        try {

            while (running) {

                // Try to connect until connected
                while (!connected && running) {

                    Thread.sleep(CONNECTION_RETRY_PERIOD_MS);

                    System.out.println("Connecting to " + host + ":" + port);

                    try {
                        socket = new Socket();
                        socket.connect(new InetSocketAddress(host, port));
                        connected = true;
                        onConnected.emit();
                    }
                    catch (ConnectException e) {
                        connected = false;
                    }

                    if (socket == null) {
                        connected = false;
                    }
                }

                // Create streams after socket is connected
                if (running) {
                    System.out.println("###############################################################");
                    System.out.println("##  (TCP) Connected to " + host + ":" + port);
                    System.out.println("###############################################################");
                    try {
                        inputStream = new DataInputStream(new BufferedInputStream(socket.getInputStream()));
                        outputStream = new DataOutputStream(new BufferedOutputStream(socket.getOutputStream()));
                    }
                    catch (IOException e) {
                        e.printStackTrace();
                        connected = false;
                        socket.close();
                    }
                }

                // While connected and running
                // 1. Check if server has not forgot us
                // 2. Continue receiving packets and deserializing them to frames
                // 3. Send all current outgoing frames

                while (running && connected) {
                    try {
                        checkServerAvailability();
                        receive();
                        send();
                    }
                    catch (IOException e) {
                        connected = false;

                        if (!socket.isClosed()) {
                            socket.close();
                        }

                        if (!socket.isClosed()) {
                            inputStream.close();
                            outputStream.close();
                        }
                    }
                }
            }
        }
        catch (InterruptedException | IOException e) {
            e.printStackTrace();
        }

        System.out.println("(TCP) Closing socket");

        try {
            if (!Objects.requireNonNull(socket).isClosed()) {
                socket.close();
            }
            if (inputStream != null) {
                inputStream.close();
            }
            if (outputStream != null) {
                outputStream.close();
            }

            connected = false;
        }

        catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * Receive incoming packets from server
     */
    private void receive() throws IOException {

        // How many bytes are available to read
        int available = inputStream.available();

        // Check in what state of reading we're in and act accordingly

        if (state == ReadingState.COMMAND && available >= 1) {
            // New packet started receiving - register start time
            packet.receiveStartTime = System.currentTimeMillis();

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
     * Get frames from outgoing queue, serialize them and send
     */
    private void send() throws IOException {

        while (!framesToSend.isEmpty()) {

            OutgoingFrame frame = framesToSend.poll();

            if (frame != null) {

                if (frame.command != Command.PING && frame.command != Command.PONG) {
                    System.out.println("------> Sending frame: " + frame);
                }
                outputStream.write(frame.serialize());
            }
        }

        outputStream.flush();
    }

    /**
     * Deserialize currently read packet to TcpIncomingFrame and store in incoming frames queue.
     */
    private void handleFullPacketReceived() {
        TcpIncomingFrame frame = (TcpIncomingFrame) packet.deserialize();
        frame.setTimeWhenReceived(System.currentTimeMillis());
        frame.setReceiveDuration(calculatePacketReceiveTime());
        onFrameReceived(frame); // update server ping time synced with current thread
        framesToProcess.add(frame);
        resetReceivingState();
    }

    /**
     * Reset receiving state - receiving will start at command byte
     */
    private void resetReceivingState() {
        packet = new Packet();
        state = ReadingState.COMMAND;
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
     * Check if server has sent PING frame to us in last SERVER_PING_TIMEOUT_MS miliseconds
     */
    private void checkServerAvailability() {
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
    }

    /**
     * Thread that loops over incoming frames queue and processes all frames in there
     */
    public class TcpProcessingThread extends Thread {
        @Override
        public void run() {
            while (running) {
                while (!framesToProcess.isEmpty()) {
                    TcpIncomingFrame frame = framesToProcess.poll();
                    if (frame != null) {
                        // Notify all subscribers
                        onFrameReceived.emit(frame);
                    }

                    if (!running) {
                        framesToProcess.clear();
                        break;
                    }
                }
            }
        }
    }

    /**
     * Enqueue frame to be sent in future
     */
    public void enqueue(OutgoingFrame frame) {
        framesToSend.add(frame);
    }

    /**
     * Kill client thread
     */
    public void kill() {
        running = false;
    }
}