#pragma once
#include "hardwareChannel.h"
#include <set>
#include <thread>
#include <atomic>
#include <future>

class serialPortChannel : public hardwareChannel {
public:
	void readData();
public:
	serialPortChannel();
	int open(std::string& pathname) override;
	int close() override;
	bool registerListener(std::shared_ptr<hardwareChannelListener> listener) override;
	bool removeListener(std::shared_ptr<hardwareChannelListener> listener) override;
	void run() override;
	~serialPortChannel() override;

private:
	using T_LISTENER = std::set<std::shared_ptr<hardwareChannelListener>>;
	int mFd;
	std::set<std::shared_ptr<hardwareChannelListener>> mListenSet;
	std::thread mThread;
	std::atomic<bool> mQuit;
};
