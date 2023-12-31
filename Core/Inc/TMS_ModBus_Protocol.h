#ifndef __TMS_MODBUS_PROTOCOL_H
#define __TMS_MODBUS_PROTOCOL_H

#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "stdint.h"
#include "stm32h7xx_hal.h"
#include <stdlib.h>

// AA 03 00 00 00 12 DC 1C 
// AA 03 24 00 01 00 64 00 01 00 05 00 00 02 58 03 20 03 20 01 F4 01 C5 01 C6 01 C5 00 00 00 00 00 00 00 00 00 03 00 08 51 0B

typedef unsigned int       uint32_t; 		// 消除vscode异常提示
#define USART_REC_LEN  			200  		// 定义最大接收字节数 200
#define RXBUFFERSIZE            1           // 缓存大小

typedef enum
{
    TMS_OK       = 0x00,
    TMS_ERROR    = 0x01,
    TMS_BUSY     = 0x02,
    TMS_TIMEOUT  = 0x03
} TMS_FunStatusTypeDef;//函数执行状态

typedef enum
{
    CoolingCMDStop          = 0x00, //  停止液冷控制
    CoolingCMDStart         = 0x01, //  启动液冷控制
    CoolingSetTemp          = 0x02, //  设置目标温度
    CoolingGetData          = 0x03, //  获取液冷状态信息
    CoolingPumpStop         = 0x04, //  停止液泵
    CoolingPumpStart        = 0x05, //  启动液泵
    CoolingCompressorStop   = 0x06, //  停止压缩机
    CoolingCompressorStart  = 0x07, //  启动压缩机
    CoolingSetAll           = 0x08, //  设置所有参数
    CoolingWait             = 0xff  //  待机，等待新指令
} CMDCodeDef;//水冷控制器状态机定义 以此判定外部指令

#pragma pack(1)
typedef struct
{
    uint16_t TMSRunState;           //0x0000 制冷开机/关机
    uint16_t PIDProportion ;        //0x0001 PID比例值(0-250)
    uint16_t PIDIntegral;           //0x0002 PID积分值(0-250)
    uint16_t PIDDifferential;       //0x0003 PID微分值(0-250)
    uint16_t PIDSelfTune;           //0x0004 PID自整定开关，取值范围0-1
    uint16_t TargetTemperature;     //0x0005 设定温度 初始化值20度
    uint16_t LowAlarmTemperature;   //0x0006 低温报警
    uint16_t HighAlarmTemperature;  //0x0007 高温报警
    uint16_t TemperatureCalibration;//0x0008 温度校准
    uint16_t OutletTemperature;     //0x0009 出口温度
    uint16_t InletTemperature;      //0x000A 入口温度
    uint16_t HotSideTemperature;    //0x000B 热端温度      
    uint16_t TMSTemperature4;       //0x000C 制冷温度4
    uint16_t TMSTemperature5;       //0x000D 制冷温度5
    uint16_t TMSTemperature6;       //0x000E 制冷温度6
    uint16_t WaterPumpFlowRate;     //0x000F 水泵流量
    uint16_t liquidheight;          //0x0010 液位高度
    uint16_t PSD;                   //0x0011 寄存器状态
    uint16_t CoolRevsionYear;       //0x0012 年版本号
    uint16_t CoolRevsionMoDa;       //0x0013 月日版本号
} Modbus_Report_Pack_TypeDef;
#pragma pack()


#pragma pack(1)
typedef struct
{
    uint8_t TMSRunState;        //0x0000 开机/关机
    uint8_t TMSFanERR ;         //0x0001 风扇报警
    uint8_t TMSPumpERR;         //0x0002 水泵报警
    uint8_t TMSHotSideERR;      //0x0003 热端报警
    uint8_t TMSLowTempERR;      //0x0004 低温报警
    uint8_t TMSHighTempERR;     //0x0005 高温报警
    uint8_t TMSPumpFlowERR;     //0x0006 流速报警
    uint8_t TMSLiquidLevelERR;  //0x0007 液位报警
    uint8_t TMSERRflag;         //0x0008 液冷故障
} TMS_PSD_TypeDef;
#pragma pack()

typedef struct
{
    uint16_t TMSTargetTemp;    //  03 SP温度设定值
    uint16_t TMSCMD;           //  31 启停控制程序
    
} TMS_CMD_Pack_TypeDef;  // 起始地址：0x2000

#pragma pack(1)

/**
 * @brief 实体调试需判定对齐规则
 * 
 */
typedef struct
{
    Modbus_Report_Pack_TypeDef modbusReport;
    CMDCodeDef CMDCode;
    UART_HandleTypeDef *huart;
    TMS_PSD_TypeDef TMS_PSD;
    TMS_CMD_Pack_TypeDef TMS_CMD_Pack;
    float currentTemperature;
    float targetTemperature;
    uint8_t modbus_count;
    uint8_t modbusDataReloadFlag;     /* TMS控制器处理数据标志，完成处理后置1，未完成清除*/
    // char *info[1000];                       /* 液冷控制器信息描述,1k缓存*/
    TMS_FunStatusTypeDef (* Init)();       /*!< 配置用户通讯接口   */
    TMS_FunStatusTypeDef (* Run)();        /*!< 启动液冷控制器  建议工作频率20hz   */      
    TMS_FunStatusTypeDef (* Stop)();       /*!< 停止液冷控制器     */      
    void (* RxCplt)();                      /*!< 液冷控制器接收数据处理     */      
    TMS_FunStatusTypeDef (* UpdataPack)();                  /*!< 通过串口发送modbus命令更新液冷数据寄存器   */      
    void (* reportAll)();                  /*!< 通过串口发送modbus命令更新液冷数据寄存器   */      
    
} TMS_HandleTypeDef;
#pragma pack()

extern TMS_HandleTypeDef* TMS_Handle; //液冷控制器单例对象
TMS_FunStatusTypeDef TMSCreate( UART_HandleTypeDef *huartTMS);

#endif /* __TMS_MODBUS_PROTOCOL_H */
