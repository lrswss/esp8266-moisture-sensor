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
#include "mqtt.h"
#include "utils.h"
#include "sensors.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);


// checks for remote commands (read interval, trigger OTA update)
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    uint16_t val;

    if (strstr(topic, "reading_interval") != NULL) {
        val = atoi((const char *)payload);
        if (val >= SENSOR_INTERVAL_MIN_SECS && val <= 3600) {
            if (settings.reading_interval_sec != val) {
                settings.reading_interval_sec = val;
                Serial.printf("Setting sensor reading interval to %d seconds\n", 
                    settings.reading_interval_sec);
            }
        }
    } else if (strstr(topic, "deep_sleep") != NULL) {
        val = atoi((const char *)payload);
        if (val == 0 || val == 1) {
            if (settings.deep_sleep_enabled != val) {
                settings.deep_sleep_enabled = val;
                Serial.printf("%s deep sleep\n", 
                    settings.deep_sleep_enabled > 0 ? "Enable" : "Disable");
            }
        }
#ifdef USE_DEEP_SLEEP
    } else if (strstr(topic, "ota_update") != NULL) {
        val = atoi((const char *)payload);
        if (val != FIRMWARE_VERSION) {
            mqtt.disconnect();
            update_firmware(val);
        } else {
            Serial.printf("Skipping OTA update, v%d already installed\n", val);
        }
#endif    
    }
}


// connect to MQTT broker with random client id
bool mqtt_connect(bool sub2topics) {
    static char clientid[16], topic[64];
    static uint8_t mqtt_error = 0;
    uint8_t loopCount = 0;

    while (!mqtt.connected()) {
        // generate pseudo random client id
        snprintf(clientid, sizeof(clientid), "MQTT_ESP8266_%lX", random(0xffff));

        Serial.printf("Connecting to MQTT Broker %s...", MQTT_BROKER);
        if (mqtt.connect(clientid)) {
            Serial.println(F("OK"));

            if (sub2topics) {
                // subscribe to command topics
                snprintf(topic, sizeof(topic)-1, "%s/%X/%s", 
                    MQTT_BASE_TOPIC, ESP.getChipId(), "reading_interval");
                mqtt.subscribe(topic);
                delay(50);
                snprintf(topic, sizeof(topic)-1, "%s/%X/%s", 
                    MQTT_BASE_TOPIC, ESP.getChipId(), "ota_update");
                mqtt.subscribe(topic);
                delay(50);
                snprintf(topic, sizeof(topic)-1, "%s/%X/%s", 
                    MQTT_BASE_TOPIC, ESP.getChipId(), "deep_sleep");
                mqtt.subscribe(topic);
                delay(50);
            }
            return true;
        } else {
            Serial.print(F("failed with error "));
            Serial.print(mqtt.state());
            Serial.println(F(", try again in 5 seconds."));

            // no mqtt connection within 30 secs
            if (mqtt_error++ >= 6) {
                if (settings.deep_sleep_enabled)
                deepSleep(settings.reading_interval_sec);
                return false;
            }
            longDelay(5000);
        }
    }

    // check for callback messages (remote commmands)
    while (loopCount++ <= 250) {
        mqtt.loop();
        delay(1);
    }

    return true;
}


// publish sensor reading and further info using MQTT
void mqtt_publish() {
    StaticJsonDocument<256> JSON;
    static char topic[64], buf[128];

    if (mqtt_connect(false)) {
        JSON.clear();
        JSON["moisture"] = sensors.moisture;
#ifdef ONEWIRE_PIN
        JSON["temp"] = sensors.temperature;
#endif
        JSON["rssi"] = WiFi.RSSI();
        JSON["interval"] = settings.reading_interval_sec;
        JSON["wakeups"] = settings.wakeup_counter;
        JSON["deepsleep"] = settings.deep_sleep_enabled;
        JSON["version"] = FIRMWARE_VERSION;

        snprintf(topic, sizeof(topic)-1, "%s/%X", MQTT_BASE_TOPIC, ESP.getChipId());
        memset(buf, 0, sizeof(buf));
        size_t s = serializeJson(JSON, buf);
        if (mqtt.publish(topic, buf, s)) {
            Serial.printf("Published %d bytes to %s\n", s, topic);
        } else {
            Serial.println(F("MQTT publish failed!"));
        }
        delay(50);
    }
}