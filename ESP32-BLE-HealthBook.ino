//Feito por Thalles Demétrio Ferreira Santos
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include "BLEDevice.h"
#include "BLEScan.h"
#include <BLE2902.h>


//------------CARACTERISTICA PARA RECEBER SINAL DA CINTA CARDIACA-------------------
#define bleServerName "HR40-40176044"
// The remote service we wish to connect to.
static BLEUUID serviceUUID("180D");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("2A37");


//------------CARACTERISTICA PARA MANDAR SINAL -------------------
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "6d68efe5-04b6-4a85-abc4-c2670b7bf7fd"
#define BOX_CHARACTERISTIC_UUID "f27b53ad-c63d-49a0-8c0f-9f297e6cc520"

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
BLECharacteristic* pCharacteristicas = NULL;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
uint16_t HRM; // Variavel para guardar o dado recebido
BLEServer* pServer = NULL;
char string[20]; // Variavel para enviar o sinal via bluetooth

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    HRM = pData[1];
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {      
    BLEClient*  pClient  = BLEDevice::createClient();

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      pClient->disconnect();
      return false;
    }
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      pClient->disconnect();
      return false;
    }
    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // We have found a device, let us now see if it contains the service we are looking for.
    if(advertisedDevice.getName() == bleServerName){
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  }
  }// onResult
}; // MyAdvertisedDeviceCallbacks

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    //Serial.println("Connected");
  };

  void onDisconnect(BLEServer *pServer)
  {
    //Serial.println("Disconnected");
  }
};


void setup() {
  Serial.begin(115200);
  BLEDevice::init("BLEExample"); //nome do bluetooth
  pServer = BLEDevice::createServer();

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristicas = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  pCharacteristicas ->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising, outros dispositvos enxergam o ble
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
} // End of setup.


// This is the Arduino main loop function.
void loop() {

  if (doConnect == true) {
    if (connectToServer()) {
      //Serial.println("We are now connected to the BLE Server.");
    } else {
      //Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
  if (connected) {
  
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }
  pCharacteristicas->setValue(itoa(HRM,string,10));
  pCharacteristicas->notify();
} // End of loop
