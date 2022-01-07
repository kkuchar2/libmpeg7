package com.mpeg7.rose.properties;

import com.mpeg7.rose.ObservableBase;

public class IntegerProperty extends ObservableBase<Integer> {

    public IntegerProperty(int v) {
        super(v);
    }

    @Override
    protected boolean equal(Integer f1, Integer f2) {
        return f1.equals(f2);
    }
}