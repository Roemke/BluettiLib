#include "CPayloadParser.h"
#include "MapFuncs.h"

//ganz werde ich den String nicht los
CPayloadParser::CPayloadParser(void (*nc)(char *, String) )
{
	notifyCallback = nc;
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
                uint8_t data_payload_field[data_end - data_start]; //hmm, ist mir unklar, kann doch kein dynamisches Array anlegen
                //oder ist uint8_t ein const, nein das ist doch nicht sinnvoll - strange, ginge doch nur, wenn der operator überladen ist
                //scheint der Fall zu sein, anders ist es nicht sinnvoll? in der operator [] geschichte findet man einen assert,
                //evtl. wird so geprüft, dass nicht zu viel Platz verwendet wird. - nein, es ist ein Zugriffsoperator 
                //alos müssen die Daten hier konstant sein ? Für ein Device wäre das m.E. die einzig sinnvolle erklärung
                int p_index = 0;
                for (int i=data_start; i<= data_end; i++){
                      data_payload_field[p_index] = pData[i-1];
                      p_index++;
                }

								char * topic = MapFuncs::map_field_name(bluettiCommand.bluetti_device_state[i].f_name);
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
                  
                  case ENUM_FIELD:// doesn't work yet, not implemented further
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
