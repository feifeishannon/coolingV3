#ifndef MODBUS_PROTOCOL_H
#define MODBUS_PROTOCOL_H

// Modbus������
#define READ_HOLDING_REGISTERS 0x03
#define WRITE_SINGLE_REGISTER 0x06

// Modbus����ṹ��
typedef struct {
    unsigned char address;
    unsigned char function_code;
    unsigned char *data;
    int data_length;
} ModbusRequest;

// Modbus��Ӧ�ṹ��
typedef struct {
    unsigned char address;
    unsigned char function_code;
    unsigned char *data;
    int data_length;
} ModbusResponse;

// ModbusЭ���װ����
typedef struct {
    int (*send_request)(void*, const ModbusRequest*);
    int (*receive_response)(void*, ModbusResponse*);
    void *user_data;
} ModbusProtocol;

// ����Modbus����
int sendModbusRequest(int serial_fd, const ModbusRequest *request);

// ����Modbus��Ӧ
int receiveModbusResponse(int serial_fd, ModbusResponse *response);

// ��װModbusЭ���03��������
int modbusReadHoldingRegisters(int serial_fd, unsigned char address, unsigned short start_register, unsigned short num_registers, unsigned short *registers);

// ��װModbusЭ���06��������
int modbusWriteSingleRegister(int serial_fd, unsigned char address, unsigned short register_address, unsigned short register_value);

#endif  // MODBUS_PROTOCOL_H