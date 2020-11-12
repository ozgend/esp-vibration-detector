#include <Wire.h>
#include "I2CBridge.h"

void writeI2C(const uint8_t deviceAddress, const uint8_t regAddress, const uint8_t data)
{
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

void readI2C(const uint8_t deviceAddress, const uint8_t regAddress, const uint8_t size)
{
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, size);
}

int readWire()
{
  return Wire.read();
}

void beginWire(const uint8_t sda, const uint8_t scl)
{
  Wire.begin(sda, scl);
}
