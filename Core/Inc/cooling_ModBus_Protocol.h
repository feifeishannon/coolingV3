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
//       num   �¶�    runtim  state

typedef unsigned int       uint32_t; 		// ����vscode�쳣��ʾ
#define USART_REC_LEN  			200  		// �����������ֽ��� 200
#define RXBUFFERSIZE            1           // �����С


typedef enum
{
    Cooling_OK       = 0x00,
    Cooling_ERROR    = 0x01,
    Cooling_BUSY     = 0x02,
    Cooling_TIMEOUT  = 0x03
} Cooling_FunStatusTypeDef;//����ִ��״̬

typedef enum
{
    Cooling_Null       = 0x00,
    Cooling_Inited     = 0x01
} Cooling_StatusTypeDef;//����ִ��״̬

typedef enum
{
    Cooling_STOP        = 0x00,
    Cooling_GET_STATE   = 0x01,
    Cooling_CHECK       = 0x02,
    Cooling_CMD         = 0x03
} Cooling_StateTypeDef;//״̬������

typedef enum
{
    SYSTEM_ON               = 0x00,         /*  ����ϵͳ����*/
    SYSTEM_OFF              = 0x01,         /*  ����ϵͳ�ػ�*/
    SYSTEM_GET_TEMP         = 0x02,         /*  ��ȡҺ���¶�*/
    SYSTEM_SET_TEMP_DATA    = 0x03,         /*  ����Ŀ���¶�*/
    SYSTEM_GET_STATE_DATA   = 0x04,         /*  ��ȡҺ��״̬*/
    SYSTEM_GET_ALL_DATA     = 0x05          /*  ��ȡҺ��״̬*/
} Cooling_OperateTypeDef;                   //�����붨��


#pragma pack(1)
typedef struct
{
    uint16_t WaterTankTemperature;  //  00 �¶Ȳ���ֵ

    uint16_t RunningTime;           //  01 ʱ������ֵ

    uint16_t CoolingRunningState;   //  02 ָʾ��״̬

    uint16_t TargetTemperature ;    //  03 SP�¶��趨ֵ

    uint16_t STTime;                //  04 STʱ���趨ֵ

    uint16_t HighAlarmTemperature;  //  05 AL���±���ֵ

    uint16_t LowAlarmTemperature;   //  06 dL������ƫ�����

    uint16_t PIDProportion;         //  07 P������

    uint16_t PIDIntegral;           //  08 I����ʱ��

    uint16_t PIDDifferential;       //  09 D΢��ʱ��

    uint16_t AlarmState;            //  0A SO����״̬

    uint16_t SDregister;            //  0B SD����ѡ��

} Modbus_ChangLiu_Report_Pack_TypeDef;  // ��ʼ��ַ��0x2000
#pragma pack()

/**
 * @brief ���ƼĴ���,��TMS����
 * 
 */
typedef struct
{
    uint16_t CoollingTargetTemp;    //  03 SP�¶��趨ֵ
    uint16_t CoollingCMD;           //  31 ��ͣ���Ƴ���
    uint16_t PIDSelfTune;           //  32 ��ͣ������
    uint16_t BeepAlarm;             //  35 ����������
    uint16_t PumpCMD ;              //  3C ��ͣѭ��ˮ��
    uint16_t PressCMD;              //  3d ��ͣ����ѹ����
} Modbus_ChangLiu_CMD_Pack_TypeDef;  // ��ʼ��ַ��0x2000
#pragma pack()

#pragma pack(1)

/**
 * @brief ״̬�Ĵ�������TMS��ȡ
 * 
 */
typedef struct
{
    uint8_t CoolingRunState;            // ����/�ػ�    7
    uint8_t CoolingHeatState ;          // ����         6
    uint8_t CoolingCoolingState;        // ����         5
    uint8_t CoolingPumpFlowAlarm;       // �����쳣     4
    uint8_t CoolingPumpState;           // ������״̬   3
    uint8_t CoolingTempAlarm;           // �¶��쳣     2
    uint8_t CoolingLiquidLevelAlarm;    // �����쳣     1
    uint8_t CoolingPower;               // ��Դ         0
} Cooling_ChangLiu_PSD_TypeDef;
#pragma pack()

#pragma pack(1)

/**
 * @brief ʵ��������ж��������
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
    // char *info[1000];                       /* Һ���������Ϣ����,1k����*/
    Cooling_StatusTypeDef coolingSYSstatus;
    Cooling_FunStatusTypeDef (* Init)();       /*!< �����û�ͨѶ�ӿ�   */
    Cooling_FunStatusTypeDef (* Run)();        /*!< ����Һ�������  ���鹤��Ƶ��20hz   */      
    Cooling_FunStatusTypeDef (* Stop)();       /*!< ֹͣҺ�������     */      
    void (* RxCplt)();                      /*!< Һ��������������ݴ���     */      
    Cooling_FunStatusTypeDef (* UpdataPack)();                  /*!< ͨ�����ڷ���modbus�������Һ�����ݼĴ���   */      
    
} Cooling_HandleTypeDef;
#pragma pack()

extern Cooling_HandleTypeDef* Cooling_Handle; //Һ���������������
Cooling_FunStatusTypeDef CoolingCreate( UART_HandleTypeDef *huartcooling);



#endif /* __COOLING_MODBUS_PROTOCOL_H */

