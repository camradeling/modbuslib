#include "modbus_client.h"
#include "modbus.h"
//----------------------------------------------------------------------------------------------------------------------
extern uint8_t modbus_crc16H[];
extern uint8_t modbus_crc16L[];
//----------------------------------------------------------------------------------------------------------------------
uint16_t ModbusPacketConstructor::calc_crc(uint8_t *arr, int length)
{
    return calc_crc_buf(MODBUS_CRC_START_VALUE, arr, length);
}
//----------------------------------------------------------------------------------------------------------------------
void ModbusPacketConstructor::wrap_pdu(std::vector<uint8_t>* data, int pdu_type, uint16_t id)
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
        head.push_back((id & 0xff00) >> 8);
        head.push_back(id & 0x00ff);
        head.push_back(0x00);
        head.push_back(0x00);
        //adding length
        head.push_back(((uint16_t)data->size() & 0xff00) >> 8);
        head.push_back((uint16_t)data->size() & 0x00ff);
        data->insert(data->begin(),head.begin(),head.end());
    }
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::build_force_coil_05_request(uint8_t addr, uint16_t coilnum, uint16_t val, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x06);
    data.push_back((coilnum & 0xff00) >> 8);
    data.push_back(coilnum & 0x00ff);
    data.push_back(val ? 0xFF : 0x00);
    data.push_back(0x00);
    wrap_pdu(&data, pdu_type, 1);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::build_write_reg_06_request(uint8_t addr, uint16_t reg,uint16_t val, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x06);
    data.push_back((reg & 0xff00) >> 8);
    data.push_back(reg & 0x00ff);
    data.push_back((val & 0xff00) >> 8);
    data.push_back(val & 0x00ff);
    wrap_pdu(&data, pdu_type, 1);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::build_loop_08_request(uint8_t addr, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x08);
    wrap_pdu(&data, pdu_type, 1);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::build_read_01_request(uint8_t addr, uint16_t coilnum, uint16_t cnt, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x01);
    data.push_back((coilnum & 0xff00) >> 8);
    data.push_back(coilnum & 0x00ff);
    data.push_back((cnt & 0xff00) >> 8);
    data.push_back(cnt & 0x00ff);
    wrap_pdu(&data, pdu_type, 1);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::build_read_03_request(uint8_t addr, uint16_t reg,uint16_t cnt, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x03);
    data.push_back((reg & 0xff00) >> 8);
    data.push_back(reg & 0x00ff);
    data.push_back((cnt & 0xff00) >> 8);
    data.push_back(cnt & 0x00ff);
    wrap_pdu(&data, pdu_type, 1);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::build_write_multi_reg_16_request(uint8_t addr, uint16_t startreg, std::vector<uint16_t> vals, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x10);
    data.push_back((startreg & 0xff00) >> 8);
    data.push_back(startreg & 0x00ff);
    uint16_t numregs = vals.size();
    data.push_back((numregs & 0xff00) >> 8);
    data.push_back(numregs & 0x00ff);
    data.push_back((uint8_t)numregs*2);
    for(auto val : vals)
    {
        data.push_back((val & 0xff00) >> 8);
        data.push_back(val & 0x00ff);
    }
    wrap_pdu(&data, pdu_type, 1);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::build_loop_08_reply(uint16_t id, uint8_t addr, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x08);
    wrap_pdu(&data, pdu_type, id);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::build_read_01_reply(uint16_t id, uint8_t addr, uint16_t cnt, std::vector<uint16_t> vals, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x01);
    // number of registers * 2 bytes each
    data.push_back(cnt);
    for(auto val : vals)
    {
        data.push_back(val & 0x00ff);
    }
    wrap_pdu(&data, pdu_type, id);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::build_read_03_reply(uint16_t id, uint8_t addr, uint16_t cnt, std::vector<uint16_t> vals, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x03);
    // number of registers * 2 bytes each
    data.push_back(cnt*2);
    for(auto val : vals)
    {
        data.push_back((val & 0xff00) >> 8);
        data.push_back(val & 0x00ff);
    }
    wrap_pdu(&data, pdu_type, id);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::build_force_coil_05_reply(uint16_t id, uint8_t addr, uint16_t coilnum, uint16_t val, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x05);
    data.push_back((coilnum & 0xff00) >> 8);
    data.push_back(coilnum & 0x00ff);
    data.push_back((val & 0xff00) >> 8);
    data.push_back(val & 0x00ff);
    wrap_pdu(&data, pdu_type, id);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::build_write_reg_06_reply(uint16_t id, uint8_t addr, uint16_t reg, uint16_t val, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x06);
    data.push_back((reg & 0xff00) >> 8);
    data.push_back(reg & 0x00ff);
    data.push_back((val & 0xff00) >> 8);
    data.push_back(val & 0x00ff);
    wrap_pdu(&data, pdu_type, id);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::build_write_multi_reg_16_reply(uint16_t id, uint8_t addr, uint16_t startreg, uint16_t cnt, int pdu_type)
{
    std::vector<uint8_t> data;
    data.push_back(addr);
    data.push_back(0x10);
    data.push_back((startreg & 0xff00) >> 8);
    data.push_back(startreg & 0x00ff);
    data.push_back((cnt & 0xff00) >> 8);
    data.push_back(cnt & 0x00ff);
    wrap_pdu(&data, pdu_type, id);
    return data;
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::serialize_request(ModbusPDU &req, int pdu_type)
{
    std::vector<uint8_t> data;
    switch (req.FunctionCode)
    {
    case MODBUS_READ_COIL_STATUS:
        return build_read_01_request(req.SlaveAddress, req.reg, req.cnt, pdu_type);
    case MODBUS_READ_HOLDING_REGISTERS:
    case MODBUS_READ_INPUT_REGISTERS:
        return build_read_03_request(req.SlaveAddress, req.reg, req.cnt, pdu_type);
    case MODBUS_FORCE_SINGLE_COIL:
        return build_force_coil_05_request(req.SlaveAddress, req.reg, req.values[0], pdu_type);
    case MODBUS_WRITE_SINGLE_REGISTER:
        return build_write_reg_06_request(req.SlaveAddress, req.reg, req.values[0], pdu_type);
    case MODBUS_LOOPBACK:
        return build_loop_08_request(req.SlaveAddress, pdu_type);
    case MODBUS_WRITE_MULTIPLE_REGISTERS:
        return build_write_multi_reg_16_request(req.SlaveAddress, req.reg, req.values, pdu_type);
    default:
        return data;
    }
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> ModbusPacketConstructor::serialize_reply(ModbusPDU &req, int pdu_type)
{
    std::vector<uint8_t> data;
    switch (req.FunctionCode)
    {
    case MODBUS_READ_COIL_STATUS:
        return build_read_01_reply(req.transactionID, req.SlaveAddress, req.cnt, req.values, pdu_type);
    case MODBUS_READ_HOLDING_REGISTERS:
    case MODBUS_READ_INPUT_REGISTERS:
        return build_read_03_reply(req.transactionID, req.SlaveAddress, req.cnt, req.values, pdu_type);
    case MODBUS_FORCE_SINGLE_COIL:
        return build_force_coil_05_reply(req.transactionID, req.SlaveAddress, req.reg, req.values[0], pdu_type);
    case MODBUS_WRITE_SINGLE_REGISTER:
        return build_write_reg_06_reply(req.transactionID, req.SlaveAddress, req.reg, req.values[0], pdu_type);
    case MODBUS_LOOPBACK:
        return build_loop_08_reply(req.transactionID, req.SlaveAddress, pdu_type);
    case MODBUS_WRITE_MULTIPLE_REGISTERS:
        return build_write_multi_reg_16_reply(req.transactionID, req.SlaveAddress, req.reg, req.cnt, pdu_type);
    default:
        return data;
    }
}
//----------------------------------------------------------------------------------------------------------------------
