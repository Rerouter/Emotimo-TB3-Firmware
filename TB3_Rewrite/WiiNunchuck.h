#ifndef WiiNunchuck_h
#define WiiNunchuck_h

#include <Arduino.h>
#include "Wire.h"

class WiiNunchuck {
public:
  WiiNunchuck();
  void initNunchuck();
  void requestData();
  uint8_t getJoyX();
  uint8_t getJoyY();
  uint16_t getAccelX();
  uint16_t getAccelY();
  uint16_t getAccelZ();
  bool getButtonC();
  bool getButtonZ();
  uint8_t getErrorCount();

private:
  uint8_t nunchuck_buf[6];
  uint8_t error_count;
  void updateBuffer(const uint8_t buf[6]);
};

#endif
