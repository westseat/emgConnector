#pragma once
#include <memory>
#include <vector>
#include <string>
#include "interface.h"

struct hardwareChannelData {
	hardwareChannelType type;
	std::shared_ptr<std::vector<unsigned char>> rawData;
};

class hardwareChannelListener {
public:
	hardwareChannelListener() {}
	virtual void onOpen() = 0;
	virtual void onClose() = 0;
	virtual void onData(std::shared_ptr<hardwareChannelData> dataPtr) = 0;
	virtual ~hardwareChannelListener() {};
};

class hardwareChannel {
public:
	hardwareChannel() {};
	virtual int open(std::string& pathname) = 0;
	virtual int close() = 0;
	virtual void run() = 0;
	virtual bool registerListener(std::shared_ptr<hardwareChannelListener> listener) = 0;
	virtual bool removeListener(std::shared_ptr<hardwareChannelListener> listener) = 0;
	virtual ~hardwareChannel() {};
};
