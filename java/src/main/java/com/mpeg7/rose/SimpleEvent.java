package com.mpeg7.rose;

import java.util.LinkedList;
import java.util.List;

public class SimpleEvent {

    private final List<Action> observers = new LinkedList<>();

    public void emit() {
        observers.forEach(Action::call);
    }

    public Subscription addObserver(Action action) {
        observers.add(action);
        return new Subscription(() -> observers.remove(action));
    }

    public int getObserverCount() {
        return observers.size();
    }
}
