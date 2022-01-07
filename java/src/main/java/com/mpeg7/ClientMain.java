package com.mpeg7;

import com.mpeg7.lib.mpeg7;
import com.mpeg7.network.Command;
import com.mpeg7.network.OutgoingFrame;
import com.mpeg7.network.TcpIncomingFrame;
import com.mpeg7.network.client.TcpClient;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.file.Files;
import java.util.function.Consumer;

public class ClientMain {

    public static byte[] simpleDownload(String urlPath, boolean logProgress) throws IOException {
        System.out.println("Downloading '" + urlPath + "'\n");

        // Create URL
        URL url = new URL(urlPath);

        // Open connection
        HttpURLConnection httpConnection = (HttpURLConnection) (url.openConnection());

        // Get size of content
        long completeFileSize = httpConnection.getContentLength();

        // Download content to byte array
        BufferedInputStream in = new java.io.BufferedInputStream(httpConnection.getInputStream());
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

        int CHUNK_SIZE = 1024 * 4;

        byte[] contentData;
        byte[] chunk = new byte[CHUNK_SIZE];

        int totalDownloadedBytes = 0;
        int currentDowloadedBytes = 0;
        double progress = 0.0;

        while ((currentDowloadedBytes = in.read(chunk, 0, CHUNK_SIZE)) >= 0) {
            outputStream.write(chunk, 0, currentDowloadedBytes);
            totalDownloadedBytes += currentDowloadedBytes;

            if (logProgress) {
                // Display progress
                progress = (float) totalDownloadedBytes / (float) completeFileSize * 100.0;
                System.out.printf("Progress: %.2f %% \n", progress);
            }
        }

        contentData = outputStream.toByteArray();

        outputStream.close();
        in.close();

        return contentData;
    }

    public static void main(String[] args) throws IOException {
        TcpClient client = new TcpClient("0.0.0.0", 5000);
        client.start();

        client.onFrameReceived.addObserver(frame -> {
            if (frame.command == Command.DESCRIBE_IMAGE_RESULT) {
                String xml = new String(frame.data);
                System.out.println(xml);
            }
        });

        File file = new File("/home/kuchkr/Desktop/small.png");
        byte[] fileContent = Files.readAllBytes(file.toPath());

        client.enqueue(OutgoingFrame.of(Command.DESCRIBE_IMAGE, fileContent));
    }
}
