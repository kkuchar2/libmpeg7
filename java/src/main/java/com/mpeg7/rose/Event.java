package com.mpeg7.rose;

import java.util.LinkedList;
import java.util.List;
import java.util.function.Consumer;

public class Event<T> {

    private final List<Consumer<T>> observers = new LinkedList<>();

    public void emit(T t) {
        observers.forEach(observer -> observer.accept(t));
    }

    public Subscription addObserver(Consumer<T> action) {
        observers.add(action);
        return new Subscription(() -> observers.remove(action));
    }

    public int getObserverCount() {
        return observers.size();
    }
}
