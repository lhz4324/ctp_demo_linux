# ctp_demo_linux

## 功能介绍：
```
	   （1）这是基于CTP编写的相关行情接口与交易接口程序

　　（2）可以通过json文件读取用户数据并将不同用户对应报单分类存储，打印日志

	   （3）有实时预警功能，不同品种期货一旦报撤单次数超过相应阈值，将其信息发送至微信接口

	   （4）可以获取深度行情tick数据，并将其转换为不同时间维度k线，存储为csv文件

```
## 使用步骤：
```
 1.进入build目录，执行cmake ..
 2.cmake执行成功后，继续在当前目录执行 make
 3.这时执行 cd ../bin
 4.配置userlist.json，更改为自己simnow账号密码，前置机也可进行配置
 5.执行./main ,程序运行成功会在同级目录生成log.txt文件，以及相应csv文件
```

## 目录结构描述
```
 ├── Readme.md                   // help
 ├── main.cpp                    // 主函数
 ├── include                     // 包含文件
 │   ├── CTP_API
 │   ├── rapidjson                
 │   ├── spdlog         
 │   ├── CustomTradeSpi.h                
 │   └── httplib.h              
 ├── CustomTradeSpi.cpp 
 ├── dll                         //动态链接库
 │   └── libthosttraderapi_sm.so
 ├── CMakeLists.txt                     
 └── bin
     └── userlist.json           // 用户配置文件
