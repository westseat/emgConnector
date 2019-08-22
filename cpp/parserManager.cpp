#include "parserManager.h"
#include "emgParser.h"

std::shared_ptr<parser> parserManager::createParser(parserDataType type)
{
    if(parserDataType::EMG_DATA == type) {
        return std::make_shared<emgParser>();
    }
    return nullptr;
}