#ifndef _LAMPSTATE_H_
#define _LAMPSTATE_H_

#include "Arduino.h"

class Color
{
public:
  uint8_t r;
  uint8_t g;
  uint8_t b;

  Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b){};

  bool operator!=(Color other)
  {
    return !(*this == other);
  }

  bool operator==(Color other)
  {
    return (r == other.r) && (g == other.g) && (b == other.b);
  };

  String toString()
  {
    return String(r) + String(",") + String(g) + String(",") + String(b);
  }

  static Color fromString(String data)
  {

    int index1 = data.indexOf(',');
    int index2 = data.indexOf(',', index1 + 1);
    uint8_t r = data.substring(0, index1).toInt();
    uint8_t g = data.substring(index1 + 1, index2).toInt();
    uint8_t b = data.substring(index2 + 1, data.length()).toInt();

    return Color(r, g, b);
  }
};

class LampState
{
public:
  uint8_t bright;
  bool power;
  Color color;

  LampState(bool power, uint8_t bright, Color color) : bright(bright), power(power), color(color){};
  LampState() : bright(50), power(false), color(Color(255, 255, 255)){};
  
  String toString()
  {
    String result = "";

    result += power ? "ON " : "OFF ";
    result += "Bright: ";
    result += String(bright);
    result += " ";
    result += color.toString();

    return result;
  }
};

#endif
