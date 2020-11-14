#include "I2CBridge.h"
#include "MPU6050-Module.h"
#include <stdint.h>

MPU6050Module ::MPU6050Module(const uint8_t address)
{
  _deviceAddress = address;
}

void MPU6050Module ::initialize()
{
  writeI2C(_deviceAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  writeI2C(_deviceAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  writeI2C(_deviceAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  writeI2C(_deviceAddress, MPU6050_REGISTER_CONFIG, 0x00);
  writeI2C(_deviceAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);
  writeI2C(_deviceAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);
  writeI2C(_deviceAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  writeI2C(_deviceAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  writeI2C(_deviceAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  writeI2C(_deviceAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}

void MPU6050Module ::readSensorValues(MPU6050Data *data, bool checkThreshold)
{
  readI2C(_deviceAddress, MPU6050_REGISTER_ACCEL_XOUT_H, (uint8_t)14);

  data->AccelX = (((int16_t)readWire() << 8) | readWire());
  data->AccelY = (((int16_t)readWire() << 8) | readWire());
  data->AccelZ = (((int16_t)readWire() << 8) | readWire());
  data->Temp = (((int16_t)readWire() << 8) | readWire());
  data->GyroX = (((int16_t)readWire() << 8) | readWire());
  data->GyroY = (((int16_t)readWire() << 8) | readWire());
  data->GyroZ = (((int16_t)readWire() << 8) | readWire());
  data->vector = (data->GyroX * data->GyroY * data->GyroZ);

  if (_isThresholdSet)
  {
    data->isThresholdExceeded = this->isThresholdExceeded(data);
  }
}

void MPU6050Module::setThreshold(MPU6050Data *min, MPU6050Data *max)
{
  _minThreshold = min;
  _maxThreshold = max;
  _minThreshold->vector = (min->GyroX * min->GyroY * min->GyroZ);
  _maxThreshold->vector = (max->GyroX * max->GyroY * max->GyroZ);
  _isThresholdSet = true;
}

bool MPU6050Module::isThresholdExceeded(MPU6050Data *data)
{
  // faulty data check
  if (data->vector == -1)
  {
    return false;
  }

  if (data->vector < _minThreshold->vector || data->vector > _maxThreshold->vector)
  {
    return true;
  }

  // if (data->GyroX > _maxThreshold->GyroX || data->GyroX < _minThreshold->GyroX || data->GyroY > _maxThreshold->GyroY || data->GyroY < _minThreshold->GyroY || data->GyroZ > _maxThreshold->GyroZ || data->GyroZ < _minThreshold->GyroZ)
  // {
  //   return true;
  // }

  return false;
}
