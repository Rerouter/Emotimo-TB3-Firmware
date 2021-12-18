/*

  (c) 2015 Brian Burling eMotimo INC


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


void Setup_AUX_ON()
{
  int8_t joyYDown = 0;

  if (first_time)
  {
    lcd.empty();
    draw(74, 1, 1); //lcd.at(1,1,"Aux Motor:");
    if (!AUX_ON )     lcd.at(1, 12, "OFF");
    else              lcd.at(1, 12, "ON");
    draw(65, 2, 1); //lcd.at(2,1,"UpDown  C-Select");
    first_time = 0;
    delay(prompt_time);
  }
  if ((millis() - NClastread) > 50)
  {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
  }

  joyYDown = joy_capture_y_map();
  if (joyYDown == -1)
  { //  up
    if (!AUX_ON)
    {
      AUX_ON = 1;
      first_time = 1;
    }
  }
  else if (joyYDown == 1)
  { //down
    if (AUX_ON)
    {
      AUX_ON = 0;
      first_time = 1;
    }
  }
  switch(HandleButtons())
  {
    case C_Pressed:
      eeprom_write(100, AUX_ON);
      progstep_forward();
      break;

    case Z_Pressed:
      eeprom_write(100, AUX_ON);
      progstep_goto(0);
      break;
  }
}


void Setup_PAUSE_ENABLED()
{
  int8_t joyYDown = 0;

  if (first_time)
  {
    lcd.empty();
    draw(62, 1, 1);                         // lcd.at(1,1,"Pause ")
    if (PAUSE_ENABLED)    draw(67, 1, 8);   // lcd.at(1,7,"Enabled")
    else                  draw(68, 1, 8);   // lcd.at(1,7,"Disabled")
    draw(65, 2, 1);                         // lcd.at(2,1,"UpDown  C-Select");
    first_time = 0;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > 50)
  {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
  }

  joyYDown = joy_capture_y_map();

  if (joyYDown == -1) { //  up
    if (!PAUSE_ENABLED) {
      PAUSE_ENABLED = 1;
      first_time = 1;
    }
  }
  else if (joyYDown == 1) { //down
    if (PAUSE_ENABLED) {
      PAUSE_ENABLED = 0;
      first_time = 1;
    }
  }
  switch(HandleButtons())
  {
    case C_Pressed:
      eeprom_write(101, PAUSE_ENABLED);
      progstep_forward();
      break;

    case Z_Pressed:
      eeprom_write(101, PAUSE_ENABLED);
      progstep_backward();
      break;
  }
}


void Setup_POWERSAVE_PT()
{
  int8_t joyYDown = 0;
  //int displayvar=0;

  if (first_time) {
    lcd.empty();
    lcd.at(1, 1, "PT Motors on");
    if      (POWERSAVE_PT == 1)	  draw(70, 2, 1); //lcd.at(2,1,"Always");
    else if (POWERSAVE_PT == 2)	  draw(71, 2, 1); //lcd.at(2,1,"Program");
    else if (POWERSAVE_PT == 3)	  draw(72, 2, 1); //lcd.at(2,1,"Shoot (accuracy)");
    else if (POWERSAVE_PT == 4)	  draw(73, 2, 1); //lcd.at(2,1,"Shoot (pwr save)");
    first_time = 0;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > 50) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
  }

  joyYDown = joy_capture_y_map();

  if (joyYDown == -1) { //  up
    POWERSAVE_PT++;
    if (POWERSAVE_PT > 4) {
      POWERSAVE_PT = 4;
    }
    else  {
      first_time = 1;
    }
  }
  else if (joyYDown == 1) { //down
    POWERSAVE_PT--;
    if (POWERSAVE_PT < 1) {
      POWERSAVE_PT = 1;
    }
    else  {
      first_time = 1;
    }
  }

  if (POWERSAVE_PT > 100) POWERSAVE_PT = 2;

  switch(HandleButtons())
  {
    case C_Pressed:
      eeprom_write(96, POWERSAVE_PT);
      progstep_forward();
      break;

    case Z_Pressed:
      eeprom_write(96, POWERSAVE_PT);
      progstep_backward();
      break;
  }
}


void Setup_POWERSAVE_AUX()
{
  int8_t joyYDown = 0;

  if (first_time) {
    lcd.empty();
    lcd.at(1, 1, "AUX Motors On:");
    if      (POWERSAVE_AUX == 1)	  draw(70, 2, 1); //lcd.at(2,1,"Always");
    else if (POWERSAVE_AUX == 2)	  draw(71, 2, 1); //lcd.at(2,1,"Program");
    else if (POWERSAVE_AUX == 3)	  draw(72, 2, 1); //lcd.at(2,1,"Shoot (accuracy)");
    else if (POWERSAVE_AUX == 4)	  draw(73, 2, 1); //lcd.at(2,1,"Shoot (pwr save)");
    first_time = 0;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > 50) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
  }

  joyYDown = joy_capture_y_map();

  if (joyYDown == -1) { //  up
    POWERSAVE_AUX++;
    if (POWERSAVE_AUX > 4) {
      POWERSAVE_AUX = 4;
    }
    else  {
      first_time = 1;
    }
  }
  else if (joyYDown == 1) { //down
    POWERSAVE_AUX--;
    if (POWERSAVE_AUX < 1) {
      POWERSAVE_AUX = 1;
    }
    else  {
      first_time = 1;
    }
  }

  if (POWERSAVE_AUX > 100) POWERSAVE_AUX = 2;

  switch(HandleButtons())
  {
    case C_Pressed:
      eeprom_write(98, POWERSAVE_AUX);
      progstep_forward();
      break;

    case Z_Pressed:
      eeprom_write(98, POWERSAVE_AUX);
      progstep_backward();
      break;
  }
}


void Setup_LCD_BRIGHTNESS_DURING_RUN()
{ //issue with this loop jumping out on first touch of up down - reads ghose C press.
  int8_t joyYDown = 0;
  if (first_time)
  {
    lcd.empty();
    lcd.at(1, 1, "BkLite On Run: ");
    lcd.at(1, 15, LCD_BRIGHTNESS_DURING_RUN);
    draw(65, 2, 1); //lcd.at(2,1,"UpDown  C-Select");
    first_time = 0;
    delay(prompt_time);
  }
  if ((millis() - NClastread) > 50)
  {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
  }

  joyYDown = joy_capture_y_map();

  if (joyYDown == -1) { //  up
    LCD_BRIGHTNESS_DURING_RUN++;
    if (LCD_BRIGHTNESS_DURING_RUN > 8) LCD_BRIGHTNESS_DURING_RUN = 8;
    lcd.bright(LCD_BRIGHTNESS_DURING_RUN); //this seems to ghost press the C
    first_time = 1;
  }

  else if (joyYDown == 1) { //down
    LCD_BRIGHTNESS_DURING_RUN--;
    if (LCD_BRIGHTNESS_DURING_RUN < 1) LCD_BRIGHTNESS_DURING_RUN = 1;
    lcd.bright(LCD_BRIGHTNESS_DURING_RUN);
    first_time = 1;
  }

  switch(HandleButtons())
  {
    case C_Pressed:
      eeprom_write(102, LCD_BRIGHTNESS_DURING_RUN);
      lcd.bright(4);
      progstep_forward();
      break;

    case Z_Pressed:
      eeprom_write(102, LCD_BRIGHTNESS_DURING_RUN);
      lcd.bright(4);
      progstep_backward();
      break;
  }
}


void Setup_Max_AUX_Motor_Speed()
{ //issue with this loop jumping out on first touch of up down - reads ghose C press.
  int8_t joyYDown = 0;

  if (first_time) {
    lcd.empty();
    lcd.at(1, 1, "Max Speed:  ");
    lcd.at(1, 12, AUX_MAX_JOG_STEPS_PER_SEC);
    draw(65, 2, 1); //lcd.at(2,1,"UpDown  C-Select");
    first_time = 0;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > 50) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
  }

  joyYDown = joy_capture_y_map();

  if (joyYDown == -1) { //  up
    AUX_MAX_JOG_STEPS_PER_SEC += 500;
    if (AUX_MAX_JOG_STEPS_PER_SEC > 20000) AUX_MAX_JOG_STEPS_PER_SEC = 20000;
    first_time = 1;
  }

  else if (joyYDown == 1) { //down
    AUX_MAX_JOG_STEPS_PER_SEC -= 500;
    if (AUX_MAX_JOG_STEPS_PER_SEC < 2000) AUX_MAX_JOG_STEPS_PER_SEC = 2000;
    first_time = 1;
  }
  switch(HandleButtons())
  {
    case C_Pressed:
      eeprom_write(104, AUX_MAX_JOG_STEPS_PER_SEC);
      progstep_forward();
      break;

    case Z_Pressed:
      eeprom_write(104, AUX_MAX_JOG_STEPS_PER_SEC);
      progstep_backward();
      break;
  }
}


void Setup_AUX_Motor_DIR()
{
  int8_t joyYDown = 0;
  //int displayvar=0;

  if (first_time) {
    //prompt_val=AUX_ON;
    lcd.empty();
    lcd.at(1, 1, "Aux Reversed:");
    if (!AUX_REV)  lcd.at(1, 14, "OFF");
    else           lcd.at(1, 14, "ON");
    draw(65, 2, 1); //lcd.at(2,1,"UpDown  C-Select");
    first_time = 0;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > 50) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
  }

  joyYDown = joy_capture_y_map();

  if (joyYDown == -1) { //  up
    if (!AUX_REV) {
      AUX_REV = 1;
      first_time = 1;
    }
  }
  else if (joyYDown == 1) { //down
    if (AUX_REV) {
      AUX_REV = 0;
      first_time = 1;
    }
  }
  switch(HandleButtons())
  {
    case C_Pressed:
      eeprom_write(106, AUX_REV);
      progtype = 0;
      progstep_goto(0);
      lcd.empty();
      lcd.at(1, 1, "Return Main Menu");
      break;

    case Z_Pressed:
      eeprom_write(106, AUX_REV);
      progstep_backward();
      break;	  
  }
}


void Set_Shot_Repeat()
{ //
  int8_t joyYDown = 0;

  if (first_time) {
    lcd.empty();
    lcd.at(1, 1, "Select Shot Type");
    if (sequence_repeat_type == 1)			 lcd.at(2, 1, "Run Once");
    if (sequence_repeat_type == 0)			 lcd.at(2, 1, "Continuous Loop");
    if (sequence_repeat_type == -1)		   lcd.at(2, 1, "Repeat Forward"); //not currently supported
    first_time = 0;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > 50) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
  }

  joyYDown = joy_capture_y_map();

  if (joyYDown == -1)
  { //  up
    sequence_repeat_type++;
    if (sequence_repeat_type > 1) {
      sequence_repeat_type = 1;
    }
    else  {
      first_time = 1;
    }
  }
  else if (joyYDown == 1)
  { //down
    sequence_repeat_type--;
    if (sequence_repeat_type < 0) {
      sequence_repeat_type = 0;
    }
    else  {
      first_time = 1;
    }
  }
  switch(HandleButtons())
  {
    case C_Pressed:
      progstep_forward();
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}
