#ifndef BLUETTI_H
#define BLUETTI_H

#include "Arduino.h"
#include "BluettiConfig.h"
#include "NimBLEDevice.h"
#include "typedefs.h"
#include "CPayloadParser.h"
#include "utils.h"

//#define DEBUG 1
/*
	Bin nicht ganz glücklich damit, sehe aber momentan keine Lösung 
	die Callback-Methode muss eine Statische sein, sonst passt die Signatur nicht
	damit hat sie aber auch keinen Zugriff auf nicht statische eigenschaften
	ble -> notifyIntern(signatur vorgeben) -> parser-Methode (muss statisch sein) -> call auf die eigentliche Callback (kann statisch sein)
*/

class CPayloadParser; //forward decl. 

//Beispiel-Projekt ist esp32ServoRelaisBluetti

class Bluetti //model vorher ueber defines festlegen
{
	public:
		Bluetti(char * bluetoothId, bluetti_command_t &bluettiCommand 
															,	void (*nc)(const char *, String ) //der callback 
															,	int maxDisconnectedTimeUntilReboot=10    //device will reboot when wlan/BT/MQTT is not connectet within x Seconds
                              , int bluetoothQueryMessageDelay = 3000);  //no reboot just message 
		~Bluetti()
		{
			//delete [] this->bluetoothId;
		}
		
		void initBluetooth();
		void handleBluetooth();
		//type ac_output_on oder dc_output_on, cmd on oder off
		//die anderen types gehen wahrscheinlich auch, brauch ich aber nicht
		void switchOut(char * type,  const char * cmd); 
		
		
  public: 
  	static BLEAdvertisedDevice* bluettiDevice;
  	static BLEUUID serviceUUID; //("0000ff00-0000-1000-8000-00805f9b34fb"); //stand in btooth.h  - gehoert zu Bluetti, typ sicher in bluetooth lib
 		static BLEUUID NOTIFY_UUID; //("0000ff01-0000-1000-8000-00805f9b34fb");
 		static BLEUUID WRITE_UUID;
 		static char bluetti_device_id[40];// = "Bluetti Blutetooth Id";
 		static bool doConnect;
 		static bool connected;
 		static bool doScan; 
 		
	private:
		//char * bluetoothId = 0;
		int maxDisconnectedTimeUntilReboot;
		int BLUETOOTH_QUERY_MESSAGE_DELAY;
		static QueueHandle_t commandHandleQueue;
		static QueueHandle_t sendQueue;
		static CPayloadParser parser;
		bool connectToServer();
		void handleBTCommandQueue();
		void sendBTCommand(bt_command_t command);
	  static void notifyCallbackIntern(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length,  bool isNotify);
		//extern void publishTopic(enum field_names field_name, String value);

		unsigned long lastBTMessage = 0;
		int pollTick = 0; 
		static bluetti_command_t bluettiCommand;

		static BLERemoteCharacteristic* pRemoteWriteCharacteristic;
		static BLERemoteCharacteristic* pRemoteNotifyCharacteristic;

};



/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class BluettiAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice *advertisedDevice); 
};



//nur onConnect und onDisconnect?
class MyClientCallback : public BLEClientCallbacks  
{
  public:
  	MyClientCallback(Bluetti * b) : BLEClientCallbacks()
  	{
  		blue = b;
  	}
  	void onConnect(BLEClient* pclient) {}
  	void onDisconnect(BLEClient* pclient) 
  	{
    	blue->connected = false;
    	Serial.println(F("onDisconnect"));
  	}
  	
  	Bluetti *blue;
};

#endif
