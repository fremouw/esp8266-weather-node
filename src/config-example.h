#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <pgmspace.h>

namespace WSConfig {
  // Wireless configuration.
  const char kHostname[] PROGMEM = "<node hostname>";
  
  const char kSsid[] PROGMEM = "<ssid>";
  const char kSsidPassword[] PROGMEM = "<password>";

  // I2C SDA/SCL pinout for sensors.
  const uint8_t kI2cSdaPin = 5;
  const uint8_t kI2cSclPin = 15;

  // NTP Timeserver to use.
  const char kNtpServerName[] PROGMEM = "ntp0.nl.net";

  // MQTT configuration. Leave empty (="") if not used.
  const char kMqttBroker[] PROGMEM = "<mqtt.broker.com>";
  const uint16_t kMqttBrokerPort = 8883;
  const char kMqttBrokerUsername[] PROGMEM = "<username>";
  const char kMqttBrokerPassword[] PROGMEM = "<password>";
  const char kMqttTopicName[] PROGMEM = "<topic to publish to>";
};

#endif /* _CONFIG_H_ */
