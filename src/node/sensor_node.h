#include <Arduino.h>
#include <Hash.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <BME280_MOD-1022.h>
#include <PubSubClient.h>
#include "time/time_client.h"
#include "climate/sensor.h"

#pragma once

namespace mfr {
    class SensorNode {
public:
    SensorNode() : mqttClient(wifiClient) {}

    void setup();
    void loop();

private:
        static const uint32_t kSleepTime;
        static const uint32_t kSleepBaseInterval;

        PubSubClient mqttClient;
        TimeClient timeClient;
        WiFiClientSecure wifiClient;
        climate::Sensor sensor;
        climate::Measurement measurement;

        enum {
            eWantsToIdle = 0,
            eWantsToPublish,
            eWantsToEnterDeepSleep,
            eWantsToUpdateTime
        } currenState = eWantsToIdle;

        bool isPublishing = false;
        bool shouldEnterDeepSleep = false;

        void publish();
        // bool measure(float& temperature, float& pressure, float& humidity);
    };
}
