#include "LCDController.h"

LCDController::LCDController(uint8_t pin)
  : _lcdSerial(pin), _currentBrightness(MAX_BRIGHTNESS) {
  _lcdSerial.begin();
}

uint32_t LCDController::handle() {
  return _lcdSerial.handle();
}

bool LCDController::isBusy() const {
  return _lcdSerial.availableSpace() <= (WRITE_BUF_SIZE / 2);
}

void LCDController::command(uint8_t value) {
  if (_lcdSerial.availableSpace() >= 2) {  // Check space for 0xFE + value
    _lcdSerial.write(0xFE);
    _lcdSerial.write(value);
  }
  // Consider handling the case where there isn't enough space
}

void LCDController::command(uint8_t value, uint8_t value2) {
  if (_lcdSerial.availableSpace() >= 3) {  // Check space for 0xFE + value + value2
    _lcdSerial.write(0xFE);
    _lcdSerial.write(value);
    _lcdSerial.write(value2);
  }
  // Consider handling the case where there isn't enough space
}

void LCDController::at(uint8_t row, uint8_t col, const String &text) {
  if (_lcdSerial.availableSpace() >= text.length()) { // Ensure the entire string fits
    moveCursor(row, col);
    for (char c : text) {
      _lcdSerial.write(c);
    }
  }
  // Consider handling the case where there isn't enough space
}

void LCDController::print(const String &text) {
  if (_lcdSerial.availableSpace() >= text.length()) { // Ensure the entire string fits
    for (char c : text) {
      _lcdSerial.write(c);
    }
  }
  // Consider handling the case where there isn't enough space
}

void LCDController::on() {
  command(0x41);
}

void LCDController::off() {
  command(0x43);
}

void LCDController::moveCursor(uint8_t row, uint8_t col) {
  col--; // Decrement the column to match 0-based index
  col += 64 * (row - 1);
  command(0x45, col);
}

void LCDController::cursorUnderline() {
  command(0x47);
}

void LCDController::CursorOff() {
  command(0x48);
  command(0x4C);
}

void LCDController::moveCursorLeft() {
  command(0x49);
}

void LCDController::moveCursorRight() {
  command(0x4A);
}

void LCDController::cursorBlink() {
  command(0x4B);
}

void LCDController::backspace() {
  command(0x4E);
}

//void LCDController::clearLCD() {
//  command(0x51);
//  delayMicroseconds(CLEAR_MICROS);
//}

void LCDController::Contrast ( uint8_t contrast ) {
  if (contrast < MIN_CONTRAST || contrast > MAX_CONTRAST) {
    return;  // Invalid contrast level, ignore
  }
  _currentContrast = contrast;
  command(0x52, contrast);
}

uint8_t LCDController::Contrast() {
  return _currentBrightness;
}


void LCDController::Brightness(uint8_t brightness) {
  if (brightness < MIN_BRIGHTNESS || brightness > MAX_BRIGHTNESS) {
    return;  // Invalid brightness level, ignore
  }

  _currentBrightness = brightness;
  command(0x53, brightness);
}

uint8_t LCDController::Brightness() {
  return _currentBrightness;
}

void LCDController::loadCustomCharacter(uint8_t location, uint8_t charmap[8]) {
  if (_lcdSerial.availableSpace() >= 11) { // Ensure space for entire command
    _lcdSerial.write(0xFE);
    _lcdSerial.write(0x54);
    _lcdSerial.write(location);
    for (uint8_t i = 0; i < 8; i++) {
      _lcdSerial.write(charmap[i]);
    }
  }
  // Consider handling the case where there isn't enough space
}

void LCDController::moveDisplayLeft() {
  command(0x55);
}

void LCDController::moveDisplayRight() {
  command(0x56);
}

void LCDController::setBaudRate(uint8_t baudRate) {
  command(0x61, baudRate);
}

void LCDController::clear() {
  if (_lcdSerial.availableSpace() >= (6 + LCD_CHARS) * LCD_ROWS) {  // 6 bytes for command + LCD_CHARS bytes for each clearString
    for (uint8_t row = 0; row < LCD_ROWS; row++) {
      // clear the specified line
      moveCursor(0, row);
      for (uint8_t i = 0; i < LCD_CHARS; i++) {
        _lcdSerial.write(0x20); // Space
      }
    }

    // return cursor to home position
    moveCursor(0, 0);
  }
}

void LCDController::clearLine(uint8_t line) {
  if (_lcdSerial.availableSpace() >= 24) {  // 4 bytes for command + 20 bytes for clearString
    // clear the specified line
    moveCursor(0, line);
    for (uint8_t i = 0; i < LCD_CHARS; i++) {
      _lcdSerial.write(0x20); // Space
    }

    // return cursor to the start of the cleared line
    moveCursor(0, line);
  }
}
