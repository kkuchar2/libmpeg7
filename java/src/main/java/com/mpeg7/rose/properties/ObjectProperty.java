package com.mpeg7.rose.properties;

import com.mpeg7.rose.ObservableBase;

public class ObjectProperty<T> extends ObservableBase<T> {

    public ObjectProperty(T t) {
        super(t);
    }

    @Override
    protected boolean equal(T t1, T t2) {
        return t1.equals(t2);
    }
}