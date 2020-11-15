# 1、代码结构
关注myserver.c 、networking.c、myclient.c这几个就行

- (1)ae_epoll.c #redis API，对epoll的封装
- (2)ae.c/ae.h  #redis API，对事件处理封装，包含时间/文件事件的创建、添加、删除
- (3)anet.c/anet.h  #redis API，对socket底层bind、accept、listen函数的封装
- (4)myserver.c     #入口
- (5)networking.c    #自己编写的处理逻辑
- (6)zmalloc.c/zmalloc.h  #redis API，对内存malloc等的封装
- (7)Makefile #用于编译
- (8)ip.sh #自己写的脚本，用于方便替换myclient.c中char ip[20]

# 2、运行
make之后，再执行ip.sh，即可出现myserver、myclien可执行文件，直接启动可执行文件就完成了。

# 3、功能讲解
- （1）群发。默认情况下，每一个客户端连接上，都会有一个唯一name标识。当客户端发送数据时，当前所有连接上的客户端都会接收到。
- （2）私聊。可以通过setname liliya，先将客户端name标识更新为liliya。其他用户通过pchat liliya Are you ok，即可将消息私发消息给liliya

# 4、并发量测试

服务端运行在虚拟机上，利用windwos的jmeter测试

当QPS为2000时，性能完全ok，在2ms内便能完成响应
当QPS达到1w时，也能处理，但性能会急速下降，平均需要2.5s才能完成响应

# 5、csdn详细介绍地址
https://blog.csdn.net/qq_35475714/article/details/109706465

