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


uint8_t CoolingRecvBuf[50] = {0};    //接收BUF
uint8_t CoolingSendBuf[50] = {0};    //发送BUF
uint8_t CoolingDataBuf[50] = {0};    //水冷数据BUF

uint8_t CoolingCount = 0;            //计数器
uint8_t CoolingRecvCount = 0;        //接收计数器
uint8_t CoolingSendCount = 0;        //发送计数器
uint8_t CoolingRecvNum = 0;          //接收个数
uint8_t CoolingSendNum = 0;          //发送个数
uint8_t CoolingTimeOutFlag = 0;      //超时使能
uint32_t CoolingTimeOutCount = 0;    //超时计数器


uint8_t cooling_Rx_dat = 0;

uint8_t coollingUSART_RX_BUF[USART_REC_LEN]; 
uint16_t cooling_USART_RX_STA;
uint8_t cooling_aRxBuffer[RXBUFFERSIZE];

// Modbus_Report_Pack_TypeDef Modbus_Report_Pack = {0};  //水冷实时数据
Cooling_HandleTypeDef* Cooling_Handle;

uint8_t aucCoolingGetALL[8]				=	{0x01, 0x03, 0x20, 0x00, 0x00, 0x04, 0x4F, 0xC9};//查询前三寄存器
uint8_t aucCoolingGetTemp[8]			=	{0x01, 0x03, 0x20, 0x00, 0x00, 0x01, 0x8F, 0xCA};//查询当前温度
uint8_t aucCoolingGetState[8]			=	{0x01, 0x03, 0x20, 0x02, 0x00, 0x01, 0x2E, 0x0A};//查询当前状态
uint8_t aucCoolingOFFCmd[8]				=	{0x01, 0x06, 0x20, 0x31, 0x00, 0x00, 0xD3, 0xC5};//关机指令
uint8_t aucCoolingONCmd[8]				=	{0x01, 0x06, 0x20, 0x31, 0x00, 0x03, 0x93, 0xC4};//开机指令
uint8_t aucPumpOFFCmd[8]				=	{0x01, 0x06, 0x20, 0x3C, 0x00, 0x00, 0x42, 0x06};//关水泵指令
uint8_t aucPumpONCmd[8]					=	{0x01, 0x06, 0x20, 0x3C, 0x00, 0x01, 0x83, 0xC6};//开水泵指令
uint8_t aucPressOFFCmd[8]				=	{0x01, 0x06, 0x20, 0x3D, 0x00, 0x00, 0x13, 0xC6};//关压缩机指令
uint8_t aucPressONCmd[8]				=	{0x01, 0x06, 0x20, 0x3D, 0x00, 0x01, 0xD2, 0x06};//开压缩机指令
uint8_t aucCoolingTargTempCmd[8]		=	{0x01, 0x06, 0x20, 0x03, 0x03, 0xE8, 0x72, 0xB4};//设置目标温度为10°
uint8_t aucCooling06Cmd[8]				=	{0};
uint8_t aucCooling10Cmd[27]				=	{0};

uint8_t cooling_modbus_slave_addr = 0x01; // 长流水冷机地址
uint8_t cooling_modbus_Tx_buff[100];	  // 发送缓冲区

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

/**
 * @brief  将Cool的报文温度转为真实温度
 * @note   
 * @param  temp: 
 * @retval 
 */
static float temp_uint2float(uint16_t temp){
	return ((float)(temp) / 100.0f);
}
/**
 * @brief  将Cool的真实温度转为报文温度
 * @note   
 * @param  temp: 
 * @retval 
 */
static uint16_t temp_float2uint(float temp){
	return (uint16_t)(temp * 100);
}

static void send_data(uint8_t *buff, uint8_t len)
{
	Cooling_Handle->ledflight=10;
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

// 启动水泵
static void CoolingOperatePumpON(){
	send_8data(aucPumpONCmd);
}

// 停止水泵
static void CoolingOperatePumpOFF(){
	send_8data(aucPumpOFFCmd);

}

// 启动压缩机
static void CoolingOperateCoolingON(){
	send_8data(aucPressONCmd);
}

// 停止压缩机
static void CoolingOperateCoolingOFF(){
	send_8data(aucPressOFFCmd);

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
static void CoolingOperateSetTemp(uint16_t value){
	uint16_t crcCheck = 0;
	aucCoolingTargTempCmd[4] = ((value) ) / 256;
	aucCoolingTargTempCmd[5] = ((value) ) % 256;
	
	crcCheck = CRC16(aucCoolingTargTempCmd, 6);
	aucCoolingTargTempCmd[6] = crcCheck % 256;
	aucCoolingTargTempCmd[7] = crcCheck / 256;

	send_8data(aucCoolingTargTempCmd);

}

/**
 * @brief  执行液冷控制器下发设置指令操作，带一个寄存器
 * @note   
 * @param  operateCMD: 
 * @param  value: 
 * @retval None
 */
static void CoolingOperateSet(Cooling_OperateTypeDef operateCMD, uint16_t value){
	switch(operateCMD)
	{
		case SYSTEM_SET_TEMP_DATA:
            CoolingOperateSetTemp(value);
            break;
		default:
			return;
			
	}
}

static void CoolingOperate(Cooling_OperateTypeDef operateCMD){
	switch(operateCMD)
	{
        
		case SYSTEM_ON:
            CoolingOperateSystemON();
			break;

		case SYSTEM_OFF:
            CoolingOperateSystemOFF();
            break;

		case SYSTEM_PUMP_ON:
			CoolingOperatePumpON();
			break;

		case SYSTEM_PUMP_OFF:
            CoolingOperatePumpOFF();
            break;

		case SYSTEM_COOLING_ON:
			CoolingOperateCoolingON();
			break;

		case SYSTEM_COOLING_OFF:
            CoolingOperateCoolingOFF();
            break;

		case SYSTEM_GET_TEMP:
            CoolingOperateGetTemp();
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
    Cooling_PSD->CoolingLiquidLevelAlarm = (CoolingRunningState >> 1) & 0x01;
    Cooling_PSD->CoolingPower = CoolingRunningState & 0x01;
	if(Cooling_Handle->currentTemperature > 42 ){
		if ((CoolingCount++)>10)
		{
			Cooling_PSD->CoolingTempAlarm = 1;
			CoolingCount=11;
		}
	}else{
		Cooling_PSD->CoolingTempAlarm = 0;
		CoolingCount = 0;
	}
	printfln("Cooling_PSD->CoolingRunState:\t %d\r\n"	
				"Cooling_PSD->CoolingHeatState:\t %d\r\n"		
				"Cooling_PSD->CoolingCoolingState:\t %d\r\n"	
				"Cooling_PSD->CoolingPumpFlowAlarm:\t %d\r\n"	
				"Cooling_PSD->CoolingTempAlarm:\t %d\r\n"		
				"Cooling_PSD->CoolingLiquidLevelAlarm:\t %d\r\n"
				"Cooling_PSD->CoolingPower:\t %d\r\n"
				"当前温度：\t %f\r\n"
				"目标温度：\t %f"
				,Cooling_PSD->CoolingRunState
				,Cooling_PSD->CoolingHeatState
				,Cooling_PSD->CoolingCoolingState
				,Cooling_PSD->CoolingPumpFlowAlarm
				,Cooling_PSD->CoolingTempAlarm
				,Cooling_PSD->CoolingLiquidLevelAlarm
				,Cooling_PSD->CoolingPower
				,Cooling_Handle->currentTemperature
				,Cooling_Handle->targetTemperature
				);
}


static void modbus_03_Receivefunction(uint8_t data_len)
{
	uint16_t value;
	uint16_t * ptr;
	
	for (size_t i = 0; i < data_len/2; i++)
	{
		value = (uint16_t)((coollingUSART_RX_BUF[i * 2  + 3 ] << 8) | coollingUSART_RX_BUF[i * 2 + 3 + 1]);
		ptr = (uint16_t*)&(Cooling_Handle->modbusReport);
		ptr[i] = value;
		
	}
	Cooling_Handle->currentTemperature = temp_uint2float(Cooling_Handle->modbusReport.WaterTankTemperature);
	updataPSD(&Cooling_Handle->Cooling_PSD, Cooling_Handle->modbusReport.CoolingRunningState);
}

static void CoolingModbus_service(){
	uint16_t data_CRC_value;   
	uint16_t data_len;		   
	uint16_t CRC_check_result; 
	if (cooling_USART_RX_STA & 0x8000){
		data_len = cooling_USART_RX_STA & 0x3fff;															 
		CRC_check_result = CRC16(coollingUSART_RX_BUF, data_len - 2);
		data_CRC_value = coollingUSART_RX_BUF[data_len - 1] << 8 | (((uint16_t)coollingUSART_RX_BUF[data_len - 2])); 
		if (CRC_check_result == data_CRC_value)
		{
			if (coollingUSART_RX_BUF[0] == cooling_modbus_slave_addr)
			{
				switch (coollingUSART_RX_BUF[1])
				{
				case 03: 
				{
					modbus_03_Receivefunction(coollingUSART_RX_BUF[2]);
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
		cooling_USART_RX_STA = 0; 
	}
}

/**
 * @brief 串口接收函数，需要在串口中断中配置对本函数的调用
 * 
 */
static void RxCplt(void)
{
	
	if ((cooling_USART_RX_STA & 0x8000) == 0) 
	{
		Cooling_Handle->modbus_count = 0;
		coollingUSART_RX_BUF[cooling_USART_RX_STA & 0X3FFF] = cooling_aRxBuffer[0];
		cooling_USART_RX_STA++;
		if (cooling_USART_RX_STA > (USART_REC_LEN - 1))
			cooling_USART_RX_STA = 0; 
	}

	HAL_UART_Receive_IT(Cooling_Handle->huart, (uint8_t *)cooling_aRxBuffer, 1);

	
}


/**
 * @brief 	液冷控制器启动函数，液冷状态机
 *        	需要将此函数放入主函数体循环中
 *        	或创建轮询任务
 * 		  	默认开机启动液冷控制，状态更新为读取设备当前信息
 * 			读取到正确信息后启动液冷控制计算，否则停止液冷
 *  			
 * @param  
 */
static Cooling_FunStatusTypeDef Run(){
	switch(Cooling_Handle->CoolingWorkStatus){
		case Cooling_STOP:{//关机状态不执行操作
		
			break;
		}
		case Cooling_GET_STATE:{//读取液冷所有寄存器数值
			CoolingOperate(SYSTEM_GET_ALL_DATA);
			printfln("获取液冷所有寄存器数据");
			Cooling_Handle->CoolingWorkStatus++;
			break;
		}

		/**
		 * @brief 检测液冷的开机状态，如果和当前状态不一致时，执行命令寄存器的状态
		 * if(Cooling_Handle->Cooling_PSD.CoolingPumpState == 0){
		 */
		case Cooling_CHECK_RunState:{//判定水冷工作状态，正常开机继续执行，异常开机返回0步骤
			if(Cooling_Handle->CMD_Pack.CoollingCMD == 0) {
				if(Cooling_Handle->Cooling_PSD.CoolingRunState == 1){
					CoolingOperate(SYSTEM_OFF);
					printfln("关闭液冷控制");
					Cooling_Handle->CoolingWorkStatus = Cooling_STOP;
					break;
				}
			}else{
				if(Cooling_Handle->Cooling_PSD.CoolingRunState == 0){
					CoolingOperate(SYSTEM_ON);
					printfln("开启液冷控制");

				}
			}
			Cooling_Handle->CoolingWorkStatus++;
			break;
		}
		case Cooling_CHECK_PumpState:{
			if(Cooling_Handle->CMD_Pack.PumpCMD == 0) {
				if(Cooling_Handle->Cooling_PSD.CoolingPumpState == 1){
					CoolingOperate(SYSTEM_PUMP_OFF);
					printfln("关闭水泵");
					
				}
			}else{
				if(Cooling_Handle->Cooling_PSD.CoolingPumpState == 0){
					CoolingOperate(SYSTEM_PUMP_ON);
					printfln("开启水泵");
				}
			}
			
			Cooling_Handle->CoolingWorkStatus++;
			
			break;
		}
		case Cooling_CHECK_CoolingState:{
			if(Cooling_Handle->CMD_Pack.PressCMD == 0) {
				if(Cooling_Handle->Cooling_PSD.CoolingCoolingState == 1){
					CoolingOperate(SYSTEM_COOLING_OFF);
					printfln("关闭压缩机");
					
				}
			}else{
				if(Cooling_Handle->Cooling_PSD.CoolingCoolingState == 0){
					CoolingOperate(SYSTEM_COOLING_ON);
					printfln("开启压缩机");
				}
			}
			
			Cooling_Handle->CoolingWorkStatus++;
			
			break;
		}
		/**
		 * @brief 当控制寄存器的温度和回包中的目标温度不一致
		 * 	表明液冷的目标温度已被更新，需要重新设置目标温度寄存器
		 * 
		 */
		case Cooling_SET_TEMP:{//设置水冷温度
			if(Cooling_Handle->CMD_Pack.CoollingTargetTemp != 
				Cooling_Handle->modbusReport.TargetTemperature) {
				if (Cooling_Handle->CMD_Pack.CoollingTargetTemp<500)
				{
					Cooling_Handle->CMD_Pack.CoollingTargetTemp = 500;
				}
				
				CoolingOperateSet(SYSTEM_SET_TEMP_DATA,
								Cooling_Handle->CMD_Pack.CoollingTargetTemp);
				printfln("液冷目标温度被重设:CMD_Pack.CoollingTargetTemp=>%d",
						Cooling_Handle->CMD_Pack.CoollingTargetTemp);
				Cooling_Handle->targetTemperature =temp_uint2float(Cooling_Handle->CMD_Pack.CoollingTargetTemp);
			}
			Cooling_Handle->CoolingWorkStatus = Cooling_GET_STATE;
			break;
		}
	
		default:
			break;
	}
	return Cooling_OK;
}

/**
 * @brief 液冷控制器停止函数
 *        
 * 
 * @param hcooling 
 */
static Cooling_FunStatusTypeDef Stop(){

	return Cooling_OK;
}

/**
 * @brief 液冷控制器初始化函数
 *        
 * 
 *  
 */
static Cooling_FunStatusTypeDef Init(){
	cooling_USART_RX_STA = 0; // 准备接收

	return Cooling_OK;
}

static Cooling_FunStatusTypeDef UpdataPack(){

	if(Cooling_Handle->modbus_count > 4 && ((cooling_USART_RX_STA & 0X3FFF) != 0)){
		cooling_USART_RX_STA |= 0x8000;
		CoolingModbus_service();
	}
	return Cooling_OK;
}

static void initRegister(){
	// Cooling_Handle->targetTemperature = 10.0f;
	// Cooling_Handle->modbusReport.TargetTemperature = Cooling_Handle->targetTemperature*100;
    // Cooling_Handle->CMD_Pack.CoollingTargetTemp = Cooling_Handle->modbusReport.TargetTemperature ; 
	Cooling_Handle->CoolingWorkStatus = Cooling_STOP;

}
/**
 * @brief 液冷控制器注册函数
 *        绑定所需结构函数
 * 
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
	HAL_UART_Receive_IT(Cooling_Handle->huart, (uint8_t *)cooling_aRxBuffer, 1);
	initRegister();
	return Cooling_OK;
}

