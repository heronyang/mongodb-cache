#!/bin/bash

## mongo-c-driver
sudo apt-get install -y pkg-config libssl-dev libsasl2-dev
wget https://github.com/mongodb/mongo-c-driver/releases/download/1.4.2/mongo-c-driver-1.4.2.tar.gz
tar xzf mongo-c-driver-1.4.2.tar.gz
cd mongo-c-driver-1.4.2
./configure
make
sudo make install
cd ..
