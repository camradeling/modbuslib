#ifndef MODBUS_H
#define MODBUS_H
//----------------------------------------------------------------------------------------------------------------------
#ifndef TXRX_BUFFER_SIZE
#define TXRX_BUFFER_SIZE 262
#warning "TXRX_BUFFER_SIZE not defined explicitly, defaulting to 262"
#endif
//----------------------------------------------------------------------------------------------------------------------
#define SWAP16(x) (((uint16_t)x << 8)|((uint16_t)x >> 8))
//----------------------------------------------------------------------------------------------------------------------
typedef struct __attribute__((packed))
{
    uint8_t data[TXRX_BUFFER_SIZE];
    uint16_t length;
} ComMessage;
//----------------------------------------------------------------------------------------------------------------------
typedef int (*register_cb)(uint16_t regnum);
//----------------------------------------------------------------------------------------------------------------------
// function codes
#define MODBUS_READ_HOLDING_REGISTERS       3
#define MODBUS_READ_INPUT_REGISTERS         4
#define MODBUS_WRITE_SINGLE_REGISTER        6
#define MODBUS_LOOPBACK                     8
#define MODBUS_WRITE_MULTIPLE_REGISTERS     16
//----------------------------------------------------------------------------------------------------------------------
// return codes
#define MODBUS_PACKET_VALID_AND_PROCESSED               0
#define MODBUS_PACKET_WRONG_ADDR                        1
#define MODBUS_PACKET_WRONG_CRC                         2
#define MODBUS_REGISTER_WRITE_PROTECTED                 3
#define MODBUS_REGISTER_NUMBER_INVALID                  4
#define MODBUS_REGISTER_WRITE_CALLBACK_FAILED           5
//----------------------------------------------------------------------------------------------------------------------
#define MODBUS_CRC_START_VALUE                          0xffff
#define MB_BROADCAST_ADDR                               0xff
//----------------------------------------------------------------------------------------------------------------------
#define MODBUS_REQUEST_SLAVE_ADDRESS_POSITION           0
#define MODBUS_REQUEST_FUNCTION_CODE_POSITION           1
#define MODBUS_REQUEST_REGISTER_ADDRESS_POSITION        2
#define MODBUS_REQUEST_REGISTER_VALUE_POSITION          4
#define MODBUS_REQUEST_REGISTER_NUMBER_POSITION         MODBUS_REQUEST_REGISTER_VALUE_POSITION
#define MODBUS_REQUEST_BYTES_NUMBER_POSITION            6
#define MODBUS_REQUEST_REGISTER_DATA_START              7
//----------------------------------------------------------------------------------------------------------------------
#define MODBUS_REPLY_SLAVE_ADDRESS_POSITION             0
#define MODBUS_REPLY_FUNCTION_CODE_POSITION             1
#define MODBUS_REPLY_BYTES_NUMBER_POSITION              2
#define MODBUS_REPLY_REGISTER_DATA_START                3
#define MODBUS_REPLY_REGISTER_ADDRESS_POSITION          2
#define MODBUS_REPLY_REGISTER_VALUE_POSITION            4
#define MODBUS_REPLY_REGISTER_NUMBER_POSITION           MODBUS_REPLY_REGISTER_VALUE_POSITION
//----------------------------------------------------------------------------------------------------------------------
#define MODBUS_03_LENGTH_IND                            2
#define MODBUS_03_DATASTART_IND                         3
#define MODBUS_06_DATASTART_IND                         4
#define MODBUS_03_1REG_REPLY_LEN_NO_CRC                 5
#define MAX_REGS_BATCH_WRITE                            64
#define MAX_REGS_TO_WRITE                               123
//----------------------------------------------------------------------------------------------------------------------
enum modbus_pdu_type_e
{
    MODBUS_RTU_PDU_TYPE = 1,
    MODBUS_TCP_PDU_TYPE
};
//----------------------------------------------------------------------------------------------------------------------
#define MODBUS_TCP_HEADER_OFFSET    6
//----------------------------------------------------------------------------------------------------------------------
typedef struct __attribute__((packed))
{
    uint16_t transaction_id;
    uint16_t protocol_id;
    uint16_t length;
    uint8_t  unit_id;
} mbap_header_s;
//----------------------------------------------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif
//----------------------------------------------------------------------------------------------------------------------
uint8_t process_net_packet(ComMessage* inPack, ComMessage* outPack, int pdu_type);
//----------------------------------------------------------------------------------------------------------------------
uint16_t calc_crc_buf(uint16_t startvalue, uint8_t *arr, int length);
//----------------------------------------------------------------------------------------------------------------------
int process_modbus(ComMessage* inPack, ComMessage* outPack, int pdu_type);
//----------------------------------------------------------------------------------------------------------------------
int CmdModbus_03_04(ComMessage* inPack, ComMessage* outPack, int offset);
//----------------------------------------------------------------------------------------------------------------------
int CmdModbus_06(ComMessage* inPack, ComMessage* outPack, int offset);
//----------------------------------------------------------------------------------------------------------------------
int CmdModbus_08(ComMessage* inPack, ComMessage* outPack, int offset);
//----------------------------------------------------------------------------------------------------------------------
int CmdModbus_16(ComMessage* inPack, ComMessage* outPack, int offset);
//----------------------------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//----------------------------------------------------------------------------------------------------------------------
#endif /*MODBUS_H*/
