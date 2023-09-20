#! /usr/bin/bash

set -x
g++ -c main.cpp
g++ -c secret.cpp
g++ -c db.cpp
g++ main.o secret.o db.o -lpqxx -lpq -lfmt
rm main.o secret.o

