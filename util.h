#pragma once

#include "parser.h"
#include <string>
#include <memory>

namespace util {
    std::shared_ptr<std::string> emgData2JsonString(const emgData& data);
}