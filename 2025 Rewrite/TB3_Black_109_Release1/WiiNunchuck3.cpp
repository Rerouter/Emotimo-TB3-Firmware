/*
* WiiNunchuck Library
*
* This library (should) work with all official and third party nunchucks, including wireless ones.
*
* Written by Grant Emsley - grant@emsley.ca - http://arduino.emsley.ca
*
* Adapted from code at http://www.windmeadow.com/node/42
*/

#include "WiiNunchuck3.h"

constexpr bool USE_NEW_WAY_INIT = true;  // see http://wiibrew.org/wiki/Wiimote#The_New_Way.  If set to false, this library won't work for third party nunchucks
constexpr uint8_t WII_TELEGRAM_LEN = 6;
constexpr uint8_t NUNCHUK_DEVICE_ID = 0x52;
constexpr uint32_t TWI_FREQ_NUNCHUCK = 400000L;

constexpr uint8_t pwrpin = PORTC3;
constexpr uint8_t gndpin = PORTC2;

constexpr bool DEBUG_NC = false;  // Use serial port debugging if defined.

int16_t centeredJoyX = 127;              // Set Sane Defaults if Calibrate() is never called
int16_t centeredJoyY = 127;

WiiNunchuck3::WiiNunchuck3() {
}



void WiiNunchuck3::clearTwiInputBuffer() {
  // clear the receive buffer from any partial data
  while (Wire.available())
    Wire.read();
}


void WiiNunchuck3::setpowerpins() {
  // Uses port C (analog in) pins as power & ground for Nunchuck
  DDRC |= _BV(pwrpin) | _BV(gndpin);
  PORTC &= ~_BV(gndpin);
  PORTC |= _BV(pwrpin);
  delay(100);  // wait for things to stabilize
}

uint8_t WiiNunchuck3::request_measurement() {
    uint8_t retry_count = 0;
    uint8_t max_retries = 3; // Maximum retries
    uint8_t error;

    while (retry_count < max_retries) {
        Wire.beginTransmission(NUNCHUK_DEVICE_ID);
        Wire.write(0x00);
        error = Wire.endTransmission();
        if (error == 0) {
            return 0; // Success
        }
        retry_count++;
    }
    return error; // Return the last error after retries
}

__attribute__ ((const))
__attribute__ ((warn_unused_result))
char WiiNunchuck3::decode_byte(const char x) {
  // Decode data format that original Nunchuck uses with old init sequence. This never worked with
  // other controllers (e.g. wireless Nunchuck from other vendors)
  if (!USE_NEW_WAY_INIT) {
    return (x ^ 0x17) + 0x17;
  }
  return x;
}

__attribute__ ((warn_unused_result))
bool WiiNunchuck3::getData() {
  bool error_flag = false;
  // Receive data back from the nunchuck,
  // returns 1 on successful read. returns 0 on failure
  uint8_t bytes_available = Wire.requestFrom(NUNCHUK_DEVICE_ID, WII_TELEGRAM_LEN);  // request data from nunchuck
  if (bytes_available != WII_TELEGRAM_LEN) {
    error_flag = 1;
  } // incorrect return size

  uint8_t cnt;
  for (cnt = 0; (cnt < WII_TELEGRAM_LEN) && Wire.available(); cnt++) {
    if (error_flag) { Wire.read(); }
    else            { data.buffer[cnt] = decode_byte(Wire.read()); }  // receive byte as an integer
  }

  if (request_measurement()) { // Request the measurement of the next value takes about 3ms to happen.
    Nunchuck.init(0);
  }
  //else if (cnt >= WII_TELEGRAM_LEN) { return 0; }  // success
  return error_flag;  //failure
}


void WiiNunchuck3::init(const bool power) {
  // do we need to power the nunchuck?
  if (power) {
    setpowerpins();
  }

  // initialize the I2C system, join the I2C bus,
  // and tell the nunchuck we're talking to it
  uint8_t timeout = 0;  // never timeout
  Wire.begin();                // join i2c bus as master
                               // we need to switch the TWI speed, because the nunchuck uses Fast-TWI
                               // normally set in hardware\libraries\Wire\utility\twi.c twi_init()
                               // this is the way of doing it without modifying the original files
  TWBR = ((16000000 / TWI_FREQ_NUNCHUCK) - 16) / 2;

  byte rc = 1;
  if (USE_NEW_WAY_INIT) {
    // disable encryption
    // look at <http://wiibrew.org/wiki/Wiimote#The_New_Way> at "The New Way"

    uint32_t time = millis();
    do {
      delay(1);
      Wire.beginTransmission(NUNCHUK_DEVICE_ID);  // transmit to device 0x52
      Wire.write(0xF0);                           // sends memory address
      Wire.write(0x55);                           // sends data.
      if (Wire.endTransmission() == 0)            // stop transmitting
      {
        Wire.beginTransmission(NUNCHUK_DEVICE_ID);  // transmit to device 0x52
        Wire.write(0xFB);                           // sends memory address
        Wire.write(0x00);                           // sends sent a zero.
        if (Wire.endTransmission() == 0)            // stop transmitting
        {
          rc = 0;
        }
      }
    } while (rc != 0 && (!timeout || ((millis() - time) < timeout)));
    
  }
  else {
    // look at <http://wiibrew.org/wiki/Wiimote#The_Old_Way> at "The Old Way"
    Wire.beginTransmission(NUNCHUK_DEVICE_ID);  // transmit to device 0x52
    Wire.write(0x40);                           // sends memory address
    Wire.write(0x00);                           // sends sent a zero.
    Wire.endTransmission();
  } 
  // Request first conversion to place valid data in the registers
  request_measurement();
  delay(5);
}


void WiiNunchuck3::printData() {
  static uint16_t i = 0;
  int8_t joy_x_axis = joyx();
  int8_t joy_y_axis = joyy();
  int16_t accel_x_axis = accelx();
  int16_t accel_y_axis = accely();
  int16_t accel_z_axis = accelz();

  bool z_button = !data.buttons.zButton;
  bool c_button = !data.buttons.cButton;

  Serial.print(i, DEC);  Serial.print('\t');

  Serial.print("joy:");  Serial.print(joy_x_axis, DEC);
  Serial.print(',');     Serial.print(joy_y_axis, DEC);
  Serial.print('\t');

  Serial.print("acc:");  Serial.print(accel_x_axis, DEC);
  Serial.print(',');     Serial.print(accel_y_axis, DEC);
  Serial.print(',');     Serial.print(accel_z_axis, DEC);
  Serial.print('\t');

  Serial.print("but:");  Serial.print(z_button, DEC);
  Serial.print(',');     Serial.print(c_button, DEC);

  Serial.print("\r\n");  // newline
  i++;
}

__attribute__ ((warn_unused_result))
bool WiiNunchuck3::zbutton() {  // Returns true if the Z button is pressed
  return !data.buttons.zButton;
}

__attribute__ ((warn_unused_result))
bool WiiNunchuck3::cbutton() {  // Returns true if the C button is pressed
  return !data.buttons.cButton;
}

__attribute__ ((warn_unused_result))
uint8_t WiiNunchuck3::joyx() {  //returns value of x-axis joystick
  return data.joyX;
}

__attribute__ ((warn_unused_result))
uint8_t WiiNunchuck3::joyy() {  // returns value of y-axis joystick
  return data.joyY;
}

__attribute__ ((warn_unused_result))
uint16_t WiiNunchuck3::accelx() { // Calculate the x-axis accelerometer value
   return (data.accelX * 4) | data.buttons.accelXOverflow;
}

__attribute__ ((warn_unused_result))
uint16_t WiiNunchuck3::accely() { // Calculate the y-axis accelerometer value
  return (data.accelY * 4) | data.buttons.accelYOverflow;
}

__attribute__ ((warn_unused_result))
uint16_t WiiNunchuck3::accelz() {  // Calculate the z-axis accelerometer value
  return (data.accelZ * 4) | data.buttons.accelZOverflow;
}

__attribute__ ((warn_unused_result))
int16_t WiiNunchuck3::vibration() {
    // [x,y,z][min,max] - min's initialized to 1024, max initialized to 0
    int16_t accelerometers[3][2] = { { 1024, 0 }, { 1024, 0 }, { 1024, 0 } };

    // Gather 10 samples of accelerometer data
    for (uint8_t loop = 0; loop < 10; loop++) {
        while(getData()) {}

        int16_t values[3] = { accelx(), accely(), accelz() };

        // Update min/max for each axis
        for (uint8_t i = 0; i < 3; i++) {
            if (values[i] < accelerometers[i][0]) {
                accelerometers[i][0] = values[i];
            }
            if (values[i] > accelerometers[i][1]) {
                accelerometers[i][1] = values[i];
            }
        }
    }

    // Calculate range for each axis
    int16_t diffs[3];
    for (uint8_t i = 0; i < 3; i++) {
        diffs[i] = accelerometers[i][1] - accelerometers[i][0];
    }

    // Return total vibration
    return diffs[0] + diffs[1] + diffs[2];
}


void WiiNunchuck3::calibrate() {
  // this function returns the average values over 10 samples, to be used for calibrating the nunchuck joystick
  // the joystick must be at rest when this is done
  while(getData()) {}
  centeredJoyX = joyx();
  centeredJoyY = joyy();
}

__attribute__ ((warn_unused_result))
int8_t WiiNunchuck3::digitalx(const int16_t threshold) {
  // Returns -1 for joystick left, 1 for joystick right, 0 for centered
  int16_t calibratedJoyX = joyx() - centeredJoyX;
  if (calibratedJoyX < -threshold) { return -1; }  // Left
  else if (calibratedJoyX > threshold) {
    return 1;
  }          // Right
  return 0;  // Centered
}

__attribute__ ((warn_unused_result))
int8_t WiiNunchuck3::digitaly(const int16_t threshold) {
  // Returns -1 for joystick up, 1 for joystick down, 0 for centered
  int16_t calibratedJoyY = static_cast<int16_t>(joyy() - centeredJoyY);
  if (calibratedJoyY < -threshold) { return -1; }  // Up
  else if (calibratedJoyY > threshold) {
    return 1;
  }          // Down
  return 0;  // Centered
}

// create the object
WiiNunchuck3 Nunchuck;