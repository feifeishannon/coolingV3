/**
 ******************************************************************************
 * @file    : cooling_ModBus_Protocol.c
 * @brief   : TMS 液冷协议应用程序
 * @version : 1.0.1
 ******************************************************************************
 * @attention
 * 
 * 
 ******************************************************************************
 */
#include "cooling_ModBus_Protocol.h"

#include "usb_device.h"

#define USART_REC_LEN 200 
#define RXBUFFERSIZE 1   

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;


uint8_t BAT_DATA_Pack = 0;


uint8_t CoolingRecvBuf[50] = {0};    //接收BUF
uint8_t CoolingSendBuf[50] = {0};    //发送BUF
uint8_t CoolingDataBuf[50] = {0};    //水冷数据BUF

uint8_t CoolingWorkStatus = 0;       //工作状态
uint8_t CoolingCount = 0;            //计数器
uint8_t CoolingRecvCount = 0;        //接收计数器
uint8_t CoolingSendCount = 0;        //发送计数器
uint8_t CoolingRecvNum = 0;          //接收个数
uint8_t CoolingSendNum = 0;          //发送个数
uint8_t CoolingTimeOutFlag = 0;      //超时使能
uint32_t CoolingTimeOutCount = 0;    //超时计数器


uint8_t Rx_dat = 0;

uint8_t USART_RX_BUF[USART_REC_LEN]; 
uint16_t USART_RX_STA;
uint8_t aRxBuffer[RXBUFFERSIZE];

// Modbus_Report_Pack_TypeDef Modbus_Report_Pack = {0};  //水冷实时数据
Cooling_HandleTypeDef* Cooling_Handle;

uint8_t aucCoolingGetALL[8]			=	{0x01, 0x03, 0x20, 0x00, 0x00, 0x03, 0x0E, 0x0B};	 //查询前三寄存器
uint8_t aucCoolingGetTemp[8]		=	{0x01, 0x03, 0x20, 0x00, 0x00, 0x01, 0x8F, 0xCA};//查询当前温度
uint8_t aucCoolingGetState[8]		=	{0x01, 0x03, 0x20, 0x02, 0x00, 0x01, 0x2E, 0x0A};//查询当前状态
uint8_t aucCoolingOFFCmd[8]			=	{0x01, 0x06, 0x20, 0x31, 0x00, 0x00, 0xD3, 0xC5};//关机指令
uint8_t aucCoolingONCmd[8]			=	{0x01, 0x06, 0x20, 0x31, 0x00, 0x03, 0x93, 0xC4};//开机指令
uint8_t aucCoolingTargTempCmd[8]	=	{0x01, 0x06, 0x20, 0x03, 0x03, 0xE8, 0x72, 0xB4};//设置目标温度为10°
uint8_t aucCooling06Cmd[8]			=	{0};
uint8_t aucCooling10Cmd[27]			=	{0};

uint8_t modbus_slave_addr = 0x01; // 长流水冷机地址
uint8_t modbus_Tx_buff[100];	  // 发送缓冲区

unsigned char *data; 
unsigned char length; 
/**
 * @brief 标准modbus-CRC
 * 
 */
static uint16_t CRC16(unsigned char *data,unsigned char length) { 
	unsigned char j;
	unsigned int reg_crc=0xffff; 
	while(length--){ 
		reg_crc^=*data++; 
		for(j=0;j<8;j++){ 
			if(reg_crc&0x01){ 
				reg_crc=(reg_crc>>1)^0xa001;
			} else{ 
				reg_crc=reg_crc>>1; 
			} 
		} 
	} 
	return reg_crc; 
}

static void send_data(uint8_t *buff, uint8_t len)
{
	HAL_UART_Transmit_IT(Cooling_Handle->huart, (uint8_t *)buff, len); // 发送数据   把buff
	// while (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC) != SET); // 等待发送结束
}

static void send_8data(uint8_t *buff)
{
	send_data((uint8_t *)buff, 8);
}

// 设置液冷开机
static void CoolingOperateSystemON(){
	send_8data(aucCoolingONCmd);
}

// 设置液冷关机
static void CoolingOperateSystemOFF(){
	send_8data(aucCoolingOFFCmd);

}

// 获取液冷温度
static void CoolingOperateGetTemp(){
	send_8data(aucCoolingGetTemp);

}

// 获取液冷状态
static void CoolingOperateGetState(){
	send_8data(aucCoolingGetState);

}

// 获取液冷所有状态
static void CoolingOperateGetALL(){
	send_8data(aucCoolingGetALL);

}

// 设置液冷温度
static void CoolingOperateSetTemp(uint8_t value){
	uint16_t crcCheck = 0;
	aucCoolingTargTempCmd[4] = ((value) * 100) / 256;
	aucCoolingTargTempCmd[5] = ((value) * 100) % 256;
	
	crcCheck = CRC16(aucCoolingTargTempCmd, 6);
	aucCoolingTargTempCmd[6] = crcCheck / 256;
	aucCoolingTargTempCmd[7] = crcCheck % 256;

	send_8data(aucCoolingTargTempCmd);

}



static void CoolingOperate(Cooling_OperateTypeDef operateCMD, uint8_t value){

	switch(operateCMD)
	{
        
		case SYSTEM_ON:
            CoolingOperateSystemON();
			break;

		case SYSTEM_OFF:
            CoolingOperateSystemOFF();
            break;

		case SYSTEM_GET_TEMP:
            CoolingOperateGetTemp();
            break;

		case SYSTEM_SET_TEMP_DATA:
            CoolingOperateSetTemp(value);
            break;

		case SYSTEM_GET_STATE_DATA:
            CoolingOperateGetState();
            break;
		
		case SYSTEM_GET_ALL_DATA:
            CoolingOperateGetALL();
            break;

		default:
			return;
			
	}
}



static void updataPSD(Cooling_ChangLiu_PSD_TypeDef* Cooling_PSD, uint16_t CoolingRunningState) {
    Cooling_PSD->CoolingRunState = (CoolingRunningState >> 7) & 0x01;
    Cooling_PSD->CoolingHeatState = (CoolingRunningState >> 6) & 0x01;
    Cooling_PSD->CoolingCoolingState = (CoolingRunningState >> 5) & 0x01;
    Cooling_PSD->CoolingPumpFlowAlarm = (CoolingRunningState >> 4) & 0x01;
    Cooling_PSD->CoolingPumpState = (CoolingRunningState >> 3) & 0x01;
    Cooling_PSD->CoolingTempAlarm = (CoolingRunningState >> 2) & 0x01;
    Cooling_PSD->CoolingLiquidLevelAlarm = (CoolingRunningState >> 1) & 0x01;
    Cooling_PSD->CoolingPower = CoolingRunningState & 0x01;
}


static void modbus_03_Receivefunction(uint8_t data_len)
{
	uint16_t value;
	uint16_t * ptr;
	
	for (size_t i = 0; i < data_len/2; i++)
	{
		value = (uint16_t)((USART_RX_BUF[i * 2  + 3 ] << 8) | USART_RX_BUF[i * 2 + 3 + 1]);
		ptr = (uint16_t*)&(Cooling_Handle->modbusReport);
		ptr[i] = value;
		
	}
	Cooling_Handle->currentTemperature = (float)Cooling_Handle->modbusReport.WaterTankTemperature/100;
	printfln("当前温度：%f",Cooling_Handle->currentTemperature);
	updataPSD(&Cooling_Handle->Cooling_PSD, Cooling_Handle->modbusReport.CoolingRunningState);
}

static void CoolingModbus_service(){
	uint16_t data_CRC_value;   
	uint16_t data_len;		   
	uint16_t CRC_check_result; 
	if (USART_RX_STA & 0x8000){
		data_len = USART_RX_STA & 0x3fff;															 
		CRC_check_result = CRC16(USART_RX_BUF, data_len - 2);
		data_CRC_value = USART_RX_BUF[data_len - 1] << 8 | (((uint16_t)USART_RX_BUF[data_len - 2])); 
		if (CRC_check_result == data_CRC_value)
		{
			if (USART_RX_BUF[0] == modbus_slave_addr)
			{
				switch (USART_RX_BUF[1])
				{
				case 03: 
				{
					modbus_03_Receivefunction(USART_RX_BUF[2]);
					break;
				}
				case 06: 
				{
					
					break;
				}
				case 16: 
				{
					
					break;
				}
				}
			}
		}
		USART_RX_STA = 0; 
	}
}

/**
 * @brief 串口接收函数，需要在串口中断中配置对本函数的调用
 * 
 */
static void RxCplt(void)
{
	
	if ((USART_RX_STA & 0x8000) == 0) 
	{
		Cooling_Handle->modbus_count = 0;
		USART_RX_BUF[USART_RX_STA & 0X3FFF] = aRxBuffer[0];
		USART_RX_STA++;
		if (USART_RX_STA > (USART_REC_LEN - 1))
			USART_RX_STA = 0; 
	}

	HAL_UART_Receive_IT(Cooling_Handle->huart, (uint8_t *)aRxBuffer, 1);

	
}

// 循环获取温度和状态
static void CoolingWorkCMD(){
	CoolingCount++;

	

	if(CoolingCount == 1) //	更新频率为20hz时，每0.5秒触发一次
	{
		CoolingOperate(SYSTEM_GET_ALL_DATA,NULL);
		#ifdef USB_DEBUG
			printfln("CoolingOperate:SYSTEM_GET_STATE_DATA=>%d",Cooling_Handle->Cooling_PSD.CoolingCoolingState);
		#endif
		if(Cooling_Handle->Cooling_PSD.CoolingCoolingState == 1)
		{
			CoolingWorkStatus = Cooling_CMD;
		}
		else
		{
			CoolingWorkStatus = Cooling_STOP;
		}
	}
	else if (CoolingCount == 2){
		/**@TODO:   
		 * 1、通过TMS目标温度设定值设置水冷目标温度设定值
		 * 		目标值和当前设定值比较，有变化后更新水冷目标温度
		 * 2、通过水冷当前温度值，控制是否启动制冷
		 * 3、低于某温度时关闭制冷
		 * 4、高于某温度时开启制冷
		 * 5、开关频率要大于10分钟每次
		 */

	}
	
	else if(CoolingCount >= 3) //更新频率为20hz时，每1.5秒触发一次
	{
		CoolingCount = 0; 
	}
}


/**
 * @brief 液冷控制器启动函数，液冷状态机
 *        需要将此函数放入主函数体循环中
 *        或创建轮询任务
 * @param hcooling 
 */
static Cooling_FunStatusTypeDef Run(uint8_t BAT_DATA_Pack){
	// BAT_DATA_Pack的初值受S485信号通讯控制,当信号为1时启动开机指令
    static Cooling_StateTypeDef CoolingWorkStatus = Cooling_STOP ;
	#ifdef USB_DEBUG
		printfln("CoolingWorkStatus:%d",CoolingWorkStatus);
	#endif
    if(BAT_DATA_Pack  > 0){
        switch(CoolingWorkStatus){
			case Cooling_STOP:
			{//关机状态下发送开机指令
				CoolingWorkStatus = Cooling_GET_STATE;
				CoolingOperate(SYSTEM_ON,NULL);
				break;
			}
			case Cooling_GET_STATE:
			{//读取液冷所有寄存器数值
				CoolingWorkStatus = Cooling_CHECK;
				CoolingOperate(SYSTEM_GET_ALL_DATA,NULL);
				break;
			}
			case Cooling_CHECK:
			{//判定水冷工作状态，正常开机继续执行，异常开机返回0步骤
				if(Cooling_Handle->Cooling_PSD.CoolingRunState == 1)
				{
					CoolingWorkStatus = Cooling_CMD;
				}
				else
				{
					CoolingWorkStatus = Cooling_STOP;
				}
				break;
			}
			case Cooling_CMD:
			{//水冷控制
				CoolingWorkCMD();
				break;
			}
			default:
				break;
		}
	}
	else
	{
		CoolingWorkStatus = Cooling_STOP;
		CoolingOperate(SYSTEM_OFF,NULL);
	
    }
	return Cooling_OK;
}

/**
 * @brief 液冷控制器停止函数
 *        
 * @todo  暂时无用
 * @param hcooling 
 */
static Cooling_FunStatusTypeDef Stop(){

	return Cooling_OK;
}

/**
 * @brief 液冷控制器初始化函数
 *        
 * @todo  初始化所需数据,读取版本号
 *  
 */
static Cooling_FunStatusTypeDef Init(){
	USART_RX_STA = 0; // 准备接收
    

	return Cooling_OK;
}

static Cooling_FunStatusTypeDef UpdataPack(){

	if(Cooling_Handle->modbus_count > 4 && ((USART_RX_STA & 0X3FFF) != 0)){
		USART_RX_STA |= 0x8000;
		CoolingModbus_service();
	}
	return Cooling_OK;
}


/**
 * @brief 液冷控制器注册函数
 *        绑定所需结构函数
 * @todo  串口绑定方式待定
 * @param huartcooling:绑定收发数据接口
 */
Cooling_FunStatusTypeDef CoolingCreate( UART_HandleTypeDef *huartcooling)
{
	Cooling_Handle = malloc(sizeof(Cooling_HandleTypeDef));

	Cooling_Handle->Run				= Run;
	Cooling_Handle->Stop			= Stop;
	Cooling_Handle->Init			= Init;
	Cooling_Handle->UpdataPack		= UpdataPack;
	Cooling_Handle->RxCplt			= RxCplt;
	
	Cooling_Handle->huart = huartcooling;
	HAL_UART_Receive_IT(Cooling_Handle->huart, (uint8_t *)aRxBuffer, 1);
	Cooling_Handle->coolingSYSstatus = Cooling_Inited;
	
	return Cooling_OK;
}

