#include "time_client.h"
#include <TimeLib.h>

static const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

TimeClient::TimeClient() {}

void TimeClient::setup(const IPAddress& ipAddress, const uint16_t port) {
    this->timeServerIP = ipAddress;
    this->port         = port;

    udp.connect(this->timeServerIP, this->port);

    udp.onPacket([this](AsyncUDPPacket& packet) {
        Serial.println("Info: received NTP packet.");
        uint8_t *packetBuffer = packet.data();

        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;

        // now convert NTP time into everyday time:
        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        const unsigned long seventyYears = 2208988800UL;

        // subtract seventy years:
        unsigned long epoch = secsSince1900 - seventyYears;

        setTime(epoch);

        didUpdateTime = true;
        this->callback(true);
        callback = nullptr;
    });
}

void TimeClient::getCurrentTime(time_t& timestamp) {
    timestamp = now();
}

void TimeClient::update(std::function<void(bool)>_callback) {
    static const uint8_t packetBuffer[NTP_PACKET_SIZE] = {
        0b11100011,                            // LI, Version, Mode
        0,                                     // Stratum, or type of clock
        6,                                     // Polling Interval
        0xec,                                  // Peer Clock Precision
        0,                0, 0, 0, 0, 0, 0, 0, // 8 bytes of zero for Root Delay & Root Dispersion
        49,
        0x4e,
        49,
        52
    };

    didUpdateTime = false;
    callback      = _callback;

    udp.write(packetBuffer, NTP_PACKET_SIZE);

    uint32_t beginWait = millis();

    while (millis() - beginWait < 5000) {
        delay(100);
    }

    if (!didUpdateTime) {
        Serial.println("Error: did not update time.");

        callback(false);

        callback = nullptr;
    }
}
