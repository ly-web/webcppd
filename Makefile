PRO=$(shell basename `realpath .`)

all:
	cd src && make && cd ..

clean:
	cd src && make clean && cd ..


install:
	cd src && make install
	
uninstall:
	cd src && make uninstall