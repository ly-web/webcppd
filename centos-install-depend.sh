#!/bin/sh
mkdir depend 
cd depend
if [ !-e cmake-3.7.1.tar.gz ];then
    wget https://cmake.org/files/v3.7/cmake-3.7.1.tar.gz
fi
tar xzfv cmake-3.7.1.tar.gz
cd cmake-3.7.1
./bootstrap --prefix=/usr
make 
sudo make install

cd ..

sudo yum -y install openssl-devel unixODBC-devel mariadb-devel
if [ !-e poco-1.7.6-all.tar.gz ];then
    wget https://pocoproject.org/releases/poco-1.7.6/poco-1.7.6-all.tar.gz
fi
tar xzfv poco-1.7.6-all.tar.gz
cd poco-1.7.6-all
mkdir cmake_build
cd cmake_build
cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
sudo ldconfig

