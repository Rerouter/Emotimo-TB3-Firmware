
/*
  allen joslin
  payson productions
  allen@joslin.net
*/

#include "NHDLCD9.h"

/* ======================================================== */

constexpr uint8_t PINOUT = 0;
constexpr uint8_t BOUNCE = 1;
constexpr uint8_t NUMROWS = 2;
constexpr uint8_t NUMCOLS = 3;


// Command Prefix
constexpr uint8_t CMD_PREFIX = 0xFE;

// Command Definitions
constexpr uint8_t CMD_DISPLAY_ON = 0x41;            // Display on
constexpr uint8_t CMD_DISPLAY_OFF = 0x42;           // Display off
constexpr uint8_t CMD_SET_CURSOR = 0x45;            // Set cursor position
constexpr uint8_t CMD_CURSOR_HOME = 0x46;           // Move cursor to home
constexpr uint8_t CMD_UNDERLINE_CURSOR_ON = 0x47;   // Turn on underline cursor
constexpr uint8_t CMD_UNDERLINE_CURSOR_OFF = 0x48;  // Turn off underline cursor
constexpr uint8_t CMD_MOVE_CURSOR_LEFT = 0x49;      // Move cursor left one position
constexpr uint8_t CMD_MOVE_CURSOR_RIGHT = 0x4A;     // Move cursor right one position
constexpr uint8_t CMD_BLINK_CURSOR_ON = 0x4B;       // Turn on blinking cursor
constexpr uint8_t CMD_BLINK_CURSOR_OFF = 0x4C;      // Turn off blinking cursor
constexpr uint8_t CMD_BACKSPACE = 0x4E;             // Backspace
constexpr uint8_t CMD_CLEAR_SCREEN = 0x51;          // This command clears the entire display and place the cursor at line 1 column 1.
constexpr uint8_t CMD_SET_CONTRAST = 0x52;          // Set the display contrast, value between 1 to 50
constexpr uint8_t CMD_SET_BRIGHTNESS = 0x53;        // Set the LCD backlight brightness level, value between 1 to 8
constexpr uint8_t CMD_LOAD_CUSTOM_CHAR = 0x54;      // Load a custom character
constexpr uint8_t CMD_MOVE_DISPLAY_LEFT = 0x55;     // Move display left one position
constexpr uint8_t CMD_MOVE_DISPLAY_RIGHT = 0x56;    // Move display right one position
constexpr uint8_t CMD_DISPLAY_FW_AND_BAUD = 0x57;   // This command display the micro-controller firmware version number. and display the current RS232 BAUD rate
constexpr uint8_t CMD_CHANGE_BAUD_RATE = 0x61;      // Change RS-232 baud rate
constexpr uint8_t CMD_CHANGE_I2C_ADDRESS = 0x62;    // Change I2C address
constexpr uint8_t CMD_DISPLAY_FW_VERSION = 0x70;    // Display firmware version number
constexpr uint8_t CMD_DISPLAY_BAUD_RATE = 0x71;     // Display RS-232 baud rate
constexpr uint8_t CMD_DISPLAY_I2C_ADDRESS = 0x72;   // Display I2C address
constexpr uint8_t CMD_READ_DATA = 0x73;             // Takes 1 Dummy Byte, and returns the character at the current cursor position

constexpr uint16_t calculateDelay(uint16_t usDelay, uint32_t baudRate) {
    // Inverted logic: remove the delay if byte transmission time is greater than the delay
    return ((1.0 / baudRate) * 10 * 1000000 > usDelay) ? 0 : usDelay;
}

// Timing Definitions (in microseconds)
constexpr uint32_t BAUD_RATE = 115200;    // Baud Rate
constexpr uint16_t EXEC_TIME_100 = calculateDelay(100, BAUD_RATE);
constexpr uint16_t EXEC_TIME_200 = calculateDelay(200, BAUD_RATE);
constexpr uint16_t EXEC_TIME_500 = calculateDelay(500, BAUD_RATE);
constexpr uint16_t EXEC_TIME_1500 = calculateDelay(1500, BAUD_RATE);
constexpr uint16_t EXEC_TIME_3000 = calculateDelay(3000, BAUD_RATE);
constexpr uint16_t EXEC_TIME_4000 = calculateDelay(4000, BAUD_RATE);
constexpr uint16_t EXEC_TIME_10000 = calculateDelay(10000, BAUD_RATE);


//--------------------------
NHDLCD9::NHDLCD9(const uint8_t pin, const uint8_t numRows, const uint8_t numCols)
  : BitBangSerial(pin, 57600) {
  _bv[PINOUT] = pin;
  _bv[BOUNCE] = 1;
  _bv[NUMROWS] = numRows;
  _bv[NUMCOLS] = numCols;
}

//--------------------------

// Functions for sending the special command values
void NHDLCD9::command(const uint8_t cmd, const uint16_t delay) {
  uint32_t current_time = micros();
  if (current_time - last_command_time < last_command_delay) {
    delayMicroseconds((last_command_time + last_command_delay) - current_time);
  }
  write(CMD_PREFIX);
  write(cmd);
  last_command_time = micros();
  last_command_delay = delay;
}

// Functions for sending the special command values
void NHDLCD9::command(const uint8_t cmd, const uint8_t value, const uint16_t delay) {
  uint32_t current_time = micros();
  if (current_time - last_command_time < last_command_delay) {
    delayMicroseconds((last_command_time + last_command_delay) - current_time);
  }
  write(CMD_PREFIX);
  write(cmd);
  write(value);
  last_command_time = micros();
  last_command_delay = delay;
}


void NHDLCD9::setup(const uint8_t startBright, const bool startEmpty) {
  if (startEmpty) {
    empty();
  }
  if (startBright) {
    bright(startBright);
  }
}

void NHDLCD9::at(const uint8_t row, const uint8_t col, const char value) {
    pos(row, col);  // Move the cursor to the desired position
    print(value); 
}


void NHDLCD9::clearRegion(const uint8_t row, const uint8_t startCol, const uint8_t endCol) {
  pos(row, startCol);
  uint32_t current_time = micros();
  if (current_time - last_command_time < last_command_delay) {
    delayMicroseconds((last_command_time + last_command_delay) - current_time);
  }
  for (uint8_t col = startCol; col <= endCol; ++col) {
      write(' ');
  }
  last_command_time = micros();
  last_command_delay = EXEC_TIME_100;
}


//--------------------------

void NHDLCD9::on() { command(CMD_DISPLAY_ON, EXEC_TIME_100); }
void NHDLCD9::off() { command(CMD_DISPLAY_OFF, EXEC_TIME_100);}

void NHDLCD9::pos(const uint8_t row, const uint8_t col) {
    // Send command to set the cursor with a one-line calculation for the address
    command(CMD_SET_CURSOR, (col - 1) + (row - 1) * 0x40, EXEC_TIME_100); // Adjusted for 0-based index and row offset
}

void NHDLCD9::cursorHome() { command(CMD_CURSOR_HOME, EXEC_TIME_1500); }
void NHDLCD9::cursorUnderline() { command(CMD_UNDERLINE_CURSOR_ON, EXEC_TIME_1500); }
void NHDLCD9::cursorBlock() { command(CMD_BLINK_CURSOR_ON, EXEC_TIME_100); }

void NHDLCD9::cursorLeft() { command(CMD_MOVE_CURSOR_LEFT, EXEC_TIME_100); }
void NHDLCD9::cursorRight() { command(CMD_MOVE_CURSOR_RIGHT, EXEC_TIME_100); }

void NHDLCD9::cursorOff() {
  command(CMD_UNDERLINE_CURSOR_OFF, EXEC_TIME_1500);
  command(CMD_BLINK_CURSOR_OFF, EXEC_TIME_1500);
}

void NHDLCD9::backspace() { command(CMD_BACKSPACE, EXEC_TIME_100); }

void NHDLCD9::empty() {
  command(CMD_CLEAR_SCREEN, EXEC_TIME_1500);
}


void NHDLCD9::bright(const uint8_t val) { command(CMD_SET_BRIGHTNESS, val, EXEC_TIME_100); } // Brightness control 0 is off 8 is max
void NHDLCD9::contrast(const uint8_t contrastval) { command(CMD_SET_CONTRAST, contrastval, EXEC_TIME_100); } // Contrast control 0 is none 50 is max

void NHDLCD9::loadCustomCharacter(const uint8_t addr, const uint8_t bitmap[8]) {
  // Ensure the address is valid (0-7)
  // Send the command to load the custom character
  uint32_t current_time = micros();
  if (current_time - last_command_time < last_command_delay) {
    delayMicroseconds((last_command_time + last_command_delay) - current_time);
  }
  write(CMD_PREFIX);            // Send the command prefix (0xFE)
  write(CMD_LOAD_CUSTOM_CHAR);  // Send the load custom character command (0x54)
  write(addr);                  // Send the custom character address

  // Send the 8-byte bitmap for the custom character
  for (uint8_t i = 0; i < 8; i++) {
    write(bitmap[i]);
  }
  // Add delay to ensure the command is processed
  last_command_time = micros();
  last_command_delay = EXEC_TIME_200;
}

void NHDLCD9::displayLeft() { command(CMD_MOVE_DISPLAY_LEFT, EXEC_TIME_100); }
void NHDLCD9::displayRight() { command(CMD_MOVE_DISPLAY_RIGHT, EXEC_TIME_100); }

void NHDLCD9::setBaud(const uint8_t baud) {
  command(CMD_CHANGE_BAUD_RATE, baud, EXEC_TIME_3000);
  // end();
  // delay(_bv[BOUNCE]);
  // switch (baud) {
  //   case 1: begin(300); break;
  //   case 2: begin(1200); break;
  //   case 3: begin(2400); break;
  //   case 4: begin(9600); break;
  //   case 5: begin(14400); break;
  //   case 6: begin(19200); break;
  //   case 7: begin(57600); break;
  //   case 8: begin(115200); break;
  //   default: begin(19200); break;
  // }
  // delay(_bv[BOUNCE]);
  // empty();
}

void NHDLCD9::setI2CAddr(const uint8_t addr) { command(CMD_CHANGE_I2C_ADDRESS, addr, EXEC_TIME_3000); }
void NHDLCD9::displayFW() { command(CMD_DISPLAY_FW_VERSION, EXEC_TIME_4000); }
void NHDLCD9::displayBaud() { command(CMD_DISPLAY_FW_VERSION, EXEC_TIME_4000); } 
void NHDLCD9::displayI2CAddr() { command(CMD_DISPLAY_FW_VERSION, EXEC_TIME_10000); } 

/* ======================================================== */
