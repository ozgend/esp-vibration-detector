#include "I2CBridge.h"
#include "MPU6050-Module.h"

#define SERIAL_BAUD 38400
#define I2C_MPU_DEVICE_REGISTER 0x68
#define I2C_SCL D6
#define I2C_SDA D7
#define PIN_LED 15

MPU6050Module mpuSensor(I2C_MPU_DEVICE_REGISTER);

MPU6050Data sensorValues;
MPU6050Data minThreshold = {0, 0, 0, 0, -871, -719, -190};
MPU6050Data maxThreshold = {0, 0, 0, 0, -22, 90, 225};

void publishSensorValues()
{
  Serial.print(sensorValues.GyroX);
  Serial.print("\t");
  Serial.print(sensorValues.GyroY);
  Serial.print("\t");
  Serial.print(sensorValues.GyroZ);
  Serial.print("\t");
  Serial.print(sensorValues.isThresholdExceeded ? -10000 : 0);
  Serial.print("\t");
  Serial.println("");
}

void checkThreshold()
{
  digitalWrite(PIN_LED, sensorValues.isThresholdExceeded ? HIGH : LOW);
}

void setup()
{
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  Serial.begin(SERIAL_BAUD);
  beginWire(I2C_SDA, I2C_SCL);
  delay(1000);
  digitalWrite(PIN_LED, LOW);
  mpuSensor.initialize();
  mpuSensor.setThreshold(&minThreshold, &maxThreshold);
}

void loop()
{
  mpuSensor.readSensorValues(&sensorValues, true);
  publishSensorValues();
  checkThreshold();
}
