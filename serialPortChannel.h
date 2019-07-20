#pragma once
#include "hardwareChannel.h"
#include <set>
#include <thread>
#include <atomic>
#include <future>

class serialPortChannel : public HardwareChannel {
public:
	void readData();
public:
	serialPortChannel();
	int open(std::string& pathname) override;
	int close() override;
	bool registerListener(std::shared_ptr<HardwareChannelListener> listener) override;
	bool removeListener(std::shared_ptr<HardwareChannelListener> listener) override;
	void run();
	~serialPortChannel() override;

private:
	using T_LISTENER = std::set<std::shared_ptr<HardwareChannelListener>>;
	int mFd;
	std::set<std::shared_ptr<HardwareChannelListener>> mListenSet;
	std::thread mThread;
	std::atomic<bool> mQuit;
};

class chello {
public:
	void read();
	void test();
};