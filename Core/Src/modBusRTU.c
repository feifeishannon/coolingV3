#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "modBusRTU.h"

/**
 * @brief 
 * 
 * @param data 
 * @param length 
 * @return unsigned short 
 * @todo ��Ҫ��ȡmodbus�ײ�Э�����Э���װ����ʱ��ʹ�õ�ǰ���װ��շ���ʽ������modBusRTU�ļ�
 */

// ����CRCУ��ֵ
unsigned short calculateCRC(unsigned char *data, int length) {
    unsigned short crc = 0xFFFF;
    int i, j;

    for (i = 0; i < length; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

static int write(int file, char *ptr, int len) {
    if (file == STDOUT_FILENO || file == STDERR_FILENO) {
        //@TODO: ��Ҫ��Ϊ�ص�������ʽ��ƥ�䲻ͬ���ͺ�������
        HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
        
        if (status == HAL_OK) {
            return len;  // ���سɹ�д����ֽ���
        } else {
            errno = EIO;  // ��������ʱ���� errno
            return -1;
        }
    }
    
    errno = EBADF;  // ��Ч���ļ�������
    return -1;
}

// ����Modbus����
int sendModbusRequest(void *obj, const ModbusRequest *request) {
    ModbusProtocol *modbus = (ModbusProtocol*)obj;
    int serial_fd = *(int*)modbus->user_data;

    unsigned char request_frame[256];
    int request_length = 0;

    // ��������֡
    request_frame[request_length++] = request->address;
    request_frame[request_length++] = request->function_code;
    memcpy(&request_frame[request_length], request->data, request->data_length);
    request_length += request->data_length;

    // ���㲢���CRCУ��
    unsigned short crc = calculateCRC(request_frame, request_length);
    request_frame[request_length++] = crc & 0xFF;
    request_frame[request_length++] = (crc >> 8) & 0xFF;

    // ��������֡
    if (write(serial_fd, request_frame, request_length) != request_length) {
        perror("Failed to write to serial port");
        return -1;
    }

    return 0;
}

// ����Modbus��Ӧ
int receiveModbusResponse(void *obj, ModbusResponse *response) {
    ModbusProtocol *modbus = (ModbusProtocol*)obj;
    int serial_fd = *(int*)modbus->user_data;

    unsigned char response_frame[256];
    int response_length = 0;

    // ������Ӧ֡
    int bytes_read = read(serial_fd, response_frame, sizeof(response_frame));
    if (bytes_read == -1) {
        perror("Failed to read from serial port");
        return -1;
    }
    response_length = bytes_read;

    // ������Ӧ֡
    if (response_length >= 3) {
            response->address = response_frame[0];
        response->function_code = response_frame[1];
        response->data_length = response_frame[2];
        response->data = malloc(response->data_length);
        memcpy(response->data, &response_frame[3], response->data_length);
    } else {
        return -1;
    }

    return 0;
}

// ��װModbusЭ���03��������
//@TODO: �շ���Ҫ��Ϊ�����̴߳������������Ϊ�ź�ͬ����ʽ����
int modbusReadHoldingRegisters( void *obj, 
                                unsigned char address, 
                                unsigned short start_register, 
                                unsigned short num_registers, 
                                unsigned short *registers) {
    ModbusProtocol *modbus = (ModbusProtocol*)obj;

    ModbusRequest request;
    ModbusResponse response;

    // ��������
    unsigned char data[] = {
        (start_register >> 8) & 0xFF,  // �Ĵ�����ʼ��ַ���ֽ�
        start_register & 0xFF,         // �Ĵ�����ʼ��ַ���ֽ�
        (num_registers >> 8) & 0xFF,   // �Ĵ����������ֽ�
        num_registers & 0xFF           // �Ĵ����������ֽ�
    };

    request.address = address;
    request.function_code = READ_HOLDING_REGISTERS;
    request.data = data;
    request.data_length = sizeof(data);

    // ��������
    if (modbus->send_request(obj, &request) != 0) {
        return -1;
    }

    // ������Ӧ
    if (modbus->receive_response(obj, &response) != 0) {
        return -1;
    }

    // ������Ӧ
    if (response.function_code != READ_HOLDING_REGISTERS) {
        return -1;
    }

    // ��ȡ�Ĵ�������
    int i;
    for (i = 0; i < response.data_length / 2; i++) {
        registers[i] = (response.data[i * 2] << 8) | response.data[i * 2 + 1];
    }

    return 0;
}

// ��װModbusЭ���06��������
int modbusWriteSingleRegister(  void *obj, 
                                unsigned char address, 
                                unsigned short register_address, 
                                unsigned short register_value) {
    ModbusProtocol *modbus = (ModbusProtocol*)obj;

    ModbusRequest request;
    ModbusResponse response;

    // ��������
    unsigned char data[] = {
        (register_address >> 8) & 0xFF,  // �Ĵ�����ַ���ֽ�
        register_address & 0xFF,         // �Ĵ�����ַ���ֽ�
        (register_value >> 8) & 0xFF,    // �Ĵ���ֵ���ֽ�
        register_value & 0xFF            // �Ĵ���ֵ���ֽ�
    };

    request.address = address;
    request.function_code = WRITE_SINGLE_REGISTER;
    request.data = data;
    request.data_length = sizeof(data);

    // ��������
    if (modbus->send_request(obj, &request) != 0) {
        return -1;
    }

    // ������Ӧ
    if (modbus->receive_response(obj, &response) != 0) {
        return -1;
    }

    // ������Ӧ
    if (response.function_code != WRITE_SINGLE_REGISTER) {
        return -1;
    }

    return 0;
}

// ��װModbusЭ��ĳ�ʼ������
void modbusProtocolInit(ModbusProtocol *modbus, int serial_fd) {
    modbus->send_request = sendModbusRequest;
    modbus->receive_response = receiveModbusResponse;
    modbus->user_data = &serial_fd;
}

