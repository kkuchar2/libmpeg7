package com.mpeg7;

import com.mpeg7.lib.DescriptorType;
import com.mpeg7.lib.LibraryException;
import com.mpeg7.lib.mpeg7;
import com.mpeg7.network.Command;
import com.mpeg7.network.OutgoingFrame;
import com.mpeg7.network.client.TcpClient;
import com.mpeg7.network.server.TcpServer;

import java.io.IOException;

public class Main {

    public static void test() {
        byte[] data1 = null;
        byte[] data2 = null;
        String libPathProperty = System.getProperty("java.library.path");
        System.out.println("java.library.path:");
        System.out.println(libPathProperty);
        try {
            data1 = mpeg7.simpleDownload("https://upload.wikimedia.org/wikipedia/en/7/7d/Lenna_%28test_image%29.png", true);
            data2 = mpeg7.simpleDownload("https://upload.wikimedia.org/wikipedia/en/7/7d/Lenna_%28test_image%29.png", true);
        }
        catch (IOException e) {
            e.printStackTrace();
        }

        String xml1 = null;
        String xml2 = null;

        try {
            xml1 = mpeg7.extractDescriptorFromData(DescriptorType.SCALABLE_COLOR_D, data1,
                    new String[] {"NumberOfCoefficients", "256", "NumberOfBitplanesDiscarded", "0"});
            xml2 = mpeg7.extractDescriptorFromData(DescriptorType.SCALABLE_COLOR_D, data2,
                    new String[] {"NumberOfCoefficients", "256", "NumberOfBitplanesDiscarded", "0"});
        }
        catch (LibraryException e) {
            e.printStackTrace();
        }

        double distance = -1;

        try {
            distance = mpeg7.calculateDistance(xml1, xml2, new String[] {});
        }
        catch (LibraryException e) {
            e.printStackTrace();
        }
        System.out.println(xml1);
        System.out.println(xml2);
        System.out.println(distance);
    }

    public static void main(String[] args) {
        TcpServer server = new TcpServer(5000);
        server.start();
        server.onFrameReceived.addObserver((frame) -> {
            if (frame.command == Command.DESCRIBE_IMAGE) {

                String xml;

                try {
                    xml = mpeg7.extractDescriptorFromData(DescriptorType.SCALABLE_COLOR_D, frame.data,
                            new String[] {"NumberOfCoefficients", "256", "NumberOfBitplanesDiscarded", "0"});
                    System.out.println(xml);

                    server.enqueue(OutgoingFrame.of(Command.DESCRIBE_IMAGE_RESULT, xml.getBytes()), frame.clientId);
                }
                catch (LibraryException e) {
                    e.printStackTrace();
                }
            }
        });
    }
}
