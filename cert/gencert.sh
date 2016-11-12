#!/bin/sh
# 
# File:   cert.sh
# Author: centos7
#
# Created on Nov 12, 2016, 3:45:31 PM
#

openssl genrsa -out rootCA.key 2048
openssl req -x509 -new -nodes -key rootCA.key -sha256 -days 1024 -out rootCA.pem
openssl genrsa -out server.key 2048
openssl req -new -key server.key -out server.csr
openssl x509 -req -in server.csr -CA rootCA.pem -CAkey rootCA.key -CAcreateserial -out server.crt -days 500 -sha256

openssl genrsa -out client.key 2048
openssl req -new -key client.key -out client.csr
openssl x509 -req -in client.csr -CA rootCA.pem -CAkey rootCA.key -CAcreateserial -out client.crt -days 500 -sha256

openssl pkcs12 -export -inkey client.key -in client.crt -out client.pfx
openssl pkcs12 -export -inkey server.key -in server.crt -out server.pfx 