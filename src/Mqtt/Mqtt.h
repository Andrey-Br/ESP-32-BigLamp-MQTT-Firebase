#include "Arduino.h"
#include "PubSubClient.h"
#include <WiFi.h>
#include "lampState.h"

class MqttControll
{

private:
  uint32_t time_pause_reconnect = 0;

  LampState lastState;

  WiFiClient espClient;
  PubSubClient client;

  void dataReciveMQTT(char *topic, uint8_t *payload, unsigned int length);

public:
  MqttControll();

  void connect();

  void publishLampState(LampState lampState);

  void publishMessage(String topic, String value, bool retained = false);

  // void subscribeTopic(String topic);

  void tick();
};

extern MqttControll mqttControll;
