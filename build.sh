#!/bin/bash

g++ -c serialPortChannel.cpp  -std=c++11 -pthread
g++ -c emgParser.cpp -std=c++11 -pthread
g++ main.cpp -o main.out serialPortChannel.o emgParser.o -std=c++11 -pthread
