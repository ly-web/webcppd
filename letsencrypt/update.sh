#!/bin/sh
# 
# File:   update.sh
# Author: centos7
#
# Created on Dec 13, 2016, 10:40:03 PM
#
sudo ./certbot-auto renew --pre-hook "service webcppd stop" --post-hook "service webcppd start"