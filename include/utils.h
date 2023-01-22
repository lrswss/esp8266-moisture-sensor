/***************************************************************************
  
  Copyright (c) 2023 Lars Wessels

  This file a part of the "ESP8266 Moisture Sensor" source code.
  https://github.com/lrswss/esp8266-moisture-sensor

  Licensed under the MIT License. You may not use this file except in
  compliance with the License. You may obtain a copy of the License at

  https://opensource.org/licenses/MIT

***************************************************************************/

#ifndef _UTILS_H
#define _UTILS_H

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "config.h"
extern "C" {
	#include "user_interface.h"  // esp8266 rtc memory
}

typedef struct {
  uint16_t wakeup_counter = 0;
  uint16_t reading_interval_sec = SENSOR_INTERVAL_SECS;
  uint16_t deep_sleep_enabled = 0;
} settings_t;

extern settings_t settings;

void update_firmware(uint8_t version);
void deepSleep(uint16_t duration);
void longDelay(uint16_t delayMs);
void printHEX8bit(uint8_t *arr, uint8_t len, bool ln, bool reverse);

#endif