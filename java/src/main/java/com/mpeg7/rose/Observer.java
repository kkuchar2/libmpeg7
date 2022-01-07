package com.mpeg7.rose;

public interface Observer<T> {
    void call(T t);
}