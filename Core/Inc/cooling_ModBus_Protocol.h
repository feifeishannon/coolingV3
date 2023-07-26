#ifndef __COOLING_MODBUS_PROTOCOL_H
#define __COOLING_MODBUS_PROTOCOL_H

#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "stdint.h"
#include "stm32h7xx_hal.h"
#include <stdlib.h>

// AA 03 00 00 00 12 DC 1C 
// AA 03 24 00 01 00 64 00 01 00 05 00 00 02 58 03 20 03 20 01 F4 01 C5 01 C6 01 C5 00 00 00 00 00 00 00 00 00 03 00 08 51 0B
// 01 03  06  02 62   00 00    E0 88    90 F9 
//       num   温度    runtim  state

typedef unsigned int       uint32_t; 		// 消除vscode异常提示
#define USART_REC_LEN  			200  		// 定义最大接收字节数 200
#define RXBUFFERSIZE            1           // 缓存大小


typedef enum
{
    Cooling_OK       = 0x00,
    Cooling_ERROR    = 0x01,
    Cooling_BUSY     = 0x02,
    Cooling_TIMEOUT  = 0x03
} Cooling_FunStatusTypeDef;//函数执行状态

typedef enum
{
    Cooling_Null       = 0x00,
    Cooling_Inited     = 0x01
} Cooling_StatusTypeDef;//函数执行状态

typedef enum
{
    Cooling_STOP        = 0x00,
    Cooling_GET_STATE   = 0x01,
    Cooling_CHECK       = 0x02,
    Cooling_CMD         = 0x03
} Cooling_StateTypeDef;//状态机定义

typedef enum
{
    SYSTEM_ON               = 0x00,         /*  设置系统开机*/
    SYSTEM_OFF              = 0x01,         /*  设置系统关机*/
    SYSTEM_GET_TEMP         = 0x02,         /*  获取液冷温度*/
    SYSTEM_SET_TEMP_DATA    = 0x03,         /*  设置目标温度*/
    SYSTEM_GET_STATE_DATA   = 0x04,         /*  获取液冷状态*/
    SYSTEM_GET_ALL_DATA     = 0x05          /*  获取液冷状态*/
} Cooling_OperateTypeDef;                   //功能码定义


#pragma pack(1)
typedef struct
{
    uint16_t WaterTankTemperature;  //  00 温度测量值

    uint16_t RunningTime;           //  01 时间运行值

    uint16_t CoolingRunningState;   //  02 指示灯状态

    uint16_t TargetTemperature ;    //  03 SP温度设定值

    uint16_t STTime;                //  04 ST时间设定值

    uint16_t HighAlarmTemperature;  //  05 AL超温报警值

    uint16_t LowAlarmTemperature;   //  06 dL制冷下偏差控制

    uint16_t PIDProportion;         //  07 P比例带

    uint16_t PIDIntegral;           //  08 I积分时间

    uint16_t PIDDifferential;       //  09 D微分时间

    uint16_t AlarmState;            //  0A SO报警状态

    uint16_t SDregister;            //  0B SD±电选择

} Modbus_ChangLiu_Report_Pack_TypeDef;  // 起始地址：0x2000
#pragma pack()

/**
 * @brief 控制寄存器,由TMS设置
 * 
 */
typedef struct
{
    uint16_t CoollingTargetTemp;    //  03 SP温度设定值
    uint16_t CoollingCMD;           //  31 启停控制程序
    uint16_t PIDSelfTune;           //  32 启停自整定
    uint16_t BeepAlarm;             //  35 蜂鸣器报警
    uint16_t PumpCMD ;              //  3C 启停循环水泵
    uint16_t PressCMD;              //  3d 启停制冷压缩机
} Modbus_ChangLiu_CMD_Pack_TypeDef;  // 起始地址：0x2000
#pragma pack()

#pragma pack(1)

/**
 * @brief 状态寄存器，供TMS读取
 * 
 */
typedef struct
{
    uint8_t CoolingRunState;            // 开机/关机    7
    uint8_t CoolingHeatState ;          // 加热         6
    uint8_t CoolingCoolingState;        // 制冷         5
    uint8_t CoolingPumpFlowAlarm;       // 流量异常     4
    uint8_t CoolingPumpState;           // 泵运行状态   3
    uint8_t CoolingTempAlarm;           // 温度异常     2
    uint8_t CoolingLiquidLevelAlarm;    // 流速异常     1
    uint8_t CoolingPower;               // 电源         0
} Cooling_ChangLiu_PSD_TypeDef;
#pragma pack()

#pragma pack(1)

/**
 * @brief 实体调试需判定对齐规则
 * 
 */
typedef struct
{
    Modbus_ChangLiu_Report_Pack_TypeDef modbusReport;
    UART_HandleTypeDef *huart;
    Modbus_ChangLiu_CMD_Pack_TypeDef CMD_Pack;
    Cooling_ChangLiu_PSD_TypeDef Cooling_PSD;
    float currentTemperature;
    float targetTemperature;
    uint8_t modbus_count;
    // char *info[1000];                       /* 液冷控制器信息描述,1k缓存*/
    Cooling_StatusTypeDef coolingSYSstatus;
    Cooling_FunStatusTypeDef (* Init)();       /*!< 配置用户通讯接口   */
    Cooling_FunStatusTypeDef (* Run)();        /*!< 启动液冷控制器  建议工作频率20hz   */      
    Cooling_FunStatusTypeDef (* Stop)();       /*!< 停止液冷控制器     */      
    void (* RxCplt)();                      /*!< 液冷控制器接收数据处理     */      
    Cooling_FunStatusTypeDef (* UpdataPack)();                  /*!< 通过串口发送modbus命令更新液冷数据寄存器   */      
    
} Cooling_HandleTypeDef;
#pragma pack()

extern Cooling_HandleTypeDef* Cooling_Handle; //液冷控制器单例对象
Cooling_FunStatusTypeDef CoolingCreate( UART_HandleTypeDef *huartcooling);



#endif /* __COOLING_MODBUS_PROTOCOL_H */

