package com.mpeg7.network;

import java.util.Arrays;
import java.util.Optional;

public enum Command {
    NONE(0),
    WHO_ARE_YOU(1),
    GET_NET_CONFIG(2),
    UNKNOWN(4),
    PING(5),
    PONG(6),
    DESCRIBE_IMAGE(7),
    DESCRIBE_IMAGE_RESULT(8),
    ;

    private final int value;

    Command(int value) {
        this.value = value;
    }

    public byte getValue() {
        return (byte) value;
    }

    public static Optional<Command> of(int value) {
        return Arrays.stream(Command.values())
                .filter(v -> v.getValue() == value).findFirst();
    }
}