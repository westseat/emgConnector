#include "emgParser.h"

emgParser::emgParser() 
{
    mFlag = emgDataStatus::UNDEFINED;
}

void emgParser::push(std::shared_ptr<std::vector<unsigned char>> data) 
{
    for(unsigned int i = 0; i < data->size(); ++i) {
        if(emgDataStatus::UNDEFINED == mFlag) {
            if((*data)[i] != 0x0b)
                continue;
            mFlag = emgDataStatus::MAGIC_NUM_1;
        } else if (emgDataStatus::MAGIC_NUM_1 == mFlag) {
            if((*data)[i] != 0x0a) {
                mFlag = emgDataStatus::UNDEFINED;
                continue;
            }
            mFlag = emgDataStatus::MAGIC_NUM_2;
        } else if (emgDataStatus::MAGIC_NUM_2 == mFlag ) {
            if((*data)[i] == 0x2c) {
                mFlag = emgDataStatus::FIRST_FLAG;
                mEmgData.ch1Average = cache2Value(mCache);
                mCache.clear();
                continue;   
            }
            mCache.push_back((*data)[i]);
        } 
        
    }
}

bool emgParser::registerListener(std::shared_ptr<parserListener> listener) 
{
    mListener.insert(listener);
    return true;
}

bool emgParser::removeListener(std::shared_ptr<parserListener> listener)
{
    mListener.erase(listener);
    return true;
}

unsigned int emgParser::cache2Value(const std::vector<unsigned char>& cache)
{
    unsigned int temValue = 0;
    for(unsigned int i = 0; i < cache.size(); ++i) {
        temValue = temValue * 10 + cache[i];
    }
    return temValue;
}