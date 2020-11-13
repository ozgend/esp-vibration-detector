#include <CloudIoTCore.h>
#include <ArduinoJson.h>
#include <string>
#include "esp8266_mqtt.h"
#include "I2CBridge.h"
#include "MPU6050-Module.h"

#define SERIAL_BAUD 38400
#define I2C_MPU_DEVICE_REGISTER 0x68
#define I2C_SCL D2
#define I2C_SDA D3
#define PIN_LED_COLLECTION 15
#define PIN_LED_SENSOR 13
#define COLLECTION_BUFFER 1024
#define COLLECTION_COOLDOWN_MILLIS 650

MPU6050Module mpuSensor(I2C_MPU_DEVICE_REGISTER);

MPU6050Data sensorValues;
MPU6050Data minThreshold = {0, 0, 0, 0, -877, -182, 82};
MPU6050Data maxThreshold = {0, 0, 0, 0, -710, -16, 222};

int sensorCollectionSize = 0;
String serializedPayload;

int16_t xValues[COLLECTION_BUFFER];
int16_t yValues[COLLECTION_BUFFER];
int16_t zValues[COLLECTION_BUFFER];

long sensorNextCooldownMillis = 0;
bool isCollectionRunning = false;

String serializeSensorValues()
{
  Serial.println("creating payload...");

  DynamicJsonDocument payload(1024);
  JsonArray items = payload.to<JsonArray>();
  JsonObject item = items.createNestedObject();

  for (int i = 0; i <= sensorCollectionSize; i++)
  {
    item["x"] = xValues[i];
    item["y"] = yValues[i];
    item["z"] = zValues[i];
    items.add(item);
  }

  serializeJson(payload, serializedPayload);

  memset(xValues, 0, sizeof(xValues));
  memset(yValues, 0, sizeof(yValues));
  memset(zValues, 0, sizeof(zValues));
  sensorCollectionSize = 0;

  Serial.println("payload created.");
  Serial.println("payload reset.");

  return serializedPayload;
}

void collectSensorValues()
{
  if (sensorCollectionSize >= COLLECTION_BUFFER)
  {
    publishSensorValues();
  }

  xValues[sensorCollectionSize] = sensorValues.GyroX;
  yValues[sensorCollectionSize] = sensorValues.GyroY;
  zValues[sensorCollectionSize] = sensorValues.GyroZ;

  sensorCollectionSize++;
}

void publishSensorValues()
{
  if (sensorCollectionSize > 0)
  {
    Serial.println("publishing payload...");
    publishTelemetry(serializeSensorValues());
    Serial.println("payload published.");
  }
}

void plotSensorValues()
{
  Serial.print(sensorValues.GyroX);
  Serial.print("\t");
  Serial.print(sensorValues.GyroY);
  Serial.print("\t");
  Serial.print(sensorValues.GyroZ);
  Serial.print("\t");
  Serial.print(isCollectionRunning ? 5000 : 0);
  Serial.println("");
}

void checkSensorValues()
{
  if (sensorValues.isThresholdExceeded)
  {
    sensorNextCooldownMillis = millis() + COLLECTION_COOLDOWN_MILLIS;
    digitalWrite(PIN_LED_SENSOR, HIGH);
  }
  else
  {
    digitalWrite(PIN_LED_SENSOR, LOW);
  }

  if (millis() <= sensorNextCooldownMillis)
  {
    isCollectionRunning = true;
    digitalWrite(PIN_LED_COLLECTION, HIGH);
    collectSensorValues();
  }
  else
  {
    publishSensorValues();
    sensorNextCooldownMillis = 0;
    isCollectionRunning = false;
    digitalWrite(PIN_LED_COLLECTION, LOW);
  }

  plotSensorValues();
}

void setup()
{
  pinMode(PIN_LED_COLLECTION, OUTPUT);
  pinMode(PIN_LED_SENSOR, OUTPUT);

  Serial.begin(SERIAL_BAUD);
  Serial.println("init...");

  digitalWrite(PIN_LED_COLLECTION, HIGH);

  setupWifi();
  setupCloudIoT();

  digitalWrite(PIN_LED_COLLECTION, LOW);
  delay(500);
  digitalWrite(PIN_LED_SENSOR, HIGH);

  beginWire(I2C_SDA, I2C_SCL);

  delay(1000);

  mpuSensor.initialize();
  mpuSensor.setThreshold(&minThreshold, &maxThreshold);

  digitalWrite(PIN_LED_SENSOR, LOW);

  Serial.println("init completed.");
}

void loop()
{
  if (!mqtt->loop())
  {
    mqtt->mqttConnect();
  }

  delay(10);

  mpuSensor.readSensorValues(&sensorValues, true);
  checkSensorValues();
}
