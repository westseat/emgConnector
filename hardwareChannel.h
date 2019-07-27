#pragma once
#include <memory>
#include <vector>
#include <string>
#include "interface.h"

struct HardwareChannelData {
	HardwareInface type;
	std::shared_ptr<std::vector<unsigned char>> rawData;
};

class HardwareChannelListener {
public:
	HardwareChannelListener() {}
	virtual void onOpen() = 0;
	virtual void onClose() = 0;
	virtual void onData(std::shared_ptr<HardwareChannelData> dataPtr) = 0;
	virtual ~HardwareChannelListener() {};
};

class HardwareChannel {
public:
	HardwareChannel() {};
	virtual int open(std::string& pathname) = 0;
	virtual int close() = 0;
	virtual bool registerListener(std::shared_ptr<HardwareChannelListener> listener) = 0;
	virtual bool removeListener(std::shared_ptr<HardwareChannelListener> listener) = 0;
	virtual ~HardwareChannel() {};
};
