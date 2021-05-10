//------------------------------------------------------------------------------
#include "includes.h"
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
  //MODBUS_HR[MBHR_DISCRETE_OUTPUTS_LOW] = MODBUS_HR[MBHR_DISCRETE_OUTPUTS_LOW]+1;
  if(inPack->data[0] != MODBUS_HR[MBHR_MY_MBADDR] && inPack->data[0] != MB_BROADCAST_ADDR)
    return MODBUS_PACKET_WRONG_ADDR;
  uint16_t tmpCRC = calc_crc(inPack->data, inPack->length - 2);
  if(tmpCRC != *(uint16_t*)&inPack->data[inPack->length - 2])
     return MODBUS_PACKET_WRONG_CRC;
  MODBUS_HR[MBHR_DISCRETE_OUTPUTS_LOW] = MODBUS_HR[MBHR_DISCRETE_INPUTS_LOW]+1;
  analise_modbus(inPack, outPack);
  tmpCRC = calc_crc(outPack->data, outPack->length);
  *(uint16_t*)&outPack->data[outPack->length] = tmpCRC;
  outPack->length += 2;
  return MODBUS_PACKET_VALID_AND_PROCESSED;
}
//------------------------------------------------------------------------------
//=== Анализ Modbus-команды ===//
void analise_modbus(ComMessage* inPack, ComMessage* outPack)
{
  outPack->data[0] = inPack->data[0];
  outPack->data[1] = inPack->data[1];
  switch(inPack->data[1])
  {		// Байт команды.
  case 3:		// <03> Чтение регистров хранения.
  case 4:		// <04> Чтение входных регистров.
    CmdModbus_03_04(inPack, outPack);
    break;
  case 6:		// <06> Запись одного регистра хранения.
    CmdModbus_06(inPack, outPack);
    break;
  case 8:   // <08> loopback
    CmdModbus_08(inPack, outPack);
    break;
  case 16:		// <16> Запись множества входных регистров.
    CmdModbus_16(inPack, outPack);
    break;
  default:
    break;
  }
}
//------------------------------------------------------------------------------
//=== <Modbus_03_04> Чтение регистров хранения/Чтение входных регистров ===//
void CmdModbus_03_04(ComMessage* inPack, ComMessage* outPack)
{
  uint16_t Len, addr;
  Len = 2*(inPack->data[5]+((uint16_t)inPack->data[4] << 8));		// Количество байт возвращаемых данных, четное.
  if(Len >= TXRX_BUFFER_SIZE - 3) 
    Len = TXRX_BUFFER_SIZE-4;		// Предохранитель.
  addr=((uint16_t)inPack->data[2] << 8) + inPack->data[3];		// Адрес первого читаемого элемента.
  outPack->data[2] = Len;		// Четное количество байт данных.
  outPack->length = 3 + Len;		// Длина ответной датаграммы - адрес, функция, количество, данные.
  //
  for(int i = 0; i < Len; i += 2)
  {		// Заполним данные.
    if(addr >= MODBUS_HR_SPACE_LAST_ADDR)
      addr = MODBUS_HR_SPACE_LAST_ADDR;
    uint16_t val = MODBUS_HR[addr];
    *(uint16_t*)&outPack->data[3+i] = SWAP16(val);		// Старший байт - первый.
    addr++;
  }
}
//------------------------------------------------------------------------------
//=== <Modbus_06> Запись одного регистра хранения ===//
void CmdModbus_06(ComMessage* inPack, ComMessage* outPack)
{
  uint16_t Len, addr;
  uint8_t rewr = 0;
  outPack->length = 6;		// Длина ответной датаграммы.
  addr=((uint16_t)inPack->data[2] << 8) + inPack->data[3];		// Адрес сохраняемого элемента.
  if(addr >= MODBUS_HR_SPACE_LAST_ADDR) 
    addr = MODBUS_HR_SPACE_LAST_ADDR;		// Предохранитель выхода адреса регистра за пределы.
  uint16_t val = ((uint16_t)inPack->data[4] << 8) + inPack->data[5];		// Сохраняемое значение.
  if(addr == MBHR_COMMAND_REG)
  {
    if(val == CMD_WRITE_FLASH_PARAMETERS_BACKUP)
    {
      rewr = 1;
    }
    else if(val == CMD_REWRITE_FLASH)
    {
      uint32_t dest = (uint32_t)reflash;
      for(int i = 0; i < 1024; i++)
        *(uint8_t*)(RAM_BOOT + i) = *(uint8_t*)(dest - 1 + i);
      ((void(*)(void))(RAM_BOOT + 1))();
    }
    else if(val == CMD_REBOOT)
    {
      REBOOT();
    }
  }
  else
    MODBUS_HR[addr]= val;
  for(int i = 1; i < 6; i++) 
    outPack->data[i] = inPack->data[i];		// Скопируем.
  if(rewr)
  {
    w25_params_write();
  }
}
//------------------------------------------------------------------------------
void CmdModbus_08(ComMessage* inPack, ComMessage* outPack)
{
  outPack->length = inPack->length-2;
  memcpy(outPack->data,inPack->data,inPack->length-2);
}
//------------------------------------------------------------------------------
//=== <Modbus_16> Запись множества входных регистров ===//
void CmdModbus_16(ComMessage* inPack, ComMessage* outPack)
{
  uint16_t addr;
  outPack->length = 6;		// Длина ответной датаграммы.
  addr=((uint16_t)inPack->data[2] << 8) + inPack->data[3];		// Адрес первого сохраняемого элемента.
  if(addr >= MODBUS_HR_SPACE_LAST_ADDR) 
    addr = MODBUS_HR_SPACE_LAST_ADDR;		// Предохранитель выхода адреса регистра за пределы.
  uint16_t cnt = inPack->data[5];		// Количество сохраняемых регистров
  for(int i = 1; i < 6; i++) 
    outPack->data[i] = inPack->data[i];		// Скопируем.
  for(int i = 0; i < cnt; i++)
  {		// Заполним данные.
    if(addr >= MODBUS_HR_SPACE_LAST_ADDR) 
      addr = MODBUS_HR_SPACE_LAST_ADDR;		// Предохранитель выхода адреса регистра за пределы.
    MODBUS_HR[addr]=((uint16_t)inPack->data[7+2*i]<<8) + inPack->data[8+2*i];		// Очередное сохраняемое значение.     
    addr++;
  }
}