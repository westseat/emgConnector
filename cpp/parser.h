# pragma once

#include <vector>
#include <memory>
#include <string>
#include "interface.h"

class parserListener {
public:
    parserListener(){};
    virtual void onParserData(parserDataType type, void* pData) = 0;
    virtual ~parserListener(){};
};

class parser {
public:
    parser(){};
    virtual void push(std::shared_ptr<std::vector<unsigned char>> data) = 0;
    virtual bool registerListener(std::shared_ptr<parserListener> listener) = 0;
    virtual bool removeListener(std::shared_ptr<parserListener> listener) = 0;
    virtual std::string getParserType() = 0;
    virtual ~parser(){};
};


struct emgData {
    unsigned int ch1Average;
    unsigned int ch1Value;
    unsigned int ch1Power;
    unsigned int ch1Strength;
    unsigned int ch2Average;
    unsigned int ch2Value;
    unsigned int ch2Power;
    unsigned int ch2Strength;
};
