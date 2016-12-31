#!/bin/sh
mkdir depend 
cd depend
sudo apt-get -y install openssl libssl-dev unixodbc-dev libmysqld-dev
if [ ! -e poco-1.7.6-all.tar.gz ];then
    wget https://pocoproject.org/releases/poco-1.7.6/poco-1.7.6-all.tar.gz
fi
tar xzfv poco-1.7.6-all.tar.gz
cd poco-1.7.6-all
if [ ! -d cmake_build ];then
    mkdir cmake_build
fi
cd cmake_build
cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
sudo ldconfig
