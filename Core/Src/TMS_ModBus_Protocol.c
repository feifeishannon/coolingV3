/**
 ******************************************************************************
 * @file    : TMS_ModBus_Protocol.c
 * @brief   : TMS TMSЭ��Ӧ�ó���
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

#if 1 //�۵�����
uint8_t TMSRecvBuf[50] = {0};    //����BUF
uint8_t TMSSendBuf[50] = {0};    //����BUF
uint8_t TMSDataBuf[50] = {0};    //ˮ������BUF

uint8_t TMSWorkStatus = 0;       //����״̬
uint8_t TMSCount = 0;            //������
uint8_t TMSRecvCount = 0;        //���ռ�����
uint8_t TMSSendCount = 0;        //���ͼ�����
uint8_t TMSRecvNum = 0;          //���ո���
uint8_t TMSSendNum = 0;          //���͸���
uint8_t TMSTimeOutFlag = 0;      //��ʱʹ��
uint32_t TMSTimeOutCount = 0;    //��ʱ������

uint8_t TMS_Rx_dat = 0;

uint8_t TMS_USART_RX_BUF[USART_REC_LEN]; 
uint16_t TMS_USART_RX_STA;
uint8_t TMS_aRxBuffer[RXBUFFERSIZE];

TMS_HandleTypeDef* TMS_Handle;

// uint8_t aucTMSCHECKALLCmd[8]	=  {0xAA, 0x03, 0x00, 0x00, 0x00, 0x12, 0xDC, 0x1C};//��ѯ12���Ĵ���ָ��
// uint8_t aucTMSOFFCmd[8]			=  {0xAA, 0x06, 0x00, 0x00, 0x00, 0x00, 0x90, 0x11};//�ػ�ָ��
// uint8_t aucTMSONCmd[27]			=  {0xAA, 0x10, 0x00, 0x00, 0x00, 0x09, 0x12, 0x00, 0x01, 
// 											0x00, 0x64, 0x00, 0x01, 0x00, 0x05, 0x00, 0x00, 0x02, 
//                                 			0x58, 0x03, 0x20, 0x03, 0x20, 0x01, 0xF4, 0x21, 0x6A}; //����ָ�Ŀ���¶�10��
// uint8_t aucTMSTargTempCmd[8]	=	{0xAA, 0x06, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00};//����ָ��
// uint8_t aucTMS06Cmd[8]			=	{0xAA, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};//����ָ��
// uint8_t aucTMS10Cmd[27]			=	{0xAA, 0x10, 0x00, 0x00, 0x00, 0x09, 0x12, 0x00, 0x01, 	\
// 									0x00, 0x64, 0x00, 0x01, 0x00, 0x05, 0x00, 0x00, 0x02, 	\
// 									0x58, 0x03, 0x20, 0x03, 0x20, 0x01, 0xF4, 0x00, 0x00}; //����ָ�Ŀ���¶�10��
// 									// У������6A21 

uint8_t TMS_modbus_slave_addr = 0xAA; // 
uint8_t TMS_modbus_Tx_buff[100];	  // ���ͻ�����

uint16_t targetTemp = 0;

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

#endif

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

/**
 * @brief �����Ĵ�������
 * 			
 * @param source 
 * @param target 
 * @param length 
 */
void copyArray(uint8_t* target, uint8_t* source, uint8_t length) {
    // memcpy(target, source, length);
	uint8_t i;
	for (i = 0; i < length; i++)
	{
		target[i] = source[i];
	}
	
}

/**
 * @brief  ��tms�ı����¶�תΪ��ʵ�¶�
 * @param  temp: 
 */
static float temp_uint2float(uint16_t temp){
	return (float)((temp - 500) / 10);
}
/**
 * @brief  ��tms����ʵ�¶�תΪ�����¶�
 * @param  temp: 
 */
static uint16_t temp_float2uint(float temp){
	return (uint16_t)(temp * 10  + 500);
}

/**
 * @brief  TMS_PSD�������
 * @note   
 */
static void	updataPSD(){
	if(TMS_Handle->modbusDataReloadFlag==0){//δ����ý�������
		return;
	}
	if( TMS_Handle->TMS_PSD.TMSRunState){
		TMS_Handle->modbusReport.PSD |= 1 << 7;
	}else{
		TMS_Handle->modbusReport.PSD &= ~(1 << 7);
	}
	if( TMS_Handle->TMS_PSD.TMSHighTempERR){
		TMS_Handle->modbusReport.PSD |= 1 << 2;
	}else{
		TMS_Handle->modbusReport.PSD &= ~(1 << 2);
	}
	if( TMS_Handle->TMS_PSD.TMSLiquidLevelERR){
		TMS_Handle->modbusReport.PSD |= 1 ;
	}else{
		TMS_Handle->modbusReport.PSD &= ~(1);
	}
	
	TMS_Handle->modbusDataReloadFlag = 0;//��������״̬���
	
}
/**
 * @brief �����������ݸ�����������Ĵ���������Ĵ���
 * 
 */
static void updata(){
	TMS_Handle->TMS_CMD_Pack.TMSTargetTemp = 
		TMS_Handle->modbusReport.TMSRunState;
	TMS_Handle->targetTemperature = 
		temp_uint2float(TMS_Handle->modbusReport.TargetTemperature);
	TMS_Handle->modbusReport.OutletTemperature= 
		temp_float2uint(TMS_Handle->currentTemperature);

	updataPSD();
}

static void send_data(uint8_t *buff, uint8_t len)
{
	HAL_UART_Transmit_IT(TMS_Handle->huart, (uint8_t *)buff, len); // ��������   ��buff
}

/**
 * @brief �ش����мĴ�����Ϣ
 * 
 */
static void reportAll(){
	uint16_t CRCvlaue = 0;
	uint16_t * ptr;
	// ��10�������������ݸ��Ƶ����ͻ�����
	//@todo: ����֤copy�����Ƿ�������ת��Һ�������
	TMS_modbus_Tx_buff[0] = TMS_modbus_slave_addr;
	TMS_modbus_Tx_buff[1] = 03;
	TMS_modbus_Tx_buff[2] = 18 * 2 -3;

	ptr = (uint16_t*)&(TMS_Handle->modbusReport);

	for (size_t i = 0; i < 18; i++)
	{
		/* code */
		TMS_modbus_Tx_buff[i * 2  + 3 ] 	= ptr[i] >> 8;
		TMS_modbus_Tx_buff[i * 2  + 3 + 1 ] = (uint8_t)ptr[i] & 0xff;
		
	}
	
	// �����������������������crc
	CRCvlaue = CRC16(TMS_modbus_Tx_buff, 36);
	TMS_modbus_Tx_buff[39] = (CRCvlaue)&0xFF;
	TMS_modbus_Tx_buff[40] = (CRCvlaue >> 8) & 0xFF;
	send_data(TMS_modbus_Tx_buff,41);
}

#if 1 //�۵�ϵͳ����
/**
 * @brief modbus�յ���Ϣ��ش����յ�����Ϣ
 * 
 * @param len �ش����ĳ���
 */
static void reportAPK(uint8_t len){
	copyArray(TMS_modbus_Tx_buff, TMS_USART_RX_BUF,len);
	send_data(TMS_modbus_Tx_buff,len);
} 

static void TMSOperateSystemON(){
	TMS_Handle->CMDCode = CoolingCMDStart;
}
static void TMSOperateSystemOFF(){
	TMS_Handle->CMDCode = CoolingCMDStop;
}
static void TMSOperatePumpStop(){
	TMS_Handle->CMDCode = CoolingPumpStop;
}
static void TMSOperatePumpStart(){
	TMS_Handle->CMDCode = CoolingPumpStart;
}
static void TMSOperateCompressorStop(){
	TMS_Handle->CMDCode = CoolingCompressorStop;
}
static void TMSOperateCompressorStart(){
	TMS_Handle->CMDCode = CoolingCompressorStart;
}
static void TMSOperateGetData(){
	TMS_Handle->CMDCode = CoolingGetData;
}
static void TMSOperateSetTemp(uint16_t value){
	TMS_Handle->CMDCode = CoolingSetTemp;
	TMS_Handle->modbusReport.TargetTemperature =(value) ;//�����趨�¶�ֵ

}

#endif

static void modbus_03_Receivefunction(uint8_t lenth)
{
	TMSOperateGetData();
}

/**
 * @brief ����06�����
 * 
 */
static void modbus_06_Receivefunction(uint16_t CMD_register, uint16_t value,uint8_t lenth)
{
	switch(CMD_register){
		// case 0x2031: 
		case 0x0000:	// ��ͣ���Ƴ���
			if (value)
			{
				TMSOperateSystemON();
			}else{
				TMSOperateSystemOFF();
			}
		break;
		
		// case 0x2003: //�����¶�
		case 0x0005: //�����¶�
			TMSOperateSetTemp(value);
		break;
		
		
		case 0x203C: //��ͣҺ��
			if (value)
			{
				TMSOperatePumpStart();
			}else{
				TMSOperatePumpStop();
			}
		break;
		
		case 0x203D: //��ͣѹ����
			if (value)
			{
				TMSOperateCompressorStart();
			}else{
				TMSOperateCompressorStop();
			}
		break;
	}

	reportAPK(lenth);
}


/**
 * @brief �������мĴ�������
 * {0xAA, 0x10, 0x00, 0x00, 0x00, 0x09, 0x12, //����ͷ 7�ֽ�
	0x00, 0x01,	0x00, 0x64, 0x00, 0x01, 0x00, //14
	0x05, 0x00, 0x00, 0x02,	0x58, 0x03, 0x2 0, //21
	0x03, 0x20, 0x01, 0xF4, 0x6A, 0x21}; //27����ָ�Ŀ���¶�10��
 * @todo 
 * ͬ���������ݵ��Ĵ�����
 */
static void modbus_10_Receivefunction(uint8_t lenth)
{
	uint16_t value;
	uint16_t * ptr;
	
	for (size_t i = 0; i < 9;i++ ){
		value = (uint16_t)((TMS_USART_RX_BUF[i * 2 + 7] << 8) | TMS_USART_RX_BUF[i * 2 + 7 + 1]);
		ptr = (uint16_t*)&(TMS_Handle->modbusReport);
		ptr[i] = value;
	}
	
	reportAPK(lenth);

	TMS_Handle->CMDCode = CoolingSetAll;
}

/**
 * @brief 
 * 
 */
static void TMSModbus_service(){
	uint16_t data_CRC_value;   
	uint16_t data_len;		   
	uint16_t CRC_check_result; 
	uint16_t CMD_register;
	uint16_t value;
	if (TMS_USART_RX_STA & 0x8000){
		data_len = TMS_USART_RX_STA & 0x3fff;															 
		CRC_check_result = CRC16(TMS_USART_RX_BUF, data_len - 2);
		data_CRC_value = TMS_USART_RX_BUF[data_len - 2] << 8 | (((uint16_t)TMS_USART_RX_BUF[data_len - 1])); 
		CMD_register = ((uint16_t)TMS_USART_RX_BUF[2]<<8) | ((uint16_t)TMS_USART_RX_BUF[3]);
		value = (uint16_t)TMS_USART_RX_BUF[4] << 8 | ((uint16_t)TMS_USART_RX_BUF[5]);
		if (CRC_check_result == data_CRC_value){
			if (TMS_USART_RX_BUF[0] == TMS_modbus_slave_addr){
				switch (TMS_USART_RX_BUF[1]){
					case 03:
						modbus_03_Receivefunction(data_len);
						break;
					case 06:
						modbus_06_Receivefunction(CMD_register,value,data_len);
						
						break;
					case 0x10:
						modbus_10_Receivefunction(data_len);
						
						break;
					}}}
		TMS_USART_RX_STA = 0;
	}
}

/**
 * @brief ���ڽ��պ�������Ҫ�ڴ����ж������öԱ������ĵ���
 * 
 */
static void RxCplt(void)
{
	if ((TMS_USART_RX_STA & 0x8000) == 0) 
	{
		TMS_Handle->modbus_count = 0;
		TMS_USART_RX_BUF[TMS_USART_RX_STA & 0X3FFF] = TMS_aRxBuffer[0];
		TMS_USART_RX_STA++;
		if (TMS_USART_RX_STA > (USART_REC_LEN - 1))
			TMS_USART_RX_STA = 0; 
	}
	HAL_UART_Receive_IT(TMS_Handle->huart, (uint8_t *)TMS_aRxBuffer, 1);
}

/**
 * @brief TMS����������������TMS״̬��
 *        ��Ҫ���˺���������������ѭ����
 *        �򴴽���ѯ����
 * @param hTMS 
 */
static TMS_FunStatusTypeDef Run(){
	// ȷ���յ���Ч���ݺ����TMS״̬λ
	updata();
	TMS_Handle->UpdataPack(); // ���´��ڽ�����������
    
	return TMS_OK;
}

/**
 * @brief TMS������ֹͣ����
 *        
 * 
 * @param hTMS 
 */
static TMS_FunStatusTypeDef Stop(){

	return TMS_OK;
}

/**
 * @brief TMS��������ʼ������
 *        
 */
static TMS_FunStatusTypeDef Init(){
	TMS_USART_RX_STA = 0; // ׼������
    
	return TMS_OK;
}

static TMS_FunStatusTypeDef UpdataPack(){

	if(TMS_Handle->modbus_count > 4 && ((TMS_USART_RX_STA & 0X3FFF) != 0)){
		TMS_USART_RX_STA |= 0x8000;
		TMSModbus_service();
	}
	return TMS_OK;
}

/**
 * @brief  ��ʼ��tms�Ĵ���������Ŀ���¶�ֵ10��
 * 			ͬʱ���»ذ����ݣ�����Ŀ���¶�
 * @note   
 * @retval None
 */
static void initRegister(){
	TMS_Handle->targetTemperature = 10.0f;
	TMS_Handle->modbusReport.TargetTemperature = 
		temp_float2uint(TMS_Handle->targetTemperature);

	// TMS_Handle->CMDCode = CoolingCMDStart;
}
/**
 * @brief TMS������ע�ắ��
 *        ������ṹ����
 *
 * @param huartTMS:���շ����ݽӿ�
 */
TMS_FunStatusTypeDef TMSCreate( UART_HandleTypeDef *huartTMS)
{
	TMS_Handle = malloc(sizeof(TMS_HandleTypeDef));

	TMS_Handle->Run				= Run;
	TMS_Handle->Stop			= Stop;
	TMS_Handle->Init			= Init;
	TMS_Handle->UpdataPack		= UpdataPack;
	TMS_Handle->RxCplt			= RxCplt;
	TMS_Handle->reportAll		= reportAll;
	
	
	TMS_Handle->huart = huartTMS;
	HAL_UART_Receive_IT(TMS_Handle->huart, (uint8_t *)TMS_aRxBuffer, 1);
	initRegister();
	
	return TMS_OK;
}

