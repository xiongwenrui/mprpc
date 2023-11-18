# 分布式网络通信rpc框架
## quick start
安装以下组件（版本仅作参考）：

zookeeper-3.8.3

boost-1.66

muduo-2.0.2

protobuf-3.16.3

cmake-3.15

gcc/g++-9.3.1

## 分布式网络通信rpc框架
单机服务器的缺点：

1. 硬件资源的限制影响并发：受限于硬件资源，聊天服务器承受的用户的并发有限
2. 模块的编译部署难：任何模块小的修改，都导致整个项目代码重新编译、部署
3. 模块对硬件资源的需求不同：各模块是CPU或IO密集型，各模块对资源需求不同

尽管集群服务器可以扩展硬件资源，提高用户的并发，但缺点2和3仍存在，此时就引出分布式服务器，分布式系统中系统由“微服务”组成，常用RPC(remote procedure call)解决**分布式系统中微服务之间的调用问题**），简言之就是**开发者能像调用本地方法一样调用远程服务**。

## RPC概述

![Cip5yF_vL2GAftSLAAOCKnZEdrY576](https://img-blog.csdnimg.cn/img_convert/efa86ddd01f5e87a7f46cddfb2452712.png)

### RPC调用过程

完整的RPC过程如下图：

![image-20220814221421663](https://img-blog.csdnimg.cn/img_convert/867ce487ebd17e5efd23718cca36fb81.png)

远程调用需传递**服务对象、函数方法、函数参数**，经序列化成字节流后传给提供服务的服务器，服务器接收到数据后反序列化成**服务对象、函数方法、函数参数**，并发起本地调用，将响应结果序列化成字节流，发送给调用方，调用方接收到后反序列化得到结果，并传给本地调用。

### 数据传输格式

考虑到可靠性和长连接，因此使用TCP协议，而TCP是字节流协议，因此需自己处理拆包粘包问题，即自定义数据传输格式！如图：

![image-20220815120520672](https://img-blog.csdnimg.cn/img_convert/4d779b864509b9b8071f666676b7c97f.png)

定义protobuf类型的结构体消息RpcHeader，包含**服务对象、函数方法、函数参数**，因为参数可变，参数长不定，服务端不知道要截取的数据长度是多少，因此不能直接序列化传输RpcHeader。需要定义每个数据段的长度，服务器根据4字节截取字符串，获取消息长度，再截取相应长度的字符串，得到完整的数据后才能反序列化转换为对象。

```protobuf
//消息头
message RpcHeader
{
    bytes service_name = 1;
    bytes method_name = 2;
    uint32 args_size = 3;
}
```

## 框架

RPC通信过程中的代码调用流程图如图：

![image-20220815133628010](https://img-blog.csdnimg.cn/img_convert/495f015a7fa66bf1390f9b00401a37e2.png)



### 日志

在打印日志时，传入string类型可能会导致日志打印乱码的问题，建议传入char*



## 总结

整个系统时序图如下所示

![image-20220815184439376](https://img-blog.csdnimg.cn/img_convert/46f80f074429cfdc14d4b0baae666603.png)

