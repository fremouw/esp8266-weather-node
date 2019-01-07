# esp8266-weather-node

Goal is to measure the outside temperature using a small uC (ESP8266) on battery power and push this using MQTT to some server. The node measures: temperature, humidity and air pressure. A Bosch BME280 is used for measuring; connected over I2C on pins 15 (SCL) and 5 (SDA), these are configurable.

It measures the temperature, humidity and pressure every 15 minutes, between measurements the ESP is in deep-sleep to save power.

## Set-up

Please have a look at src/config.h on how to configure your node. For now it's hardcoded, somewhere in the future this should be configurable.

## Hardware

ToDo
