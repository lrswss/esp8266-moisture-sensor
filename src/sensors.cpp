/***************************************************************************
  
  Copyright (c) 2023 Lars Wessels

  This file a part of the "ESP8266 Moisture Sensor" source code.
  https://github.com/lrswss/esp8266-moisture-sensor

  Licensed under the MIT License. You may not use this file except in
  compliance with the License. You may obtain a copy of the License at

  https://opensource.org/licenses/MIT

***************************************************************************/

#include "config.h"
#include "sensors.h"
#include "wlan.h"
#include "mqtt.h"
#include "utils.h"


OneWire oneWire(ONEWIRE_PIN);
DallasTemperature ds1820(&oneWire);
static DeviceAddress deviceAddress;
sensorReadings_t sensors;


// initialize DS1820 one-wire temperature sensor
#ifdef ONEWIRE_PIN
void initDS1820() {
    ds1820.begin();
    if (ds1820.getAddress(deviceAddress, 0)) {
        Serial.print("Found sensor ");
        if (deviceAddress[0] == 0x10) {
            Serial.print(F("DS1820/DS18S20 (9bit)"));
        } else if (deviceAddress[0] == 0x28) {
            Serial.printf("DS18B20 (%d bit)", DS18B20_RESOLUTION);
            ds1820.setResolution(deviceAddress, DS18B20_RESOLUTION);
            ds1820.requestTemperatures(); // blocking
        }
        Serial.print(F(", address "));
        printHEX8bit(deviceAddress, sizeof(deviceAddress), true, false);
        delay(250);
    } else {
        Serial.println(F("No DS18X20 sensor not found"));
    }
}


void readTemp() {
    static float degC = 0.0;

    ds1820.requestTemperatures(); // blocking, wating for conversion
    degC = ds1820.getTempCByIndex(0); // only one sensor connected to one-wire bus
    if (degC != 85.0 && degC > -127) {
        Serial.print(F("Temperature: "));
        Serial.print(degC);
        Serial.println(F(" C"));
        sensors.temperature = int(degC * 10) / 10.0;
    } else {
        Serial.println(F("DS18X20: Failed to read temperature!"));
        sensors.temperature = -99;
    }
}
#endif


// read analog value from moisture sensor
void readMoisture() {
    uint16_t sensorVal = 0;

    digitalWrite(POWER_PIN, HIGH);
    longDelay(SENSOR_WARMUP_MS);
    for (uint8_t i = 0; i < 10; i++) {
        sensorVal += analogRead(SENSOR_PIN);
    }
    digitalWrite(POWER_PIN, LOW);
    sensors.moisture = int(sensorVal/10);
    Serial.print(F("Moisture reading: "));
    Serial.println(sensors.moisture);
}