# pragma once

#include "parser.h"
#include <set>

enum class emgDataStatus : unsigned char {
    MAGIC_NUM_1 = 0x01,
    MAGIC_NUM_2,
    FIRST_FLAG,
    SECOND_FLAG,
    THIRD_FLAG,
    FOURTH_FLAG,
    FIFTH_FLAG,
    SIXTH_FLAG,
    SEVENTH_FLAG,
    UNDEFINED
};

class emgParser : public parser {
public:
    emgParser(){}
    void push(std::shared_ptr<std::vector<unsigned char>> data) override;
    bool registerListener(std::shared_ptr<parserListener> listener) override;
    bool removeListener(std::shared_ptr<parserListener> listener) override;
    std::string getParserType() override;
    ~emgParser() override;
private:
    std::string mParserType;
    std::vector<unsigned char> mCache;
    emgData mEmgData;
    std::set<std::shared_ptr<parserListener>> mListener;
    emgDataStatus mFlag;
};
