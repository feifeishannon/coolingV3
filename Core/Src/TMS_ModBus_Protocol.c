/**
 ******************************************************************************
 * @file    : TMS_ModBus_Protocol.c
 * @brief   : TMS 液冷协议应用程序
 * @version : 1.0.1
 ******************************************************************************
 * @attention
 * 
 * 
 ******************************************************************************
 */
#include "TMS_ModBus_Protocol.h"


#define USART_REC_LEN 200 
#define RXBUFFERSIZE 1   




uint8_t TMSRecvBuf[50] = {0};    //接收BUF
uint8_t TMSSendBuf[50] = {0};    //发送BUF
uint8_t TMSDataBuf[50] = {0};    //水冷数据BUF

uint8_t TMSWorkStatus = 0;       //工作状态
uint8_t TMSCount = 0;            //计数器
uint8_t TMSRecvCount = 0;        //接收计数器
uint8_t TMSSendCount = 0;        //发送计数器
uint8_t TMSRecvNum = 0;          //接收个数
uint8_t TMSSendNum = 0;          //发送个数
uint8_t TMSTimeOutFlag = 0;      //超时使能
uint32_t TMSTimeOutCount = 0;    //超时计数器

uint8_t Rx_dat = 0;

uint8_t USART_RX_BUF[USART_REC_LEN]; 
uint16_t USART_RX_STA;
uint8_t aRxBuffer[RXBUFFERSIZE];

TMS_HandleTypeDef* TMS_Handle;


// uint8_t aucTMSCHECKALLCmd[8]	=  {0xAA, 0x03, 0x00, 0x00, 0x00, 0x12, 0xDC, 0x1C};//查询12个寄存器指令
// uint8_t aucTMSOFFCmd[8]			=  {0xAA, 0x06, 0x00, 0x00, 0x00, 0x00, 0x90, 0x11};//关机指令
// uint8_t aucTMSONCmd[27]			=  {0xAA, 0x10, 0x00, 0x00, 0x00, 0x09, 0x12, 0x00, 0x01, 
// 											0x00, 0x64, 0x00, 0x01, 0x00, 0x05, 0x00, 0x00, 0x02, 
//                                 			0x58, 0x03, 0x20, 0x03, 0x20, 0x01, 0xF4, 0x21, 0x6A}; //开机指令，目标温度10度
// uint8_t aucTMSTargTempCmd[8]	=	{0xAA, 0x06, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00};//设置指令
// uint8_t aucTMS06Cmd[8]			=	{0xAA, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};//设置指令
// uint8_t aucTMS10Cmd[27]			=	{0xAA, 0x10, 0x00, 0x00, 0x00, 0x09, 0x12, 0x00, 0x01, 	\
// 									0x00, 0x64, 0x00, 0x01, 0x00, 0x05, 0x00, 0x00, 0x02, 	\
// 									0x58, 0x03, 0x20, 0x03, 0x20, 0x01, 0xF4, 0x00, 0x00}; //开机指令，目标温度10度
// 									// 校验结果是6A21 



uint8_t modbus_slave_addr = 0x01; // 长流水冷机地址
uint8_t modbus_Tx_buff[100];	  // 发送缓冲区

uint16_t targetTemp = 0;

void copyArray(int source[], int target[], int length) {
    memcpy(target, source, length * sizeof(int));
}

void aucTMS(){
    TMS_Handle->modbus_count++;
}

uint8_t auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40};

char auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
0x43, 0x83, 0x41, 0x81, 0x80, 0x40};

static uint16_t CRC16(uint8_t *puchMsg, uint16_t usDataLen)
{
	uint8_t uchCRCHi = 0xFF;
	uint8_t uchCRCLo = 0xFF;
	unsigned uIndex = 0;
	while (usDataLen--)
	{
		uIndex = uchCRCHi ^ *puchMsg++;
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
		uchCRCLo = auchCRCLo[uIndex];
	}
	return (uchCRCHi<<8 | uchCRCLo);
}

static void send_data(uint8_t *buff, uint8_t len)
{
	HAL_UART_Transmit_IT(TMS_Handle->huart, (uint8_t *)buff, len); // 发送数据   把buff
	// while (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC) != SET); // 等待发送结束
}

static void TMSOperateSystemON(){
	send_data(aucTMSONCmd, 27);
}

static void TMSOperateSystemOFF(){
	send_data(aucTMSOFFCmd, 8);

}

static void TMSOperateGetData(){
	send_data(aucTMSCHECKALLCmd, 8);

}

static void TMSOperateSetTemp(uint8_t value){
	uint16_t crcCheck = 0;
	aucTMSTargTempCmd[4] = ((value + 50) * 10) / 256;
	aucTMSTargTempCmd[5] = ((value + 50) * 10) % 256;
	
	crcCheck = CRC16(aucTMSTargTempCmd, 6);
	aucTMSTargTempCmd[6] = crcCheck / 256;
	aucTMSTargTempCmd[7] = crcCheck % 256;

	send_data(aucTMSTargTempCmd, 8);

}



static void TMSOperate(TMS_OperateTypeDef operateCMD, uint8_t value){

	switch(operateCMD)
	{
        
		case SYSTEM_ON:
            TMSOperateSystemON();

			break;

		case SYSTEM_OFF:
            TMSOperateSystemOFF();
			
            break;

		case SYSTEM_GET_DATA:
            TMSOperateGetData();

            break;

		case SYSTEM_SET_TEMP_DATA:
            TMSOperateSetTemp(value);

            break;

		default:
			return;
			
	}
}

static void	updataPSD(){
	uint8_t	index=0;
	uint8_t	bitindex=0;
	if(TMS_Handle->modbusReport.PSD & (1 << index++)){//水位报警
		//水位有三个状态
		TMS_Handle->TMS_PSD.TMSLiquidLevelERR = TMS_Handle->modbusReport.liquidheight;
		TMS_Handle->TMS_PSD.TMSERRflag |= (1 << bitindex++);
	}else{
		TMS_Handle->TMS_PSD.TMSLiquidLevelERR = 0;
		TMS_Handle->TMS_PSD.TMSERRflag &= ~(1 << bitindex++);
	}

	if(TMS_Handle->modbusReport.PSD & (1 << index++)){//水流报警
		TMS_Handle->TMS_PSD.TMSPumpFlowERR = 1;
		TMS_Handle->TMS_PSD.TMSERRflag |= (1 << bitindex++);
	}else{
		TMS_Handle->TMS_PSD.TMSPumpFlowERR = 0;
		TMS_Handle->TMS_PSD.TMSERRflag &= ~(1 << bitindex++);
	}

	if(TMS_Handle->modbusReport.PSD & (1 << index++)){//高温报警
		TMS_Handle->TMS_PSD.TMSHighTempERR = 1;
		TMS_Handle->TMS_PSD.TMSERRflag |= (1 << bitindex++);
	}else{
		TMS_Handle->TMS_PSD.TMSHighTempERR = 0;
		TMS_Handle->TMS_PSD.TMSERRflag &= ~(1 << bitindex++);
	}

	if(TMS_Handle->modbusReport.PSD & (1 << index++)){//低温报警
		TMS_Handle->TMS_PSD.TMSLowTempERR = 1;
		TMS_Handle->TMS_PSD.TMSERRflag |= (1 << bitindex++);
	}else{
		TMS_Handle->TMS_PSD.TMSLowTempERR = 0;
		TMS_Handle->TMS_PSD.TMSERRflag &= ~(1 << bitindex++);
	}

	if(TMS_Handle->modbusReport.PSD & (1 << index++)){//热端报警
		TMS_Handle->TMS_PSD.TMSHotSideERR = 1;
		TMS_Handle->TMS_PSD.TMSERRflag |= (1 << bitindex++);
	}else{
		TMS_Handle->TMS_PSD.TMSHotSideERR = 0;
		TMS_Handle->TMS_PSD.TMSERRflag &= ~(1 << bitindex++);
	}

	if(TMS_Handle->modbusReport.PSD & (1 << index++)){//水泵报警
		TMS_Handle->TMS_PSD.TMSPumpERR = 1;
		TMS_Handle->TMS_PSD.TMSERRflag |= (1 << bitindex++);
	}else{
		TMS_Handle->TMS_PSD.TMSPumpERR = 0;
		TMS_Handle->TMS_PSD.TMSERRflag &= ~(1 << bitindex++);
	}

	if(TMS_Handle->modbusReport.PSD & (1 << index++)){//风扇报警
		TMS_Handle->TMS_PSD.TMSFanERR = 1;
		TMS_Handle->TMS_PSD.TMSERRflag |= (1 << bitindex++);
	}else{
		TMS_Handle->TMS_PSD.TMSFanERR = 0;
		TMS_Handle->TMS_PSD.TMSERRflag &= ~(1 << bitindex++);
	}
	
	if(TMS_Handle->modbusReport.PSD & (1 << index++)){//制冷开关
		TMS_Handle->TMS_PSD.TMSRunState = 1;
		TMS_Handle->TMS_PSD.TMSERRflag |= (1 << bitindex++);
	}else{
		TMS_Handle->TMS_PSD.TMSRunState = 0;
		TMS_Handle->TMS_PSD.TMSERRflag &= ~(1 << bitindex++);
	}

}

static void modbus_03_Receivefunction(uint8_t data_len)
{
	uint16_t value;
	uint16_t * ptr;
	
	for (size_t i = 0; i < 20; i++)
	{
		
		value = (uint16_t)((USART_RX_BUF[i * 2  + 3 ] << 8) | USART_RX_BUF[i * 2 + 3 + 1]);
		ptr = (uint16_t*)&(TMS_Handle->modbusReport);
		ptr[i] = value;
		
	}
	if(data_len<42)
	{
		TMS_Handle->modbusReport.CoolRevsionYear = 0;	
		TMS_Handle->modbusReport.CoolRevsionMoDa = 0;	
	}
	updataPSD();
	
	
}

static void TMSModbus_service(){
	uint16_t data_CRC_value;   
	uint16_t data_len;		   
	uint16_t CRC_check_result; 
	if (USART_RX_STA & 0x8000){
		data_len = USART_RX_STA & 0x3fff;															 
		CRC_check_result = CRC16(USART_RX_BUF, data_len - 2);
		data_CRC_value = USART_RX_BUF[data_len - 2] << 8 | (((uint16_t)USART_RX_BUF[data_len - 1])); 
		if (CRC_check_result == data_CRC_value)
		{
			if (USART_RX_BUF[0] == modbus_slave_addr)
			{
				switch (USART_RX_BUF[1])
				{
				case 03: 
				{
					modbus_03_Receivefunction(data_len);
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
		TMS_Handle->modbus_count = 0;
		USART_RX_BUF[USART_RX_STA & 0X3FFF] = aRxBuffer[0];
		USART_RX_STA++;
		if (USART_RX_STA > (USART_REC_LEN - 1))
			USART_RX_STA = 0; 
	}
	HAL_UART_Receive_IT(TMS_Handle->huart, (uint8_t *)aRxBuffer, 1);
}


static void TMSWorkCMD(){
	TMSCount++;
	if(TMSCount == 1) //	更新频率为20hz时，每0.5秒触发一次
	{
		TMSOperate(SYSTEM_GET_DATA,NULL);
		
		if(TMS_Handle->modbusReport.TMSRunState == 1)
		{
			TMSWorkStatus = TMS_CMD;
		}
		else
		{
			TMSWorkStatus = TMS_STOP;
		}
	}
	else if(TMSCount == 2) //更新频率为20hz时，每1秒触发一次
	{
		if(targetTemp > 25){
			TMSOperate(SYSTEM_SET_TEMP_DATA,5);
		}
		else if(targetTemp > 20 && targetTemp <= 25)
		{
			TMSOperate(SYSTEM_SET_TEMP_DATA,7);
		}
		else if(targetTemp > 18 && targetTemp <= 20)
		{
			TMSOperate(SYSTEM_SET_TEMP_DATA,9);
		}
		else if(targetTemp > 16 && targetTemp <= 18)
		{
			//盲区
		}
		else if(targetTemp > 5 && targetTemp <= 16)
		{
			TMSOperate(SYSTEM_SET_TEMP_DATA,17);
		}
		else if(targetTemp < 5)
		{
			TMSOperate(SYSTEM_SET_TEMP_DATA,17);
		}
	}
	else if(TMSCount >= 3) //更新频率为20hz时，每1.5秒触发一次
	{
		TMSCount = 0; 
	}
}


/**
 * @brief 液冷控制器启动函数，液冷状态机
 *        需要将此函数放入主函数体循环中
 *        或创建轮询任务
 * @todo  需完成数据更新、断帧判定、目标温度计算、报警处理
 *        1. 获取液冷模组数据状态
 *        2. 判断开机调节，查询是否开机
 *        3. 检查开机是否完成，
 *        4. 
 * @param hTMS 
 */
static TMS_FunStatusTypeDef Run(){
    static TMS_StateTypeDef TMSWorkStatus = TMS_STOP ;
	// uint8_t BAT_DATA_Pack =0 ;
    if(BAT_DATA_Pack  > 0){
        switch(TMSWorkStatus){
			case TMS_STOP:
			{//关机状态下发送开机指令
				TMSWorkStatus = TMS_GET_STATE;
				TMSOperate(SYSTEM_ON,NULL);
				break;
			}
			case TMS_GET_STATE:
			{//读取液冷所有寄存器数值
				TMSWorkStatus = TMS_CHECK;
				TMSOperate(SYSTEM_GET_DATA,NULL);
				break;
			}
			case TMS_CHECK:
			{//判定水冷工作状态，正常开机继续执行，异常开机返回0步骤
				if(TMS_Handle->modbusReport.TMSRunState == 1)
				{
					TMSWorkStatus = TMS_CMD;
				}
				else
				{
					TMSWorkStatus = TMS_STOP;
				}
				break;
			}
			case TMS_CMD:
			{//水冷控制
				TMSWorkCMD();
				break;
			}
			default:
				break;
		}
	}
	else
	{
		TMSWorkStatus = TMS_STOP;
		TMSOperate(SYSTEM_OFF,NULL);
	
    }
	return TMS_OK;
}

/**
 * @brief 液冷控制器停止函数
 *        
 * @todo  暂时无用
 * @param hTMS 
 */
static TMS_FunStatusTypeDef Stop(){

	return TMS_OK;
}

/**
 * @brief 液冷控制器初始化函数
 *        
 * @todo  初始化所需数据,读取版本号
 *  
 */
static TMS_FunStatusTypeDef Init(){
	USART_RX_STA = 0; // 准备接收
    

	return TMS_OK;
}

static TMS_FunStatusTypeDef UpdataPack(){

	if(TMS_Handle->modbus_count > 4 && ((USART_RX_STA & 0X3FFF) != 0)){
		USART_RX_STA |= 0x8000;
		TMSModbus_service();
	}
	return TMS_OK;
}


/**
 * @brief 液冷控制器注册函数
 *        绑定所需结构函数
 * @todo  串口绑定方式待定
 * @param huartTMS:绑定收发数据接口
 */
TMS_FunStatusTypeDef TMSCreate( UART_HandleTypeDef *huartTMS)
{
	TMS_Handle = malloc(sizeof(TMS_HandleTypeDef));

	TMS_Handle->Run				= Run;
	TMS_Handle->Stop			= Stop;
	TMS_Handle->Init			= Init;
	TMS_Handle->UpdataPack		= UpdataPack;
	TMS_Handle->RxCplt			= RxCplt;
	
	TMS_Handle->huart = huartTMS;
	HAL_UART_Receive_IT(TMS_Handle->huart, (uint8_t *)aRxBuffer, 1);
	TMS_Handle->TMSSYSstatus = TMS_Inited;
	
	return TMS_OK;
}

