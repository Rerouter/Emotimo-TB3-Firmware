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
	return( ! saved );
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
	for( uint8_t i = 0; i < len; i++ )
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
	for(uint8_t i = 0; i < len; i++) 
		*p++ = EEPROM.read(pos++);
}
	

void set_defaults_in_ram()
{
	//non stored variables	
	FLAGS.Repeat_Capture = false;

	//EEPROM Variables

	//build_version=10930;//			Serial.println(build_version);
	FLAGS.redraw = true;//			Serial.println(FLAGS.redraw);
	//EEPROM_STORED.progtype=0;//			Serial.println(EEPROM_STORED.progtype);
	EEPROM_STORED.intval=20;//			Serial.println(EEPROM_STORED.intval);
	EEPROM_STORED.interval=2000;//			Serial.println(EEPROM_STORED.interval);
	EEPROM_STORED.camera_fired=0;//			Serial.println(EEPROM_STORED.camera_fired);
	EEPROM_STORED.camera_moving_shots=240;//			Serial.println(EEPROM_STORED.camera_moving_shots);
	EEPROM_STORED.camera_total_shots=0;//			Serial.println(EEPROM_STORED.camera_total_shots);
	EEPROM_STORED.overaldur=20;//			Serial.println(EEPROM_STORED.overaldur);
	EEPROM_STORED.prefire_time=1;//			Serial.println(EEPROM_STORED.prefire_time);
	EEPROM_STORED.rampval=30;//			Serial.println(EEPROM_STORED.rampval);
	EEPROM_STORED.static_tm=1;//			Serial.println(EEPROM_STORED.static_tm);
	EEPROM_STORED.lead_in=1;//			Serial.println(EEPROM_STORED.lead_in);
	EEPROM_STORED.lead_out=1;//			Serial.println(EEPROM_STORED.lead_out);
	EEPROM_STORED.motor_steps_pt[2][0]=0;//			Serial.println(EEPROM_STORED.motor_steps_pt[2][0]);
	EEPROM_STORED.motor_steps_pt[2][1]=0;//			Serial.println(EEPROM_STORED.motor_steps_pt[2][1]);
	EEPROM_STORED.motor_steps_pt[2][2]=0;//			Serial.println(EEPROM_STORED.motor_steps_pt[2][2]);
	EEPROM_STORED.keyframe[0][0]=0;//			Serial.println(EEPROM_STORED.keyframe[0][0]);
	EEPROM_STORED.keyframe[0][1]=0;//			Serial.println(EEPROM_STORED.keyframe[0][1]);
	EEPROM_STORED.keyframe[0][2]=0;//			Serial.println(EEPROM_STORED.keyframe[0][2]);
	EEPROM_STORED.keyframe[0][3]=0;//			Serial.println(EEPROM_STORED.keyframe[0][3]);
	EEPROM_STORED.keyframe[0][4]=0;//			Serial.println(EEPROM_STORED.keyframe[0][4]);
	EEPROM_STORED.keyframe[0][5]=0;//			Serial.println(EEPROM_STORED.keyframe[0][5]);
	EEPROM_STORED.linear_steps_per_shot[0]=0;//			Serial.println(EEPROM_STORED.linear_steps_per_shot[0]);
	EEPROM_STORED.linear_steps_per_shot[1]=0;//			Serial.println(EEPROM_STORED.linear_steps_per_shot[1]);
	EEPROM_STORED.linear_steps_per_shot[2]=0;//			Serial.println(EEPROM_STORED.linear_steps_per_shot[2]);
	EEPROM_STORED.ramp_params_steps[0]=0;//			Serial.println(EEPROM_STORED.ramp_params_steps[0]);
	EEPROM_STORED.ramp_params_steps[1]=0;//			Serial.println(EEPROM_STORED.ramp_params_steps[1]);
	EEPROM_STORED.ramp_params_steps[2]=0;//			Serial.println(EEPROM_STORED.ramp_params_steps[2]);
	EEPROM_STORED.current_steps.x=0;//			Serial.println(EEPROM_STORED.current_steps.x);
	EEPROM_STORED.current_steps.y=0;//			Serial.println(EEPROM_STORED.current_steps.y);
	EEPROM_STORED.current_steps.z=0;//			Serial.println(EEPROM_STORED.current_steps.z);
	//EEPROM_STORED.progstep=0;//			Serial.println(EEPROM_STORED.progstep);
	//FLAGS.Program_Engaged=0;//			Serial.println(FLAGS.Program_Engaged);
	//SETTINGS.POWERSAVE_PT=PWR_SHOOTMOVE_ON;//			Serial.println(SETTINGS.POWERSAVE_PT);
	//SETTINGS.POWERSAVE_AUX=PWR_SHOOTMOVE_ON;//			Serial.println(SETTINGS.POWERSAVE_AUX);
	//SETTINGS.AUX_ON=3;//			Serial.println(SETTINGS.AUX_ON);
	//SETTINGS.PAUSE_ENABLED=3;//			Serial.println(SETTINGS.PAUSE_ENABLED);
	//SETTINGS.LCD_BRIGHTNESS_DURING_RUN=3;//			Serial.println(SETTINGS.LCD_BRIGHTNESS_DURING_RUN);
	//SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC=15000;//			Serial.println(SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC);
	//SETTINGS.AUX_REV=2;//			Serial.println(SETTINGS.AUX_REV);
}


void set_defaults_in_setup()
{
	SETTINGS.POWERSAVE_PT=PWR_SHOOTMOVE_ON;//			Serial.println(SETTINGS.POWERSAVE_PT);
	SETTINGS.POWERSAVE_AUX=PWR_SHOOTMOVE_ON;//			Serial.println(SETTINGS.POWERSAVE_AUX);
	SETTINGS.AUX_ON=1;//					Serial.println(SETTINGS.AUX_ON);
	SETTINGS.PAUSE_ENABLED=1;//			Serial.println(SETTINGS.PAUSE_ENABLED);
	SETTINGS.LCD_BRIGHTNESS_DURING_RUN=3;//		Serial.println(SETTINGS.LCD_BRIGHTNESS_DURING_RUN);
	SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC=15000;//	Serial.println(SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC);
	SETTINGS.AUX_REV=0;//					Serial.println(SETTINGS.AUX_REV);
	
	eeprom_write(1, EEPROM_STORED.build_version);
	eeprom_write(96, SETTINGS.POWERSAVE_PT);
	eeprom_write(98, SETTINGS.POWERSAVE_AUX);
	eeprom_write(100, SETTINGS.AUX_ON);
	eeprom_write(101, SETTINGS.PAUSE_ENABLED);
	eeprom_write(102, SETTINGS.LCD_BRIGHTNESS_DURING_RUN);
	eeprom_write(104, SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC);
	eeprom_write(106, SETTINGS.AUX_REV);
}


void write_defaults_to_eeprom_memory()
{
	set_defaults_in_ram();
	write_all_ram_to_eeprom();
}


void write_all_ram_to_eeprom()
{
	//eeprom_write(1, EEPROM_STORED.build_version);       // uint32_t
	//eeprom_write(5, FLAGS.redraw);          // boolean
	eeprom_write(7, EEPROM_STORED.progtype);              // uint8_t
	eeprom_write(9, EEPROM_STORED.intval);               
	eeprom_write(11, EEPROM_STORED.interval);
	eeprom_write(15, EEPROM_STORED.camera_fired);
	eeprom_write(17, EEPROM_STORED.camera_moving_shots);
	eeprom_write(19, EEPROM_STORED.camera_total_shots);
	eeprom_write(21, EEPROM_STORED.overaldur);
	eeprom_write(23, EEPROM_STORED.prefire_time);
	eeprom_write(25, EEPROM_STORED.rampval);
	eeprom_write(27, EEPROM_STORED.static_tm);
	eeprom_write(29, EEPROM_STORED.lead_in);
	eeprom_write(31, EEPROM_STORED.lead_out);
	eeprom_write(33, EEPROM_STORED.motor_steps_pt[2][0]);
	eeprom_write(37, EEPROM_STORED.motor_steps_pt[2][1]);
	eeprom_write(41, EEPROM_STORED.motor_steps_pt[2][2]);
	eeprom_write(45, EEPROM_STORED.keyframe[0][0]);
	eeprom_write(47, EEPROM_STORED.keyframe[0][1]);
	eeprom_write(49, EEPROM_STORED.keyframe[0][2]);
	eeprom_write(51, EEPROM_STORED.keyframe[0][3]);
	eeprom_write(53, EEPROM_STORED.keyframe[0][4]);
	eeprom_write(55, EEPROM_STORED.keyframe[0][5]);
	eeprom_write(57, EEPROM_STORED.linear_steps_per_shot[0]);
	eeprom_write(61, EEPROM_STORED.linear_steps_per_shot[1]);
	eeprom_write(65, EEPROM_STORED.linear_steps_per_shot[2]);
	eeprom_write(69, EEPROM_STORED.ramp_params_steps[0]);
	eeprom_write(73, EEPROM_STORED.ramp_params_steps[1]);
	eeprom_write(77, EEPROM_STORED.ramp_params_steps[2]);
	eeprom_write(81, EEPROM_STORED.current_steps.x);
	eeprom_write(85, EEPROM_STORED.current_steps.y);
	eeprom_write(89, EEPROM_STORED.current_steps.z);
	eeprom_write(93, EEPROM_STORED.progstep);
	//eeprom_write(95, FLAGS.Program_Engaged);
	//eeprom_write(96, SETTINGS.POWERSAVE_PT);
	//eeprom_write(98, SETTINGS.POWERSAVE_AUX);
	//eeprom_write(100, SETTINGS.AUX_ON);
	//eeprom_write(101, SETTINGS.PAUSE_ENABLED);
	//eeprom_write(102, SETTINGS.LCD_BRIGHTNESS_DURING_RUN);
	//eeprom_write(104, SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC);
	//eeprom_write(106, SETTINGS.AUX_REV);
}


// restore memory
 
void restore_from_eeprom_memory()
{
	//eeprom_read(1, EEPROM_STORED.build_version);
	//eeprom_read(5, FLAGS.redraw);
	eeprom_read(7, EEPROM_STORED.progtype);
	eeprom_read(9, EEPROM_STORED.intval);
	eeprom_read(11, EEPROM_STORED.interval);
	eeprom_read(15, EEPROM_STORED.camera_fired);
	eeprom_read(17, EEPROM_STORED.camera_moving_shots);
	eeprom_read(19, EEPROM_STORED.camera_total_shots);
	eeprom_read(21, EEPROM_STORED.overaldur);
	eeprom_read(23, EEPROM_STORED.prefire_time);
	eeprom_read(25, EEPROM_STORED.rampval);
	eeprom_read(27, EEPROM_STORED.static_tm);
	eeprom_read(29, EEPROM_STORED.lead_in);
	eeprom_read(31, EEPROM_STORED.lead_out);
	eeprom_read(33, EEPROM_STORED.motor_steps_pt[2][0]);
	eeprom_read(37, EEPROM_STORED.motor_steps_pt[2][1]);
	eeprom_read(41, EEPROM_STORED.motor_steps_pt[2][2]);
	eeprom_read(45, EEPROM_STORED.keyframe[0][0]);
	eeprom_read(47, EEPROM_STORED.keyframe[0][1]);
	eeprom_read(49, EEPROM_STORED.keyframe[0][2]);
	eeprom_read(51, EEPROM_STORED.keyframe[0][3]);
	eeprom_read(53, EEPROM_STORED.keyframe[0][4]);
	eeprom_read(55, EEPROM_STORED.keyframe[0][5]);
	eeprom_read(57, EEPROM_STORED.linear_steps_per_shot[0]);
	eeprom_read(61, EEPROM_STORED.linear_steps_per_shot[1]);
	eeprom_read(65, EEPROM_STORED.linear_steps_per_shot[2]);
	eeprom_read(69, EEPROM_STORED.ramp_params_steps[0]);
	eeprom_read(73, EEPROM_STORED.ramp_params_steps[1]);
	eeprom_read(77, EEPROM_STORED.ramp_params_steps[2]);
	eeprom_read(81, EEPROM_STORED.current_steps.x);
	eeprom_read(85, EEPROM_STORED.current_steps.y);
	eeprom_read(89, EEPROM_STORED.current_steps.z);
	//eeprom_read(93, EEPROM_STORED.progstep);
	//eeprom_read(95, FLAGS.Program_Engaged);
	eeprom_read(96, SETTINGS.POWERSAVE_PT);
	eeprom_read(98, SETTINGS.POWERSAVE_AUX);
	eeprom_read(100, SETTINGS.AUX_ON);
	eeprom_read(101, SETTINGS.PAUSE_ENABLED);
	eeprom_read(102, SETTINGS.LCD_BRIGHTNESS_DURING_RUN);
	eeprom_read(104, SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC);
	eeprom_read(106, SETTINGS.AUX_REV);
	//delay(100);
}


void review_RAM_Contents()
{
	Serial.println(EEPROM_STORED.build_version);
	Serial.println(FLAGS.redraw);
	Serial.println(EEPROM_STORED.intval);
	Serial.println(EEPROM_STORED.interval);
	//Serial.println(camera_exp_tm);
	Serial.println(EEPROM_STORED.camera_fired);
	Serial.println(EEPROM_STORED.camera_moving_shots);
	Serial.println(EEPROM_STORED.camera_total_shots);
	Serial.println(EEPROM_STORED.overaldur);
	Serial.println(EEPROM_STORED.prefire_time);
	Serial.println(EEPROM_STORED.rampval);
	Serial.println(EEPROM_STORED.static_tm);
	Serial.println(EEPROM_STORED.lead_in);
	Serial.println(EEPROM_STORED.lead_out);
	Serial.println(EEPROM_STORED.motor_steps_pt[2][0]);
	Serial.println(EEPROM_STORED.motor_steps_pt[2][1]);
	Serial.println(EEPROM_STORED.motor_steps_pt[2][2]);
	Serial.println(EEPROM_STORED.keyframe[0][0]);
	Serial.println(EEPROM_STORED.keyframe[0][1]);
	Serial.println(EEPROM_STORED.keyframe[0][2]);
	Serial.println(EEPROM_STORED.keyframe[0][3]);
	Serial.println(EEPROM_STORED.keyframe[0][4]);
	Serial.println(EEPROM_STORED.keyframe[0][5]);
	Serial.println(EEPROM_STORED.linear_steps_per_shot[0]);
	Serial.println(EEPROM_STORED.linear_steps_per_shot[1]);
	Serial.println(EEPROM_STORED.linear_steps_per_shot[2]);
	Serial.println(EEPROM_STORED.ramp_params_steps[0]);
	Serial.println(EEPROM_STORED.ramp_params_steps[1]);
	Serial.println(EEPROM_STORED.ramp_params_steps[2]);
	Serial.println(EEPROM_STORED.current_steps.x);
	Serial.println(EEPROM_STORED.current_steps.y);
	Serial.println(EEPROM_STORED.current_steps.z);
	Serial.println(EEPROM_STORED.progstep);
	Serial.println(FLAGS.Program_Engaged);
	Serial.println(SETTINGS.POWERSAVE_PT);
	Serial.println(SETTINGS.POWERSAVE_AUX);
	Serial.println(SETTINGS.AUX_ON);
	Serial.println(SETTINGS.PAUSE_ENABLED);
	Serial.println(SETTINGS.LCD_BRIGHTNESS_DURING_RUN);
	Serial.println(SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC);
}
