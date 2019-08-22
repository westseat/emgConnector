#pragma once

#include "interface.h"
#include "parser.h"

class parserManager {
public:
    static std::shared_ptr<parser> createParser(parserDataType type);
    ~parserManager() = delete;
};