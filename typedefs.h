#ifndef TYPEDEFS_H
#define TYPEDEFS_H
typedef struct __attribute__ ((packed)) {
  uint8_t prefix;            // 1 byte   
  uint8_t field_update_cmd;  // 1 byte 
  uint8_t page;              // 1 byte 
  uint8_t offset;            // 1 byte
  uint16_t len;              // 2 bytes  
  uint16_t check_sum;        // 2 bytes  
} bt_command_t; 


typedef struct {
	device_field_data_t * bluetti_polling_command;
	int so_b_p_c;
	device_field_data_t * bluetti_device_command;
	int so_b_d_c;
	device_field_data_t * bluetti_device_state;
	int so_b_d_s;
} bluetti_command_t ;
#endif
