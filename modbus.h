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
typedef struct
{
    uint8_t data[TXRX_BUFFER_SIZE];
    uint16_t length;
}ComMessage;
//----------------------------------------------------------------------------------------------------------------------
typedef int (*register_cb)(uint16_t regnum);
//----------------------------------------------------------------------------------------------------------------------
//return codes
#define MODBUS_PACKET_VALID_AND_PROCESSED               0
#define MODBUS_PACKET_WRONG_ADDR                        1
#define MODBUS_PACKET_WRONG_CRC                         2
#define MODBUS_REGISTER_WRITE_PROTECTED                 3
#define MODBUS_REGISTER_NUMBER_INVALID                  4
#define MODBUS_REGISTER_WRITE_CALLBACK_FAILED           5
//----------------------------------------------------------------------------------------------------------------------
#define MODBUS_CRC_START_VALUE                          0xffff
#define MB_BROADCAST_ADDR                               0xff
#define MODBUS_03_LENGTH_IND                            2
#define MODBUS_03_DATASTART_IND                         3
#define MODBUS_06_DATASTART_IND                         4
#define MODBUS_03_1REG_REPLY_LEN_NO_CRC                 5
#define MAX_REGS_BATCH_WRITE                            64
//----------------------------------------------------------------------------------------------------------------------
#define MODBUS_RTU_PDU_TYPE         1
#define MODBUS_TCP_PDU_TYPE         2
#define MODBUS_TCP_HEADER_OFFSET    6
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
