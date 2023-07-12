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

typedef unsigned int       uint32_t; 		// ����vscode�쳣��ʾ
#define USART_REC_LEN  			200  		// �����������ֽ��� 200
#define RXBUFFERSIZE            1           // �����С

typedef enum
{
    TMS_OK       = 0x00,
    TMS_ERROR    = 0x01,
    TMS_BUSY     = 0x02,
    TMS_TIMEOUT  = 0x03
} TMS_FunStatusTypeDef;//����ִ��״̬

typedef enum
{
    TMS_Null       = 0x00,
    TMS_Inited     = 0x01
} TMS_StatusTypeDef;//����ִ��״̬

typedef enum
{
    TMS_STOP        = 0x00,
    TMS_GET_STATE   = 0x01,
    TMS_CHECK       = 0x02,
    TMS_CMD         = 0x03
} TMS_StateTypeDef;//״̬������



typedef enum
{
    CoolingStop         = 0x00,
    CoolingStart        = 0x01,
    CoolingSetTemp      = 0x02,
    CoolingGetData      = 0x03
} CMDCodeDef;//ˮ�������״̬������

typedef enum
{
    SYSTEM_ON               = 0x00,         /*  ����ϵͳ����*/
    SYSTEM_OFF              = 0x01,         /*  ����ϵͳ�ػ�*/
    SYSTEM_GET_DATA         = 0x02,         /*  ��ȡҺ������*/
    SYSTEM_SET_TEMP_DATA    = 0x03          /*  ����Ŀ���¶�*/
} TMS_OperateTypeDef;                   //�����붨��

#pragma pack(1)
typedef struct
{
    uint16_t TMSRunState;           //0x0000 ���俪��/�ػ�
    uint16_t PIDProportion ;        //0x0001 PID����ֵ(0-250)
    uint16_t PIDIntegral;           //0x0002 PID����ֵ(0-250)
    uint16_t PIDDifferential;       //0x0003 PID΢��ֵ(0-250)
    uint16_t PIDSelfTune;           //0x0004 PID���������أ�ȡֵ��Χ0-1
    uint16_t TargetTemperature;     //0x0005 �趨�¶� ��ʼ��ֵ20��
    uint16_t LowAlarmTemperature;   //0x0006 ���±���
    uint16_t HighAlarmTemperature;  //0x0007 ���±���
    uint16_t TemperatureCalibration;//0x0008 �¶�У׼
    uint16_t OutletTemperature;     //0x0009 �����¶�
    uint16_t InletTemperature;      //0x000A ����¶�
    uint16_t HotSideTemperature;    //0x000B �ȶ��¶�      
    uint16_t TMSTemperature4;       //0x000C �����¶�4
    uint16_t TMSTemperature5;       //0x000D �����¶�5
    uint16_t TMSTemperature6;       //0x000E �����¶�6
    uint16_t WaterPumpFlowRate;     //0x000F ˮ������
    uint16_t liquidheight;          //0x0010 Һλ�߶�
    uint16_t PSD;                   //0x0011 �Ĵ���״̬
    uint16_t CoolRevsionYear;       //0x0012 ��汾��
    uint16_t CoolRevsionMoDa;       //0x0013 ���հ汾��
} Modbus_Report_Pack_TypeDef;
#pragma pack()


#pragma pack(1)
typedef struct
{
    uint8_t TMSRunState;        //0x0000 ����/�ػ�
    uint8_t TMSFanERR ;         //0x0001 ���ȱ���
    uint8_t TMSPumpERR;         //0x0002 ˮ�ñ���
    uint8_t TMSHotSideERR;      //0x0003 �ȶ˱���
    uint8_t TMSLowTempERR;      //0x0004 ���±���
    uint8_t TMSHighTempERR;     //0x0005 ���±���
    uint8_t TMSPumpFlowERR;     //0x0006 ���ٱ���
    uint8_t TMSLiquidLevelERR;  //0x0007 Һλ����
    uint8_t TMSERRflag;         //0x0008 Һ�����
} TMS_PSD_TypeDef;
#pragma pack()


/**
 * @brief ʵ��������ж��������
 * 
 */
typedef struct
{
    Modbus_Report_Pack_TypeDef modbusReport;
    CMDCodeDef CMDCode;
    UART_HandleTypeDef *huart;
    TMS_PSD_TypeDef TMS_PSD;
    float currentTemperature;
    float targetTemperature;
    uint8_t modbus_count;
    // char *info[1000];                       /* Һ���������Ϣ����,1k����*/
    TMS_StatusTypeDef TMSSYSstatus;
    TMS_FunStatusTypeDef (* Init)();       /*!< �����û�ͨѶ�ӿ�   */
    TMS_FunStatusTypeDef (* Run)();        /*!< ����Һ�������  ���鹤��Ƶ��20hz   */      
    TMS_FunStatusTypeDef (* Stop)();       /*!< ֹͣҺ�������     */      
    void (* RxCplt)();                      /*!< Һ��������������ݴ���     */      
    TMS_FunStatusTypeDef (* UpdataPack)();                  /*!< ͨ�����ڷ���modbus�������Һ�����ݼĴ���   */      
    
} TMS_HandleTypeDef;

extern TMS_HandleTypeDef* TMS_Handle; //Һ���������������
TMS_FunStatusTypeDef TMSCreate( UART_HandleTypeDef *huartTMS);



#endif /* __TMS_MODBUS_PROTOCOL_H */

