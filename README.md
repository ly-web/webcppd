# webcppd



Fast and Lightweight Web Server for Applications Written in C++



## 依赖
### Poco C++ Framework

## 依赖安装
###  Centos
`yum -y install epel-release`

`yum -y install poco-devel`

### Ubuntu
`apt-get -y install libpoco-dev`


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
见 /etc/webcppd/webcppd.properties

