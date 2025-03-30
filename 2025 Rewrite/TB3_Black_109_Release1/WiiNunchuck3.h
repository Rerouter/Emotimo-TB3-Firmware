/*
 * Nunchuck functions
 *
 * ***************
 * Modified to work with Third Party Nunchucks by Grant Emsley, http://arduino.emsley.ca
 * Added several useful functions
 * ***************
 *
 * Originally by:
 * 2007 Tod E. Kurt, http://todbot.com/blog/
 * The Wii Nunchuck reading code originally from Windmeadow Labs
 *   http://www.windmeadow.com/node/42
 */

#ifndef Nunchuck_h
#define Nunchuck_h

#include <Arduino.h>
#include <Wire.h>

constexpr uint8_t JOYSTICK_THRESHOLD = 60;
static_assert(JOYSTICK_THRESHOLD <= 255, "Joystick Threshold must be in the range 0-255");

union NunchuckData {
    uint8_t buffer[6];
    struct {
        uint8_t joyX;      // Joystick X-axis
        uint8_t joyY;      // Joystick Y-axis
        uint8_t accelX;    // Accelerometer X-axis (base value)
        uint8_t accelY;    // Accelerometer Y-axis (base value)
        uint8_t accelZ;    // Accelerometer Z-axis (base value)
        struct {
            bool zButton : 1;  // Bit 0: Z button
            bool cButton : 1;  // Bit 1: C button
            uint8_t accelXOverflow : 2; // Bits 2-3: Accelerometer X overflow
            uint8_t accelYOverflow : 2; // Bits 4-5: Accelerometer Y overflow
            uint8_t accelZOverflow : 2; // Bits 6-7: Accelerometer Z overflow
        } buttons;
    };
    struct {
        uint64_t raw : 48; // Combined buffer for error checking (6 bytes, 48 bits)
    } ErrorView;  // Combined view for simplified error checking
};

class WiiNunchuck3 {
public:
  WiiNunchuck3(void);
  NunchuckData data;
  bool getData(void);
  void printData(void);
  void init(const bool);
  bool zbutton(void);
  bool cbutton(void);
  uint8_t joyx(void);
  uint8_t joyy(void);
  uint16_t accelx(void);
  uint16_t accely(void);
  uint16_t accelz(void);
  int16_t vibration(void);
  void calibrate(void);
  int8_t digitalx(const int16_t threshold = JOYSTICK_THRESHOLD);
  int8_t digitaly(const int16_t threshold = JOYSTICK_THRESHOLD);
private:
  uint8_t centeredJoyX;
  uint8_t centeredJoyY;
  uint8_t nunchuck_buf[6];

  uint8_t request_measurement(void);
  void clearTwiInputBuffer(void);
  void setpowerpins(void);
  uint8_t send_request(void);
  char decode_byte(char);
};

#endif
extern WiiNunchuck3 Nunchuck;
