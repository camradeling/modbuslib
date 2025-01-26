#ifndef MODBUS_CLIENT_H
#define MODBUS_CLIENT_H
//----------------------------------------------------------------------------------------------------------------------
#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include <vector>
#include "modbus.h"
//----------------------------------------------------------------------------------------------------------------------
#ifndef timespec_t
typedef struct timespec timespec_t;
#endif
//----------------------------------------------------------------------------------------------------------------------
#define MB_BROADCAST_ADDR                              0xff
//----------------------------------------------------------------------------------------------------------------------
typedef struct
{
	uint8_t SlaveAddress = 0;
	uint8_t FunctionCode = 0;
	uint16_t reg = 0;
	uint16_t cnt = 0;
	std::vector<uint16_t> values;
	timespec_t timestamp = {0,0};
} ModbusRequest;
//----------------------------------------------------------------------------------------------------------------------
class ModbusMasterPacketConstructor
{
public:
	ModbusMasterPacketConstructor(){}
	virtual ~ModbusMasterPacketConstructor(){}
	static void wrap_pdu(std::vector<uint8_t>* data, int pdu_type);
	static std::vector<uint8_t> build_loop_08(uint8_t addr, int pdu_type);
	static std::vector<uint8_t> build_read_03(uint8_t addr, uint16_t reg,uint16_t cnt, int pdu_type);
	static std::vector<uint8_t> build_write_reg_06(uint8_t addr, uint16_t reg,uint16_t val, int pdu_type);
	static std::vector<uint8_t> build_write_multreg_16(uint8_t addr, uint16_t startreg,std::vector<uint16_t> vals, int pdu_type);
	static uint16_t calc_crc(uint8_t *arr, int length);
	static std::vector<uint8_t> serialize_request(ModbusRequest &req, int pdu_type);
};
//----------------------------------------------------------------------------------------------------------------------
#endif /*MODBUS_CLIENT_H*/
