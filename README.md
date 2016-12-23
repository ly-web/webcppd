# webcppd

面向C++ web编程的应用服务器 

## 概述

webcppd 是为了把 C++ 语言变成一种 Web 开发语言而准备的。利用 webcppd,C++ 语言能够
轻松地与 HTML、JS、CSS 打成一片,无障碍地融入到 Web 开发的实践当中。
因而,你能够把 C++ 语言当作是更好、更快的 PHP 语言来对待。
当然,你也可以把 webcppd 当然 C++ 版的 tomcat。

## 特性


- HTTP/1.0 and HTTP/1.1
- WebSocket
- HTTPS
- C++
- 会话管理
- 缓存管理
- ip动、静态黑名单
- 正则路由
- 防盗链
- 日志
- more

## 依赖安装
###  Centos
`yum -y install epel-release`

`yum -y install poco-devel`

推荐执行`centos-install-depend.sh`

### Ubuntu
不要安装libpoco-dev，它的版本太低。请执行`ubuntu-install-depend.sh`


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

