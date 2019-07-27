#include "emgParser.h"

emgParser::emgParser() 
{
    mFlag = emgDataStatus::UNDEFINED;
}

emgParser::~emgParser()
{

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
        } else if (emgDataStatus::FIRST_FLAG == mFlag) {
            if((*data)[i] == 0x2c) {
                mFlag = emgDataStatus::SECOND_FLAG;
                mEmgData.ch1Value = cache2Value(mCache);
                mCache.clear();
                continue;
            }
            mCache.push_back((*data)[i]);
        } else if(emgDataStatus::SECOND_FLAG == mFlag) {
            if((*data)[i] == 0x2c) {
                mFlag = emgDataStatus::THIRD_FLAG;
                mEmgData.ch1Power = cache2Value(mCache);
                mCache.clear();
                continue;
            }
            mCache.push_back((*data)[i]);
        } else if (emgDataStatus::THIRD_FLAG == mFlag) {
            if((*data)[i] != 0x2c) {
                mCache.push_back((*data)[i]);
                continue;
            }
            mFlag = emgDataStatus::FOURTH_FLAG;
            mEmgData.ch1Strength = cache2Value(mCache);
            mCache.clear();
        } else if (emgDataStatus::FOURTH_FLAG == mFlag) {
            if((*data)[i] != 0x2c) {
                mCache.push_back((*data)[i]);
                continue;
            }
            mFlag = emgDataStatus::FIFTH_FLAG;
            mEmgData.ch2Average = cache2Value(mCache);
            mCache.clear();
        } else if (emgDataStatus::FIFTH_FLAG == mFlag) {
            if((*data)[i] != 0x2c) {
                mCache.push_back((*data)[i]);
                continue;
            }
            mFlag = emgDataStatus::SIXTH_FLAG;
            mEmgData.ch2Value = cache2Value(mCache);
            mCache.clear();
        } else if (emgDataStatus::SIXTH_FLAG == mFlag) {
            if((*data)[i] != 0x2c) {
                mCache.push_back((*data)[i]);
                continue;
            }
            mFlag = emgDataStatus::SEVENTH_FLAG;
            mEmgData.ch2Power = cache2Value(mCache);
            mCache.clear(); 
        } else if (emgDataStatus::SEVENTH_FLAG == mFlag) {
            if ((*data)[i] != 0x0b) {
                mCache.push_back((*data)[i]);
                continue;
            }
            mFlag = emgDataStatus::MAGIC_NUM_1;
            mEmgData.ch2Strength = cache2Value(mCache);
            mCache.clear();
            emgData* ptr = new emgData;
            (*ptr) = mEmgData;
            for(auto it = mListener.begin(); it != mListener.end(); ++it) {
                (*it)->onParserData(DataType::EMG_DATA, static_cast<void*>(ptr));
            }
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

std::string emgParser::getParserType() 
{
    return mParseType;
}