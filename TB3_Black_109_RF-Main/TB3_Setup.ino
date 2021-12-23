/*
  (c) 2015 Brian Burling eMotimo INC - Original 109 Release
  (c) 2021 Ryan Favelle - Modifications

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
  if (redraw)
  {
    lcd.empty();
    draw(74, 1, 1); //lcd.at(1,1,"Aux Motor:");
    if (!AUX_ON )     lcd.at(1, 12, "OFF");
    else              lcd.at(1, 12, "ON");
    draw(65, 2, 1); //lcd.at(2,1,"UpDown  C-Select");
    redraw = false;
    delay(prompt_time);
  }
  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1: // Up
        if (!AUX_ON)
        {
          AUX_ON = true;
          redraw = true;
        }
        break;
    
      case 1: // Down
        if (AUX_ON)
        {
          AUX_ON = false;
          redraw = true;
        }
        break;
    }
    
    switch(HandleButtons())
    {
      case C_Pressed:
        eeprom_write(100, AUX_ON);
        progstep_forward();
        break;
    
      case Z_Pressed:
        eeprom_write(100, AUX_ON);
        ReturnToMenu();
        break;
    }
  }
}


void Setup_PAUSE_ENABLED()
{
  if (redraw)
  {
    lcd.empty();
    draw(62, 1, 1);                         // lcd.at(1,1,"Pause ")
    if (PAUSE_ENABLED)    draw(67, 1, 8);   // lcd.at(1,7,"Enabled")
    else                  draw(68, 1, 8);   // lcd.at(1,7,"Disabled")
    draw(65, 2, 1);                         // lcd.at(2,1,"UpDown  C-Select");
    redraw = false;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1: // Up
        if (!PAUSE_ENABLED) {
          PAUSE_ENABLED = true;
          redraw = true;
        }
        break;
  
      case 1: // Down
        if (PAUSE_ENABLED) {
          PAUSE_ENABLED = false;
          redraw = true;
        }
        break;
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
}


void Setup_POWERSAVE_PT()
{
  if (redraw) {
    lcd.empty();
    lcd.at(1, 1, "PT Motors on");
    switch (POWERSAVE_PT)
    {
      case PWR_ALWAYS_ON:    draw(70, 2, 1); break; //lcd.at(2,1,"AlwaysOn");
      case PWR_PROGRAM_ON:   draw(71, 2, 1); break; //lcd.at(2,1,"ProgramOn");
      case PWR_SHOOTMOVE_ON: draw(72, 2, 1); break; //lcd.at(2,1,"ShootMoveOn");
      case PWR_MOVEONLY_ON:  draw(73, 2, 1); break; //lcd.at(2,1,"MoveOnly");
    }
    redraw = false;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1: // Up
        if (POWERSAVE_PT == PWR_MOVEONLY_ON) { POWERSAVE_PT = PWR_ALWAYS_ON; }
        else                                 { POWERSAVE_PT++; }
        redraw = true;
        break;
  
      case 1: // Down
        if (POWERSAVE_PT == PWR_ALWAYS_ON) { POWERSAVE_PT = PWR_MOVEONLY_ON; }
        else                               { POWERSAVE_PT--; }
        redraw = true;
        break;
    }
  
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
}


void Setup_POWERSAVE_AUX()
{
  if (redraw) {
    lcd.empty();
    lcd.at(1, 1, "AUX Motors On:");
    switch (POWERSAVE_AUX)
    {
      case PWR_ALWAYS_ON:    draw(70, 2, 1); break; //lcd.at(2,1,"AlwaysOn");
      case PWR_PROGRAM_ON:   draw(71, 2, 1); break; //lcd.at(2,1,"ProgramOn");
      case PWR_SHOOTMOVE_ON: draw(72, 2, 1); break; //lcd.at(2,1,"ShootMoveOn");
      case PWR_MOVEONLY_ON:  draw(73, 2, 1); break; //lcd.at(2,1,"MoveOnly");
    }
    redraw = false;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1: // Up
        if (POWERSAVE_AUX == PWR_MOVEONLY_ON) { POWERSAVE_AUX = PWR_ALWAYS_ON; }
        else                                  { POWERSAVE_AUX++; }
        redraw = true;
        break;
  
      case 1: // Down
        if (POWERSAVE_AUX == PWR_ALWAYS_ON)   { POWERSAVE_AUX = PWR_MOVEONLY_ON; }
        else                                  { POWERSAVE_AUX--; }
        redraw = true;
        break;
    }
  
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
}


void Setup_LCD_BRIGHTNESS_DURING_RUN()
{ //issue with this loop jumping out on first touch of up down - reads ghose C press.
  if (redraw)
  {
    lcd.empty();
    lcd.at(1, 1, "BkLite On Run: ");
    lcd.at(1, 15, LCD_BRIGHTNESS_RUNNING);
    draw(65, 2, 1); //lcd.at(2,1,"UpDown  C-Select");
    redraw = false;
    delay(prompt_time);
  }
  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1: // Up
        if (LCD_BRIGHTNESS_RUNNING == 8) { LCD_BRIGHTNESS_RUNNING = 1; }
        else                             { LCD_BRIGHTNESS_RUNNING++; }
        lcd.bright(LCD_BRIGHTNESS_RUNNING);
        redraw = true;
        break;
  
      case 1: // Down
        if (LCD_BRIGHTNESS_RUNNING == 1) { LCD_BRIGHTNESS_RUNNING = 8; }
        else                             { LCD_BRIGHTNESS_RUNNING--; }
        lcd.bright(LCD_BRIGHTNESS_RUNNING); //this seems to ghost press the C
        redraw = true;
        break;
    }
  
    switch(HandleButtons())
    {
      case C_Pressed:
        eeprom_write(102, LCD_BRIGHTNESS_RUNNING);
        lcd.bright(LCD_BRIGHTNESS_MENU);
        progstep_forward();
        break;
  
      case Z_Pressed:
        eeprom_write(102, LCD_BRIGHTNESS_RUNNING);
        lcd.bright(LCD_BRIGHTNESS_MENU);
        progstep_backward();
        break;
    }
  }
}


void Setup_Max_AUX_Motor_Speed()
{ //issue with this loop jumping out on first touch of up down - reads ghose C press.
  if (redraw) {
    lcd.empty();
    lcd.at(1, 1, "Max Speed:  ");
    lcd.at(1, 12, AUX_MAX_JOG_STEPS_PER_SEC);
    draw(65, 2, 1); //lcd.at(2,1,"UpDown  C-Select");
    redraw = false;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1:  // Up
        AUX_MAX_JOG_STEPS_PER_SEC += 500;
        if (AUX_MAX_JOG_STEPS_PER_SEC > 20000) AUX_MAX_JOG_STEPS_PER_SEC = 20000;
        redraw = true;
        break;
  
      case 1:  // Down
        AUX_MAX_JOG_STEPS_PER_SEC -= 500;
        if (AUX_MAX_JOG_STEPS_PER_SEC < 2000) AUX_MAX_JOG_STEPS_PER_SEC = 2000;
        redraw = true;
        break;
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
}


void Setup_AUX_Motor_DIR()
{
  if (redraw) {
    lcd.empty();
    lcd.at(1, 1, "Aux Reversed:");
    if (!AUX_REV)  lcd.at(1, 14, "OFF");
    else           lcd.at(1, 14, "ON");
    draw(65, 2, 1); //lcd.at(2,1,"UpDown  C-Select");
    redraw = false;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1: // Up
        if (!AUX_REV) {
          AUX_REV = 1;
          redraw = true;
        }
        break;
  
      case 1: // Down
        if (AUX_REV) {
          AUX_REV = 0;
          redraw = true;
        }
        break;
    }
    
    switch(HandleButtons())
    {
      case C_Pressed:
        eeprom_write(106, AUX_REV);
        progstep_forward();
        break;
  
      case Z_Pressed:
        eeprom_write(106, AUX_REV);
        progstep_backward();
        break;	  
    }
  }
}


void Set_Shot_Repeat()
{ //
  if (redraw) {
    lcd.empty();
    lcd.at(1, 1, "Select Shot Type");
    if (!sequence_repeat_type) lcd.at(2, 1, "Run Once");
    else                       lcd.at(2, 1, "Continuous Loop");
    redraw = false;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
      case -1: // Up
        if (!sequence_repeat_type) {
          sequence_repeat_type = true;
          redraw = true;
        }
        break;
  
      case 1: // Down
        if (sequence_repeat_type) {
          sequence_repeat_type = false;
          redraw = true;
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
