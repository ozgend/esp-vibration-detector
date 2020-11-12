#ifndef __I2C_BRIDGE_H__
#define __I2C_BRIDGE_H__

#include <stdint.h>

void writeI2C(const uint8_t deviceAddress, const uint8_t regAddress, const uint8_t data);
void readI2C(const uint8_t deviceAddress, const uint8_t regAddress, const uint8_t size);
int readWire();
void beginWire(const uint8_t sda, const uint8_t scl);

#endif