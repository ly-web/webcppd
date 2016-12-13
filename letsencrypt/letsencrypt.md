## 安装

修改`install.sh`中的域名和邮箱，然后执行该安装脚本

## 配置

其中的域名`webcpp.net`需要修改为你的域名。具体可查看`/etc/letsencrypt/live`目录

http.enableSSL=true

http.certPrivateKeyFile=/etc/letsencrypt/live/webcpp.net/privkey.pem

http.certCertificateFile=/etc/letsencrypt/live/webcpp.net/fullchain.pem

http.certRootCertificateFile=/etc/letsencrypt/live/webcpp.net/cert.pem

## 续签
执行`update.sh`