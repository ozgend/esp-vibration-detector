#ifndef __MPU6050__MODULE_H__
#define __MPU6050__MODULE_H__

#define MPU6050_REGISTER_SMPLRT_DIV 0x19
#define MPU6050_REGISTER_USER_CTRL 0x6A
#define MPU6050_REGISTER_PWR_MGMT_1 0x6B
#define MPU6050_REGISTER_PWR_MGMT_2 0x6C
#define MPU6050_REGISTER_CONFIG 0x1A
#define MPU6050_REGISTER_GYRO_CONFIG 0x1B
#define MPU6050_REGISTER_ACCEL_CONFIG 0x1C
#define MPU6050_REGISTER_FIFO_EN 0x23
#define MPU6050_REGISTER_INT_ENABLE 0x38
#define MPU6050_REGISTER_ACCEL_XOUT_H 0x3B
#define MPU6050_REGISTER_SIGNAL_PATH_RESET 0x68

struct MPU6050Data
{
  int16_t AccelX;
  int16_t AccelY;
  int16_t AccelZ;
  int16_t Temp;
  int16_t GyroX;
  int16_t GyroY;
  int16_t GyroZ;
  long vector;
  bool isThresholdExceeded;
};

class MPU6050Module
{
private:
  uint8_t _deviceAddress;
  MPU6050Data *_minThreshold;
  MPU6050Data *_maxThreshold;
  bool _isThresholdSet;

public:
  MPU6050Module(const uint8_t address);
  void initialize();
  void readSensorValues(MPU6050Data *data, bool checkLimit);
  void setThreshold(MPU6050Data *min, MPU6050Data *max);
  bool isThresholdExceeded(MPU6050Data *data);
};

#endif