/***************************************************************************
  
  Copyright (c) 2023 Lars Wessels

  This file a part of the "ESP8266 Moisture Sensor" source code.
  https://github.com/lrswss/esp8266-moisture-sensor

  Licensed under the MIT License. You may not use this file except in
  compliance with the License. You may obtain a copy of the License at

  https://opensource.org/licenses/MIT

***************************************************************************/

#include "config.h"
#include "wlan.h"
#include "utils.h"


// static ip address reduces initial wifi connect times
#ifdef USE_STATIC_IP
IPAddress localip(10, 1, 30, 140);
IPAddress gateway(10, 1, 30, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primarydns(10, 1, 30, 1);
#endif

// (re)connect to wifi
void wifi_connect() {
    char rssi[18];
    uint8_t retry_cnt = 0;

#ifdef USE_STATIC_IP
    if (!WiFi.config(localip, gateway, subnet, primarydns))
        Serial.println(F("Failed to configure WiFi STA mode!"));
#else
    WiFi.mode(WIFI_STA);
#endif
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(F("."));
        if (retry_cnt++ > 20) {
            Serial.println();
            Serial.print(F("WiFi connection failed..."));
            if (settings.deep_sleep_enabled) {
                Serial.println();
                deepSleep(settings.reading_interval_sec);
            } else {
                Serial.println(F("restarting!"));
                ESP.restart();
            }
        }
        longDelay(1000);
    }
    Serial.print(WiFi.localIP());
    sprintf(rssi, " (RSSI %d dBm)", WiFi.RSSI());
    Serial.println(rssi);
}