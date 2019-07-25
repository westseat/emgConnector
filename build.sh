#!/bin/bash

g++ -c serialPortChannel.cpp -std=c++11 -pthread
g++ main.cpp -o main.out serialPortChannel.o -std=c++11 -pthread
