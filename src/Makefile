PRO=webcppd
SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp,%.o,${SRC})

INSTALL_DIR=/usr/local/bin
CONFIG_DIR=/etc/webcppd
LN_DIR=/usr/bin
RUN_DIR=/var/webcppd

CXXFLAGS+=-O3 -std=c++11 -Wall -I../include
LIBS+=-lPocoUtil -lPocoNetSSL -lPocoCrypto -lPocoNet -lPocoFoundation

all:$(PRO)
	
$(PRO):$(OBJ)
	g++ -o ../$@ $^ $(CXXFLAGS) $(LIBS) 
	
clean:
	rm -f $(OBJ) ../$(PRO)
	
install:
	mkdir -pv $(INSTALL_DIR)
	mkdir -pv $(CONFIG_DIR)
	mkdir -pv $(RUN_DIR)
	mkdir -pv $(RUN_DIR)/log
	mkdir -pv $(RUN_DIR)/tpl
	mkdir -pv $(RUN_DIR)/mod
	mkdir -pv $(RUN_DIR)/www
	mkdir -pv $(RUN_DIR)/www/upload
	mkdir -pv $(RUN_DIR)/www/assets
	mkdir -pv $(RUN_DIR)/cert
	install ../$(PRO) $(INSTALL_DIR)
	install -b ../etc/*.* $(CONFIG_DIR)
	install ../www/*.* $(RUN_DIR)/www
	install ../www/assets/*.* $(RUN_DIR)/www/assets
	install ../script/webcppd-ctrl.sh $(LN_DIR)
	install ../script/webcppd-service.sh $(LN_DIR)
	install ../script/webcppd.service /etc/systemd/system
	install ../cert/*.* $(RUN_DIR)/cert
	ln -fs $(INSTALL_DIR)/$(PRO) $(LN_DIR)
	mkdir -pv /usr/local/include/webcppd
	install ../include/*.*  /usr/local/include/webcppd
	
	
uninstall:
	rm -f $(INSTALL_DIR)/webcppd \
		$(LN_DIR)/webcppd-ctrl.sh \
		$(LN_DIR)/webcppd-service.sh \
		/etc/systemd/system/webcppd.service
	rm -rf $(CONFIG_DIR)
	rm -rf $(RUN_DIR)