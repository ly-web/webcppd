#!/bin/sh
mkdir depend 
cd depend
sudo apt-get install openssl libssl-dev libiodbc2 libiodbc2-dev libmysqlclient-dev
wget https://pocoproject.org/releases/poco-1.7.6/poco-1.7.6-all.tar.gz
tar xzfv poco-1.7.6-all.tar.gz
cd poco-1.7.6-all
mkdir cmake_build
cd cmake_build
cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
sudo ldconfig
cd ../../
