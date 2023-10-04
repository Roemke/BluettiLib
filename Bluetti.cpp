#include "Bluetti.h"
#include "BluettiConfig.h"


//alle statischen müssen  angelegt werden 
BLEUUID Bluetti::serviceUUID("0000ff00-0000-1000-8000-00805f9b34fb"); //stand in btooth.h  - gehoert zu Bluetti, typ sicher in bluetooth lib
BLEUUID Bluetti::WRITE_UUID ("0000ff02-0000-1000-8000-00805f9b34fb"); //so, das gleiche
BLEUUID Bluetti::NOTIFY_UUID("0000ff01-0000-1000-8000-00805f9b34fb");

char Bluetti::bluetti_device_id[40];// = "Bluetti Blutetooth Id"; //scheint nicht zu gehen ...

bool Bluetti::doConnect = false;
bool Bluetti::doScan = false;
bool Bluetti::connected = false;

BLEAdvertisedDevice* Bluetti::bluettiDevice;
QueueHandle_t Bluetti::commandHandleQueue;
QueueHandle_t Bluetti::sendQueue;

bluetti_command_t Bluetti::bluettiCommand;
BLERemoteCharacteristic* Bluetti::pRemoteWriteCharacteristic;
BLERemoteCharacteristic* Bluetti::pRemoteNotifyCharacteristic;
//CPayloadParser Bluetti::parser(Bluetti::notifyCallback);//nee, so nicht, der notifyCallback ist nicht definiert
CPayloadParser Bluetti::parser(0);

Bluetti::Bluetti(char * bluetoothId, bluetti_command_t &bluettiCommand 
															,	void (*nc)(String, String) //der callback 
															,	int maxDisconnectedTimeUntilReboot    //device will reboot when wlan/BT/MQTT is not connectet within x Minutes
                              , int bluetoothQueryMessageDelay )  
{
	//this->bluetoothId = new char[strlen(bluetoothId) + 1];
	this->maxDisconnectedTimeUntilReboot = maxDisconnectedTimeUntilReboot;
	BLUETOOTH_QUERY_MESSAGE_DELAY = bluetoothQueryMessageDelay;
	//strcpy (this->bluetoothId,bluetoothId);
	strcpy(Bluetti::bluetti_device_id,bluetoothId); //das geht, ist aber doof, da ein konstruktor nicht dafür genutzt werden sollte 
	                                                            //statische eigenschaften zu initialisieren
	Bluetti::bluettiCommand = bluettiCommand;
	Bluetti::parser.notifyCallback  = nc; 
	
}
void Bluetti::initBluetooth()
{
	#ifdef DEBUG
	Serial.println("in initBluethooth()");
	#endif
	
	BLEDevice::init("");
	BLEScan* pBLEScan = BLEDevice::getScan();
	pBLEScan->setAdvertisedDeviceCallbacks(new BluettiAdvertisedDeviceCallbacks()); //wer gibt dass denn wieder frei?
	pBLEScan->setInterval(1349);
	pBLEScan->setWindow(449);
	pBLEScan->setActiveScan(true);
	pBLEScan->start(5, false);
	Bluetti::commandHandleQueue = xQueueCreate( 5, sizeof(bt_command_t ) );
	sendQueue = xQueueCreate( 5, sizeof(bt_command_t) );
}


void Bluetti::sendBTCommand(bt_command_t command)
{
    bt_command_t cmd = command;
    xQueueSend(sendQueue, &cmd, 0);
}
//void (* Bluetti::notifyCallback)(String , String);  //ah ja, so geht es, brauchen wir aber nicht
//void Bluetti::notifyCallback(String, String); //waere die vereinfachte syntax, geht auf jeden Fall nicht
 
void Bluetti::handleBluetooth(){
	#ifdef DEBUG
  static bool fc = true; 
  if (fc)
  {
  	fc = false;
  	Serial.println("first time in hande bluetooth"); 
  	if (doConnect)
  		Serial.println("try to connect");
  	else 
  		Serial.println("do connect is false");
  }
  #endif
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println(F("We are now connected to the Bluetti BLE Server."));
    } else {
      Serial.println(F("We have failed to connect to the server; there is nothing more we will do."));
    }
    doConnect = false;
  }

  if ((millis() - lastBTMessage) > (maxDisconnectedTimeUntilReboot * 60000)){ 
    Serial.println(F("BT is disconnected over allowed limit, reboot device"));
    #ifdef SLEEP_TIME_ON_BT_NOT_AVAIL
        esp_deep_sleep_start();
    #else
        ESP.restart();
    #endif
  }

  if (connected) {
    // poll for device state
    if ( millis() - lastBTMessage > BLUETOOTH_QUERY_MESSAGE_DELAY){

       bt_command_t command;
       command.prefix = 0x01;
       command.field_update_cmd = 0x03;
       command.page = bluettiCommand.bluetti_polling_command[pollTick].f_page;
       command.offset = bluettiCommand.bluetti_polling_command[pollTick].f_offset;
       command.len = (uint16_t) bluettiCommand.bluetti_polling_command[pollTick].f_size << 8;
       command.check_sum = modbus_crc((uint8_t*)&command,6);

       xQueueSend(commandHandleQueue, &command, portMAX_DELAY);
       xQueueSend(sendQueue, &command, portMAX_DELAY);

       if (pollTick == bluettiCommand.so_b_p_c/sizeof(device_field_data_t)-1 ){
           pollTick = 0;
       } else {
           pollTick++;
       }
            
      lastBTMessage = millis();
    }

    handleBTCommandQueue();
    
  }else if(doScan){
    BLEDevice::getScan()->start(0);  
  }
}



void Bluetti::switchOut(String type,String cmd) 
{
	  bt_command_t command;
	  command.prefix = 0x01;
  	command.field_update_cmd = 0x06;
	  
	  //String type = "ac_output_on";
		for (int i = 0; i< bluettiCommand.so_b_d_c/sizeof(device_field_data_t); i++)
		{
			if (map_field_name(bluettiCommand.bluetti_device_command[i].f_name) == type)
			{ //element gefunden
				command.page = bluettiCommand.bluetti_device_command[i].f_page;
        command.offset = bluettiCommand.bluetti_device_command[i].f_offset;
        cmd = map_command_value(type,cmd);
        Serial.print("Found command on "); Serial.println(type+" command is " + cmd);
        break;
			}
		}
		command.len = swap_bytes(cmd.toInt());
  	command.check_sum = modbus_crc((uint8_t*)&command,6);
  
  	sendBTCommand(command);
}

void Bluetti::handleBTCommandQueue()
{
    bt_command_t command;
    if(xQueueReceive(sendQueue, &command, 0)) {
      
#ifdef DEBUG
    Serial.print("Write Request FF02 - Value: ");
    
    for(int i=0; i<8; i++){
       if ( i % 2 == 0){ Serial.print(" "); };
       Serial.printf("%02x", ((uint8_t*)&command)[i]);
    }
    
    Serial.println("");
#endif
      pRemoteWriteCharacteristic->writeValue((uint8_t*)&command, sizeof(command),true);
 
     };  
}
bool Bluetti::connectToServer() {
    Serial.print(F("Forming a connection to "));
    Serial.println(bluettiDevice->getAddress().toString().c_str());

    BLEDevice::setMTU(517); // set client to request maximum MTU from server (default is 23 otherwise)
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(F(" - Created client"));

    pClient->setClientCallbacks(new MyClientCallback(this));

    // Connect to the remove BLE Server.
    pClient->connect(bluettiDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(F(" - Connected to server"));
    // pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
  
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print(F("Failed to find our service UUID: "));
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(F(" - Found our service"));


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteWriteCharacteristic = pRemoteService->getCharacteristic(WRITE_UUID);
    if (pRemoteWriteCharacteristic == nullptr) {
      Serial.print(F("Failed to find our characteristic UUID: "));
      Serial.println(WRITE_UUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(F(" - Found our Write characteristic"));

        // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteNotifyCharacteristic = pRemoteService->getCharacteristic(NOTIFY_UUID);
    if (pRemoteNotifyCharacteristic == nullptr) {
      Serial.print(F("Failed to find our characteristic UUID: "));
      Serial.println(NOTIFY_UUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(F(" - Found our Notifyite characteristic"));

    // Read the value of the characteristic.
    if(pRemoteWriteCharacteristic->canRead()) {
      std::string value = pRemoteWriteCharacteristic->readValue();
      Serial.print(F("The characteristic value was: "));
      Serial.println(value.c_str());
    }

    if(pRemoteNotifyCharacteristic->canNotify())
      pRemoteNotifyCharacteristic->registerForNotify(Bluetti::notifyCallbackIntern);

    connected = true;

    return true;
}

//war statisch in BTooth.cpp - also nur in dem Modul sichtbar
void Bluetti::notifyCallbackIntern(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {

#ifdef DEBUG
    Serial.println("F01 - Write Response");
    /* pData Debug... */
    for (int i=1; i<=length; i++){
       
      Serial.printf("%02x", pData[i-1]);
      
      if(i % 2 == 0){
        Serial.print(" "); 
      } 

      if(i % 16 == 0){
        Serial.println();  
      }
    }
    Serial.println();
#endif

    bt_command_t command_handle;
    if(xQueueReceive(commandHandleQueue, &command_handle, 500)){
      parser.parse_bluetooth_data(	Bluetti::bluettiCommand , command_handle.page, command_handle.offset, pData, length);
    }
   
}

//map-Funktionen 
String Bluetti::map_field_name(enum field_names f_name)
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
//There is no reflection to do string to enum
//There are a couple of ways to work aroung it... but basically are just "case" statements
//Wapped them in a fuction
String Bluetti::map_command_value(String command_name, String value)
{
  String toRet = value;
  value.toUpperCase();
  command_name.toUpperCase(); //force case indipendence

  //on / off commands
  if(command_name == "POWER_OFF" || command_name == "AC_OUTPUT_ON" || command_name == "DC_OUTPUT_ON" || command_name == "ECO_ON" || command_name == "POWER_LIFTING_ON") {
    if (value == "ON") {
      toRet = "1";
    }
    if (value == "OFF") {
      toRet = "0";
    }
  }

  //See DEVICE_EB3A enums
  if(command_name == "LED_MODE"){
    if (value == "LED_LOW") {
      toRet = "1";
    }
    if (value == "LED_HIGH") {
      toRet = "2";
    }
    if (value == "LED_SOS") {
      toRet = "3";
    }
    if (value == "LED_OFF") {
      toRet = "4";
    }
  }

  //See DEVICE_EB3A enums
  if(command_name == "ECO_SHUTDOWN"){
    if (value == "ONE_HOUR") {
      toRet = "1";
    }
    if (value == "TWO_HOURS") {
      toRet = "2";
    }
    if (value == "THREE_HOURS") {
      toRet = "3";
    }
    if (value == "FOUR_HOURS") {
      toRet = "4";
    }
  }

  //See DEVICE_EB3A enums
  if(command_name == "CHARGING_MODE"){
    if (value == "STANDARD") {
      toRet = "0";
    }
    if (value == "SILENT") {
      toRet = "1";
    }
    if (value == "TURBO") {
      toRet = "2";
    }
  }


  return toRet;
}

//--------------------------------------------------------------

  void BluettiAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice *advertisedDevice) {
    #ifdef DEBUG
    Serial.println("In BLEAdvertised Callback");
    #endif
    Serial.print(F("BLE Advertised Device found: "));
    Serial.println(advertisedDevice->toString().c_str());
	  
    // We have found a device, let us now see if it contains the service we are looking for.
    
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(Bluetti::serviceUUID) 
    	&& (strcmp(advertisedDevice->getName().c_str(),Bluetti::bluetti_device_id)==0) ) {
      BLEDevice::getScan()->stop();
      Bluetti::bluettiDevice = advertisedDevice;
      Bluetti::doConnect = true;
      Bluetti::doScan = true;
      #ifdef DEBUG
      Serial.println("Device wird genutzt");
      #endif
    }
    else
    {
      #ifdef DEBUG
      if (advertisedDevice->haveServiceUUID())
      	Serial.println("Device has Service UUID");
      else 
      	Serial.println("No Service UUID");
      	
    	Serial.println("Device kann nicht genutzt werden");
    	Serial.print("We have advertised device ");
    	Serial.print(advertisedDevice->getName().c_str());
    	Serial.print(" and Bluetti ");
    	Serial.println(Bluetti::bluetti_device_id);
    	#endif 
  	}
  } 



