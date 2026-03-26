// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <stdexcept>
#include <thread>
#include "TimedDoor.h"

class MockTimerClient : public TimerClient {
 public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
 public:
    MOCK_METHOD(bool, isDoorOpened, (), (override));
    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
};

class TimedDoorTest : public ::testing::Test {
 protected:
    TimedDoor* door;
    DoorTimerAdapter* adapter;

    void SetUp() override {
        door = new TimedDoor(500);
        adapter = new DoorTimerAdapter(*door);
        door->lock();
    }

    void TearDown() override {
        delete adapter;
        delete door;
    }
};


TEST_F(TimedDoorTest, HasTimeoutVal) {
    EXPECT_EQ(500, door->getTimeOut());
}

TEST_F(TimedDoorTest, InitiallyClosedDoor) {
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, IsDoorOpenedAfterOpening) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, IsDoorLockedAfterLockingOpened) {
    door->unlock();
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, IsDoorLockedAfterRepeatedlyBeingOpened) {
    door->unlock();
    door->lock();
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, NoThrowForClosedDoor) {
    EXPECT_NO_THROW(door->throwState());
}

TEST_F(TimedDoorTest, DoorOpenedForTooLong) {
    door->unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, DoorOpenedForNotTooLong) {
    door->unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    EXPECT_NO_THROW(door->throwState());
    door->lock();
}

TEST_F(TimedDoorTest, AdapterCallsTimeout) {
    Timer timer;
    MockTimerClient mockClient;

    EXPECT_CALL(mockClient, Timeout()).Times(1);
    timer.tregister(1, &mockClient);
}

TEST_F(TimedDoorTest, MockDoorState) {
    MockDoor mockDoor;
    EXPECT_CALL(mockDoor, isDoorOpened()).WillOnce(::testing::Return(false));
    EXPECT_FALSE(mockDoor.isDoorOpened());
}

TEST_F(TimedDoorTest, MockDoorLock) {
    MockDoor mockDoor;
    EXPECT_CALL(mockDoor, lock()).Times(1);
    mockDoor.lock();
}

TEST_F(TimedDoorTest, MockDoorUnlock) {
    MockDoor mockDoor;
    EXPECT_CALL(mockDoor, unlock()).Times(1);
    EXPECT_CALL(mockDoor, isDoorOpened()).WillOnce(::testing::Return(true));
    mockDoor.unlock();
    EXPECT_TRUE(mockDoor.isDoorOpened());
}

TEST_F(TimedDoorTest, RegitsterWithNullDoesntThrow) {
    Timer timer;
    EXPECT_NO_THROW(timer.tregister(0, nullptr));
}

TEST_F(TimedDoorTest, MultipleAdapters) {
    TimedDoor door(0);

    door.lock();

    DoorTimerAdapter ad1(door);
    DoorTimerAdapter ad2(door);

    EXPECT_NO_THROW(ad1.Timeout());
    EXPECT_NO_THROW(ad2.Timeout());

    door.unlock();

    ad1.Timeout();
    EXPECT_THROW(door.throwState(), std::runtime_error);
    ad2.Timeout();
    EXPECT_THROW(door.throwState(), std::runtime_error);
}
