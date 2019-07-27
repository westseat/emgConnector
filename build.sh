#!/bin/bash

g++ -c serialPortChannel.cpp  -std=c++11 -pthread
g++ -c emgParser.cpp -std=c++11 -pthread
g++ -c parserManager.cpp -std=c++11 -pthread
g++ -c hardwareChannelManager.cpp -std=c++11 -pthread
g++ main.cpp -o main.out serialPortChannel.o emgParser.o parserManager.o hardwareChannelManager.o -std=c++11 -pthread
