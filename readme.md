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

### ubuntu linux服务器端编译
* 进入server目录，运行ubuntu_3rd_build.sh
* 然后运行build_other.sh
* 进入server/juansfu目录，新建build目录，进入build目录，执行cmake ..


### 运行
* 将juanSFU/server/juansfu目录下的webrtc_config.json配置文件拷贝到可执行文件所在目录(juanSFU/server/juansfu/bin)
* 修改配置文件中的server_ip字段为本地ip

### 客户端说明
* 只支持一端推流一端拉流
* 首先加入房间的用户，必须推流
* 后面加入房间的用户拉流
