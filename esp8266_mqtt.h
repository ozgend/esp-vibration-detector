/******************************************************************************
 * Copyright 2018 Google
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
// This file contains static methods for API requests using Wifi / MQTT

#include <ESP8266WiFi.h>
#include "FS.h"

// You need to set certificates to All SSL cyphers and you may need to
// increase memory settings in Arduino/cores/esp8266/StackThunk.cpp:
//   https://github.com/esp8266/Arduino/issues/6811
#include "WiFiClientSecureBearSSL.h"
#include <time.h>

#include <MQTT.h>

#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>
#include "ciotc_config.h" // Wifi configuration here

// !!REPLACEME!!
// The MQTT callback function for commands and configuration updates
// Place your message handler code here.
void messageReceivedAdvanced(MQTTClient *client, char topic[], char bytes[], int length)
{
  if (length > 0)
  {
    Serial.printf("incoming: %s - %s\n", topic, bytes);
  }
  else
  {
    Serial.printf("0\n"); // Success but no message
  }
}
///////////////////////////////

// Initialize WiFi and MQTT for this board
static MQTTClient *mqttClient;
static BearSSL::WiFiClientSecure netClient;
static BearSSL::X509List certList;
static CloudIoTCoreDevice device(project_id, location, registry_id, device_id);
CloudIoTCoreMqtt *mqtt;

bool pauseCollection = false;

String getJwt()
{
  // Disable software watchdog as these operations can take a while.
  ESP.wdtDisable();
  time_t iat = time(nullptr);
  Serial.println("refreshing jwt...");
  String jwt = device.createJWT(iat, jwt_exp_secs);
  ESP.wdtEnable(0);
  return jwt;
}

static void setupCertAndPrivateKey()
{
  certList.append(primary_ca);
  certList.append(backup_ca);
  netClient.setTrustAnchors(&certList);

  device.setPrivateKey(private_key);

  Serial.println("private-key setup done.");
}

static void setupWifi()
{
  WiFi.begin(ssid, password);
  Serial.println("connecting wifi...");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
  }

  Serial.println("wifi connected.");
  Serial.println(WiFi.localIP());

  configTime(0, 0, ntp_primary, ntp_secondary);

  Serial.println("waiting on sync-time...");

  while (time(nullptr) < 1510644967)
  {
    delay(10);
  }

  Serial.println("sync-time done.");
}

///////////////////////////////
// Orchestrates various methods from preceeding code.
///////////////////////////////
bool publishTelemetry(const String &data)
{
  return mqtt->publishTelemetry(data);
}

bool publishTelemetry(const char *data, int length)
{
  return mqtt->publishTelemetry(data, length);
}

void setupCloudIoT()
{
  setupCertAndPrivateKey();

  Serial.println("connecting iot mqtt...");

  mqttClient = new MQTTClient(512);
  mqttClient->setOptions(600, true, 5000); // keepAlive, cleanSession, timeout

  if (!mqttClient->connected())
  {
    Serial.println(getJwt());
  }

  mqtt = new CloudIoTCoreMqtt(mqttClient, &netClient, &device);
  mqtt->setUseLts(true);
  mqtt->startMQTTAdvanced(); // Opens connection using advanced callback

  Serial.println("iot mqtt initalized.");
}
