#include "Mqtt/Mqtt.h"

#include "lampWorking.h"
#include "secret.h"


MqttControll mqttControll;

void MqttControll::connect()
{
  try
  {
    client.setServer(MQTT_SERVER, MQTT_PORT);

    client.setCallback([&](char *a, uint8_t *b, unsigned int c)
                       { dataReciveMQTT(a, b, c); });

    if (client.connect("Lamp", MQTT_USER, MQTT_PASSWORD))
    {
      Serial.println("MQTT Connected!");
      client.subscribe(String("biglamp/color/set").c_str());
      client.subscribe(String("biglamp/bright/set").c_str());
      client.subscribe(String("biglamp/power/set").c_str());

    }
    else
    {
      Serial.println("MQTT error!");
    }
  }
  catch (const char *e)
  {
    Serial.println(e);
  }
  catch (...)
  {
    Serial.println("!!!!! Exeption! !!!!!!");
  };
};

void MqttControll::publishLampState(LampState lampState)
{
  if (lampState.bright != lastState.bright)
  {
    lastState.bright = lampState.bright;
    publishMessage("biglamp/bright/get", String(lastState.bright), true);
  }

  if (lampState.color != lastState.color)
  {
    lastState.color = lampState.color;
    publishMessage("biglamp/color/get", lastState.color.toString(), true);
  }

  if (lampState.power != lastState.power)
  {
    lastState.power = lampState.power;
    publishMessage("biglamp/power/get", lastState.power ? "1" : "0", true);
  }
};

void MqttControll::publishMessage(String topic, String value, bool retained)
{
  if (client.connected())
  {
    client.publish(topic.c_str(), value.c_str(), retained);
  };
};

void MqttControll::dataReciveMQTT(char *topic, uint8_t *payload, unsigned int length)
{
  String data;
  for (int i = 0; i < length; i++)
  {
    data += String((char)payload[i]);
  }

  if (String(topic) == "biglamp/power/set")
  {
    Serial.println("change power");
    lampWorking.setPower(data == "1");
    lampWorking.sendStateToFirebase("mqtt");
  }
  else

      if (String(topic) == "biglamp/bright/set")
  {
    Serial.println("change bright");
    lampWorking.setBright(data.toInt());
    lampWorking.sendStateToFirebase("mqtt");
  }
  else

      if (String(topic) == "biglamp/color/set")
  {
    Serial.println("change color");
    lampWorking.setColor(Color::fromString(data));
    lampWorking.sendStateToFirebase("mqtt");
  }
  else
  {

    Serial.print("Mqtt topic:[");
    Serial.print(topic);

    Serial.print("] data: ");
    Serial.println(data);
  };
}

void MqttControll::tick()
{
  client.loop();

  if (client.connected() == false && millis() - time_pause_reconnect > 5000)
  {
    Serial.println("Reconnect MQTT");
    time_pause_reconnect = millis();
    connect();
  }
};

MqttControll::MqttControll()
{
  client.setClient(espClient);
};