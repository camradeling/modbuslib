//modbus_config.h file should be created and
//MBHR_SPACE_SIZE and
//TXRX_BUFFER_SIZE should be defined in it
//also MODBUS_HR and 
//MODBUS_WRITABLE_MASK array may be added as extern in the progam
//------------------------------------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>
//------------------------------------------------------------------------------------------------------------------------------
#include "modbus_config.h"
#include "modbus.h"
//------------------------------------------------------------------------------------------------------------------------------
//#ifdef FREERTOS
#include "freertos_atomic.h"
//#endif
//------------------------------------------------------------------------------------------------------------------------------
uint16_t MODBUS_HR[MBHR_SPACE_SIZE];
#ifdef WRITABLE_MASK_ARRAY_DECLARATION
uint8_t MODBUS_WRITABLE_MASK[MBHR_SPACE_SIZE/sizeof(uint8_t)+(MBHR_SPACE_SIZE%sizeof(uint8_t))?1:0];
#endif
uint16_t* MyMBAddr=NULL; // main program may or may not initialise it
register_cb isregwrtbl_cb = NULL; // main program may or may not initialise it
register_cb regwr_cb = NULL; // main program may or may not initialise it
//------------------------------------------------------------------------------------------------------------------------------
uint8_t process_net_packet(ComMessage* inPack, ComMessage* outPack, int pdu_type)
{
    int offset = (pdu_type == MODBUS_TCP_PDU_TYPE) ? MODBUS_TCP_HEADER_OFFSET : 0;
    if(MyMBAddr != NULL)
    {
        if(inPack->data[MODBUS_REQUEST_SLAVE_ADDRESS_POSITION+offset] != *MyMBAddr 
                && inPack->data[MODBUS_REQUEST_SLAVE_ADDRESS_POSITION+offset] != MB_BROADCAST_ADDR)
            return MODBUS_PACKET_WRONG_ADDR;
    }
    else if(inPack->data[MODBUS_REQUEST_SLAVE_ADDRESS_POSITION+offset] != MB_BROADCAST_ADDR)
        return MODBUS_PACKET_WRONG_ADDR;
    uint16_t tmpCRC;
    if(pdu_type != MODBUS_TCP_PDU_TYPE)
    {
        tmpCRC = calc_crc_buf(0xFFFF, inPack->data, inPack->length - 2);
#ifndef MODBUS_CRC_LITTLE_ENDIAN
        tmpCRC = SWAP16(tmpCRC);
#endif
        if(tmpCRC != *(uint16_t*)&inPack->data[inPack->length - 2])
            return MODBUS_PACKET_WRONG_CRC;
    }
    int res = process_modbus(inPack, outPack, pdu_type);
    if(pdu_type == MODBUS_TCP_PDU_TYPE && res == MODBUS_PACKET_VALID_AND_PROCESSED)
    {
        outPack->data[4] = ((outPack->length & 0xff00) >> 8);
        outPack->data[5] = (outPack->length & 0x00ff);
        outPack->length += offset;
    }
    if(pdu_type == MODBUS_RTU_PDU_TYPE && res == MODBUS_PACKET_VALID_AND_PROCESSED)
    {
        tmpCRC = calc_crc_buf(0xFFFF, outPack->data, outPack->length);
#ifndef MODBUS_CRC_LITTLE_ENDIAN
        tmpCRC = SWAP16(tmpCRC);
#endif
        *(uint16_t*)&outPack->data[outPack->length] = tmpCRC;
        outPack->length += 2;
    }
    return res;
}
//------------------------------------------------------------------------------------------------------------------------------
//=== Анализ Modbus-команды ===//
int process_modbus(ComMessage* inPack, ComMessage* outPack, int pdu_type)
{
    int res = MODBUS_PACKET_VALID_AND_PROCESSED;
    int offset = 0;
    if(pdu_type == MODBUS_TCP_PDU_TYPE)
    {
        offset = MODBUS_TCP_HEADER_OFFSET;
        outPack->data[0] = inPack->data[0];
        outPack->data[1] = inPack->data[1];
        outPack->data[2] = inPack->data[2];
        outPack->data[3] = inPack->data[3];
    }
    outPack->data[MODBUS_REQUEST_SLAVE_ADDRESS_POSITION+offset] = inPack->data[MODBUS_REQUEST_SLAVE_ADDRESS_POSITION+offset];
    outPack->data[MODBUS_REQUEST_FUNCTION_CODE_POSITION+offset] = inPack->data[MODBUS_REQUEST_FUNCTION_CODE_POSITION+offset];
    switch(inPack->data[MODBUS_REQUEST_FUNCTION_CODE_POSITION+offset])
    {		// Байт команды.
    case MODBUS_READ_HOLDING_REGISTERS:		// <03> holding registers read.
    case MODBUS_READ_INPUT_REGISTERS:		// <04> input registers read.
        res = CmdModbus_03_04(inPack, outPack,offset);
        break;
    case MODBUS_WRITE_SINGLE_REGISTER:		// <06> single holding register write.
        res = CmdModbus_06(inPack, outPack,offset);
        break;
    case MODBUS_LOOPBACK:   // <08> loopback
        res = CmdModbus_08(inPack, outPack,offset);
        break;
    case MODBUS_WRITE_MULTIPLE_REGISTERS:		// <16> multiple holding registers write.
        res = CmdModbus_16(inPack, outPack,offset);
        break;
    default:
        break;
    }
    return res;
}
//------------------------------------------------------------------------------------------------------------------------------
//=== <Modbus_03_04> holding/input registers read ===//
int CmdModbus_03_04(ComMessage* inPack, ComMessage* outPack, int offset)
{
    uint16_t Len, addr;
    Len = 2*(inPack->data[5+offset]+((uint16_t)inPack->data[4+offset] << 8));		// bytes to read
    if(Len >= TXRX_BUFFER_SIZE - 3-offset) 
        Len = TXRX_BUFFER_SIZE-4-offset;		// preventing segfault
    addr=((uint16_t)inPack->data[2+offset] << 8) + inPack->data[3+offset];		// first register to read
    if((addr + Len/2) > MBHR_SPACE_SIZE)
        return MODBUS_REGISTER_NUMBER_INVALID;
    outPack->data[2+offset] = Len;		// number of bytes.
    outPack->length = 3 + Len;		// reply length
    for(int i = 0; i < Len; i += 2)
    {		
        // filling data
        uint16_t val = a_load(MODBUS_HR[addr]);
        *(uint16_t*)&outPack->data[3+offset+i] = SWAP16(val);		// Big endian here
        addr++;
    }
    return MODBUS_PACKET_VALID_AND_PROCESSED;
}
//------------------------------------------------------------------------------------------------------------------------------
//=== <Modbus_06> single holding register write ===//
int CmdModbus_06(ComMessage* inPack, ComMessage* outPack, int offset)
{
    uint16_t Len, addr;
    uint8_t res = 0;
    outPack->length = 6;		// reply length
    addr=((uint16_t)inPack->data[2+offset] << 8) + inPack->data[3+offset];		// address to write
    if(addr > MBHR_SPACE_SIZE-1) 
        return MODBUS_REGISTER_NUMBER_INVALID;		// preventing segfault  
    int wrtbl=1;
    if(isregwrtbl_cb)
        wrtbl = isregwrtbl_cb(addr);
    if(!wrtbl)
        return MODBUS_REGISTER_WRITE_PROTECTED;
    uint16_t val = ((uint16_t)inPack->data[4+offset] << 8) + inPack->data[5+offset];		// value to write
    a_store(MODBUS_HR[addr], val);
    if(regwr_cb)
        res = regwr_cb(addr);
    if(res)//if callback failed
    {
        //
    }
    for(int i = 1; i < 6; i++) 
        outPack->data[i+offset] = inPack->data[i+offset];		// copy some bytes to reply
    return MODBUS_PACKET_VALID_AND_PROCESSED;
}
//------------------------------------------------------------------------------------------------------------------------------
//=== <Modbus_08> loopback ===//
int CmdModbus_08(ComMessage* inPack, ComMessage* outPack, int offset)
{
    if(!offset)
        outPack->length = inPack->length-2;
    else
        outPack->length = inPack->length;
    for(int i = 0; i < outPack->length; i++)
    {
        outPack->data[i+offset] = inPack->data[i+offset];
    }
    return MODBUS_PACKET_VALID_AND_PROCESSED;
}
//------------------------------------------------------------------------------------------------------------------------------
//=== <Modbus_16> multiple holding registers write ===//
int CmdModbus_16(ComMessage* inPack, ComMessage* outPack, int offset)
{
    uint16_t addr;
    int res = 0;
    outPack->length = 6;		// reply length
    addr=((uint16_t)inPack->data[2+offset] << 8) + inPack->data[3+offset];		// first register to copy
    if(addr > MBHR_SPACE_SIZE-1) 
        return MODBUS_REGISTER_NUMBER_INVALID;		// preventing segfault
    uint16_t cnt = inPack->data[4+offset];		// registers number
    if(addr + cnt > MBHR_SPACE_SIZE) 
        return MODBUS_REGISTER_NUMBER_INVALID;    // preventing segfault
    for(int i = 0; i < cnt; i++)
    {		
        // filling the date.
        int wrtbl=1;
        if(isregwrtbl_cb)
            wrtbl = isregwrtbl_cb(addr);
        if(!wrtbl)
            return MODBUS_REGISTER_WRITE_PROTECTED;
        uint16_t val = (((uint16_t)inPack->data[5+2*i+offset]<<8) + inPack->data[6+2*i+offset]);
        a_store(MODBUS_HR[addr], val);// and another register value
        if(regwr_cb)
            res = regwr_cb(addr);
        if(res)//if callback failed
            return MODBUS_REGISTER_WRITE_CALLBACK_FAILED;
        addr++;
    }
    for(int i = 1; i < 6; i++) 
        outPack->data[i+offset] = inPack->data[i+offset];   // copy to reply
    return MODBUS_PACKET_VALID_AND_PROCESSED;
}
//------------------------------------------------------------------------------------------------------------------------------
