# webcppd
基于C++ Poco框架的HTTP动态服务器

纯C++开发，基于Poco框架，高性能、多线程

目标单一：纯动态资源服务器

# 依赖
* PocoUtil 
* PocoNet 
* PocoFoundation
* memcached
* libmemcached

# 使用方法
webcppd是基于动态库加载的web动态服务器

通过配置文件配置动态库搜索目录

通过url(/a/b/c)指定动态库(a.so)和需要调用的类(webcpp::a::b)，默认动态库名为home.so，默认调用类是webcpp::home::index。系统仅仅捕捉a和b，其余部分可选，例如c的含义由开发者自行定义使用。

编译:
make
即可

清理:
make clean
即可

运行:
make run
即可

停止:
make stop
即可

# 示例
[Poco HTTP 服务器](http://www.webcpp.net/blog/post/90.html)
