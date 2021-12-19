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
  if (FLAGS.redraw)
  {
    lcd.empty();
    draw(74, 1, 1); //lcd.at(1,1,"Aux Motor:");
    if (!SETTINGS.AUX_ON )  lcd.at(1, 12, "OFF");
    else                    lcd.at(1, 12, "ON");
    draw(65, 2, 1); //lcd.at(2,1,"UpDown  C-Select");
    FLAGS.redraw = false;
    delay(GLOBAL.prompt_time);
  }
  if ((millis() - GLOBAL.NClastread) > 50)
  {
    GLOBAL.NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1: // Up
        if (!SETTINGS.AUX_ON)
        {
          SETTINGS.AUX_ON = true;
          FLAGS.redraw = true;
        }
        break;
    
      case 1: // Down
        if (SETTINGS.AUX_ON)
        {
          SETTINGS.AUX_ON = false;
          FLAGS.redraw = true;
        }
        break;
    }
    
    switch(HandleButtons())
    {
      case C_Pressed:
        eeprom_write(100, SETTINGS.AUX_ON);
        progstep_forward();
        break;
    
      case Z_Pressed:
        eeprom_write(100, SETTINGS.AUX_ON);
        ReturnToMenu();
        break;
    }
  }
}


void Setup_PAUSE_ENABLED()
{
  if (FLAGS.redraw)
  {
    lcd.empty();
    draw(62, 1, 1);                         // lcd.at(1,1,"Pause ")
    if (SETTINGS.PAUSE_ENABLED)    draw(67, 1, 8);   // lcd.at(1,7,"Enabled")
    else                  draw(68, 1, 8);   // lcd.at(1,7,"Disabled")
    draw(65, 2, 1);                         // lcd.at(2,1,"UpDown  C-Select");
    FLAGS.redraw = false;
    delay(GLOBAL.prompt_time);
  }

  if ((millis() - GLOBAL.NClastread) > 50)
  {
    GLOBAL.NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1: // Up
        if (!SETTINGS.PAUSE_ENABLED) {
          SETTINGS.PAUSE_ENABLED = true;
          FLAGS.redraw = true;
        }
        break;
  
      case 1: // Down
        if (SETTINGS.PAUSE_ENABLED) {
          SETTINGS.PAUSE_ENABLED = false;
          FLAGS.redraw = true;
        }
        break;
    }
    
    switch(HandleButtons())
    {
      case C_Pressed:
        eeprom_write(101, SETTINGS.PAUSE_ENABLED);
        progstep_forward();
        break;
  
      case Z_Pressed:
        eeprom_write(101, SETTINGS.PAUSE_ENABLED);
        progstep_backward();
        break;
    }
  }
}


void Setup_POWERSAVE_PT()
{
  if (FLAGS.redraw) {
    lcd.empty();
    lcd.at(1, 1, "PT Motors on");
    if      (SETTINGS.POWERSAVE_PT == 1)	  draw(70, 2, 1); //lcd.at(2,1,"Always");
    else if (SETTINGS.POWERSAVE_PT == 2)	  draw(71, 2, 1); //lcd.at(2,1,"Program");
    else if (SETTINGS.POWERSAVE_PT == 3)	  draw(72, 2, 1); //lcd.at(2,1,"Shoot (accuracy)");
    else if (SETTINGS.POWERSAVE_PT == 4)	  draw(73, 2, 1); //lcd.at(2,1,"Shoot (pwr save)");
    FLAGS.redraw = false;
    delay(GLOBAL.prompt_time);
  }

  if ((millis() - GLOBAL.NClastread) > 50) {
    GLOBAL.NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1: // Up
        SETTINGS.POWERSAVE_PT++;
        if (SETTINGS.POWERSAVE_PT > 4) { SETTINGS.POWERSAVE_PT = 4; }
        else  { FLAGS.redraw = true; }
        break;
  
      case 1: // Down
        SETTINGS.POWERSAVE_PT--;
        if (SETTINGS.POWERSAVE_PT < 1) { SETTINGS.POWERSAVE_PT = 1; }
        else  { FLAGS.redraw = true;  }
        break;
    }
  
    switch(HandleButtons())
    {
      case C_Pressed:
        eeprom_write(96, SETTINGS.POWERSAVE_PT);
        progstep_forward();
        break;
  
      case Z_Pressed:
        eeprom_write(96, SETTINGS.POWERSAVE_PT);
        progstep_backward();
        break;
    }
  }
}


void Setup_POWERSAVE_AUX()
{
  if (FLAGS.redraw) {
    lcd.empty();
    lcd.at(1, 1, "AUX Motors On:");
    if      (SETTINGS.POWERSAVE_AUX == 1)	  draw(70, 2, 1); //lcd.at(2,1,"Always");
    else if (SETTINGS.POWERSAVE_AUX == 2)	  draw(71, 2, 1); //lcd.at(2,1,"Program");
    else if (SETTINGS.POWERSAVE_AUX == 3)	  draw(72, 2, 1); //lcd.at(2,1,"Shoot (accuracy)");
    else if (SETTINGS.POWERSAVE_AUX == 4)	  draw(73, 2, 1); //lcd.at(2,1,"Shoot (pwr save)");
    FLAGS.redraw = false;
    delay(GLOBAL.prompt_time);
  }

  if ((millis() - GLOBAL.NClastread) > 50) {
    GLOBAL.NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    
    switch(joy_capture_y_map())
    {
      case -1: // Up
        SETTINGS.POWERSAVE_AUX++;
        if (SETTINGS.POWERSAVE_AUX > 4) {
          SETTINGS.POWERSAVE_AUX = 4;
        }
        else  {
          FLAGS.redraw = true;
        }
        break;
  
      case 1: // Down
        SETTINGS.POWERSAVE_AUX--;
        if (!SETTINGS.POWERSAVE_AUX) {
          SETTINGS.POWERSAVE_AUX = 1;
        }
        else  {
          FLAGS.redraw = true;
        }
        break;
    }
  
    switch(HandleButtons())
    {
      case C_Pressed:
        eeprom_write(98, SETTINGS.POWERSAVE_AUX);
        progstep_forward();
        break;
  
      case Z_Pressed:
        eeprom_write(98, SETTINGS.POWERSAVE_AUX);
        progstep_backward();
        break;
    }
  }
}


void Setup_LCD_BRIGHTNESS_DURING_RUN()
{ //issue with this loop jumping out on first touch of up down - reads ghose C press.
  if (FLAGS.redraw)
  {
    lcd.empty();
    lcd.at(1, 1, "BkLite On Run: ");
    lcd.at(1, 15, SETTINGS.LCD_BRIGHTNESS_DURING_RUN);
    draw(65, 2, 1); //lcd.at(2,1,"UpDown  C-Select");
    FLAGS.redraw = false;
    delay(GLOBAL.prompt_time);
  }
  if ((millis() - GLOBAL.NClastread) > 50)
  {
    GLOBAL.NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1: // Up
        SETTINGS.LCD_BRIGHTNESS_DURING_RUN++;
        if (SETTINGS.LCD_BRIGHTNESS_DURING_RUN > 8) SETTINGS.LCD_BRIGHTNESS_DURING_RUN = 8;
        lcd.bright(SETTINGS.LCD_BRIGHTNESS_DURING_RUN); //this seems to ghost press the C
        FLAGS.redraw = true;
        break;
  
      case 1: // Down
        SETTINGS.LCD_BRIGHTNESS_DURING_RUN--;
        if (SETTINGS.LCD_BRIGHTNESS_DURING_RUN < 1) SETTINGS.LCD_BRIGHTNESS_DURING_RUN = 1;
        lcd.bright(SETTINGS.LCD_BRIGHTNESS_DURING_RUN);
        FLAGS.redraw = true;
        break;
    }
  
    switch(HandleButtons())
    {
      case C_Pressed:
        eeprom_write(102, SETTINGS.LCD_BRIGHTNESS_DURING_RUN);
        lcd.bright(4);
        progstep_forward();
        break;
  
      case Z_Pressed:
        eeprom_write(102, SETTINGS.LCD_BRIGHTNESS_DURING_RUN);
        lcd.bright(4);
        progstep_backward();
        break;
    }
  }
}


void Setup_Max_AUX_Motor_Speed()
{ //issue with this loop jumping out on first touch of up down - reads ghose C press.

  if (FLAGS.redraw) {
    lcd.empty();
    lcd.at(1, 1, "Max Speed:  ");
    lcd.at(1, 12, SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC);
    draw(65, 2, 1); //lcd.at(2,1,"UpDown  C-Select");
    FLAGS.redraw = false;
    delay(GLOBAL.prompt_time);
  }

  if ((millis() - GLOBAL.NClastread) > 50) {
    GLOBAL.NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1:  // Up
        SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC += 500;
        if (SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC > 20000) SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC = 20000;
        FLAGS.redraw = true;
        break;
  
      case 1:  // Down
        SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC -= 500;
        if (SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC < 2000) SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC = 2000;
        FLAGS.redraw = true;
        break;
    }
    
    switch(HandleButtons())
    {
      case C_Pressed:
        eeprom_write(104, SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC);
        progstep_forward();
        break;
  
      case Z_Pressed:
        eeprom_write(104, SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC);
        progstep_backward();
        break;
    }
  }
}


void Setup_AUX_Motor_DIR()
{
  if (FLAGS.redraw) {
    lcd.empty();
    lcd.at(1, 1, "Aux Reversed:");
    if (!SETTINGS.AUX_REV)  lcd.at(1, 14, "OFF");
    else                    lcd.at(1, 14, "ON");
    draw(65, 2, 1); //lcd.at(2,1,"UpDown  C-Select");
    FLAGS.redraw = false;
    delay(GLOBAL.prompt_time);
  }

  if ((millis() - GLOBAL.NClastread) > 50) {
    GLOBAL.NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1: // Up
        if (!SETTINGS.AUX_REV) {
          SETTINGS.AUX_REV = 1;
          FLAGS.redraw = true;
        }
        break;
  
      case 1: // Down
        if (SETTINGS.AUX_REV) {
          SETTINGS.AUX_REV = 0;
          FLAGS.redraw = true;
        }
        break;
    }
    
    switch(HandleButtons())
    {
      case C_Pressed:
        eeprom_write(106, SETTINGS.AUX_REV);
        progstep_forward();
        break;
  
      case Z_Pressed:
        eeprom_write(106, SETTINGS.AUX_REV);
        progstep_backward();
        break;	  
    }
  }
}


void Set_Shot_Repeat()
{ //
  if (FLAGS.redraw) {
    lcd.empty();
    lcd.at(1, 1, "Select Shot Type");
    if (SETTINGS.sequence_repeat_type == 1)			 lcd.at(2, 1, "Run Once");
    if (SETTINGS.sequence_repeat_type == 0)			 lcd.at(2, 1, "Continuous Loop");
    if (SETTINGS.sequence_repeat_type == -1)		 lcd.at(2, 1, "Repeat Forward"); //not currently supported
    FLAGS.redraw = false;
    delay(GLOBAL.prompt_time);
  }

  if ((millis() - GLOBAL.NClastread) > 50) {
    GLOBAL.NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1: // Up
        SETTINGS.sequence_repeat_type++;
        if (SETTINGS.sequence_repeat_type > 1) {
          SETTINGS.sequence_repeat_type = 1;
        }
        else  {
          FLAGS.redraw = true;
        }
        break;
  
      case 1: // Down
        SETTINGS.sequence_repeat_type--;
        if (SETTINGS.sequence_repeat_type < 0) {
          SETTINGS.sequence_repeat_type = 0;
        }
        else  {
          FLAGS.redraw = true;
        }
        break;
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
}
