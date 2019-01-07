#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncUDP.h>

#ifndef TIME_CLIENT_H_
#define TIME_CLIENT_H_

class TimeClient {
public:
        TimeClient();

        void setup(const IPAddress& ipAddress, const uint16_t port = 123);

        void update(std::function<void(bool)> callback);

        void getCurrentTime(time_t& timestamp);
private:
        IPAddress timeServerIP;
        uint16_t port;
        AsyncUDP udp;
        std::function<void(bool)> callback;
        bool didUpdateTime;
};

#endif // TIME_CLIENT_H_
