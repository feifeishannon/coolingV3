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


#define USART_REC_LEN 200 
#define RXBUFFERSIZE 1   




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


uint8_t aucCoolingCHECKALLCmd[8]	=  {0xAA, 0x03, 0x00, 0x00, 0x00, 0x12, 0xDC, 0x1C};//查询12个寄存器指令
uint8_t aucCoolingOFFCmd[8]		=  {0xAA, 0x06, 0x00, 0x00, 0x00, 0x00, 0x90, 0x11};//关机指令
uint8_t aucCoolingONCmd[27]		=  {0xAA, 0x10, 0x00, 0x00, 0x00, 0x09, 0x12, 0x00, 0x01, 
											0x00, 0x64, 0x00, 0x01, 0x00, 0x05, 0x00, 0x00, 0x02, 
                                			0x58, 0x03, 0x20, 0x03, 0x20, 0x01, 0xF4, 0x21, 0x6A}; //开机指令，目标温度10度
uint8_t aucCoolingTargTempCmd[8]		=	{0xAA, 0x06, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00};//设置指令
uint8_t aucCooling06Cmd[8]				=	{0xAA, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};//设置指令
uint8_t aucCooling10Cmd[27]				=	{0xAA, 0x10, 0x00, 0x00, 0x00, 0x09, 0x12, 0x00, 0x01, 	\
											0x00, 0x64, 0x00, 0x01, 0x00, 0x05, 0x00, 0x00, 0x02, 	\
                							0x58, 0x03, 0x20, 0x03, 0x20, 0x01, 0xF4, 0x00, 0x00}; //开机指令，目标温度10度
											// 校验结果是6A21 

uint8_t modbus_slave_addr = 0xAA; // 从机地址
uint8_t modbus_Tx_buff[100];	  // 发送缓冲区

void copyArray(int source[], int target[], int length) {
    memcpy(target, source, length * sizeof(int));
}

void aucCooling(){
    Cooling_Handle->modbus_count++;
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

unsigned char *data; 
unsigned char length; 
/**
 * @brief 标准modbus-CRC
 * 
 */
unsigned int crc_chk(unsigned char *data,unsigned char length) { 
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

static void CoolingOperateSystemON(){
	send_data(aucCoolingONCmd, 27);
}

static void CoolingOperateSystemOFF(){
	send_data(aucCoolingOFFCmd, 8);

}

static void CoolingOperateGetData(){
	send_data(aucCoolingCHECKALLCmd, 8);

}

static void CoolingOperateSetTemp(uint8_t value){
	uint16_t crcCheck = 0;
	aucCoolingTargTempCmd[4] = ((value + 50) * 10) / 256;
	aucCoolingTargTempCmd[5] = ((value + 50) * 10) % 256;
	
	crcCheck = CRC16(aucCoolingTargTempCmd, 6);
	aucCoolingTargTempCmd[6] = crcCheck / 256;
	aucCoolingTargTempCmd[7] = crcCheck % 256;

	send_data(aucCoolingTargTempCmd, 8);

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

		case SYSTEM_GET_DATA:
            CoolingOperateGetData();

            break;

		case SYSTEM_SET_TEMP_DATA:
            CoolingOperateSetTemp(value);

            break;

		default:
			return;
			
	}
}

static void	updataPSD(){
	uint8_t	index=0;
	uint8_t	bitindex=0;
	if(Cooling_Handle->modbusReport.PSD & (1 << index++)){//水位报警
		//水位有三个状态
		Cooling_Handle->Cooling_PSD.CoolingLiquidLevelERR = Cooling_Handle->modbusReport.liquidheight;
		Cooling_Handle->Cooling_PSD.CoolingERRflag |= (1 << bitindex++);
	}else{
		Cooling_Handle->Cooling_PSD.CoolingLiquidLevelERR = 0;
		Cooling_Handle->Cooling_PSD.CoolingERRflag &= ~(1 << bitindex++);
	}

	if(Cooling_Handle->modbusReport.PSD & (1 << index++)){//水流报警
		Cooling_Handle->Cooling_PSD.CoolingPumpFlowERR = 1;
		Cooling_Handle->Cooling_PSD.CoolingERRflag |= (1 << bitindex++);
	}else{
		Cooling_Handle->Cooling_PSD.CoolingPumpFlowERR = 0;
		Cooling_Handle->Cooling_PSD.CoolingERRflag &= ~(1 << bitindex++);
	}

	if(Cooling_Handle->modbusReport.PSD & (1 << index++)){//高温报警
		Cooling_Handle->Cooling_PSD.CoolingHighTempERR = 1;
		Cooling_Handle->Cooling_PSD.CoolingERRflag |= (1 << bitindex++);
	}else{
		Cooling_Handle->Cooling_PSD.CoolingHighTempERR = 0;
		Cooling_Handle->Cooling_PSD.CoolingERRflag &= ~(1 << bitindex++);
	}

	if(Cooling_Handle->modbusReport.PSD & (1 << index++)){//低温报警
		Cooling_Handle->Cooling_PSD.CoolingLowTempERR = 1;
		Cooling_Handle->Cooling_PSD.CoolingERRflag |= (1 << bitindex++);
	}else{
		Cooling_Handle->Cooling_PSD.CoolingLowTempERR = 0;
		Cooling_Handle->Cooling_PSD.CoolingERRflag &= ~(1 << bitindex++);
	}

	if(Cooling_Handle->modbusReport.PSD & (1 << index++)){//热端报警
		Cooling_Handle->Cooling_PSD.CoolingHotSideERR = 1;
		Cooling_Handle->Cooling_PSD.CoolingERRflag |= (1 << bitindex++);
	}else{
		Cooling_Handle->Cooling_PSD.CoolingHotSideERR = 0;
		Cooling_Handle->Cooling_PSD.CoolingERRflag &= ~(1 << bitindex++);
	}

	if(Cooling_Handle->modbusReport.PSD & (1 << index++)){//水泵报警
		Cooling_Handle->Cooling_PSD.CoolingPumpERR = 1;
		Cooling_Handle->Cooling_PSD.CoolingERRflag |= (1 << bitindex++);
	}else{
		Cooling_Handle->Cooling_PSD.CoolingPumpERR = 0;
		Cooling_Handle->Cooling_PSD.CoolingERRflag &= ~(1 << bitindex++);
	}

	if(Cooling_Handle->modbusReport.PSD & (1 << index++)){//风扇报警
		Cooling_Handle->Cooling_PSD.CoolingFanERR = 1;
		Cooling_Handle->Cooling_PSD.CoolingERRflag |= (1 << bitindex++);
	}else{
		Cooling_Handle->Cooling_PSD.CoolingFanERR = 0;
		Cooling_Handle->Cooling_PSD.CoolingERRflag &= ~(1 << bitindex++);
	}
	
	if(Cooling_Handle->modbusReport.PSD & (1 << index++)){//制冷开关
		Cooling_Handle->Cooling_PSD.CoolingRunState = 1;
		Cooling_Handle->Cooling_PSD.CoolingERRflag |= (1 << bitindex++);
	}else{
		Cooling_Handle->Cooling_PSD.CoolingRunState = 0;
		Cooling_Handle->Cooling_PSD.CoolingERRflag &= ~(1 << bitindex++);
	}

}

static void modbus_03_Receivefunction(uint8_t data_len)
{
	uint16_t value;
	uint16_t * ptr;
	
	for (size_t i = 0; i < 20; i++)
	{
		
		value = (uint16_t)((USART_RX_BUF[i * 2  + 3 ] << 8) | USART_RX_BUF[i * 2 + 3 + 1]);
		ptr = (uint16_t*)&(Cooling_Handle->modbusReport);
		ptr[i] = value;
		
	}
	if(data_len<42)
	{
		Cooling_Handle->modbusReport.CoolRevsionYear = 0;	
		Cooling_Handle->modbusReport.CoolRevsionMoDa = 0;	
	}
	updataPSD();
	
	
}

static void CoolingModbus_service(){
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
		Cooling_Handle->modbus_count = 0;
		USART_RX_BUF[USART_RX_STA & 0X3FFF] = aRxBuffer[0];
		USART_RX_STA++;
		if (USART_RX_STA > (USART_REC_LEN - 1))
			USART_RX_STA = 0; 
	}

	HAL_UART_Receive_IT(Cooling_Handle->huart, (uint8_t *)aRxBuffer, 1);

	
}


static void CoolingWorkCMD(){
	CoolingCount++;
	if(CoolingCount == 1) //	更新频率为20hz时，每0.5秒触发一次
	{
		CoolingOperate(SYSTEM_GET_DATA,NULL);
		
		if(Cooling_Handle->modbusReport.CoolingRunState == 1)
		{
			CoolingWorkStatus = Cooling_CMD;
		}
		else
		{
			CoolingWorkStatus = Cooling_STOP;
		}
	}
	// else if(CoolingCount == 2) //更新频率为20hz时，每1秒触发一次
	// {
	// 	if(BAT_DATA_Pack.BAT_Temperature > 25){
	// 		CoolingOperate(SYSTEM_SET_TEMP_DATA,5);
	// 	}
	// 	else if(BAT_DATA_Pack.BAT_Temperature > 20 && BAT_DATA_Pack.BAT_Temperature <= 25)
	// 	{
	// 		CoolingOperate(SYSTEM_SET_TEMP_DATA,7);
	// 	}
	// 	else if(BAT_DATA_Pack.BAT_Temperature > 18 && BAT_DATA_Pack.BAT_Temperature <= 20)
	// 	{
	// 		CoolingOperate(SYSTEM_SET_TEMP_DATA,9);
	// 	}
	// 	else if(BAT_DATA_Pack.BAT_Temperature > 16 && BAT_DATA_Pack.BAT_Temperature <= 18)
	// 	{
	// 		//盲区
	// 	}
	// 	else if(BAT_DATA_Pack.BAT_Temperature > 5 && BAT_DATA_Pack.BAT_Temperature <= 16)
	// 	{
	// 		CoolingOperate(SYSTEM_SET_TEMP_DATA,17);
	// 	}
	// 	else if(BAT_DATA_Pack.BAT_Temperature < 5)
	// 	{
	// 		CoolingOperate(SYSTEM_SET_TEMP_DATA,17);
	// 	}
	// }
	else if(CoolingCount >= 3) //更新频率为20hz时，每1.5秒触发一次
	{
		CoolingCount = 0; 
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
 * @param hcooling 
 */
static Cooling_FunStatusTypeDef Run(){
    static Cooling_StateTypeDef CoolingWorkStatus = Cooling_STOP ;
	uint8_t BAT_DATA_Pack =0 ;
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
				CoolingOperate(SYSTEM_GET_DATA,NULL);
				break;
			}
			case Cooling_CHECK:
			{//判定水冷工作状态，正常开机继续执行，异常开机返回0步骤
				if(Cooling_Handle->modbusReport.CoolingRunState == 1)
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

