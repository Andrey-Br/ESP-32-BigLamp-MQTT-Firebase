#include <Arduino.h>

#define WIFI_SSID "Wi-Fi Drs"
#define WIFI_PASSWORD "121234345656"

#include "WiFi.h"
#include "Mqtt/Mqtt.h"
#include "Firebase/Firebase.h"
#include "lampWorking.h"

void setup() {
  Serial.begin(115200);

  Serial.print("WiFi connect to ");
  Serial.print(WIFI_SSID);
  Serial.print(" [");
  Serial.print(WIFI_PASSWORD);
  Serial.print("]");

  
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  };

  Serial.println("");
  Serial.println("Connected!");

  lampWorking.init();
  mqttControll.connect();
  firebaseControll.init();

}

void loop() {

  // Firebase.RTDB.htt

  
  delay(1);
  lampWorking.tick();
  firebaseControll.tick();
  mqttControll.tick();
}


