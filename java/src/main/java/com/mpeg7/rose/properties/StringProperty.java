package com.mpeg7.rose.properties;

import com.mpeg7.rose.ObservableBase;

public class StringProperty extends ObservableBase<String> {

    public StringProperty(String v) {
        super(v);
    }

    @Override
    protected boolean equal(String s1, String s2) {
        return s1.equals(s2);
    }
}