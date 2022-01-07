package com.mpeg7.rose;

import java.util.LinkedList;
import java.util.List;

public class SubscriptionContainer {

    private final List<Subscription> subscriptions = new LinkedList<>();

    public void addSubscription(Subscription subscription) {
        subscriptions.add(subscription);
    }

    public void unsubscribe(Subscription subscription) {
        subscription.unsubscribe();
        subscriptions.remove(subscription);
    }

    public void unsubscribe() {
        subscriptions.forEach(Subscription::unsubscribe);
        subscriptions.clear();
    }
}
