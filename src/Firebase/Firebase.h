#include <Firebase_ESP_Client.h>
#include "lampState.h"

class FirebaseControll
{
private:
  

public:
  FirebaseData fbdo;
  FirebaseData stream;

  FirebaseAuth auth;
  FirebaseConfig config;

  void sandLampStateToServer(LampState lampState, String user);

  void sandLampStateToServerYadro(LampState lampState, String user);

  void init();

  bool tick();

  void test();

  void sandLampStateToServerSecondCore(LampState lampState, String user);
};

extern FirebaseControll firebaseControll;