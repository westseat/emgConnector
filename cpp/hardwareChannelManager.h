#pragma once

#include "interface.h"
#include "hardwareChannel.h"

class hardwareChannelManager {
public:
    static std::shared_ptr<hardwareChannel> createHardwareChannel(hardwareChannelType type);
    ~hardwareChannelManager() = delete;
};