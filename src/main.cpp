/***************************************************************************

   Battery powered moisture sensor based on ESP8266 controller

   Read analog value from moisture sensor which consists of two brass rods 
   (and optionally a DS1820 one-wire temperature) and publish the readings 
   via MQTT. Use deep sleep inbetween sensor readings to save battery power.

   Use MQTT messages to configure the sensor remotely or trigger OTA.

   When using a bare ESP12F chip (Generic 8266 Module) with a few external
   components compile with reset method 'ck' to initially upload the firmware
   with a usb-serial-adapter.

   (c) 2023 Lars Wessels <software@bytebox.org>

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify,
   merge, publish, distribute, sublicense, and/or sell copies of the
   Software, and to permit persons to whom the Software is furnished
   to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THEAUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

***************************************************************************/

#include "config.h"
#include "mqtt.h"
#include "wlan.h"
#include "utils.h"
#include "sensors.h"


void setup() {
    rst_info *rsti;

    Serial.begin(9600);
    delay(500);

    pinMode(POWER_PIN, OUTPUT);
    Serial.printf("\n\nFirmware: v%d (%s, %s)\n", FIRMWARE_VERSION, __TIME__, __DATE__);
    Serial.print(F("Starting ESP8266 moisture sensor "));

#ifdef USE_DEEP_SLEEP
    settings.deep_sleep_enabled = 1;
#else
    settings.deep_sleep_enabled = 0;
#endif

    rsti = ESP.getResetInfoPtr();  // tools/sdk/include/user_interface.h
    switch (rsti->reason) {
        case REASON_WDT_RST:
        case REASON_SOFT_WDT_RST:
            Serial.println(F("after watchdog-reset"));
            break;
        case REASON_EXCEPTION_RST:
            Serial.println(F("after exception"));
            break;
        case REASON_SOFT_RESTART:
            Serial.println(F("after soft-restart"));
            break;
        case REASON_DEEP_SLEEP_AWAKE:
            Serial.println(F("after deep-sleep"));
            Serial.print(F("Sleep cycles since power-up: "));
            system_rtc_mem_read(65, &settings, sizeof(settings));
            Serial.println(settings.wakeup_counter);
            break;
        case REASON_DEFAULT_RST:
        case REASON_EXT_SYS_RST:
            Serial.println(F("after power-up/reset"));
            system_rtc_mem_write(65, &settings, sizeof(settings));
            break;
    }

    // HW info
    Serial.print(F("\nChip ID: "));
    Serial.println(ESP.getChipId(), HEX);
    Serial.print(F("Flash ID: "));
    Serial.println(ESP.getFlashChipId(), HEX);
    Serial.print(F("MAC address: "));
    Serial.println(WiFi.macAddress());
  
    // connect to WLAN
    Serial.print(F("\nConnecting to SSID "));
    Serial.print(WIFI_SSID);
    wifi_connect();

    // setup temperature sensor
#ifdef ONEWIRE_PIN
    initDS1820();
#endif

    // set mqtt server
    mqtt.setServer(MQTT_BROKER, 1883);
    mqtt.setCallback(mqtt_callback);
    ESP.wdtEnable(0);
}


void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print(F("Reconnecting to SSID "));
        Serial.print(WIFI_SSID);
        wifi_connect();
    }

    if (WiFi.status() == WL_CONNECTED) {
#ifdef ONEWIRE_PIN
        readTemp();
#endif
        readMoisture();
        if (mqtt_connect(true))
            mqtt_publish();
    }

    if (settings.deep_sleep_enabled) {
        deepSleep(settings.reading_interval_sec);
    } else {
        Serial.printf("Waiting %d seconds...\n", settings.reading_interval_sec);
        longDelay(settings.reading_interval_sec*1000);
    }

    ESP.wdtFeed();
}
