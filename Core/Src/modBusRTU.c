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
 * @todo 需要抽取modbus底层协议完成协议封装，暂时先使用当前简易版收发方式，屏蔽modBusRTU文件
 */

// 计算CRC校验值
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
        //@TODO: 需要改为回调函数形式，匹配不同发送函数功能
        HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
        
        if (status == HAL_OK) {
            return len;  // 返回成功写入的字节数
        } else {
            errno = EIO;  // 发生错误时设置 errno
            return -1;
        }
    }
    
    errno = EBADF;  // 无效的文件描述符
    return -1;
}

// 发送Modbus请求
int sendModbusRequest(void *obj, const ModbusRequest *request) {
    ModbusProtocol *modbus = (ModbusProtocol*)obj;
    int serial_fd = *(int*)modbus->user_data;

    unsigned char request_frame[256];
    int request_length = 0;

    // 构建请求帧
    request_frame[request_length++] = request->address;
    request_frame[request_length++] = request->function_code;
    memcpy(&request_frame[request_length], request->data, request->data_length);
    request_length += request->data_length;

    // 计算并添加CRC校验
    unsigned short crc = calculateCRC(request_frame, request_length);
    request_frame[request_length++] = crc & 0xFF;
    request_frame[request_length++] = (crc >> 8) & 0xFF;

    // 发送请求帧
    if (write(serial_fd, request_frame, request_length) != request_length) {
        perror("Failed to write to serial port");
        return -1;
    }

    return 0;
}

// 接收Modbus响应
int receiveModbusResponse(void *obj, ModbusResponse *response) {
    ModbusProtocol *modbus = (ModbusProtocol*)obj;
    int serial_fd = *(int*)modbus->user_data;

    unsigned char response_frame[256];
    int response_length = 0;

    // 接收响应帧
    int bytes_read = read(serial_fd, response_frame, sizeof(response_frame));
    if (bytes_read == -1) {
        perror("Failed to read from serial port");
        return -1;
    }
    response_length = bytes_read;

    // 解析响应帧
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

// 封装Modbus协议的03码请求函数
//@TODO: 收发需要分为两个线程处理，本程序需改为信号同步方式处理
int modbusReadHoldingRegisters( void *obj, 
                                unsigned char address, 
                                unsigned short start_register, 
                                unsigned short num_registers, 
                                unsigned short *registers) {
    ModbusProtocol *modbus = (ModbusProtocol*)obj;

    ModbusRequest request;
    ModbusResponse response;

    // 构建请求
    unsigned char data[] = {
        (start_register >> 8) & 0xFF,  // 寄存器起始地址高字节
        start_register & 0xFF,         // 寄存器起始地址低字节
        (num_registers >> 8) & 0xFF,   // 寄存器数量高字节
        num_registers & 0xFF           // 寄存器数量低字节
    };

    request.address = address;
    request.function_code = READ_HOLDING_REGISTERS;
    request.data = data;
    request.data_length = sizeof(data);

    // 发送请求
    if (modbus->send_request(obj, &request) != 0) {
        return -1;
    }

    // 接收响应
    if (modbus->receive_response(obj, &response) != 0) {
        return -1;
    }

    // 解析响应
    if (response.function_code != READ_HOLDING_REGISTERS) {
        return -1;
    }

    // 提取寄存器数据
    int i;
    for (i = 0; i < response.data_length / 2; i++) {
        registers[i] = (response.data[i * 2] << 8) | response.data[i * 2 + 1];
    }

    return 0;
}

// 封装Modbus协议的06码请求函数
int modbusWriteSingleRegister(  void *obj, 
                                unsigned char address, 
                                unsigned short register_address, 
                                unsigned short register_value) {
    ModbusProtocol *modbus = (ModbusProtocol*)obj;

    ModbusRequest request;
    ModbusResponse response;

    // 构建请求
    unsigned char data[] = {
        (register_address >> 8) & 0xFF,  // 寄存器地址高字节
        register_address & 0xFF,         // 寄存器地址低字节
        (register_value >> 8) & 0xFF,    // 寄存器值高字节
        register_value & 0xFF            // 寄存器值低字节
    };

    request.address = address;
    request.function_code = WRITE_SINGLE_REGISTER;
    request.data = data;
    request.data_length = sizeof(data);

    // 发送请求
    if (modbus->send_request(obj, &request) != 0) {
        return -1;
    }

    // 接收响应
    if (modbus->receive_response(obj, &response) != 0) {
        return -1;
    }

    // 解析响应
    if (response.function_code != WRITE_SINGLE_REGISTER) {
        return -1;
    }

    return 0;
}

// 封装Modbus协议的初始化函数
void modbusProtocolInit(ModbusProtocol *modbus, int serial_fd) {
    modbus->send_request = sendModbusRequest;
    modbus->receive_response = receiveModbusResponse;
    modbus->user_data = &serial_fd;
}

