# tuya ble sdk介绍

## 总体描述

TUYA BLE SDK主要封装了和涂鸦智能手机APP之间的通信协议以及实现了简易事件调度机制，使用该SDK的设备无须关心具体的通信协议实现细节，通过调用SDK提供的API和Call Back即可与APP互联互通。

## 系统架构

![](https://i.imgur.com/0Iu4AgI.png)

## 模块介绍

- Platform：
所使用的芯片平台，芯片+协议栈由芯片公司维护。

- Port：TUYA BLE SDK所需要的接口抽象，需要用户根据具体的芯片平台移植实现。

- Tuya ble sdk：sdk封装了涂鸦ble通信协议，提供构建涂鸦ble应用所需的服务接口。

- Application：基于tuya ble sdk构建的应用。

- Tuya ble sdk API：SDK提供相关API用于设备实现BLE相关的管理、通信等，如果使用OS，API的调用将采用基于消息的异步机制，API的执行结果将会以message或者call back的方式通知给设备的application，如果是非OS，API的返回值即为执行结果。

- Sdk config：SDK可裁剪可配置，通过config文件中的宏定义操作，例如配置SDK适用于多协议设备的通用配网模式，蓝牙单点设备、基于ECDH秘钥协商加密模式、是否使用OS等。

- Main process function：
为SDK的主引擎，设备application需要一直调用，如果platform基于OS，SDK会基于port层提供的OS相关api自动创建一个任务用于执行Main process function，如果是非OS平台，需要设备application循环调用。

- Message or Call Back：
SDK通过message或者设备app注册的call back函数向设备APP发送数据（状态、数据等）。

## OS支持

TUYA BLE SDK可运行在基于RTOS的芯片平台下（linux暂不支持）。如果使用RTOS，API的调用将采用基于消息的异步机制，初始化SDK时，SDK将会根据tuya\_ble\_config.h文件的相关配置自动创建一个任务或者使用用户提供的task用于处理SDK的核心逻辑，同时自动创建一个消息队列或者使用移植的消息队列用于接收API的执行请求，API的执行结果也将会以message的方式通知给设备的application，所以用户application需要创建一个消息队列并在调用tuya\_ble\_sdk\_init()后调用tuya\_ble\_callback\_queue\_register()将消息队列注册至SDK中。

## 事件队列

先进先出，用于缓存设备application以及platform层发送来消息事件（api调用、ble底层数据接收等），Main process function模块循环查询消息队列并取出处理。

## SDK目录

![](https://i.imgur.com/R1mt7Q9.png)

- Application：	存放sdk提供的各应用示例demo。
- doc：	说明文档。
- modules：	功能抽象模块。
- port：	各平台移植代码。
- sdk：	tuya ble sdk核心代码。
- tuya\_ble\_config.h：	ble sdk配置文件。
- tuya\_ble\_sdk\_version.h：	sdk版本.h文件。
- tuya\_ble\_sdk\_version.txt：	sdk版本说明文件。

## TUYA BLE SERVICE

Tuya ble sdk不提供初始化service相关接口，Application需要在初始化sdk前实现下图所定义的sevice characteristics，否则sdk将不能正常工作。
![](https://i.imgur.com/LLITQqU.png)


## TUYA BLE 广播数据格式

sdk在初始化时会自动更新广播内容，但是为了保证多平台兼容性，Application在初始化tuya ble sdk之前的初始广播内容应按照下图所示的格式：
![](https://i.imgur.com/srA7nIo.png)

## PORT

如下图所示，tuya\_ble\_port.h和tuya\_ble\_port\_peripheral.h里定义的所有接口都需要用户根据具体的芯片平台移植实现，如果用户平台是非OS的，OS相关接口不需要实现。tuya\_ble\_port.c和tuya\_ble\_port\_peripheral.c是对tuya\_ble\_port.h和tuya\_ble\_port\_peripheral.h所定义接口的弱实现，用户不能在该文件里实现具体的平台接口，应该新建一个c文件，例如新建一个tuya\_ble\_port\_nrf52832.c文件。以平台名字命令的文件里是sdk已经适配移植好的平台实现，用户可以直接使用。

![](https://i.imgur.com/uJZUfYY.png)

