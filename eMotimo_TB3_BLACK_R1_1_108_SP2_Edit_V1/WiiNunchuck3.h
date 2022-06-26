/*
   Nunchuck functions

 * ***************


   Modified to work with Third Party Nunchucks by Grant Emsley, http://arduino.emsley.ca
   Added several useful functions

 * ***************

   Originally by:
   2007 Tod E. Kurt, http://todbot.com/blog/
   The Wii Nunchuck reading code originally from Windmeadow Labs
     http://www.windmeadow.com/node/42
*/

#ifndef Nunchuck_h
#define Nunchuck_h

#undef int
#include <stdio.h>
#include <stdint.h>
#include <Arduino.h>

#define JOYSTICK_THRESHOLD 60

class WiiNunchuck3
{
  public:
    WiiNunchuck3(void);
    uint8_t  getData(void);
    void     clearData(void);
    void     printData(void) const;
    void     init(bool);
    bool     zbutton(void) const;
    bool     cbutton(void) const;
    uint8_t  joyx(void) const;
    uint8_t  joyy(void) const;
    uint16_t accelx(void) const;
    uint16_t accely(void) const;
    uint16_t accelz(void) const;
    int      vibration(void);
    void     calibrate(void);
    int8_t   digitalx(int8_t threshold = JOYSTICK_THRESHOLD) const;
    int8_t   digitaly(int8_t threshold = JOYSTICK_THRESHOLD) const;
  private:
    uint8_t  cnt;
    uint8_t  centeredJoyX;
    uint8_t  centeredJoyY;
    uint8_t  nunchuck_buf[6];

    static void send_zero(void);
    static void clearTwiInputBuffer(void);
    static void setpowerpins(void);
    static void send_request(void);
    char decode_byte(char);
};

#endif
extern WiiNunchuck3 Nunchuck;
