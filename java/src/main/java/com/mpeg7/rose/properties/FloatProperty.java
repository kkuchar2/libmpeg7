package com.mpeg7.rose.properties;

import com.mpeg7.rose.FloatingPointComparator;
import com.mpeg7.rose.ObservableBase;

public class FloatProperty extends ObservableBase<Float> {

    public FloatProperty(float v) {
        super(v);
    }

    @Override
    protected boolean equal(Float f1, Float f2) {
        return FloatingPointComparator.approximately(f1, f2);
    }
}