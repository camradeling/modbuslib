#include "modbus_client.h"
#include "modbus.h"
//----------------------------------------------------------------------------------------------------------------------
extern uint8_t modbus_crc16H[];
extern uint8_t modbus_crc16L[];
//----------------------------------------------------------------------------------------------------------------------
uint16_t ModbusClient::calc_crc(uint8_t *arr, int length) 
{
    return calc_crc_buf(0xFFFF, arr, length);
}
//----------------------------------------------------------------------------------------------------------------------
void ModbusClient::wrap_pdu(std::vector<uint8_t>* data)
{
    if(pdu_type == MODBUS_RTU_PDU_TYPE)
    {
        uint16_t crc = calc_crc(data->data(), data->size());
#ifndef MODBUS_CRC_LITTLE_ENDIAN
        crc = SWAP16(crc);
#endif
        data->push_back(((uint8_t*)&crc)[0]);
        data->push_back(((uint8_t*)&crc)[1]);
    }
    else if(pdu_type == MODBUS_TCP_PDU_TYPE)
    {
        std::vector<uint8_t> head;
        head.push_back(0x00);
        head.push_back(0x01);
        head.push_back(0x00);
        head.push_back(0x00);
        //adding length
        head.push_back(((uint16_t)data->size() & 0xff00) >> 8);
        head.push_back((uint16_t)data->size() & 0x00ff);
        data->insert(data->begin(),head.begin(),head.end());
    }
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusClient::build_write_reg_06(uint8_t addr, uint16_t reg,uint16_t val)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x06);
    data.push_back((reg & 0xff00) >> 8);
    data.push_back(reg & 0x00ff);
    data.push_back((val & 0xff00) >> 8);
    data.push_back(val & 0x00ff);
    wrap_pdu(&data);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusClient::build_loop_08(uint8_t addr)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x08);
    wrap_pdu(&data);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusClient::build_read_03(uint8_t addr, uint16_t reg,uint16_t cnt)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x03);
    data.push_back((reg & 0xff00) >> 8);
    data.push_back(reg & 0x00ff);
    data.push_back((cnt & 0xff00) >> 8);
    data.push_back(cnt & 0x00ff);
    wrap_pdu(&data);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusClient::build_write_multreg_16(uint8_t addr, uint16_t startreg,std::vector<uint16_t> vals)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x10);
    data.push_back((startreg & 0xff00) >> 8);
    data.push_back(startreg & 0x00ff);
    data.push_back(vals.size());
    for(auto val : vals)
    {
        data.push_back((val & 0xff00) >> 8);
        data.push_back(val & 0x00ff);
    }
    wrap_pdu(&data);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
