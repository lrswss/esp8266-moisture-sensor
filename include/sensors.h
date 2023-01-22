/***************************************************************************
  
  Copyright (c) 2023 Lars Wessels

  This file a part of the "ESP8266 Moisture Sensor" source code.
  https://github.com/lrswss/esp8266-moisture-sensor

  Licensed under the MIT License. You may not use this file except in
  compliance with the License. You may obtain a copy of the License at

  https://opensource.org/licenses/MIT

***************************************************************************/

#ifndef _SENSORS_H
#define _SENSORS_H

#include <OneWire.h>
#include <DallasTemperature.h>

// DS18B20 conversion time vs. LSB (°C)
// 9bit/0.5/93.75ms, 10bit/0.25/187.5ms, 11bit/0.125/375ms, 12bit/0.0625/750ms
#define DS18B20_RESOLUTION 11


typedef struct  {
    uint16_t moisture;
    float temperature;
} sensorReadings_t;

extern sensorReadings_t sensors;

void initDS1820();
void readTemp();
void readMoisture();

#endif