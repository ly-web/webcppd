# webcppd



Fast and Lightweight Web Server for Applications Written in C++



## 依赖
### Poco C++ Framework

## 依赖安装
###  Centos
`yum -y install epel-release`

`yum -y install poco-devel`

推荐执行`centos-install-depend.sh`

### Ubuntu
不要安装libpoco-dev，它的版本太低

请执行`ubuntu-install-depend.sh`



## 编译
`make`

## 清理
`make clean`

## 安装
`sudo make install`

## 卸载
`sudo make uninstall` or `sudo webcppd-ctrl.sh uninstall`

## 启动|停止|重启|状态
`sudo systemctl (start|stop|restart|status) webcppd`


## 开机自启动
`sudo systemctl enable webcppd`

## 配置
见 `/etc/webcppd/webcppd.properties`

## HTTPS
如果需要https
请参考`letsencrypt/letsencrypt.md`

