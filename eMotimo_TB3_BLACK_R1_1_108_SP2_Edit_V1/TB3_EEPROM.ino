/*
  ========================================
  EEPROM write/read functions
  ========================================

*/
/*

    This code started with Chris Church's OpenMoco Time Lapse Engine and was modified by Brian Burling
    to work with the eMotimo products.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


unsigned long  check_version() {
  unsigned int check_version_from_eeprom;
  eeprom_read(1, check_version_from_eeprom);
  return check_version_from_eeprom;
}


boolean eeprom_saved()
{
  // read eeprom saved status
  byte saved = EEPROM.read(0);

  // EEPROM memory is by default set to 1, so we
  // return zero if no data written data to eeprom
  return ( ! saved );
}

void eeprom_saved( boolean saved )
{
  // set eeprom saved status
  // EEPROM memory is by default set to 1, so we
  // set it to zero if we've written data to eeprom

  EEPROM.write(0, !saved);
}

void eeprom_write( uint16_t pos, bool& val )
{
  EEPROM.write(pos, val);
  // indicate that memory has been saved
  eeprom_saved(true);
}

void eeprom_write( uint16_t pos, uint8_t& val )
{
  EEPROM.write(pos, val);
  // indicate that memory has been saved
  eeprom_saved(true);
}


void eeprom_write( uint16_t pos, int8_t& val )
{
  EEPROM.write(pos, val);
  // indicate that memory has been saved
  eeprom_saved(true);
}


void eeprom_write( uint16_t pos, uint16_t& val )
{
  byte* p = (byte*)(void*)&val;
  eeprom_write_final(pos, *p, sizeof(uint16_t));
}


void eeprom_write( uint16_t pos, int16_t& val )
{
  byte* p = (byte*)(void*)&val;
  eeprom_write_final(pos, *p, sizeof(int16_t));
}


void eeprom_write( uint16_t pos, uint32_t& val )
{
  byte* p = (byte*)(void*)&val;
  eeprom_write_final(pos, *p, sizeof(uint32_t));
}


void eeprom_write( uint16_t pos, int32_t& val )
{
  byte* p = (byte*)(void*)&val;
  eeprom_write_final(pos, *p, sizeof(int32_t));
}


void eeprom_write( uint16_t pos, float& val )
{
  byte* p = (byte*)(void*)&val;
  eeprom_write_final(pos, *p, sizeof(float));
}


void eeprom_write_final( uint16_t pos, byte& val, byte len )
{
  byte* p = (byte*)(void*)&val;
  for ( uint8_t i = 0; i < len; i++ )
    EEPROM.write(pos++, *p++);

  // indicate that memory has been saved
  eeprom_saved(true);
}


// read functions

void eeprom_read( uint16_t pos, bool& val )
{
  val = EEPROM.read(pos);
}


void eeprom_read( uint16_t pos, uint8_t& val )
{
  val = EEPROM.read(pos);
}


void eeprom_read( uint16_t pos, int8_t& val )
{
  val = EEPROM.read(pos);
}


void eeprom_read( uint16_t pos, int16_t& val )
{
  byte* p = (byte*)(void*)&val;
  eeprom_read(pos, *p, sizeof(int16_t));
}


void eeprom_read( uint16_t pos, uint16_t& val )
{
  byte* p = (byte*)(void*)&val;
  eeprom_read(pos, *p, sizeof(uint16_t));
}


void eeprom_read( uint16_t pos, int32_t& val )
{
  byte* p = (byte*)(void*)&val;
  eeprom_read(pos, *p, sizeof(int32_t));
}


void eeprom_read( uint16_t pos, uint32_t& val )
{
  byte* p = (byte*)(void*)&val;
  eeprom_read(pos, *p, sizeof(uint32_t));
}


void eeprom_read( uint16_t pos, float& val )
{
  byte* p = (byte*)(void*)&val;
  eeprom_read(pos, *p, sizeof(float));
}


void eeprom_read( uint16_t pos, byte& val, byte len )
{
  byte* p = (byte*)(void*)&val;
  for (uint8_t i = 0; i < len; i++)
    *p++ = EEPROM.read(pos++);
}


void set_defaults_in_ram()
{
  //build_version=10822;//              Serial.println(build_version);
  redraw = 1; //			Serial.println(first_time);
  intval = 10; //			Serial.println(intval); //default to video mode
  interval = 2000; //			Serial.println(interval);
  camera_exp_tm = 100; //	        Serial.println(camera_exp_tm);
  camera_fired = 0; //			Serial.println(camera_fired);
  camera_moving_shots = 240; //		Serial.println(camera_moving_shots);
  camera_total_shots = 0; //		Serial.println(camera_total_shots);
  overaldur = 30; //			Serial.println(overaldur);
  prefire_time = 1; //			Serial.println(prefire_time);
  rampval = 50; //			Serial.println(rampval);
  static_tm = 1; //			Serial.println(static_tm);
  lead_in = 1; //			Serial.println(lead_in);
  lead_out = 1; //			Serial.println(lead_out);
  motor_steps_pt[2][0] = 0.0; //		Serial.println(motor_steps_pt[2][0]);
  motor_steps_pt[2][1] = 0.0; //		Serial.println(motor_steps_pt[2][1]);
  motor_steps_pt[2][2] = 0.0; //		Serial.println(motor_steps_pt[2][2]);
  keyframe[0][0] = 0; //			Serial.println(keyframe[0][0]);
  keyframe[0][1] = 0; //			Serial.println(keyframe[0][1]);
  keyframe[0][2] = 0; //			Serial.println(keyframe[0][2]);
  keyframe[0][3] = 0; //			Serial.println(keyframe[0][3]);
  keyframe[0][4] = 0; //			Serial.println(keyframe[0][4]);
  keyframe[0][5] = 0; //			Serial.println(keyframe[0][5]);
  linear_steps_per_shot[0] = 0.0; //	Serial.println(linear_steps_per_shot[0]);
  linear_steps_per_shot[1] = 0.0; //	Serial.println(linear_steps_per_shot[1]);
  linear_steps_per_shot[2] = 0.0; //	Serial.println(linear_steps_per_shot[2]);
  ramp_params_steps[0] = 0.0; //	        Serial.println(ramp_params_steps[0]);
  ramp_params_steps[1] = 0.0; //		Serial.println(ramp_params_steps[1]);
  ramp_params_steps[2] = 0.0; //		Serial.println(ramp_params_steps[2]);
  current_steps.x = 0.0; //		Serial.println(current_steps.x);
  current_steps.y = 0.0; //		Serial.println(current_steps.y);
  current_steps.z = 0.0; //		Serial.println(current_steps.z);
  //progstep = 0; //			Serial.println(progstep);
  //Program_Engaged = 0; //		Serial.println(Program_Engaged);
  //POWERSAVE_PT = 3; //			Serial.println(POWERSAVE_PT);
  //POWERSAVE_AUX = 3; //		Serial.println(POWERSAVE_AUX);
  //AUX_ON = 1; //			Serial.println(AUX_ON);
  //PAUSE_ENABLED = 0; //		Serial.println(PAUSE_ENABLED);
  //LCD_BRIGHTNESS_DURING_RUN = 3; //	Serial.println(LCD_BRIGHTNESS_DURING_RUN);
}


void set_defaults_in_setup()
{
  POWERSAVE_PT = 3; //			Serial.println(POWERSAVE_PT);
  POWERSAVE_AUX = 3; //			Serial.println(POWERSAVE_AUX);
  AUX_ON = 1; //			        Serial.println(AUX_ON);
  PAUSE_ENABLED = 0; //			Serial.println(PAUSE_ENABLED);
  LCD_BRIGHTNESS_DURING_RUN = 3; //	Serial.println(LCD_BRIGHTNESS_DURING_RUN);

  eeprom_write(1, build_version);
  eeprom_write(98, POWERSAVE_PT);
  eeprom_write(100, POWERSAVE_AUX);
  eeprom_write(102, AUX_ON);
  eeprom_write(103, PAUSE_ENABLED);
  eeprom_write(104, LCD_BRIGHTNESS_DURING_RUN);
}


void write_defaults_to_eeprom_memory()
{
  set_defaults_in_ram();
  write_all_ram_to_eeprom();
}


void write_all_ram_to_eeprom()
{
  //eeprom_write(1, build_version);
  //eeprom_write(5, first_time);
  eeprom_write(7, intval);
  eeprom_write(9, interval);
  eeprom_write(13, camera_exp_tm);
  eeprom_write(17, camera_fired);
  eeprom_write(19, camera_moving_shots);
  eeprom_write(21, camera_total_shots);
  eeprom_write(23, overaldur);
  eeprom_write(25, prefire_time);
  eeprom_write(27, rampval);
  eeprom_write(29, static_tm);
  eeprom_write(31, lead_in);
  eeprom_write(33, lead_out);
  eeprom_write(35, motor_steps_pt[2][0]);
  eeprom_write(39, motor_steps_pt[2][1]);
  eeprom_write(43, motor_steps_pt[2][2]);
  eeprom_write(47, keyframe[0][0]);
  eeprom_write(49, keyframe[0][1]);
  eeprom_write(51, keyframe[0][2]);
  eeprom_write(53, keyframe[0][3]);
  eeprom_write(55, keyframe[0][4]);
  eeprom_write(57, keyframe[0][5]);
  eeprom_write(59, linear_steps_per_shot[0]);
  eeprom_write(63, linear_steps_per_shot[1]);
  eeprom_write(67, linear_steps_per_shot[2]);
  eeprom_write(71, ramp_params_steps[0]);
  eeprom_write(75, ramp_params_steps[1]);
  eeprom_write(79, ramp_params_steps[2]);
  eeprom_write(83, current_steps.x);
  eeprom_write(87, current_steps.y);
  eeprom_write(91, current_steps.z);
  eeprom_write(95, progstep);
  eeprom_write(97, Program_Engaged);
  //eeprom_write(98, POWERSAVE_PT);
  //eeprom_write(100, POWERSAVE_AUX);
  //eeprom_write(102, AUX_ON);
  //eeprom_write(103, PAUSE_ENABLED);
  //eeprom_write(104, LCD_BRIGHTNESS_DURING_RUN);
}


// restore memory

void restore_from_eeprom_memory()
{
  //eeprom_read(1, build_version);
  //eeprom_read(5, first_time);
  eeprom_read(7, intval);
  eeprom_read(9, interval);
  eeprom_read(13, camera_exp_tm);
  eeprom_read(17, camera_fired);
  eeprom_read(19, camera_moving_shots);
  eeprom_read(21, camera_total_shots);
  eeprom_read(23, overaldur);
  eeprom_read(25, prefire_time);
  eeprom_read(27, rampval);
  eeprom_read(29, static_tm);
  eeprom_read(31, lead_in);
  eeprom_read(33, lead_out);
  eeprom_read(35, motor_steps_pt[2][0]);
  eeprom_read(39, motor_steps_pt[2][1]);
  eeprom_read(43, motor_steps_pt[2][2]);
  eeprom_read(47, keyframe[0][0]);
  eeprom_read(49, keyframe[0][1]);
  eeprom_read(51, keyframe[0][2]);
  eeprom_read(53, keyframe[0][3]);
  eeprom_read(55, keyframe[0][4]);
  eeprom_read(57, keyframe[0][5]);
  eeprom_read(59, linear_steps_per_shot[0]);
  eeprom_read(63, linear_steps_per_shot[1]);
  eeprom_read(67, linear_steps_per_shot[2]);
  eeprom_read(71, ramp_params_steps[0]);
  eeprom_read(75, ramp_params_steps[1]);
  eeprom_read(79, ramp_params_steps[2]);
  eeprom_read(83, current_steps.x);
  eeprom_read(87, current_steps.y);
  eeprom_read(91, current_steps.z);
  //eeprom_read(95, progstep);
  eeprom_read(97, Program_Engaged);
  eeprom_read(98, POWERSAVE_PT);
  eeprom_read(100, POWERSAVE_AUX);
  eeprom_read(102, AUX_ON);
  eeprom_read(103, PAUSE_ENABLED);
  eeprom_read(104, LCD_BRIGHTNESS_DURING_RUN);
}


void review_RAM_Contents()
{
  Serial.println(build_version);
  Serial.println(redraw);
  Serial.println(intval);
  Serial.println(interval);
  Serial.println(camera_exp_tm);
  Serial.println(camera_fired);
  Serial.println(camera_moving_shots);
  Serial.println(camera_total_shots);
  Serial.println(overaldur);
  Serial.println(prefire_time);
  Serial.println(rampval);
  Serial.println(static_tm);
  Serial.println(lead_in);
  Serial.println(lead_out);
  Serial.println(motor_steps_pt[2][0]);
  Serial.println(motor_steps_pt[2][1]);
  Serial.println(motor_steps_pt[2][2]);
  Serial.println(keyframe[0][0]);
  Serial.println(keyframe[0][1]);
  Serial.println(keyframe[0][2]);
  Serial.println(keyframe[0][3]);
  Serial.println(keyframe[0][4]);
  Serial.println(keyframe[0][5]);
  Serial.println(linear_steps_per_shot[0]);
  Serial.println(linear_steps_per_shot[1]);
  Serial.println(linear_steps_per_shot[2]);
  Serial.println(ramp_params_steps[0]);
  Serial.println(ramp_params_steps[1]);
  Serial.println(ramp_params_steps[2]);
  Serial.println(current_steps.x);
  Serial.println(current_steps.y);
  Serial.println(current_steps.z);
  Serial.println(progstep);
  Serial.println(Program_Engaged);
  Serial.println(POWERSAVE_PT);
  Serial.println(POWERSAVE_AUX);
  Serial.println(AUX_ON);
  Serial.println(PAUSE_ENABLED);
  Serial.println(LCD_BRIGHTNESS_DURING_RUN);
}