#include "hardwareChannelManager.h"
#include "serialPortChannel.h"

std::shared_ptr<HardwareChannel> hardwareChannelManager::createHardwareChannel(HardwareInface type)
{
    if(HardwareInface::EMG_UART == type) {
        return std::make_shared<serialPortChannel>();
    }
    return nullptr;
}