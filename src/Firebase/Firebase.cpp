#include "Mqtt/Mqtt.h"

#include "lampWorking.h"
#include "Firebase.h"
#include "soc/rtc_wdt.h"

#include "secret.h"

FirebaseControll firebaseControll;

/// Текущее состояние на сервере firebase
FirebaseJson jsonLampState;

/// Указатель на отправку сообщения сервера. Если null, то ничего отправлять не нужно.
FirebaseJson *jsonSendLampState = nullptr;

TaskHandle_t TaskLoopSending;
TaskHandle_t TaskLoopWatchdog;

void loopWatchdog(void *pvParameters){
  for (;;) {
  
  delay(100);

  }}

void sendToFirebaseTask(void *pvParameters)
{

rtc_wdt_protect_off();    // Turns off the automatic wdt service
rtc_wdt_enable();         // Turn it on manually
rtc_wdt_set_time(RTC_WDT_STAGE0, 20000);  // Define how long you desire to let dog wait.

  Serial.println("Start Sending to Firebase Task in core: ");

  FirebaseJson *jsonTask = nullptr;

        FirebaseData data;

  data.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  data.setResponseSize(2048);

  

  for (;;)
  {

    ///Если есть какие то данные которые нужно отправить, записываем их себе и удаляем указатель
    if (jsonSendLampState != nullptr)
    {
      if (jsonTask != nullptr) {
        delete jsonTask;
      }

      jsonTask = new FirebaseJson;
      *jsonTask = *jsonSendLampState;

      delete jsonSendLampState;
      jsonSendLampState = nullptr;
    }

      /// Если что то, нужно отправить
      if (jsonTask != nullptr)
      {

        try
        {
             rtc_wdt_feed();
          if (Firebase.RTDB.set(&data, "biglamp/set", jsonTask))
          {

            delete jsonTask;
            jsonTask = nullptr;
          } else {
            Serial.println(data.errorReason());

            delay(1000);
          };
        }
        catch (...)
        {
          Serial.println("Exception! in Task");
        }
      }

    rtc_wdt_feed();
    delay(100);
  };
}

LampState getLampState()
{
  uint8_t bright = 50;
  bool power = false;
  Color color(255, 255, 255);

  FirebaseJsonData result;

  if (jsonLampState.get(result, "bright"))
  {
    bright = result.intValue;
  };

  if (jsonLampState.get(result, "power"))
  {
    power = result.intValue == 1;
  };

  if (jsonLampState.get(result, "color"))
  {
    color = Color::fromString(result.stringValue);
  }

  LampState lampState(power, bright, color);

  return lampState;
}

void streamCallback(FirebaseStream data)
{

  if (data.dataType() == "int")
  {
    Serial.print("Firebase stream int: ");

    int intData = data.intData();
    Serial.println(intData);

    mqttControll.publishMessage("firebase", String(intData));
  }
  else if (data.dataType() == "null")
  {
    Serial.println("Firebase stream: null");
  }
  else if (data.dataType() == "json")
  {
    auto json = data.jsonObject();
    FirebaseJsonData result;

    if (json.get(result, "bright"))
    {
      jsonLampState.set("bright", result.intValue);
    };

    if (json.get(result, "power"))
    {
      jsonLampState.set("power", result.intValue);
    };

    if (json.get(result, "color"))
    {
      jsonLampState.set("color", result.stringValue);
    };

    if (json.get(result, "user"))
    {
      jsonLampState.set("user", result.stringValue);
    };

    lampWorking.set(getLampState());
    lampWorking.sendStateToMQTT();

    // firebaseControll.updateLampStateToServer();
  }
  else
  {
    Serial.print("Firebase stream dataType: ");
    Serial.println(data.dataType());
  }
}

void streamTimeoutCallback(bool timeout)
{
  // if (timeout)
  // {
  //   Serial.println("stream timed out, resuming...");
  // }
}

void FirebaseControll::init()
{
  /* Assign the api key (required) */
  config.api_key = FIREBASE_API;

  /* Assign the RTDB URL (required) */
  config.database_url = FIREBASE_URL;

  // config.signer.test_mode = true;

  auth.user.email = "plate@gmail.com";
  auth.user.password = "password";

  Firebase.begin(&config, &auth);
  Firebase.reconnectNetwork(true);

  if (!Firebase.RTDB.beginStream(&stream, "/biglamp/set"))
    Serial_Printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

  // #################

  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  /// Запускаем задачу на втором ядре, которая будет при необходимости, отправлять данные в Firebase
  xTaskCreatePinnedToCore(sendToFirebaseTask, "Firebase send", 10000, NULL, 0, &TaskLoopSending, 0);

  /// Запускаем задачу которая обновляет watchDog, иначе при долгой отправке, он сбрасывает контроллер
  // xTaskCreatePinnedToCore(loopWatchdog, "Watchdog Loop", 10000, NULL, 0, &TaskLoopWatchdog, 0);

};

bool FirebaseControll::tick()
{
  // test();
  return Firebase.ready();
};

void FirebaseControll::test()
{

  static uint32_t time = millis();
  static int count = 0;

  if (millis() - time > 1000)
  {
    time = millis();

    if (Firebase.RTDB.set(&fbdo, "/test", count))
    {
      Serial.print("Sended ");
      Serial.println(count);
      count++;
    }
    else
    {
      Serial.print("Error send: ");
      Serial.println(fbdo.errorReason());
    }
  };
}

void FirebaseControll::sandLampStateToServer(LampState lampState, String user)
{
  FirebaseJson json;

  json.add("bright", lampState.bright);
  json.add("color", lampState.color.toString());
  json.add("power", lampState.power ? 1 : 0);
  json.add("user", user);

  Firebase.RTDB.set(&fbdo, "/biglamp/set", &json);
  return;
}

void FirebaseControll::sandLampStateToServerSecondCore(LampState lampState, String user)
{

/// Если этот параметр отправки нового сообщения не null, значит процедура
/// Отпрвки была занята и не успела его обработать, а значит и не сможет удалить при подмене 
if (jsonSendLampState!=nullptr){
  delete jsonSendLampState;
  jsonSendLampState = nullptr;
}

  FirebaseJson *ptrJson = new FirebaseJson;
  ptrJson->add("power", lampState.power ? 1 : 0);
  ptrJson->add("bright", lampState.bright);
  ptrJson->add("color", lampState.color.toString());
  ptrJson->add("user", user);

  jsonSendLampState = ptrJson;
};