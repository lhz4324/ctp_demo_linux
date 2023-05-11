# ctp_demo_linux
## 使用步骤：
### 1.进入build目录，执行cmake ..
### 2.cmake执行成功后，继续在当前目录执行 make
### 3.这时执行 cd ../bin
### 4.配置userlist.json，更改为自己simnow账号密码，前置机也可进行配置
### 5.执行./main ,程序运行成功会在同级目录生成log.txt文件，以及相应csv文件


## 目录结构描述
##### ├── Readme.md                   // help
##### ├── main.cpp                    // 主函数
##### ├── include                     // 包含文件
##### │   ├── CTP_API
##### │   ├── rapidjson                
##### │   ├── spdlog         
##### │   ├── CustomTradeSpi.h                
##### │   └── httplib.h              
##### ├── CustomTradeSpi.cpp 
##### ├── dll                         //动态链接库
##### │   └── libthosttraderapi_sm.so
##### ├── CMakeLists.txt                     
##### └── bin
#####     └── userlist.json           // 用户配置文件
