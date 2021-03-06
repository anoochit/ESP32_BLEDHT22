#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <iostream>
#include <string>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

#define DHTPIN 23
#define DHTTYPE DHT22

DHT_Unified dht(DHTPIN, DHTTYPE);

float humidity;
float temperature;

uint32_t delayMS;

#define SERVICE_UUID  "95298e6a-6dbc-11ea-93d4-63607a02728e"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


void setup() {
  Serial.begin(115200);

  dht.begin();
  sensor_t sensor;
  delayMS = sensor.min_delay / 1000;

  BLEDevice::init("BLE-DHT22");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  //BLEService *pService = pServer->createService(SERVICE_UUID);
  BLEService *pService = pServer->createService((uint16_t)0x181A);

  pCharacteristic = pService->createCharacteristic(
                      (uint16_t)0x181A,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client to connect...");
}

void loop() {

  // connected
  if (deviceConnected) {
    delay(delayMS);

    // Get temperature event and print its value.
    sensors_event_t event;

    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println(F("Error reading data!"));
    }
    else {
      temperature = event.temperature;
      //    Serial.print(F("Temperature: "));
      //    Serial.print(event.temperature);
      //    Serial.println(F("°C"));
    }

    dht.humidity().getEvent(&event);
    if ( isnan(event.relative_humidity)) {
      Serial.println(F("Error reading data!"));
    }
    else {
      humidity = event.relative_humidity;
      //    Serial.print(F("Humidity: "));
      //    Serial.print(event.relative_humidity);
      //    Serial.println(F("%"));
    }

    char humidityString[2];
    char temperatureString[2];
    dtostrf(humidity, 1, 2, humidityString);
    dtostrf(temperature, 1, 2, temperatureString);

    char dhtDataString[12];
    sprintf(dhtDataString, "%.02f,%.02f", temperature, humidity);

    pCharacteristic->setValue(dhtDataString);

    pCharacteristic->notify();
    Serial.print("*** Data String: ");
    Serial.print(dhtDataString);
    Serial.println(" ***");
  }

//  // disconnecting
//  if (!deviceConnected && oldDeviceConnected) {
//    delay(500); // give the bluetooth stack the chance to get things ready
//    pServer->startAdvertising(); // restart advertising
//    Serial.println("start advertising");
//    oldDeviceConnected = deviceConnected;
//  }
//  // connecting
//  if (deviceConnected && !oldDeviceConnected) {
//    // do stuff here on connecting
//    oldDeviceConnected = deviceConnected;
//  }

}
