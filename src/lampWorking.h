#ifndef _LAMPWORKING_H_
#define _LAMPWORKING_H_

#include "Arduino.h"
#include "lampState.h"
#include "FastLED.h"

#define NUM_LEDS 447
#define PIN_LEDS 13
#define PIN_BTN 4
#define WIDTH 21
#define HEIGH 20

class LampWorking
{
private:
  /// как Быстро перетекает яркость и цвета
  uint8_t deltaChange = 3;
  /// массив цветов для светодиодов
  CRGB leds[NUM_LEDS+1]; 
  
  /// что прямо сейчас отображается на лампеб, если разные с нжным,
  /// то плавно меняется со скоростью deltaChange
  LampState currentLampState = LampState(false, 0 , Color(255,255,255));
  
  /// что должно отоброжаться
  LampState needLampState = currentLampState;

  void fillAll(CRGB color);

public:
  void tick();

  void init();

  /// отправить текущее состояние на сервер MQTT
  void sendStateToMQTT();

  /// отправить текущее состояние на сервер Firebase
  void sendStateToFirebase(String user);

  /// установить необходимую яркость и скорость с которой поменяться
  void setBright(uint8_t bright, uint8_t delta = 3);

  /// установить необходимый цвет и скорость с которой поменяться
  void setColor(Color color, uint8_t delta = 3);

  /// установить необходимое включение  и скорость с которой поменяться
  void setPower(bool power, uint8_t delta = 3);

  void set(LampState state, uint8_t delta = 15);
};

extern LampWorking lampWorking;

#endif
