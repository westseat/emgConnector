#include "util.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>

namespace util {
    namespace pt = boost::property_tree;
    std::shared_ptr<std::string> emgData2JsonString(const emgData& data)
    {
        pt::ptree tree;
        tree.put("version", "1.0");
        tree.put("type", "emg");
        tree.put("ch1Average", data.ch1Average);
        tree.put("ch1Value", data.ch1Value);
        tree.put("ch1Powser", data.ch1Power);
        tree.put("ch1Strength", data.ch1Strength);
        tree.put("ch2Average",data.ch2Average);
        tree.put("ch2Value", data.ch2Value);
        tree.put("ch2Power", data.ch2Power);
        tree.put("ch2Strength", data.ch2Strength);
        std::ostringstream ostr;
        pt::write_json(ostr, tree);
        return std::make_shared<std::string>(ostr.str());
    }
}

