#include "hardwareChannelManager.h"
#include "serialPortChannel.h"

std::shared_ptr<hardwareChannel> hardwareChannelManager::createHardwareChannel(hardwareChannelType type)
{
    if(hardwareChannelType::EMG_UART == type) {
        return std::make_shared<serialPortChannel>();
    }
    return nullptr;
}