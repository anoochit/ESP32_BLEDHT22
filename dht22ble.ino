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

//#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


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

  BLEService *pService = pServer->createService((uint16_t)0x181A);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client to connect...");
}

void loop() {


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

    char dhtDataString[16];
    sprintf(dhtDataString, "%.02f,%.02f", temperature, humidity);

    pCharacteristic->setValue(dhtDataString);

    pCharacteristic->notify();
    Serial.print("*** Data String: ");
    Serial.print(dhtDataString);
    Serial.println(" ***");
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }

}
