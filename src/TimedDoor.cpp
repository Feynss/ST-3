// Copyright 2021 GHA Test Team
#include "TimedDoor.h"

#include <chrono>
#include <stdexcept>
#include <thread>

DoorTimerAdapter::DoorTimerAdapter(TimedDoor& timedDoor) : door(timedDoor) {}

void DoorTimerAdapter::Timeout() {
    if (door.isDoorOpened()) {
        door.isThrowen = true;
    }
}

TimedDoor::TimedDoor(int timeoutVal) : 
    adapter(new DoorTimerAdapter(*this)), 
    iTimeout(timeoutVal), isOpened(false) {}

TimedDoor::~TimedDoor() {
    if (timer_thread) {
        timer_thread->join();
        delete timer_thread;
    }
}

bool TimedDoor::isDoorOpened() {
    return isOpened;
}

void TimedDoor::unlock() {
    isOpened = true;
    timer_thread = new std::thread([this]() {
        Timer timer;
        timer.tregister(iTimeout, adapter);
    });
}

void TimedDoor::lock() {
    isOpened = false;
    if (timer_thread) {
        timer_thread->join();
        delete timer_thread;
    }
    timer_thread = nullptr;
}

int TimedDoor::getTimeOut() const {
    return iTimeout;
}

void TimedDoor::throwState() {
    if (isThrowen) {
        throw  std::runtime_error("Door is still open");
    }
}

void Timer::sleep(int timeout) {
    if (timeout > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    }
}

void Timer::tregister(int timeout, TimerClient *timerClient) {
    client = timerClient;
    if (timeout > 0) {
        sleep(timeout);
    }
    if (client != nullptr) {
        client->Timeout();
    }
}
