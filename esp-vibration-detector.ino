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
#define COLLECTION_BUFFER 256
#define COLLECTION_COOLDOWN_MILLIS 750

const bool sandbox = false;

MPU6050Module mpuSensor(I2C_MPU_DEVICE_REGISTER);
MPU6050Data sensorValues;

// covers E 1.5
MPU6050Data minThreshold = {0.0727};
MPU6050Data maxThreshold = {0.0827};

int sensorVectorCollectionSize = 0;
float sensorVectorValues[COLLECTION_BUFFER];

long sensorNextCooldownMillis = 0;
bool isCollectionRunning = false;

String serializedPayload;

String serializeSensorValues()
{
  Serial.println("creating payload...");

  DynamicJsonDocument payload(1024);
  JsonArray items = payload.to<JsonArray>();

  for (int i = 0; i <= sensorVectorCollectionSize; i++)
  {
    items.add(sensorVectorValues[i]);
  }

  serializeJson(payload, serializedPayload);

  Serial.println("payload created.");

  return serializedPayload;
}

void collectSensorValues()
{
  if (sensorVectorCollectionSize >= COLLECTION_BUFFER)
  {
    publishSensorValues();
    memset(sensorVectorValues, 0, sizeof(sensorVectorValues));
    sensorVectorCollectionSize = 0;
  }

  sensorVectorValues[sensorVectorCollectionSize] = sensorValues.Vector;

  sensorVectorCollectionSize++;
}

void publishSensorValues()
{
  if (sensorVectorCollectionSize > 0 && !sandbox)
  {
    Serial.println("publishing payload...");
    publishTelemetry(serializeSensorValues());      
    Serial.println("payload published.");
  }
}

void plotSensorValues()
{
  /*
  Serial.print(sensorValues.GyroX);
  Serial.print("\t");  
  Serial.print(sensorValues.GyroY);
  Serial.print("\t");  
  Serial.print(sensorValues.GyroZ);
  Serial.print("\t");
  */    
  Serial.print(sensorValues.Vector,6);
  Serial.print("\t");
  Serial.print(isCollectionRunning ? 0.1 : 0.0);
  
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
  Serial.begin(SERIAL_BAUD);
  Serial.println("init...");
  Serial.println(sandbox ? "-- sandbox --" : "+++");

  if(!sandbox)
  {
    pinMode(PIN_LED_COLLECTION, OUTPUT);
    digitalWrite(PIN_LED_COLLECTION, HIGH);
    setupWifi();
    setupCloudIoT();
    digitalWrite(PIN_LED_COLLECTION, LOW);
  }

  delay(500);
  
  pinMode(PIN_LED_SENSOR, OUTPUT);
  digitalWrite(PIN_LED_SENSOR, HIGH);

  beginWire(I2C_SDA, I2C_SCL);

  delay(1000);

  mpuSensor.initialize();
  mpuSensor.setThreshold(&minThreshold, &maxThreshold);

  Serial.print("minThreshold: ");
  Serial.print(minThreshold.Vector);
  Serial.print("\t");
  Serial.print("maxThreshold: ");
  Serial.print(maxThreshold.Vector);

  digitalWrite(PIN_LED_SENSOR, LOW);

  Serial.println("init completed.");
}

void loop()
{
  if (!sandbox && !mqtt->loop())
  {
    mqtt->mqttConnect();
  }
  
  delay(10);

  mpuSensor.readSensorValues(&sensorValues, true);
  checkSensorValues();
}
