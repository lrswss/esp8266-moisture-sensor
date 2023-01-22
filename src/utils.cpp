/***************************************************************************
  
  Copyright (c) 2023 Lars Wessels

  This file a part of the "ESP8266 Moisture Sensor" source code.
  https://github.com/lrswss/esp8266-moisture-sensor

  Licensed under the MIT License. You may not use this file except in
  compliance with the License. You may obtain a copy of the License at

  https://opensource.org/licenses/MIT

***************************************************************************/


#include "config.h"
#include "utils.h"
#include "wlan.h"
#include "utils.h"
#include "mqtt.h"

settings_t settings;


// put system to deep sleep to save battery
void deepSleep(uint16_t duration) {
#ifdef USE_DEEP_SLEEP
    WiFi.disconnect();
    Serial.printf("Deep sleep for %d seconds...\n", duration);
    Serial.flush();
    settings.wakeup_counter++;
    system_rtc_mem_write(65, &settings, sizeof(settings));
    ESP.deepSleep(duration*1000000);
    yield();
#endif
}

// check for firmware update for this device on webserver
// expected file on server: esp8266_XXXXXX_vYYY.bin where XXXXXX are
// last three octets of MAC and YYY is the firmware's version number
// WARNING: rather simple setup with authenticity checks of either server or firmware
void update_firmware(uint8_t version) {
    static char binpath[128], verstr[8];

    Serial.printf("Updating firmware from v%d to v%d...\n", FIRMWARE_VERSION, version);
    ESP.wdtDisable(); // only SW watchdog!
    snprintf(verstr, sizeof(verstr)-1, "%.3d", version);
    snprintf(binpath, sizeof(binpath)-1, "%s/esp8266_%X_v%s.bin", OTA_UPDATE_PATH, ESP.getChipId(), verstr);
    t_httpUpdate_return ret = ESPhttpUpdate.update(espClient, OTA_UPDATE_HOST, 80, binpath);
    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("OTA error: %s\n", ESPhttpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("OTA no update available");
            break;
        case HTTP_UPDATE_OK:
            Serial.println("OTA successful");
            break;
    }
}


// delay function that keeps feeding the ESP's HW/SW watchdogs
void longDelay(uint16_t delayMs) {
    uint16_t loopCount = 0;
    while (loopCount++ <= delayMs/100) {
        delay(100);
        ESP.wdtFeed();
    }
}

// turn hex byte array of given length into a null-terminated hex string
static void array2string(const byte *arr, int len, char *buf, bool reverse) {
    for (int i = 0; i < len; i++)
        sprintf(buf + i * 2, "%02X", reverse ? arr[len-1-i]: arr[i]);
}


// print byte array as hex string, optionally masking last 4 bytes
void printHEX8bit(uint8_t *arr, uint8_t len, bool ln, bool reverse) {
    char hex[len * 2 + 1];
    array2string(arr, len, hex, reverse);
    Serial.print(hex);
    if (ln)
        Serial.println();
}

