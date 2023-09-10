#include "WiiNunchuck.h"

#define WII_NUNCHUCK_TWI_ADR ((uint8_t)0x52)

WiiNunchuck::WiiNunchuck() : error_count(0) {
  initNunchuck();
}

void WiiNunchuck::initNunchuck() {
  Wire.begin(); // join I2C bus
  Wire.beginTransmission(WII_NUNCHUCK_TWI_ADR);
  Wire.write((uint8_t)0xF0);
  Wire.write((uint8_t)0x55);
  Wire.endTransmission();
  delay(1);
  Wire.beginTransmission(WII_NUNCHUCK_TWI_ADR);
  Wire.write((uint8_t)0xFB);
  Wire.write((uint8_t)0x00);
  Wire.endTransmission();
}

void WiiNunchuck::requestData() {
  uint8_t temp_buf[6];
  
  Wire.beginTransmission(WII_NUNCHUCK_TWI_ADR);
  Wire.write((uint8_t)0x00);
  Wire.endTransmission();
  Wire.requestFrom(WII_NUNCHUCK_TWI_ADR, uint8_t(6));
  
  uint8_t receivedChecksum = 0;
  for (uint8_t i = 0; i < sizeof(temp_buf); i++) {
    temp_buf[i] = Wire.read();
    if (i < 5) {
      receivedChecksum ^= temp_buf[i];
    }
  }
  
  if (temp_buf[0] == 0 && temp_buf[1] == 0 && temp_buf[2] == 0 && temp_buf[3] == 0 && temp_buf[4] == 0) {
    if (error_count < 255) error_count++;
    return; 
  }
  
  if (temp_buf[0] == 255 && temp_buf[1] == 255 && temp_buf[2] == 255 && temp_buf[3] == 255 && temp_buf[4] == 255) {
    if (error_count < 255) error_count++;
    return; 
  }
  
  if (temp_buf[0] == 0 && temp_buf[1] == 0) {
    if (error_count < 255) error_count++;
    return; 
  }
  
  if (receivedChecksum == ((temp_buf[5] >> 2) & 0b11)) {
    error_count = 0;
    updateBuffer(temp_buf);
  }
}

void WiiNunchuck::updateBuffer(const uint8_t buf[6]) {
  memcpy(nunchuck_buf, buf, sizeof(nunchuck_buf));
}

uint8_t WiiNunchuck::getErrorCount() {
  return error_count;
}

uint8_t WiiNunchuck::getJoyX() {
  return nunchuck_buf[0];
}

uint8_t WiiNunchuck::getJoyY() {
  return nunchuck_buf[1];
}

uint16_t WiiNunchuck::getAccelX() {
  uint16_t acc_x_axis = nunchuck_buf[2] << 2;
  acc_x_axis += (nunchuck_buf[5] >> 2) & 3;
  return acc_x_axis;
}

uint16_t WiiNunchuck::getAccelY() {
  uint16_t accel_y_axis = nunchuck_buf[3] << 2;
  accel_y_axis += (nunchuck_buf[5] >> 4) & 3;
  return accel_y_axis;
}

uint16_t WiiNunchuck::getAccelZ() {
  uint16_t accel_z_axis = nunchuck_buf[4] << 2;
  accel_z_axis += (nunchuck_buf[5] >> 6) & 3;
  return accel_z_axis;
}

bool WiiNunchuck::getButtonC() {
  return !(nunchuck_buf[5] & 1);
}

bool WiiNunchuck::getButtonZ() {
  return !((nunchuck_buf[5] >> 1) & 1);
}
