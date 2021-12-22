/*
* WiiNunchuck Library
*
* This library (should) work with all official and third party nunchucks, including wireless ones.
*
* Written by Grant Emsley - grant@emsley.ca - http://arduino.emsley.ca
*
* Adapted from code at http://www.windmeadow.com/node/42
*/

#include <Arduino.h>
#include "WiiNunchuck3.h"
#include "Wire.h"

#undef int
#include <stdio.h>

#define USE_NEW_WAY_INIT 1 // see http://wiibrew.org/wiki/Wiimote#The_New_Way.  If set to 0, this library won't work for third party nunchucks
#define WII_IDENT_LEN ((byte)6)
#define WII_TELEGRAM_LEN ((byte)6)
#define WII_NUNCHUCK_TWI_ADR ((byte)0x52)


uint8_t nunchuck_buf[WII_TELEGRAM_LEN]; // array to store arduino output

uint8_t centeredJoyX = 0;
uint8_t centeredJoyY = 0;

WiiNunchuck3::WiiNunchuck3() {

}


void WiiNunchuck3::init(bool power)
{
  // do we need to power the nunchuck, only for orange wired nunchuck
  if (power) {
    setpowerpins();
  }

  // initialize the I2C system, join the I2C bus,
  // and tell the nunchuck we're talking to it
  uint8_t timeout = 0; // never timeout
  Wire.begin();       // join i2c bus as master
  // we need to switch the TWI speed, because the nunchuck uses Fast-TWI
  // normally set in hardware\libraries\Wire\utility\twi.c twi_init()
  // this is the way of doing it without modifying the original files
#define TWI_FREQ_NUNCHUCK 400000L
  TWBR = ((16000000 / TWI_FREQ_NUNCHUCK) - 16) / 2;

#ifndef USE_NEW_WAY_INIT
  // look at <http://wiibrew.org/wiki/Wiimote#The_Old_Way> at "The Old Way"
  Wire.beginTransmission (WII_NUNCHUCK_TWI_ADR); // transmit to device 0x52
  //Wire.write (0x40); // sends memory address
  //Wire.write (0x00); // sends sent a zero.
  Wire.write ((uint8_t)64); //this is x40
  Wire.write ((uint8_t)0); // sends sent a zero.
  Wire.endTransmission (); // stop transmitting
#else
  // disable encryption
  // look at <http://wiibrew.org/wiki/Wiimote#The_New_Way> at "The New Way"

  uint32_t time = millis();

  do {
    Wire.beginTransmission (WII_NUNCHUCK_TWI_ADR); // transmit to device 0x52
    Wire.write ((uint8_t)0xF0); // sends memory address
    Wire.write ((uint8_t)0x55); // sends data.
    
    if (Wire.endTransmission() == 0) // stop transmitting
    {
      Wire.beginTransmission (WII_NUNCHUCK_TWI_ADR); // transmit to device 0x52
      Wire.write ((uint8_t)0xFB); // sends memory address
      Wire.write ((uint8_t)0x00); // sends sent a zero.
      if (Wire.endTransmission () == 0) // stop transmitting
      {
        break;
      }
    }
  }
  while ((!timeout || ((millis() - time) < timeout)));
#endif
  // Sometimes the first request seems to get garbage data.
  // Get some data now so when the main program calls getData(), it will get good data.
  getData();
}


static void WiiNunchuck3::setpowerpins()
{
  // Uses port C (analog in) pins as power & ground for Nunchuck
#define pwrpin PORTC3
#define gndpin PORTC2
  DDRC |= _BV(pwrpin) | _BV(gndpin);
  PORTC &= ~ _BV(gndpin);
  PORTC |=  _BV(pwrpin);
  delay(100);  // wait for things to stabilize
}


static void WiiNunchuck3::send_zero () {
  // I don't know why, but it only works correct when doing this exactly 3 times
  // otherwise only each 3rd call reads data from the controller (cnt will be 0 the other times)
  for (uint8_t i = 0; i < 3; i++) {
    Wire.beginTransmission (WII_NUNCHUCK_TWI_ADR); // transmit to device 0x52
    //Wire.write (0x00); // sends one byte
    Wire.write ((uint8_t)0); // sends one byte
    Wire.endTransmission (); // stop transmitting
  }
}


static void WiiNunchuck3::clearTwiInputBuffer(void)
{
  // clear the receive buffer from any partial data
  while ( Wire.available ())
    Wire.read ();
}


static void WiiNunchuck3::send_request()
{
  // Send a request for data to the nunchuck
  Wire.beginTransmission(WII_NUNCHUCK_TWI_ADR);// transmit to device 0x52
  //Wire.write(0x00);// sends one byte
  Wire.write ((uint8_t)0);
  Wire.endTransmission();// stop transmitting
}


char WiiNunchuck3::decode_byte (char x)
{
  // Decode data format that original Nunchuck uses with old init sequence. This never worked with
  // other controllers (e.g. wireless Nunchuck from other vendors)
#ifndef USE_NEW_WAY_INIT
  x = (x ^ 0x17) + 0x17;
#endif
  return x;
}


uint8_t WiiNunchuck3::getData()
{
  static uint8_t failcount = 0;
  static uint8_t last = 0;
  uint8_t chksum = 0;
  uint8_t cnt = 0;
  
  // Receive data back from the nunchuck,
  // returns 1 on successful read. returns 0 on failure
  
  Wire.requestFrom (WII_NUNCHUCK_TWI_ADR, WII_TELEGRAM_LEN); // request data from nunchuck
  for (cnt = 0; (cnt < WII_TELEGRAM_LEN && Wire.available()); cnt++)
  {
    nunchuck_buf[cnt] = decode_byte (Wire.read ()); // receive byte as an integer
    chksum ^= nunchuck_buf[cnt];
  }
  clearTwiInputBuffer();
  if (cnt >= WII_TELEGRAM_LEN && chksum != last) {
    send_zero();
    if(chksum !=0 && chksum != 255) {last = chksum;  failcount = 0;}
    return 0;   // success
  }

  if(failcount < 250) { failcount++; }
  return failcount; //failure
}


void WiiNunchuck3::clearData()
{
    nunchuck_buf[0] = 127; // reset joy_x_axis
    nunchuck_buf[1] = 127; // reset joy_y_axis
    nunchuck_buf[2] = 128; // reset acc_x_axis
    nunchuck_buf[3] = 128; // reset accel_y_axis
    nunchuck_buf[4] = 128; // reset accel_z_axis
    nunchuck_buf[5] = 3;   // reset buttons
}


void WiiNunchuck3::printData() const
{
  // Print the input data we have recieved
  // accel data is 10 bits long
  // so we read 8 bits, then we have to add
  // on the last 2 bits.  That is why I
  // multiply them by 2 * 2

  uint8_t joy_x_axis = nunchuck_buf[0];
  uint8_t joy_y_axis = nunchuck_buf[1];
  uint16_t acc_x_axis = nunchuck_buf[2] << 2;
  uint16_t accel_y_axis = nunchuck_buf[3] << 2;
  uint16_t accel_z_axis = nunchuck_buf[4] << 2;

  bool z_button = 0;
  bool c_button = 0;

  // byte nunchuck_buf[5] contains bits for z and c buttons
  // it also contains the least significant bits for the accelerometer data
  // so we have to check each bit of byte outbuf[5]
  if ((nunchuck_buf[5] >> 0) & 1)   z_button = 1;
  if ((nunchuck_buf[5] >> 1) & 1)   c_button = 1;

  acc_x_axis += ((nunchuck_buf[5] >> 2) & 3);

  accel_y_axis += ((nunchuck_buf[5] >> 4) & 3);

  accel_z_axis += ((nunchuck_buf[5] >> 6) & 3);

  static int i = 0;
  Serial.print(i, DEC);
  Serial.print("\t");

  Serial.print("joy:");
  Serial.print(joy_x_axis, DEC);
  Serial.print(",");
  Serial.print(joy_y_axis, DEC);
  Serial.print("  \t");

  Serial.print("acc:");
  Serial.print(acc_x_axis, DEC);
  Serial.print(",");
  Serial.print(accel_y_axis, DEC);
  Serial.print(",");
  Serial.print(accel_z_axis, DEC);
  Serial.print("\t");

  Serial.print("but:");
  Serial.print(z_button, DEC);
  Serial.print(",");
  Serial.print(c_button, DEC);

  Serial.print("\r\n");  // newline
  i++;
}


bool WiiNunchuck3::zbutton() const
{
  // returns zbutton state: 1 = pressed, 0 = notpressed
  return ((nunchuck_buf[5] >> 0) & 1) ? 0 : 1;  // Inverting the bit to the button state
}


bool WiiNunchuck3::cbutton() const
{
  // returns cbutton state: 1 = pressed, 0 = notpressed
  return ((nunchuck_buf[5] >> 1) & 1) ? 0 : 1;  // Inverting the bit to the button state
}


uint8_t WiiNunchuck3::joyx() const
{
  return nunchuck_buf[0];
}


uint8_t WiiNunchuck3::joyy() const
{
  return nunchuck_buf[1];
}


uint16_t WiiNunchuck3::accelx() const
{
  // returns value of x-axis accelerometer
  uint16_t acc_x_axis = nunchuck_buf[2] << 2;
  acc_x_axis += ((nunchuck_buf[5] >> 2) & 3);
  return acc_x_axis;
}


uint16_t WiiNunchuck3::accely() const
{
  // returns value of y-axis accelerometer
  uint16_t accel_y_axis = nunchuck_buf[3] << 2;
  accel_y_axis += ((nunchuck_buf[5] >> 4) & 3);
  return accel_y_axis;
}


uint16_t WiiNunchuck3::accelz() const
{
  // returns value of z-axis accelerometer
  uint16_t accel_z_axis = nunchuck_buf[4] << 2;
  accel_z_axis += ((nunchuck_buf[5] >> 6) & 3);
  return accel_z_axis;
}


int16_t WiiNunchuck3::vibration()
{
  // calculates the "total vibration"
  // does NOT require a getdata() call first
  // takes 10 samples, finds the min and max, figures out the range for each accelerometer and adds them
  uint16_t accelerometers[3][2] = {{1024, 0}, {1024, 0}, {1024, 0}}; // [x,y,z][min,max] - min's initialized to 1024, max initialized to 0

  // gather 10 samples of accelerometer data, and record the highest and lowest values
  for (uint8_t loop = 0; loop < 10; loop++) {
    getData();

    uint16_t x = accelx();
    uint16_t y = accely();
    uint16_t z = accelz();

    if (x < accelerometers[0][0])	accelerometers[0][0] = x;
    if (x > accelerometers[0][1])	accelerometers[0][1] = x;

    if (y < accelerometers[1][0])	accelerometers[1][0] = y;
    if (y > accelerometers[1][1])	accelerometers[1][1] = y;

    if (z < accelerometers[2][0])	accelerometers[2][0] = z;
    if (z > accelerometers[2][1])	accelerometers[2][1] = z;
  }
  int16_t xdiff = accelerometers[0][1] - accelerometers[0][0];
  int16_t ydiff = accelerometers[1][1] - accelerometers[1][0];
  int16_t zdiff = accelerometers[2][1] - accelerometers[2][0];

  return xdiff + ydiff + zdiff;
}


void WiiNunchuck3::calibrate()
{
  // this function returns the average values over 10 samples, to be used for calibrating the nunchuck joystick
  // the joystick must be at rest when this is done
  getData();
  centeredJoyX = joyx();
  centeredJoyY = joyy();
}


int8_t WiiNunchuck3::digitalx(int8_t threshold) const
{
  // returns 0 for centered, 1 for joystick left, o for joystick right
  // threshold is how far from center it has to be
  int8_t calibratedjoyx = joyx() - centeredJoyX;
  if (calibratedjoyx < (threshold * -1) ) {
    return -1;
  } else if (calibratedjoyx > threshold) {
    return 1;
  } else {
    return 0;
  }
}


int8_t WiiNunchuck3::digitaly(int8_t threshold) const
{
  // returns 0 for centered, 1 for joystick up, 2 for joystick down
  // threshold is how far from center it has to be
  int8_t calibratedjoyy = joyy() - centeredJoyY;
  if (calibratedjoyy < (threshold * -1) ) {
    return 1;
  } else if (calibratedjoyy > threshold) {
    return -1;
  } else 	{
    return 0;
  }
}


// create the object
WiiNunchuck3 Nunchuck;
