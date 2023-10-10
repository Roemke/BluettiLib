#ifndef MAPFUNCS_H
#define MAPFUNCS_H

#include "DeviceType.h"
#include <strings.h>

class MapFuncs {
	public: 
		static const char * map_field_name(enum field_names f_name)
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
		static uint16_t map_command_value(char * command_name, char * value)
		{
			//String toRet = value;
			int toRet = 0; //off is default
			//value.toUpperCase();
			//command_name.toUpperCase(); //force case indipendence

			//on / off commands
			if( !strcasecmp(command_name ,"POWER_OFF") || !strcasecmp(command_name, "AC_OUTPUT_ON") || !strcasecmp(command_name ,"DC_OUTPUT_ON") || 
				!strcasecmp(command_name , "ECO_ON") || !strcasecmp(command_name, "POWER_LIFTING_ON")) {
				if (!strcasecmp(value , "ON")) {
				  toRet = 1;
				}
			}

			//See DEVICE_EB3A enums
			else if(!strcasecmp(command_name , "LED_MODE")){
				if (!strcasecmp(value , "LED_LOW")) {
				  toRet = 1;
				}
				else if (!strcasecmp(value , "LED_HIGH")) {
				  toRet = 2;
				}
				else if (!strcasecmp(value , "LED_SOS")) {
				  toRet = 3;
				}
				else if (!strcasecmp(value , "LED_OFF")) {
				  toRet = 4;
				}
			}

			//See DEVICE_EB3A enums
			else if(!strcasecmp(command_name ,"ECO_SHUTDOWN")){
				if (!strcasecmp(value , "ONE_HOUR")) {
				  toRet = 1;
				}
				else if (!strcasecmp(value ,"TWO_HOURS")) {
				  toRet = 2;
				}
				else if (!strcasecmp(value , "THREE_HOURS")) {
				  toRet = 3;
				}
				else if (!strcasecmp(value , "FOUR_HOURS")) {
				  toRet = 4;
				}
			}

			//See DEVICE_EB3A enums
			else if(!strcasecmp(command_name , "CHARGING_MODE")){
				//if (!strcasecmp(value , "STANDARD")) {
				//  toRet = "0";
				//}
				if (!strcasecmp(value ,"SILENT")) {
				  toRet = 1;
				}
				else if (!strcasecmp(value , "TURBO")) {
				  toRet = 2;
				}
			}
			return toRet;
		}

};
#endif
