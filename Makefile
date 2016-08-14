PRO=$(shell basename `realpath .`)

all:
	cd src && make && cd ..

clean:
	cd src && make clean && cd ..
	
run:
	./$(PRO) --daemon --pidfile=webcppd.pid

stop:
	kill -9 `cat webcppd.pid`
