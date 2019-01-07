#include "sensor_node.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "config.h"

namespace mfr {
  const uint32_t SensorNode::kSleepTime     = 900e6; // Sleep for 600 seconds
  // const uint32_t SensorNode::kSleepTime     = 300e6; // Sleep for 300 seconds
  // const uint32_t SensorNode::kSleepTime     = 15e6; // Sleep for 300 seconds
  // const uint32_t SensorNode::kSleepBaseInterval = 30 * 1000000; // Sleep for x minutes, then check temperature again.

  void SensorNode::setup() {
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);

    Serial.setDebugOutput(true);

    Serial.print(F("\r\nBooting... v1.0-"));
    Serial.print(SENSOR_VERSION);
    Serial.print(" (");
    Serial.print(__TIMESTAMP__);
    Serial.println(F(")."));
    Serial.print(F("Running at: "));
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(F("MHz"));

    Serial.print(F("SDK version: "));
    Serial.println(ESP.getSdkVersion());
    Serial.print(F("Core version: "));
    Serial.println(ESP.getCoreVersion());
    Serial.print(F("Boot version: "));
    Serial.println(ESP.getBootVersion());

    Wire.begin(WSConfig::kI2cSdaPin, WSConfig::kI2cSclPin);

    // Configure BME/P280 sensor. We set it to sample once p/s, put it in forced mode, and disable filtering.
    sensor.setup(WSConfig::kI2cSdaPin, WSConfig::kI2cSclPin, true);

    String hostname(WSConfig::kHostname);
    hostname += String(ESP.getChipId(), HEX);
    WiFi.hostname(hostname);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WSConfig::kSsid, WSConfig::kSsidPassword);

    Serial.println(F("Info: in publish mode."));
    Serial.print(F("Info: connecting to wireless network "));

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(F("."));
    }

    Serial.println(F(" done"));
    Serial.print(F("IP Address: "));
    Serial.println(WiFi.localIP());

    Serial.println(F("mqtt disconnect"));
    this->mqttClient.disconnect();

    Serial.println(F("mqtt set server"));
    this->mqttClient.setServer(WSConfig::kMqttBroker, WSConfig::kMqttBrokerPort);

    Serial.println(F("is connected"));
    //
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(F("Error: unable to connect to wireless network, entering deep sleep."));

      ESP.deepSleep(SensorNode::kSleepTime, WAKE_RF_DEFAULT);

      return;
    }

    // 
    // WifiClient is not async, verify fingerprint using the ASyncTCP lib.
    //
    // Serial.println(F("check mqtt broker"));
    // if (!this->wifiClient.connect(mqttBroker.c_str(), mqttBrokerPort)) {
    //   Serial.println("Error: could not connect to MQTT broker, entering deep sleep.");

    //   ESP.deepSleep(SensorNode::kSleepTime, WAKE_RF_DEFAULT);

    //   return;
    // }

    // Serial.println(F("verify mqtt broker"));
    // if (this->wifiClient.verify(mqttBrokerFingerprint, mqttBroker.c_str())) {
    //   Serial.println(
    //       F("Info: MQTT broker fingerprint matches stored fingerprint."));
    // } else {
    //   Serial.println(F("Error: MQTT broker fingerprint does not match stored fingerprint."));

    //   ESP.deepSleep(SensorNode::kSleepTime, WAKE_RF_DEFAULT);

    //   return;
    // }

    IPAddress ipAddress;
    WiFi.hostByName(WSConfig::kNtpServerName, ipAddress);
    timeClient.setup(ipAddress);
  }

  void SensorNode::loop() {
    if (WiFi.status() == WL_CONNECTED) {
      if (shouldEnterDeepSleep) {
        this->mqttClient.disconnect();

        delay(500);

        Serial.println(F("Info: published data, entering (deep) sleep."));

        ESP.deepSleep(kSleepTime, WAKE_RF_DEFAULT);
      } else {
        this->publish();
      }
    } 
  }

  void SensorNode::publish() {
    Serial.println("Info: publishing data to MQTT broker.");

    if (!mqttClient.connected()) {
      Serial.println("Debug: not connected to MQTT broker.");
      String hostname(WSConfig::kHostname);
      hostname += String(ESP.getChipId(), HEX);

      if (this->mqttClient.connect(hostname.c_str(), WSConfig::kMqttBrokerUsername, WSConfig::kMqttBrokerPassword)) {
      }
    }

    this->mqttClient.loop();

    Serial.print(F("Debug: isPublishing="));
    Serial.print(this->isPublishing);
    Serial.println(".");

    if (this->mqttClient.connected() && !isPublishing) {
      Serial.println(F("Info: connected to MQTT broker and will publish data."));
      this->isPublishing = true;

      // Fake data for testing.
      // this->measurement.temperature = 22.85554;
      // this->measurement.humidity = 40.3;
      // this->measurement.pressure = 1024.1;
      if(!sensor.measure(this->measurement)) {
        shouldEnterDeepSleep = true;
        isPublishing = false;

        Serial.println(F("Info: could not measure temperature."));

        return;
      }

      // Update the time, when failed, retry.
      this->timeClient.update([this](bool success) {
        if (success) {
          StaticJsonBuffer<400> jsonBuffer;
          time_t timestamp;
          timeClient.getCurrentTime(timestamp);

          JsonObject& root = jsonBuffer.createObject();
          root["temperature"] = round(this->measurement.temperature * 100.0) / 100.0;
          root["humidity"] = round(this->measurement.humidity * 100.0) / 100.0;
          root["pressure"] = round(this->measurement.pressure * 100.0) / 100.0;
          root["time"] = timestamp;

          String msg;
          root.printTo(msg);

          Serial.print(F("Info: publishing: "));
          Serial.println(msg);

          bool success = this->mqttClient.publish(WSConfig::kMqttTopicName, msg.c_str(), true);
          Serial.print(F("success="));
          Serial.println(success);

          this->shouldEnterDeepSleep = true;
          this->isPublishing = false;
        } else {
          Serial.println(F("Info: could not get time."));
          this->isPublishing = false;
        }
      });
    }
  }
}
