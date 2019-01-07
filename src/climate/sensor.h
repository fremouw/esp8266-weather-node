#include <Arduino.h>

#ifndef SENSOR_H_
#define SENSOR_H_

namespace climate {
  struct Measurement {
    float temperature;
    float pressure;
    float humidity;
  };

  class Sensor {
    private:
      bool isEnabled = false;
      bool forcedMode = false;

    public:
      void setup(const int sda, const int scl, const bool setForced = false);
      bool measure(climate::Measurement &measurement);
  };
}
#endif // SENSOR_H_
