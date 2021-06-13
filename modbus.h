#ifndef MODBUS_H
#define MODBUS_H
//------------------------------------------------------------------------------
#include "modbus_config.h"
//------------------------------------------------------------------------------
#define SWAP16(x) (((uint16_t)x << 8)|((uint16_t)x >> 8))
//------------------------------------------------------------------------------
typedef struct
{
  uint8_t data[TXRX_BUFFER_SIZE];
  uint16_t length;
}ComMessage;
//------------------------------------------------------------------------------
uint16_t* MyMBAddr;
uint16_t MODBUS_HR[];
//------------------------------------------------------------------------------
//return codes
#define MODBUS_PACKET_VALID_AND_PROCESSED              0
#define MODBUS_PACKET_WRONG_ADDR                       1
#define MODBUS_PACKET_WRONG_CRC                        2
//------------------------------------------------------------------------------
#define MB_BROADCAST_ADDR                              0xff
//------------------------------------------------------------------------------
uint8_t process_net_packet(ComMessage* inPack, ComMessage* outPack);
//------------------------------------------------------------------------------
uint16_t calc_crc(uint8_t *arr, uint8_t length);
//------------------------------------------------------------------------------
void process_modbus(ComMessage* inPack, ComMessage* outPack);
//------------------------------------------------------------------------------
void CmdModbus_03_04(ComMessage* inPack, ComMessage* outPack);
//------------------------------------------------------------------------------
void CmdModbus_06(ComMessage* inPack, ComMessage* outPack);
//------------------------------------------------------------------------------
void CmdModbus_08(ComMessage* inPack, ComMessage* outPack);
//------------------------------------------------------------------------------
void CmdModbus_16(ComMessage* inPack, ComMessage* outPack);
//------------------------------------------------------------------------------
#endif /*MODBUS_H*/