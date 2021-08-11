//modbus_config.h file should be created and
//MBHR_SPACE_SIZE and
//TXRX_BUFFER_SIZE should be defined in it
//also MODBUS_HR and 
//MODBUS_WRITABLE_MASK array should be added as extern in the progam
//------------------------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>
//------------------------------------------------------------------------------
#include "modbus_config.h"
#include "modbus.h"
//------------------------------------------------------------------------------
uint16_t MODBUS_HR[MBHR_SPACE_SIZE];
uint8_t MODBUS_WRITABLE_MASK[MBHR_SPACE_SIZE/sizeof(uint8_t)+(MBHR_SPACE_SIZE%sizeof(uint8_t))?1:0];
uint16_t* MyMBAddr=NULL; // main program may or may not initialise it
register_cb isregwrtbl_cb = NULL; // main program may or may not initialise it
register_cb regwr_cb = NULL; // main program may or may not initialise it
//------------------------------------------------------------------------------
const uint8_t modbus_crc16H[256] =
{
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
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};
//------------------------------------------------------------------------------
const uint8_t modbus_crc16L[256] =
{
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
  0x43, 0x83, 0x41, 0x81, 0x80, 0x40
} ;
//------------------------------------------------------------------------------
uint16_t calc_crc(uint8_t *arr, uint8_t length) 
{
  uint8_t ind;
  uint8_t i;
  uint8_t cksumHigh = 0xFF;
  uint8_t cksumLow = 0xFF;
  if(length > 0) 
  {
    for(i=0; i<length; i++) 
    {
      ind = cksumHigh ^ arr[i];
      cksumHigh = cksumLow ^ modbus_crc16H[ind];
      cksumLow = modbus_crc16L[ind];
    }
  }
  return cksumLow |(cksumHigh << 8);
}
//------------------------------------------------------------------------------
uint8_t process_net_packet(ComMessage* inPack, ComMessage* outPack)
{
  if(MyMBAddr != NULL)
  {
    if(inPack->data[0] != *MyMBAddr && inPack->data[0] != MB_BROADCAST_ADDR)
      return MODBUS_PACKET_WRONG_ADDR;
  }
  else if(inPack->data[0] != MB_BROADCAST_ADDR)
    return MODBUS_PACKET_WRONG_ADDR;
  uint16_t tmpCRC = calc_crc(inPack->data, inPack->length - 2);
  if(tmpCRC != *(uint16_t*)&inPack->data[inPack->length - 2])
     return MODBUS_PACKET_WRONG_CRC;
  int res = process_modbus(inPack, outPack);
  if(res == MODBUS_PACKET_VALID_AND_PROCESSED)
  {
    tmpCRC = calc_crc(outPack->data, outPack->length);
    *(uint16_t*)&outPack->data[outPack->length] = tmpCRC;
    outPack->length += 2;
  }
  return res;
}
//------------------------------------------------------------------------------
//=== Анализ Modbus-команды ===//
int process_modbus(ComMessage* inPack, ComMessage* outPack)
{
  int res = MODBUS_PACKET_VALID_AND_PROCESSED;
  outPack->data[0] = inPack->data[0];
  outPack->data[1] = inPack->data[1];
  switch(inPack->data[1])
  {		// Байт команды.
  case 3:		// <03> holding registers read.
  case 4:		// <04> input registers read.
    res = CmdModbus_03_04(inPack, outPack);
    break;
  case 6:		// <06> single holding register write.
    res = CmdModbus_06(inPack, outPack);
    break;
  case 8:   // <08> loopback
    res = CmdModbus_08(inPack, outPack);
    break;
  case 16:		// <16> multiple holding registers write.
    res = CmdModbus_16(inPack, outPack);
    break;
  default:
    break;
  }
  return res;
}
//------------------------------------------------------------------------------
//=== <Modbus_03_04> holding/input registers read ===//
int CmdModbus_03_04(ComMessage* inPack, ComMessage* outPack)
{
  uint16_t Len, addr;
  Len = 2*(inPack->data[5]+((uint16_t)inPack->data[4] << 8));		// bytes to read
  if(Len >= TXRX_BUFFER_SIZE - 3) 
    Len = TXRX_BUFFER_SIZE-4;		// preventing segfault
  addr=((uint16_t)inPack->data[2] << 8) + inPack->data[3];		// first register to read
  if((addr + Len/2) >= MBHR_SPACE_SIZE-1)
    return MODBUS_REGISTER_NUMBER_INVALID;
  outPack->data[2] = Len;		// number of bytes.
  outPack->length = 3 + Len;		// reply length
  //
  for(int i = 0; i < Len; i += 2)
  {		// filling data
    uint16_t val = MODBUS_HR[addr];
    *(uint16_t*)&outPack->data[3+i] = SWAP16(val);		// Big endian here
    addr++;
  }
  return MODBUS_PACKET_VALID_AND_PROCESSED;
}
//------------------------------------------------------------------------------
//=== <Modbus_06> single holding register write ===//
int CmdModbus_06(ComMessage* inPack, ComMessage* outPack)
{
  uint16_t Len, addr;
  uint8_t rewr = 0;
  outPack->length = 6;		// reply length
  addr=((uint16_t)inPack->data[2] << 8) + inPack->data[3];		// address to write
  if(addr >= MBHR_SPACE_SIZE-1) 
    return MODBUS_REGISTER_NUMBER_INVALID;		// preventing segfault  
  int wrtbl=1;
  if(isregwrtbl_cb)
    wrtbl = isregwrtbl_cb(addr);
  if(!wrtbl)
    return MODBUS_REGISTER_WRITE_PROTECTED;
  uint16_t val = ((uint16_t)inPack->data[4] << 8) + inPack->data[5];		// value to write
  MODBUS_HR[addr]= val;
  if(regwr_cb)
    regwr_cb(addr);
  for(int i = 1; i < 6; i++) 
    outPack->data[i] = inPack->data[i];		// copy some bytes to reply
  return MODBUS_PACKET_VALID_AND_PROCESSED;
}
//------------------------------------------------------------------------------
//=== <Modbus_08> loopback ===//
int CmdModbus_08(ComMessage* inPack, ComMessage* outPack)
{
  outPack->length = inPack->length-2;
  for(int i = 0; i < outPack->length; i++)
  {
    outPack->data[i] = inPack->data[i];
  }
  return MODBUS_PACKET_VALID_AND_PROCESSED;
}
//------------------------------------------------------------------------------
//=== <Modbus_16> multiple holding registers write ===//
int CmdModbus_16(ComMessage* inPack, ComMessage* outPack)
{
  uint16_t addr;
  outPack->length = 6;		// reply length
  addr=((uint16_t)inPack->data[2] << 8) + inPack->data[3];		// first register to copy
  if(addr >= MBHR_SPACE_SIZE-1) 
    return MODBUS_REGISTER_NUMBER_INVALID;		// preventing segfault
  uint16_t cnt = inPack->data[5];		// registers number
  if(addr + cnt >= MBHR_SPACE_SIZE-1) 
      return MODBUS_REGISTER_NUMBER_INVALID;    // preventing segfault
  for(int i = 1; i < 6; i++) 
    outPack->data[i] = inPack->data[i];		// copy to reply
  for(int i = 0; i < cnt; i++)
  {		// filling the date.
    int wrtbl=1;
    if(isregwrtbl_cb)
      wrtbl = isregwrtbl_cb(addr);
    if(!wrtbl)
      return MODBUS_REGISTER_WRITE_PROTECTED;
    MODBUS_HR[addr]=((uint16_t)inPack->data[7+2*i]<<8) + inPack->data[8+2*i];		// and another register value
    if(regwr_cb)
      regwr_cb(addr);
    addr++;
  }
  return MODBUS_PACKET_VALID_AND_PROCESSED;
}
//------------------------------------------------------------------------------
