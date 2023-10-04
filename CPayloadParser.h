#ifndef PAYLOAD_PARSER_H
#define PAYLOAD_PARSER_H
#include "Arduino.h"
#include "DeviceType.h"
#include "typedefs.h"
#include "Bluetti.h"
#include "BluettiConfig.h"

#define HEADER_SIZE 4
#define CHECKSUM_SIZE 2

class CPayloadParser {
	private:
		uint16_t parse_uint_field(uint8_t data[]);
		bool parse_bool_field(uint8_t data[]);
		float parse_decimal_field(uint8_t data[], uint8_t scale);
		uint64_t parse_serial_field(uint8_t data[]);
		float parse_version_field(uint8_t data[]);
		String parse_string_field(uint8_t data[]);
		String parse_enum_field(uint8_t data[]);
	
	public:
		void (*notifyCallback) (char *, String);


	public:
		CPayloadParser( void (*nc)(char * , String) );
		void parse_bluetooth_data(bluetti_command_t &bluettiCommand,uint8_t page, uint8_t offset, uint8_t* pData, size_t length);
};
#endif
