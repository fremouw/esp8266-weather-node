#include "sensor.h"
#include <Wire.h>
#include <SPI.h>
#include <BME280_MOD-1022.h>

namespace climate {
  void Sensor::setup(const int sda, const int scl, const bool setForced) {
    this->forcedMode = setForced;

    BME280.readCompensationParams();

    uint8_t chipId = BME280.readChipId();
    if(chipId == 0xff) {
      Serial.println(F("sensor BME280: sensor not found."));
    } else {
      // find the chip ID out just for fun
      Serial.print(F("sensor BME280: chip identifier=0x"));
      Serial.println(chipId, HEX);

      this->isEnabled = true;
    }

    if(this->isEnabled && !this->forcedMode) {
      Serial.println(F("sensor BME280: normal mode enabled."));
      BME280.writeFilterCoefficient(fc_off);
      BME280.writeFilterCoefficient(fc_16);      // IIR Filter coefficient 16

      BME280.writeOversamplingPressure(os2x);
      BME280.writeOversamplingTemperature(os2x);
      BME280.writeOversamplingHumidity(os1x);

      BME280.writeStandbyTime(tsb_1000ms);        // tsb = 0.5ms
      BME280.writeOversamplingPressure(os16x);    // pressure x16
      BME280.writeOversamplingTemperature(os2x);  // temperature x2
      BME280.writeOversamplingHumidity(os1x);     // humidity x1

      BME280.writeMode(smNormal);
    }
  }

  bool Sensor::measure(climate::Measurement &measurement) {
    if(!this->isEnabled) {
      return false;
    }

    Serial.print(F("sensor BME280: measuring "));
    static const unsigned long delayInMicroseconds = 50;
    static const unsigned long timeOutInMicroseconds = 10000;

    uint16_t iterations = timeOutInMicroseconds / delayInMicroseconds;
    
    BME280.readCompensationParams();

    if(this->forcedMode) {
      Serial.println(F("sensor BME280: forced mode enabled."));

      BME280.writeOversamplingPressure(os1x);  // 1x over sampling (ie, just one sample)
      BME280.writeOversamplingTemperature(os1x);
      BME280.writeOversamplingHumidity(os1x);
      BME280.writeFilterCoefficient(fc_off);

      BME280.writeMode(smForced);
    }

    while (BME280.isMeasuring()) {
      Serial.print(".");
      delay(delayInMicroseconds);

      if(iterations-- < 1) {
        Serial.println(F("\r\nerror: timeout, could not read temperature sensor."));

        return false;
      }
    }
    Serial.println(F("\r\ndone."));

    BME280.readMeasurements();

    Serial.print("Temp=");
    Serial.println(BME280.getTemperature());  // must get temp first
    Serial.print("Humidity=");
    Serial.println(BME280.getHumidity());
    Serial.print("Pressure=");
    Serial.println(BME280.getPressure());

    measurement.temperature = BME280.getTemperatureMostAccurate();
    measurement.pressure    = BME280.getPressureMostAccurate();
    measurement.humidity    = BME280.getHumidityMostAccurate();

    return true;
  }
}
