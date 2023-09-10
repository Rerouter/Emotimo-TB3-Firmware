#ifndef LCDCONTROLLER_H
#define LCDCONTROLLER_H

#include "BitBangSerial.h"

constexpr uint8_t MAX_BRIGHTNESS = 8;  // for NHDLCD9
constexpr uint8_t MIN_BRIGHTNESS = 1;  // for NHDLCD9
constexpr uint8_t MAX_CONTRAST = 50;   // for NHDLCD9
constexpr uint8_t MIN_CONTRAST = 1;    // for NHDLCD9
constexpr uint8_t LCD_CHARS = 20;
constexpr uint8_t LCD_ROWS = 2; 

class LCDController {
  private:
    BitBangSerial _lcdSerial;
    uint8_t _currentContrast;
    uint8_t _currentBrightness;
    
    void command(uint8_t value);
    void command(uint8_t value, uint8_t value2);

  public:
    LCDController(uint8_t pin);
    uint32_t handle();
    bool isBusy() const;
    void at(uint8_t row, uint8_t col, const String &text);
    void print(const String &text);
    void moveCursor(uint8_t col, uint8_t row);
    void on();
    void off();
    void cursorUnderline();
    void CursorOff();
    void moveCursorLeft();
    void moveCursorRight();
    void cursorBlink();
    void backspace();
    void Contrast(uint8_t contrast);
    uint8_t Contrast();
    void Brightness(uint8_t brightness);
    uint8_t Brightness();
    void loadCustomCharacter(uint8_t location, uint8_t charmap[8]);
    void moveDisplayLeft();
    void moveDisplayRight();
    void setBaudRate(uint8_t baudRate);
    void clearLine(uint8_t line);
    void clear();
};

#endif  // LCDCONTROLLER_H
