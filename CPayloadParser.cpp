
#include "CPayloadParser.h"

CPayloadParser::CPayloadParser(void (*nc)(String, String) )
{
	notifyCallback = nc;
}
String CPayloadParser::map_field_name(enum field_names f_name)
{
   switch(f_name) {
      case DC_OUTPUT_POWER:
        return "dc_output_power";
        break; 
      case AC_OUTPUT_POWER:
        return "ac_output_power";
        break; 
      case DC_OUTPUT_ON:
        return "dc_output_on";
        break; 
      case AC_OUTPUT_ON:
        return "ac_output_on";
        break; 
      case AC_OUTPUT_MODE:
        return "ac_output_mode";
        break; 
      case POWER_GENERATION:
        return "power_generation";
        break;       
      case TOTAL_BATTERY_PERCENT:
        return "total_battery_percent";
        break; 
      case DC_INPUT_POWER:
        return "dc_input_power";
        break;
      case AC_INPUT_POWER:
        return "ac_input_power";
        break;
      case AC_INPUT_VOLTAGE:
        return "ac_input_voltage";
        break;
      case AC_INPUT_FREQUENCY:
        return "ac_input_frequency";
        break;
      case PACK_VOLTAGE:
        return "pack_voltage";
        break;
      case INTERNAL_PACK_VOLTAGE:
        return "internal_pack_voltage";
        break;
      case SERIAL_NUMBER:
        return "serial_number";
        break;
      case ARM_VERSION:
        return "arm_version";
        break;
      case DSP_VERSION:
        return "dsp_version";
        break;
      case DEVICE_TYPE:
        return "device_type";
        break;
      case UPS_MODE:
        return "ups_mode";
        break;
      case AUTO_SLEEP_MODE:
        return "auto_sleep_mode";
        break;
      case GRID_CHARGE_ON:
        return "grid_charge_on";
        break;
      case INTERNAL_AC_VOLTAGE:
        return "internal_ac_voltage";
        break;
      case INTERNAL_AC_FREQUENCY:
        return "internal_ac_frequency";
        break;
      case INTERNAL_CURRENT_ONE:
        return "internal_current_one";
        break;
      case INTERNAL_POWER_ONE:
        return "internal_power_one";
        break;
      case INTERNAL_CURRENT_TWO:
        return "internal_current_two";
        break;
      case INTERNAL_POWER_TWO:
        return "internal_power_two";
        break;
      case INTERNAL_CURRENT_THREE:
        return "internal_current_three";
        break;
      case INTERNAL_POWER_THREE:
        return "internal_power_three";
        break;
      case PACK_NUM_MAX:
        return "pack_max_num";
        break;
      case PACK_NUM:
        return "pack_num";
        break;
      case PACK_BATTERY_PERCENT:
        return "pack_battery_percent";
        break;
      case INTERNAL_DC_INPUT_VOLTAGE:
        return "internal_dc_input_voltage";
        break;
      case INTERNAL_DC_INPUT_POWER:
        return "internal_dc_input_power";
        break;
      case INTERNAL_DC_INPUT_CURRENT:
        return "internal_dc_input_current";
        break;
      case INTERNAL_CELL01_VOLTAGE:
        return "internal_cell01_voltage";    
        break;
      case INTERNAL_CELL02_VOLTAGE:
        return "internal_cell02_voltage";    
        break;
      case INTERNAL_CELL03_VOLTAGE:
        return "internal_cell03_voltage";    
        break;
      case INTERNAL_CELL04_VOLTAGE:
        return "internal_cell04_voltage";    
        break;
      case INTERNAL_CELL05_VOLTAGE:
        return "internal_cell05_voltage";    
        break;
      case INTERNAL_CELL06_VOLTAGE:
        return "internal_cell06_voltage";    
        break;
      case INTERNAL_CELL07_VOLTAGE:
        return "internal_cell07_voltage";    
        break;
      case INTERNAL_CELL08_VOLTAGE:
        return "internal_cell08_voltage";    
        break;
      case INTERNAL_CELL09_VOLTAGE:
        return "internal_cell09_voltage";    
        break;
      case INTERNAL_CELL10_VOLTAGE:
        return "internal_cell10_voltage";    
        break;
      case INTERNAL_CELL11_VOLTAGE:
        return "internal_cell11_voltage";    
        break;
      case INTERNAL_CELL12_VOLTAGE:
        return "internal_cell12_voltage";    
        break;
      case INTERNAL_CELL13_VOLTAGE:
        return "internal_cell13_voltage";    
        break;
      case INTERNAL_CELL14_VOLTAGE:
        return "internal_cell14_voltage";    
        break;
      case INTERNAL_CELL15_VOLTAGE:
        return "internal_cell15_voltage";    
        break;
      case INTERNAL_CELL16_VOLTAGE:
        return "internal_cell16_voltage";    
        break;     
      case LED_MODE:
        return "led_mode";
        break;
      case POWER_OFF:
        return "power_off";
        break;
      case ECO_ON:
        return "eco_on";
        break;
      case ECO_SHUTDOWN:
        return "eco_shutdown";
        break;
      case CHARGING_MODE:
        return "charging_mode";
        break;
      case POWER_LIFTING_ON:
        return "power_lifting_on";
        break;
      case AC_INPUT_POWER_MAX:
        return "ac_input_power_max";
        break;
      case AC_INPUT_CURRENT_MAX:
        return "ac_input_current_max";
        break;
      case AC_OUTPUT_POWER_MAX:
        return "ac_output_power_max";
        break;
      case AC_OUTPUT_CURRENT_MAX:
        return "ac_output_current_max";
        break;
      case BATTERY_MIN_PERCENTAGE:
        return "battery_min_percentage";
        break;
      case AC_CHARGE_MAX_PERCENTAGE:
        return "ac_charge_max_percentage";
        break;
      default:
        #ifdef DEBUG
          Serial.println(F("Info 'map_field_name' found unknown field!"));
        #endif
        return "unknown";
        break;
   }
  
}


uint16_t CPayloadParser::parse_uint_field(uint8_t data[]) {
  return ((uint16_t)data[0] << 8) | (uint16_t)data[1];
}

bool CPayloadParser::parse_bool_field(uint8_t data[]) {
  return (data[1]) == 1;
}

float CPayloadParser::parse_decimal_field(uint8_t data[], uint8_t scale) {
  uint16_t raw_value = ((uint16_t)data[0] << 8) | (uint16_t)data[1];
  return (raw_value) / pow(10, scale);
}

float CPayloadParser::parse_version_field(uint8_t data[]) {

  uint16_t low = ((uint16_t)data[0] << 8) | (uint16_t)data[1];
  uint16_t high = ((uint16_t)data[2] << 8) | (uint16_t)data[3];
  long val = (low) | (high << 16);

  return (float)val / 100;
}

uint64_t CPayloadParser::parse_serial_field(uint8_t data[]) {

  uint16_t val1 = ((uint16_t)data[0] << 8) | (uint16_t)data[1];
  uint16_t val2 = ((uint16_t)data[2] << 8) | (uint16_t)data[3];
  uint16_t val3 = ((uint16_t)data[4] << 8) | (uint16_t)data[5];
  uint16_t val4 = ((uint16_t)data[6] << 8) | (uint16_t)data[7];

  uint64_t sn = ((((uint64_t)val1) | ((uint64_t)val2 << 16)) | ((uint64_t)val3 << 32)) | ((uint64_t)val4 << 48);

  return sn;
}

String CPayloadParser::parse_string_field(uint8_t data[]) {
  return String((char*)data);
}

// not implemented yet, leads to nothing
String CPayloadParser::parse_enum_field(uint8_t data[]){
    return "";
}

void CPayloadParser::parse_bluetooth_data(bluetti_command_t &bluettiCommand, uint8_t page, uint8_t offset, 
																					uint8_t* pData, size_t length){
  char Byte_In_Hex_offset[3];
  char Byte_In_Hex_page[3];
  sprintf(Byte_In_Hex_offset, "%x", offset);
  sprintf(Byte_In_Hex_page, "%x", page);
  
    switch(pData[1]){
      // range request

    case 0x03:

      for (int i = 0; i < bluettiCommand.so_b_d_s / sizeof(device_field_data_t); i++) {


            // filter fields not in range, reworked by https://github.com/AlexBurghardt
            // the original code didn't work completely and skipped some fields to be published
            if(
              // it's the correct page
              bluettiCommand.bluetti_device_state[i].f_page == page && 
              // data offset greater than or equal to page offset
              bluettiCommand.bluetti_device_state[i].f_offset >= offset &&
              // local offset does not exceed the page length, likely not needed because of the last condition check
              ((2* ((int)bluettiCommand.bluetti_device_state[i].f_offset - (int)offset)) + HEADER_SIZE) <= length &&
              // local offset + data size do not exceed the page length
              ((2* ((int)bluettiCommand.bluetti_device_state[i].f_offset - (int)offset 
              		+ bluettiCommand.bluetti_device_state[i].f_size)) + HEADER_SIZE) <= length
            ){
                uint8_t data_start = (2* ((int)bluettiCommand.bluetti_device_state[i].f_offset - (int)offset)) + HEADER_SIZE;
                uint8_t data_end = (data_start + 2 * bluettiCommand.bluetti_device_state[i].f_size);
                uint8_t data_payload_field[data_end - data_start];
                
                int p_index = 0;
                for (int i=data_start; i<= data_end; i++){
                      data_payload_field[p_index] = pData[i-1];
                      p_index++;
                }

								String topic = map_field_name(bluettiCommand.bluetti_device_state[i].f_name);
#ifdef DEBUG
  							// nee zuviel : Serial.println("have information for field: " + topic  );
#endif 


                switch (bluettiCommand.bluetti_device_state[i].f_type){
                 
                  case UINT_FIELD:
                    notifyCallback(topic, String(parse_uint_field(data_payload_field)));
                    break;
    
                  case BOOL_FIELD:
                    notifyCallback(topic, String((int)parse_bool_field(data_payload_field)));
                    break;
    
                  case DECIMAL_FIELD:
                    notifyCallback(topic, String(parse_decimal_field(data_payload_field, bluettiCommand.bluetti_device_state[i].f_scale ), 2) );
                    break;
    
                  case SN_FIELD:  
                    char sn[16];
                    sprintf(sn, "%lld", parse_serial_field(data_payload_field));
                    notifyCallback(topic, String(sn));
                    break;
    
                  case VERSION_FIELD:
                    notifyCallback(topic, String(parse_version_field(data_payload_field),2) );    
                    break;

                  case STRING_FIELD:
                    notifyCallback(topic, parse_string_field(data_payload_field));
                    break;
                  // doesn't work yet, not implemented further
                  case ENUM_FIELD:
                    notifyCallback(topic, parse_enum_field(data_payload_field));
                    break;
                  default:
                    break;
                  
                }
                
            }
            else{
              /* causes way too many messages, for debugging only
              //AddtoMsgView(String(millis()) + ": skip filtered field: "+ Byte_In_Hex_page + " offset: " + Byte_In_Hex_offset);
              */
            }
        }
        
        break; 
      case 0x06: //denke diese sind unnötig
        //AddtoMsgView(String(millis()) + ":skip 0x06 request! page: " + Byte_In_Hex_page + " offset: " + Byte_In_Hex_offset);
        break;
      default:
        //AddtoMsgView(String(millis()) + ":skip unknow request! page: " + Byte_In_Hex_page + " offset: " + Byte_In_Hex_offset);
        break;

    }
    
}
