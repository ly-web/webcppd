#!/bin/sh
# 
# File:   webcppd-ctrl.sh
# Author: centos7
#
# Created on Oct 24, 2016, 3:35:09 PM
#

case $1 in 
    start) 
        /usr/local/bin/webcppd --config=/etc/webcppd/webcppd.properties --pidfile=/var/run/webcppd.pid --daemon
    ;;
    stop) 
        kill -9 `pidof webcppd`
    ;;
    uninstall)
       	echo 'run: rm -f /usr/local/bin/webcppd  \
                        /usr/local/include/webcppd \
                        /usr/bin/webcppd-ctrl.sh \
                        /usr/bin/webcppd \
                        /usr/bin/webcppd-service.sh \
                        /etc/systemd/system/webcppd.service'
	echo 'run: rm -rf /etc/webcppd'
	echo 'run: rm -rf /var/webcppd'
    ;;
    *)
    echo "Usage: $0 {start|stop|uninstall}"
    ;;
esac
