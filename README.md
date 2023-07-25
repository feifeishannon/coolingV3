# cooling
## TODO:
- tms数据接收
    - tms数据包解析
        - 数据封装
        - 方法封装
    - tms数据回传
- 水冷命令控制
    - 水冷命令封包
        - 数据封装
        - 方法封装
    - 水冷数据拆包
    - 水冷数据同步
    - 自定义制冷控制
        - 去除加热线
        - 窗口调节压缩机启停
- crc校验算法验证
---
- 搭建模拟环境
    - 搭建cooler端环境
        1. 开机
    - 搭建tms端环境
---
- 需增加数据重发机制，防止丢包后无处理状态
---
# DEBUG:
- tms包
    - ~~tms回包数据与cool中状态量不一致：如 运行状态 cool中为1 tms中为0~~
        - ~~拷贝次序出错~~
    - ~~tms回包全为0~~
        - ~~tms回包封包函数出错~~
    - ~~tms设置包目标温度解析异常~~
        - ~~cool同步到tms数据时错将只写数据同步导致~~
    - 设置完tms目标温度后，cool端未成功执行向下设置操作



