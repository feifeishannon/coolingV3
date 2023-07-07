#ifndef MODBUS_PROTOCOL_H
#define MODBUS_PROTOCOL_H

// Modbus功能码
#define READ_HOLDING_REGISTERS 0x03
#define WRITE_SINGLE_REGISTER 0x06

// Modbus请求结构体
typedef struct {
    unsigned char address;
    unsigned char function_code;
    unsigned char *data;
    int data_length;
} ModbusRequest;

// Modbus响应结构体
typedef struct {
    unsigned char address;
    unsigned char function_code;
    unsigned char *data;
    int data_length;
} ModbusResponse;

// Modbus协议封装对象
typedef struct {
    int (*send_request)(void*, const ModbusRequest*);
    int (*receive_response)(void*, ModbusResponse*);
    void *user_data;
} ModbusProtocol;

// 发送Modbus请求
int sendModbusRequest(int serial_fd, const ModbusRequest *request);

// 接收Modbus响应
int receiveModbusResponse(int serial_fd, ModbusResponse *response);

// 封装Modbus协议的03码请求函数
int modbusReadHoldingRegisters(int serial_fd, unsigned char address, unsigned short start_register, unsigned short num_registers, unsigned short *registers);

// 封装Modbus协议的06码请求函数
int modbusWriteSingleRegister(int serial_fd, unsigned char address, unsigned short register_address, unsigned short register_value);

#endif  // MODBUS_PROTOCOL_H