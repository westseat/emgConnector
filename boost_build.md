# Brief

Introduce how to install least boost library on ubuntu.


# Usage

```sh
$ sudo add-apt-repository ppa:mhier/libboost-latest
$ sudo apt-get update
```

# Build

```sh
$ g++ -o bin xx.cpp -pthread -lboost_system
```