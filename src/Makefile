PRO=webcppd
SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp,%.o,${SRC})

INSTALL_DIR=/usr/local/bin
CONFIG_DIR=/etc/webcppd
LN_DIR=/usr/bin

CXXFLAGS+=-O3 -std=c++11 -Wall
LIBS+=-lPocoUtil -lPocoNet -lPocoFoundation

all:$(PRO)
	
$(PRO):$(OBJ)
	g++ $(CXXFLAGS) $(LIBS) -o ../$@ $^
	
clean:
	rm -f $(OBJ) ../$(PRO)
	
install:
	mkdir -pv /usr/local/bin
	mkdir -pv /etc/webcppd
	mkdir -pv /var/www/
	mkdir -pv /var/www/webcppd
	mkdir -pv /var/www/webcppd/log
	mkdir -pv /var/www/webcppd/tpl
	mkdir -pv /var/www/webcppd/mod
	mkdir -pv /var/www/webcppd/www
	mkdir -pv /var/www/webcppd/www/upload
	install ../$(PRO) $(INSTALL_DIR)
	install ../$(PRO).properties $(CONFIG_DIR)
	install ../etc/*.* $(CONFIG_DIR)
	install ../www/*.* /var/www/webcppd/www
	install ../script/webcppd-ctrl.sh $(LN_DIR)
	install ../script/webcppd-service.sh $(LN_DIR)
	install ../script/webcppd.service /etc/systemd/system
	ln -fs $(INSTALL_DIR)/$(PRO) $(LN_DIR)
	
	
uninstall:
	rm -f /usr/local/bin/webcppd /usr/bin/webcppd-ctrl.sh /usr/bin/webcppd-service.sh /etc/systemd/system/webcppd.service
	rm -rf /etc/webcppd
	rm -rf /var/www/webcppd