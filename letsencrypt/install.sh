#!/bin/sh
# 
# File:   install.sh
# Author: centos7
#
# Created on Dec 13, 2016, 10:36:44 PM
#

wget https://dl.eff.org/certbot-auto
chmod +x ./certbot-auto
sudo systemctl stop webcppd
sudo ./certbot-auto  --redirect  --standalone --standalone-supported-challenges tls-sni-01 certonly --email admin@webcpp.net  -d webcpp.net -d www.webcpp.net 