/***************************************************************************
  
  Copyright (c) 2023 Lars Wessels

  This file a part of the "ESP8266 Moisture Sensor" source code.
  https://github.com/lrswss/esp8266-moisture-sensor

  Licensed under the MIT License. You may not use this file except in
  compliance with the License. You may obtain a copy of the License at

  https://opensource.org/licenses/MIT

***************************************************************************/

#ifndef _CONFIG_H
#define _CONFIG_H

#define WIFI_SSID  "<Your-SSID>"
#define WIFI_PASS  "<SSID-Password>"

#define MQTT_BROKER  "<ip-address>"
#define MQTT_BASE_TOPIC  "moisture"

// webserver to server firmware updates
#define OTA_UPDATE_HOST   "<ip-address>"
#define OTA_UPDATE_PATH   "/update"

#define SENSOR_INTERVAL_SECS  300  // interval to read moisture sensor
#define SENSOR_INTERVAL_MIN_SECS  60  // lower bound for setting interval with mqtt
#define SENSOR_WARMUP_MS  2000  // time to power sensor before reading it's analog ouput

#define USE_STATIC_IP   // ip, gw, etc. is set in wlan.h
#define USE_DEEP_SLEEP  // required for battery powered device

#define SENSOR_PIN A0   // connected to analog sensor output AO
#define POWER_PIN 4     // to turn analog sensor on/off
#define ONEWIRE_PIN 5  // DS1820 temperature sensor

#endif
