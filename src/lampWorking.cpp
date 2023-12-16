
#include "lampWorking.h"
#include "Firebase/Firebase.h"
#include "Mqtt/Mqtt.h"

LampWorking lampWorking;

void changeParam(uint8_t &current, uint8_t need, uint8_t delta)
{
  if (current != need)
  {
    if (current > need)
    {
      if (current - need > delta) {
        current-=delta;
      } else {
        current = need;
      }
    }
    else
    {
      if (need - current > delta) {
        current += delta;
      } else {
        current = need;
      }
    }
  }
}


///  Функция преобразования яроксти из 100 -> 255
uint8_t getGlobalBright(uint8_t localBright){
  return localBright * 2 / 3;
};

void LampWorking::init()
{
  FastLED.addLeds<NEOPIXEL, PIN_LEDS>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 9500);
  FastLED.setBrightness(0);
  FastLED.show();
};

void LampWorking::sendStateToMQTT()
{
  mqttControll.publishLampState(needLampState);
};

void LampWorking::sendStateToFirebase(String user)
{
  return;// Убрать как придумаю, как стабильно отправлять на сервер
  firebaseControll.sandLampStateToServerSecondCore(needLampState, user);
};

void LampWorking::setBright(uint8_t bright, uint8_t delta)
{
  needLampState.bright = bright;
  deltaChange = delta;
};

void LampWorking::setColor(Color color, uint8_t delta)
{
  needLampState.color = color;
  deltaChange = delta;
};

void LampWorking::setPower(bool power, uint8_t delta)
{
  needLampState.power = power;
  currentLampState.power = power;
  deltaChange = delta;
};

void LampWorking::set(LampState state, uint8_t delta)
{
  deltaChange = delta;
  needLampState = state;
  Serial.println("set: " + needLampState.toString());
};

void LampWorking::tick()
{

  static uint32_t time_delay = 0;

  if (millis() - time_delay < 30)
  {
    return;
  };

  time_delay = millis();

  // Обновляем параметры с плавным переходом
  changeParam(currentLampState.bright, needLampState.power ? needLampState.bright : (uint8_t)0, deltaChange);
  changeParam(currentLampState.color.r, needLampState.color.r, deltaChange);
  changeParam(currentLampState.color.g, needLampState.color.g, deltaChange);
  changeParam(currentLampState.color.b, needLampState.color.b, deltaChange);



  fillAll(CRGB(currentLampState.color.r, currentLampState.color.g,currentLampState.color.b));
  FastLED.setBrightness(getGlobalBright(currentLampState.bright));
  FastLED.show();
};


void LampWorking::fillAll(CRGB color)
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = color;
  }
};

