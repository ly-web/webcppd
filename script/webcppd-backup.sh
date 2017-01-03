#!/bin/sh
# 
# File:   webcppd-backup.sh
# Author: ubuntu1610
#
# Created on Jan 3, 2017, 6:16:19 PM
#
MAIN_CONF=/etc/webcppd/webcppd.properties
IPDENY_CONF=/etc/webcppd/ipdeny.conf
ROUTE_CONF=/etc/webcppd/route.conf

case $1 in
    backup)
        for i in $MAIN_CONF $IPDENY_CONF $ROUTE_CONF;
        do
            if test -e $i;then
                cp $i $i.bk
            else
                echo $i 'not exists.'
            fi;
        done
    ;;
    restore)
        for i in $MAIN_CONF $IPDENY_CONF $ROUTE_CONF;
        do
            if test -e $i.bk;then
                cp $i.bk  $i
            else
                echo $i.bk 'not exists.'
            fi;
        done
    ;;
    *)
    echo "Usage: $0 {backup|restore}"
    ;;
esac