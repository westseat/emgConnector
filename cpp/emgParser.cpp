#include "emgParser.h"
#include <iostream>

emgParser::emgParser() 
{
    mFlag = emgDataStatus::UNDEFINED;
}

emgParser::~emgParser()
{

}

void emgParser::push(std::shared_ptr<std::vector<unsigned char>> data) 
{
    //std::cout << "start to push function" << std::endl;
    //std::cout << "size: " << data->size() << std::endl;
    for(unsigned int i = 0; i < data->size(); ++i) {
        if(emgDataStatus::UNDEFINED == mFlag) {
            //std::cout <<"UNDEFINED STATUS" <<std::endl;
            if((*data)[i] != 0x0d)
                continue;
            mFlag = emgDataStatus::MAGIC_NUM_1;
        } else if (emgDataStatus::MAGIC_NUM_1 == mFlag) {
            //std::cout <<"MAGIC_NUM_1" <<std::endl;
            if((*data)[i] != 0x0a) {
                mFlag = emgDataStatus::UNDEFINED;
                continue;
            }
            mFlag = emgDataStatus::MAGIC_NUM_2;
        } else if (emgDataStatus::MAGIC_NUM_2 == mFlag ) {
            //std::cout <<"MAGIC_NUM_2" << std::endl;
            if((*data)[i] == 0x2c) {
                mFlag = emgDataStatus::FIRST_FLAG;
                mEmgData.ch1Average = cache2Value(mCache);
                mCache.clear();
                continue;   
            }
            mCache.push_back((*data)[i]);
        } else if (emgDataStatus::FIRST_FLAG == mFlag) {
            //std::cout <<"FIRST_FLAG" << std::endl;
            if((*data)[i] == 0x2c) {
                mFlag = emgDataStatus::SECOND_FLAG;
                mEmgData.ch1Value = cache2Value(mCache);
                mCache.clear();
                continue;
            }
            mCache.push_back((*data)[i]);
        } else if(emgDataStatus::SECOND_FLAG == mFlag) {
            //std::cout <<"SECOND_FLAG" << std::endl;
            if((*data)[i] == 0x2c) {
                mFlag = emgDataStatus::THIRD_FLAG;
                mEmgData.ch1Power = cache2Value(mCache);
                mCache.clear();
                continue;
            }
            mCache.push_back((*data)[i]);
        } else if (emgDataStatus::THIRD_FLAG == mFlag) {
            //std::cout <<"THIRD_FLAG" << std::endl;
            if((*data)[i] != 0x2c) {
                mCache.push_back((*data)[i]);
                continue;
            }
            mFlag = emgDataStatus::FOURTH_FLAG;
            mEmgData.ch1Strength = cache2Value(mCache);
            mCache.clear();
        } else if (emgDataStatus::FOURTH_FLAG == mFlag) {
            //std::cout <<"FOURTH_FLAG" << std::endl;
            if((*data)[i] != 0x2c) {
                mCache.push_back((*data)[i]);
                continue;
            }
            mFlag = emgDataStatus::FIFTH_FLAG;
            mEmgData.ch2Average = cache2Value(mCache);
            mCache.clear();
        } else if (emgDataStatus::FIFTH_FLAG == mFlag) {
            //std::cout <<"FIFTH_FLAG" << std::endl;
            if((*data)[i] != 0x2c) {
                mCache.push_back((*data)[i]);
                continue;
            }
            mFlag = emgDataStatus::SIXTH_FLAG;
            mEmgData.ch2Value = cache2Value(mCache);
            mCache.clear();
        } else if (emgDataStatus::SIXTH_FLAG == mFlag) {
            //std::cout <<"SIXTH_FLAG" <<std::endl;
            if((*data)[i] != 0x2c) {
                mCache.push_back((*data)[i]);
                continue;
            }
            mFlag = emgDataStatus::SEVENTH_FLAG;
            mEmgData.ch2Power = cache2Value(mCache);
            mCache.clear(); 
        } else if (emgDataStatus::SEVENTH_FLAG == mFlag) {
            //std::cout <<"SEVENTH_FLAG" << std::endl;
            if ((*data)[i] != 0x0d) {
                mCache.push_back((*data)[i]);
                continue;
            }
            mFlag = emgDataStatus::MAGIC_NUM_1;
            mEmgData.ch2Strength = cache2Value(mCache);
            mCache.clear();
            emgData* ptr = new emgData;
            (*ptr) = mEmgData;
            for(auto it = mListener.begin(); it != mListener.end(); ++it) {
                (*it)->onParserData(parserDataType::EMG_DATA, static_cast<void*>(ptr));
            }
        }
        
    }
    //std::cout <<"end the push function" << std::endl;
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
        temValue = temValue * 10 + ( cache[i] - '0' );
    }
    return temValue;
}

std::string emgParser::getParserType() 
{
    return mParseType;
}