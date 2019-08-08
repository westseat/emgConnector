#!/bin/bash

rm *.o
rm *.out
g++ -c serialPortChannel.cpp  -std=c++11 -pthread
g++ -c emgParser.cpp -std=c++11 -pthread
g++ -c parserManager.cpp -std=c++11 -pthread
g++ -c hardwareChannelManager.cpp -std=c++11 -pthread
g++ -c websocket_server_async.cpp -pthread -lboost_system
g++ main.cpp -o main.out serialPortChannel.o emgParser.o parserManager.o hardwareChannelManager.o websocket_server_async.o -std=c++11 -pthread -lboost_system
