#!/bin/sh
mkdir depend 
cd depend
wget https://cmake.org/files/v3.7/cmake-3.7.1.tar.gz
tar xzfv cmake-3.7.1.tar.gz
cd cmake-3.7.1
./bootstrap --prefix=/usr
make 
sudo make install

sudo yum -y install openssl-devel unixODBC-devel mariadb mariadb-server mariadb-devel

wget https://pocoproject.org/releases/poco-1.7.6/poco-1.7.6-all.tar.gz
tar xzfv poco-1.7.6-all.tar.gz
cd poco-1.7.6-all
mkdir cmake_build
cd cmake_build
cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
sudo ldconfig

