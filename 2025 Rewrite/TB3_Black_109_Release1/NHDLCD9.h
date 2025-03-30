
/*
  allen joslin
  payson productions
  allen@joslin.net
*/

#ifndef NHDLCD9_h
#define NHDLCD9_h

#include "Arduino.h"
#include "BitBangSerial.hpp"

/******************************************************************************************************/
/* NHDLCD9 -- manages the NewHaven Design SerLCD, based on SoftwareSerial to aid pinning and printing */
/*                                                                                                    */
/*     some cmds are cached so repeated calls will not actually be sent which can cause               */
/*     flickering of the display, printed values are not cached and are always sent                   */
/*                                                                                                    */
/*     autoOn: turn off the display and turn it back on with the next command                         */
/*                                                                                                    */
/*     on/off: display of characters, not backlight                                                   */
/*                                                                                                    */
/*     bright: backlight control, by percentage                                                       */
/*                                                                                                    */
/*     scrolling: scrolling is slow because of the amount of time the LCD takes to redraw.            */
/*     scrolling is persistant and moves the x-origin a single column at a time                       */
/*                                                                                                    */
/******************************************************************************************************/

class NHDLCD9 : public BitBangSerial {
private:
  uint8_t _bv[4];
  uint32_t last_command_time = 0;
  uint32_t last_command_delay = 0;
  void command(const uint8_t cmd, const uint16_t delay);
  void command(const uint8_t cmd, const uint8_t value, const uint16_t delay);

public:
  NHDLCD9(const uint8_t pin, const uint8_t numRows = 2, const uint8_t numCols = 16);
  void setup(const uint8_t startBright = 8, const bool startEmpty = true);
  void setBaud(const uint8_t baud);

  void on();
  void off();

  void empty();
  void clearRegion(const uint8_t row, const uint8_t startCol, const uint8_t endCol);
  void backspace();

  void bright(const uint8_t backlightval);
  void contrast(const uint8_t contrastval);
  void pos(const uint8_t row, const uint8_t col);

  void cursorHome();
  void cursorUnderline();
  void cursorBlock();
  void cursorOff();
  void cursorLeft();
  void cursorRight();

  void displayLeft();
  void displayRight();

  void loadCustomCharacter(const uint8_t addr, const uint8_t bitmap[8]);

  void setI2CAddr(const uint8_t addr);
  void displayFW();
  void displayBaud();
  void displayI2CAddr();

  template <typename T>
  void at(const uint8_t row, const uint8_t col, const T& value) {
      pos(row, col);  // Move the cursor to the desired position
      print(value);   // Use the print function to handle the value
  }
  void at(const uint8_t row, const uint8_t col, const char value);
};

#endif