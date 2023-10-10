#include "Bluetti.h"
#include "BluettiConfig.h"
#include "MapFuncs.h"

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
															,	void (*nc)(const char *, String ) //der callback 
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
	
	BLEDevice::init(""); // wie in doku
	BLEScan* pBLEScan = BLEDevice::getScan(); //ebenfalls, scannen
	//folgendes scanned immer und setzt 
	pBLEScan->setAdvertisedDeviceCallbacks(new BluettiAdvertisedDeviceCallbacks()); //wer gibt dass denn wieder frei?
	pBLEScan->setInterval(80);//1349); // How often the scan occurs / switches channels; in milliseconds
	pBLEScan->setWindow(0x10);//449); // How long to scan during the interval; in milliseconds.
	pBLEScan->setActiveScan(true);// Set active scanning, this will get more data from the advertiser.
	pBLEScan->start(5, false); //5 sekunden scannen?
	Bluetti::commandHandleQueue = xQueueCreate( 5, sizeof(bt_command_t ) );
	Bluetti::sendQueue = xQueueCreate( 5, sizeof(bt_command_t) );
}


void Bluetti::sendBTCommand(bt_command_t command)
{
    bt_command_t cmd = command;
    xQueueSend(Bluetti::sendQueue, &cmd, 0);
}
//void (* Bluetti::notifyCallback)(String , String);  //ah ja, so geht es, brauchen wir aber nicht
//void Bluetti::notifyCallback(String, String); //waere die vereinfachte syntax, geht auf jeden Fall nicht
 
void Bluetti::handleBluetooth()
{
	#ifdef DEBUG	
  static bool fc = true; 
  if (fc)
  {
  	fc = false;
  	Serial.println("first time in hande bluetooth"); 
  	if (Bluetti::doConnect)
  		Serial.println("try to connect");
  	else 
  		Serial.println("do connect is false");
  }
  #endif
  if (Bluetti::doConnect) 
  {
    if (connectToServer()) 
    {
      Serial.println(F("We are now connected to the Bluetti BLE Server."));
    } else 
    {
      Serial.println(F("We have failed to connect to the server; there is nothing more we will do."));
    }
    Bluetti::doConnect = false;
  }


  if ((millis() - lastBTMessage) > (maxDisconnectedTimeUntilReboot * 1000 )){ //60000)){ 
    Serial.println(F("BT is disconnected over allowed limit, dont reboot device"));
//    #ifdef SLEEP_TIME_ON_BT_NOT_AVAIL
//        esp_deep_sleep_start();
//    #else
//        ESP.restart();
//    #endif
  }

  if (Bluetti::connected) {
    // poll for device state
    if ( millis() - lastBTMessage > BLUETOOTH_QUERY_MESSAGE_DELAY){
       #ifdef DEBUG
         Serial.println("Try to send polling command");
       #endif
       bt_command_t command;
       command.prefix = 0x01;
       command.field_update_cmd = 0x03;
       command.page = bluettiCommand.bluetti_polling_command[pollTick].f_page;
       command.offset = bluettiCommand.bluetti_polling_command[pollTick].f_offset;
       command.len = (uint16_t) bluettiCommand.bluetti_polling_command[pollTick].f_size << 8;
       command.check_sum = modbus_crc((uint8_t*)&command,6);


       xQueueSend(Bluetti::commandHandleQueue, &command, portMAX_DELAY);
       xQueueSend(Bluetti::sendQueue, &command, portMAX_DELAY);

       if (pollTick == bluettiCommand.so_b_p_c/sizeof(device_field_data_t)-1 ){
           pollTick = 0;
       } else {
           pollTick++;
       }
      lastBTMessage = millis();
    }
     
    handleBTCommandQueue();
    
  }
  else if(Bluetti::doScan)
  {
  	Bluetti::doScan = false; //nur einmal starten
  	Serial.println("BLE: Start scan again");
    BLEDevice::getScan()->start(0);  
  }
  else 
  {
  	Serial.println("BLE no connect, no scan, das sollte nicht sein ");
  }
}



void Bluetti::switchOut(char * type, const char * cmd) 
{
	  bt_command_t command;
	  command.prefix = 0x01;
  	command.field_update_cmd = 0x06;
  	uint16_t cmdInt;
	  
	  //String type = "ac_output_on";
		for (int i = 0; i< bluettiCommand.so_b_d_c/sizeof(device_field_data_t); i++)
		{
			if (!strcasecmp(MapFuncs::map_field_name(bluettiCommand.bluetti_device_command[i].f_name) , type))
			{ //element gefunden
				command.page = bluettiCommand.bluetti_device_command[i].f_page;
        command.offset = bluettiCommand.bluetti_device_command[i].f_offset;
        cmdInt = MapFuncs::map_command_value(type,(char *) cmd);
        Serial.print("Found command on "); Serial.print(type);Serial.print(" command is ");Serial.println(cmdInt);
        break;
			}
		}
		command.len = swap_bytes(cmdInt);
  	command.check_sum = modbus_crc((uint8_t*)&command,6);
  
  	sendBTCommand(command);
}

void Bluetti::handleBTCommandQueue()
{
    bt_command_t command;
    if(xQueueReceive(Bluetti::sendQueue, &command, 0)) {
      
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
    //toString liefert const - keinen Einfluss auf den typ
    Serial.println(bluettiDevice->getAddress().toString().c_str());

    BLEDevice::setMTU(512); // set client to request maximum MTU from server (default is 23 otherwise)
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
      Serial.println(serviceUUID.toString().c_str()); //auch kein Einfluss, duerfte auch static sein 
      pClient->disconnect();
      return false;
    }
    Serial.println(F(" - Found our service"));


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteWriteCharacteristic = pRemoteService->getCharacteristic(WRITE_UUID);
    if (pRemoteWriteCharacteristic == nullptr) {
      Serial.print(F("Failed to find our characteristic UUID: "));
      Serial.println(WRITE_UUID.toString().c_str());//s.o.
      pClient->disconnect();
      return false;
    }
    Serial.println(F(" - Found our Write characteristic"));

        // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteNotifyCharacteristic = pRemoteService->getCharacteristic(NOTIFY_UUID);
    if (pRemoteNotifyCharacteristic == nullptr) {
      Serial.print(F("Failed to find our characteristic UUID: "));
      Serial.println(NOTIFY_UUID.toString().c_str());//s.o.
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
    {
      pRemoteNotifyCharacteristic->subscribe(true,Bluetti::notifyCallbackIntern);
      Serial.println("Have subscribed to notify");
		}
    Bluetti::connected = true;

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
    if(xQueueReceive(Bluetti::commandHandleQueue, &command_handle, 500)){
      parser.parse_bluetooth_data(	Bluetti::bluettiCommand , command_handle.page, command_handle.offset, pData, length);
    }
   
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
    	&& (strcmp(advertisedDevice->getName().c_str(),Bluetti::bluetti_device_id)==0) ) 
    {
      BLEDevice::getScan()->stop();
      Bluetti::bluettiDevice = advertisedDevice;
      Bluetti::doConnect = true;
      Bluetti::doScan = true; //wenn die verbindung abbricht, kann wieder gescannt werden
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
      	
    	Serial.println("Device kann nicht genutzt werden: ");
    	Serial.print(advertisedDevice->getName().c_str());
    	Serial.print(" Bluetti has: ");
    	Serial.println(Bluetti::bluetti_device_id);
    	#endif 
  	}
  } 



