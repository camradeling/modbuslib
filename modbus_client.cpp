#include "modbus_client.h"
#include "modbus.h"
//----------------------------------------------------------------------------------------------------------------------
extern uint8_t modbus_crc16H[];
extern uint8_t modbus_crc16L[];
//----------------------------------------------------------------------------------------------------------------------
uint16_t ModbusMasterPacketConstructor::calc_crc(uint8_t *arr, int length)
{
    return calc_crc_buf(MODBUS_CRC_START_VALUE, arr, length);
}
//----------------------------------------------------------------------------------------------------------------------
void ModbusMasterPacketConstructor::wrap_pdu(std::vector<uint8_t>* data, int pdu_type)
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
std::vector<uint8_t> ModbusMasterPacketConstructor::build_write_reg_06(uint8_t addr, uint16_t reg,uint16_t val, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x06);
    data.push_back((reg & 0xff00) >> 8);
    data.push_back(reg & 0x00ff);
    data.push_back((val & 0xff00) >> 8);
    data.push_back(val & 0x00ff);
    wrap_pdu(&data, pdu_type);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusMasterPacketConstructor::build_loop_08(uint8_t addr, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x08);
    wrap_pdu(&data, pdu_type);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusMasterPacketConstructor::build_read_03(uint8_t addr, uint16_t reg,uint16_t cnt, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x03);
    data.push_back((reg & 0xff00) >> 8);
    data.push_back(reg & 0x00ff);
    data.push_back((cnt & 0xff00) >> 8);
    data.push_back(cnt & 0x00ff);
    wrap_pdu(&data, pdu_type);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusMasterPacketConstructor::build_write_multreg_16(uint8_t addr, uint16_t startreg,std::vector<uint16_t> vals, int pdu_type)
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
    wrap_pdu(&data, pdu_type);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusMasterPacketConstructor::serialize_request(ModbusRequest &req, int pdu_type)
{
    std::vector<uint8_t> data;
    switch (req.FunctionCode)
    {
    case MODBUS_READ_HOLDING_REGISTERS:
    case MODBUS_READ_INPUT_REGISTERS:
        return build_read_03(req.SlaveAddress, req.reg, req.cnt, pdu_type);
    case MODBUS_WRITE_SINGLE_REGISTER:
        return build_write_reg_06(req.SlaveAddress, req.reg, req.values[0], pdu_type);
    case MODBUS_LOOPBACK:
        return build_loop_08(req.SlaveAddress, pdu_type);
    case MODBUS_WRITE_MULTIPLE_REGISTERS:
        return build_write_multreg_16(req.SlaveAddress, req.reg, req.values, pdu_type);
    default:
        return data;
    }
}
//----------------------------------------------------------------------------------------------------------------------
