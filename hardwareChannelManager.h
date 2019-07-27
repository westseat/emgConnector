#pragma once

#include "interface.h"
#include "hardwareChannel.h"

class hardwareChannelManager {
public:
    static std::shared_ptr<HardwareChannel> createHardwareChannel(hardwareChannelType type);
    ~hardwareChannelManager() = delete;
};