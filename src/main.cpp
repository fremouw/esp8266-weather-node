#include <ESP8266WiFi.h>
#include "node/sensor_node.h"

mfr::SensorNode node;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  node.setup();
}

void loop() {
  node.loop();
}
