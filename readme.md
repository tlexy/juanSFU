## juansfu

### 目录结构
#### webrtc-client
webrtc客户端程序，在main.js中配置信令地址
```
let zal_rtc = new ZalRtc("ws://192.168.110.10:5000/signaling");
```
#### server
服务器端程序

### 依赖
* openssl
* [uvnet](https://github.com/tlexy/uvnet)
* [libsrtp](https://github.com/cisco/libsrtp)
* [abseil-cpp](https://github.com/abseil/abseil-cpp)

### windows编译
* 根据项目的依赖配置库路径
* 打开juansfu.sln执行编译

### linux服务器端编译（cmake未编写完成，可自行修改cmake以完成编译）
* 安装openSSL开发库
* 下载并编译[uvnet](https://github.com/tlexy/uvnet)，编译完成后会得到一个libuvnet.a库
* 下载并编译[libsrtp](https://github.com/cisco/libsrtp)，编译完成后安装
* 将[juanSFU](https://github.com/tlexy/juanSFU)下载到与uvnet的同级目录，安装juanSFU/server/3rd目录下的jsoncpp库
* 进入juanSFU/server/juansfu目录，新建libs目录，将libuvnet.a拷贝到这里。
* 在juanSFU/server/juansfu新建build目录，进入build目录，执行
```
cmake ..
make
```

### 运行
* 将juanSFU/server/juansfu目录下的webrtc_config.json配置文件拷贝到可执行文件所在目录(juanSFU/server/juansfu/bin)
* 修改配置文件中的server_ip字段为本地ip
