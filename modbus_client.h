#ifndef MODBUS_CLIENT_H
#define MODBUS_CLIENT_H
//----------------------------------------------------------------------------------------------------------------------
#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include <vector>
#include "modbus.h"
//----------------------------------------------------------------------------------------------------------------------
#define MB_BROADCAST_ADDR                              0xff
//----------------------------------------------------------------------------------------------------------------------
class ModbusClient
{
public:
	ModbusClient(int type){pdu_type = type;}
	ModbusClient(){pdu_type = MODBUS_RTU_PDU_TYPE;}
	~ModbusClient(){}
	int pdu_type;
	void wrap_pdu(std::vector<uint8_t>* data);
	std::vector<uint8_t> build_loop_08(uint8_t addr);
	std::vector<uint8_t> build_read_03(uint8_t addr, uint16_t reg,uint16_t cnt);
	std::vector<uint8_t> build_write_reg_06(uint8_t addr, uint16_t reg,uint16_t val);
	std::vector<uint8_t> build_write_multreg_16(uint8_t addr, uint16_t startreg,std::vector<uint16_t> vals);
	uint16_t calc_crc(uint8_t *arr, int length);
};
//----------------------------------------------------------------------------------------------------------------------
#endif /*MODBUS_CLIENT_H*/