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

#define Settings_Options  16    //up this when code for gotoframe
enum Settings : uint8_t {
  SETTING_AUX_ENABLE          = 0,
  SETTING_AUX_DIRECTION       = 1,
  SETTING_POWERSAVE_PT        = 2,
  SETTING_POWERSAVE_AUX       = 3,
  SETTING_LCD_BRIGHTNESS_RUN  = 4,
  SETTING_LCD_BRIGHTNESS_MENU = 5,
  SETTING_MAX_SPEED_AUX       = 6,
  SETTING_MAX_SPEED_PAN       = 7,
  SETTING_MAX_SPEED_TILT      = 8,
  SETTING_PAUSE_ENABLE        = 9,
  SETTING_PANO_SERPENTINE     = 10,
  SETTING_JOY_X_INVERT        = 11,
  SETTING_JOY_Y_INVERT        = 12,
  SETTING_ACC_X_INVERT        = 13,
  SETTING_SWAP_X_Y_AXIS       = 14,
  SETTING_SWAP_X_Z_AXIS       = 15
};
uint8_t Setting_Item = 0;


void Settings_Menu()
{
  if (redraw)
  {
    lcd.at(1,1," Settings Menu  ");
    switch(Setting_Item)
    {                                             //0123456789ABCDEF
      case SETTING_AUX_ENABLE:          lcd.at(2,1,"Aux Axis Enable ");  break;
      case SETTING_AUX_DIRECTION:       lcd.at(2,1," Aux Direction  ");  break;
      case SETTING_POWERSAVE_PT:        lcd.at(2,1,"Power Pan / Tilt");  break;
      case SETTING_POWERSAVE_AUX:       lcd.at(2,1,"    Power Aux   ");  break;
      case SETTING_LCD_BRIGHTNESS_RUN:  lcd.at(2,1," LCD Bright Run ");  break;
      case SETTING_LCD_BRIGHTNESS_MENU: lcd.at(2,1," LCD Bright Menu");  break;
      case SETTING_MAX_SPEED_AUX:       lcd.at(2,1," AUX Max Speed  ");  break;
      case SETTING_MAX_SPEED_PAN:       lcd.at(2,1," Pan Max Speed  ");  break;
      case SETTING_MAX_SPEED_TILT:      lcd.at(2,1," Tilt Max Speed ");  break;
      case SETTING_PAUSE_ENABLE:        lcd.at(2,1,"  Pause Enable  ");  break;
      case SETTING_PANO_SERPENTINE:     lcd.at(2,1,"Pano Serpentine ");  break;
      case SETTING_JOY_X_INVERT:        lcd.at(2,1,"Invt Joy X Axis ");  break;
      case SETTING_JOY_Y_INVERT:        lcd.at(2,1,"Invt Joy Y Axis ");  break;
      case SETTING_ACC_X_INVERT:        lcd.at(2,1,"Invt Acc X Axis ");  break;
      case SETTING_SWAP_X_Y_AXIS:       lcd.at(2,1,"Swap X / Y Axis ");  break;
      case SETTING_SWAP_X_Z_AXIS:       lcd.at(2,1,"Swap X / Z Axis ");  break;
    }
    delay(prompt_time);
    redraw = false;
  }

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch(joy_capture_y_map())
    {
       case 1: // Up
        if (Setting_Item == (Settings_Options - 1)) Setting_Item = 0;
        else                                        Setting_Item++;
        redraw = true;
        break;
        
      case -1: // Down
        if (Setting_Item) Setting_Item--;
        else              Setting_Item = Settings_Options - 1;
        redraw = true;
        break;
    }
    
    switch(HandleButtons())
    {
      case C_Pressed:
        switch(Setting_Item)
        {                                     //123456789ABCDEF
          case SETTING_AUX_ENABLE:          progstep_goto(901);  break;
          case SETTING_AUX_DIRECTION:       progstep_goto(902);  break;
          case SETTING_POWERSAVE_PT:        progstep_goto(903);  break;
          case SETTING_POWERSAVE_AUX:       progstep_goto(904);  break;
          case SETTING_LCD_BRIGHTNESS_RUN:  progstep_goto(905);  break;
          case SETTING_LCD_BRIGHTNESS_MENU: progstep_goto(906);  break;
          case SETTING_MAX_SPEED_AUX:       progstep_goto(907);  break;
          case SETTING_MAX_SPEED_PAN:       progstep_goto(908);  break;
          case SETTING_MAX_SPEED_TILT:      progstep_goto(909);  break;
          case SETTING_PAUSE_ENABLE:        progstep_goto(910);  break;
          case SETTING_PANO_SERPENTINE:     progstep_goto(911);  break;
          case SETTING_JOY_X_INVERT:        progstep_goto(912);  break;
          case SETTING_JOY_Y_INVERT:        progstep_goto(913);  break;
          case SETTING_ACC_X_INVERT:        progstep_goto(914);  break;
          case SETTING_SWAP_X_Y_AXIS:       progstep_goto(915);  break;
          case SETTING_SWAP_X_Z_AXIS:       progstep_goto(916);  break;
        }
        break;
    
      case Z_Pressed:
        ReturnToMenu();
        break;
    }
  }
}


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
      case 1: // Up
        if (!AUX_ON)
        {
          AUX_ON = true;
          redraw = true;
        }
        break;
    
      case -1: // Down
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
        progstep = 900;
        redraw = true;
        break;
    
      case Z_Pressed:
        progstep = 900;
        redraw = true;
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
      case 1: // Up
        if (!AUX_REV) {
          AUX_REV = 1;
          redraw = true;
        }
        break;
  
      case -1: // Down
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
        progstep = 900;
        redraw = true;
        break;
  
      case Z_Pressed:
        progstep = 900;
        redraw = true;
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
      case 1: // Up
        if (POWERSAVE_PT == PWR_MOVEONLY_ON) { POWERSAVE_PT = PWR_ALWAYS_ON; }
        else                                 { POWERSAVE_PT++; }
        redraw = true;
        break;
  
      case -1: // Down
        if (POWERSAVE_PT == PWR_ALWAYS_ON) { POWERSAVE_PT = PWR_MOVEONLY_ON; }
        else                               { POWERSAVE_PT--; }
        redraw = true;
        break;
    }
  
    switch(HandleButtons())
    {
      case C_Pressed:
        eeprom_write(96, POWERSAVE_PT);
        progstep = 900;
        redraw = true;
        break;
  
      case Z_Pressed:
        progstep = 900;
        redraw = true;
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
      case 1: // Up
        if (POWERSAVE_AUX == PWR_MOVEONLY_ON) { POWERSAVE_AUX = PWR_ALWAYS_ON; }
        else                                  { POWERSAVE_AUX++; }
        redraw = true;
        break;
  
      case -1: // Down
        if (POWERSAVE_AUX == PWR_ALWAYS_ON)   { POWERSAVE_AUX = PWR_MOVEONLY_ON; }
        else                                  { POWERSAVE_AUX--; }
        redraw = true;
        break;
    }
  
    switch(HandleButtons())
    {
      case C_Pressed:
        eeprom_write(98, POWERSAVE_AUX);
        progstep = 900;
        redraw = true;
        break;
  
      case Z_Pressed:
        progstep = 900;
        redraw = true;
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
      case 1: // Up
        if (LCD_BRIGHTNESS_RUNNING == 8) { LCD_BRIGHTNESS_RUNNING = 1; }
        else                             { LCD_BRIGHTNESS_RUNNING++; }
        lcd.bright(LCD_BRIGHTNESS_RUNNING);
        redraw = true;
        break;
  
      case -1: // Down
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
        progstep = 900;
        redraw = true;
        break;
  
      case Z_Pressed:
        lcd.bright(LCD_BRIGHTNESS_MENU);
        progstep = 900;
        redraw = true;
        break;
    }
  }
}


void Setup_LCD_BRIGHTNESS_DURING_MENU()
{ //issue with this loop jumping out on first touch of up down - reads ghose C press.
  if (redraw)
  {
    lcd.empty();//0123456789ABCDEF
    lcd.at(1, 1, "BkLite in Menu: ");
    lcd.at(1, 16, LCD_BRIGHTNESS_MENU);
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
      case 1: // Up
        if (LCD_BRIGHTNESS_MENU == 8) { LCD_BRIGHTNESS_MENU = 1; }
        else                          { LCD_BRIGHTNESS_MENU++; }
        lcd.bright(LCD_BRIGHTNESS_MENU);
        redraw = true;
        break;
  
      case -1: // Down
        if (LCD_BRIGHTNESS_RUNNING == 1) { LCD_BRIGHTNESS_MENU = 8; }
        else                             { LCD_BRIGHTNESS_MENU--; }
        lcd.bright(LCD_BRIGHTNESS_MENU); //this seems to ghost press the C
        redraw = true;
        break;
    }
  
    switch(HandleButtons())
    {
      case C_Pressed:
        lcd.bright(LCD_BRIGHTNESS_MENU);
        progstep = 900;
        redraw = true;
        break;
  
      case Z_Pressed:
        lcd.bright(LCD_BRIGHTNESS_MENU);
        progstep = 900;
        redraw = true;
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
      case 1:  // Up
        AUX_MAX_JOG_STEPS_PER_SEC += 500;
        if (AUX_MAX_JOG_STEPS_PER_SEC > 65000) AUX_MAX_JOG_STEPS_PER_SEC = 1000;
        redraw = true;
        break;
  
      case -1:  // Down
        AUX_MAX_JOG_STEPS_PER_SEC -= 500;
        if (AUX_MAX_JOG_STEPS_PER_SEC < 1000) AUX_MAX_JOG_STEPS_PER_SEC = 65000;
        redraw = true;
        break;
    }
    
    switch(HandleButtons())
    {
      case C_Pressed:
        eeprom_write(104, AUX_MAX_JOG_STEPS_PER_SEC);
        progstep = 900;
        redraw = true;
        break;
  
      case Z_Pressed:
        progstep = 900;
        redraw = true;
        break;
    }
  }
}


void Setup_Max_PAN_Motor_Speed()
{ //issue with this loop jumping out on first touch of up down - reads ghose C press.
  if (redraw) {
    lcd.empty();
    lcd.at(1, 1, "Max Speed:  ");
    lcd.at(1, 12, PAN_MAX_JOG_STEPS_PER_SEC);
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
      case 1:  // Up
        PAN_MAX_JOG_STEPS_PER_SEC += 500;
        if (PAN_MAX_JOG_STEPS_PER_SEC > 65000) PAN_MAX_JOG_STEPS_PER_SEC = 1000;
        redraw = true;
        break;
  
      case -1:  // Down
        PAN_MAX_JOG_STEPS_PER_SEC -= 500;
        if (PAN_MAX_JOG_STEPS_PER_SEC < 1000) PAN_MAX_JOG_STEPS_PER_SEC = 65000;
        redraw = true;
        break;
    }
    
    switch(HandleButtons())
    {
      case C_Pressed:
        progstep = 900;
        redraw = true;
        break;
  
      case Z_Pressed:
        progstep = 900;
        redraw = true;
        break;
    }
  }
}


void Setup_Max_TILT_Motor_Speed()
{ //issue with this loop jumping out on first touch of up down - reads ghose C press.
  if (redraw) {
    lcd.empty();
    lcd.at(1, 1, "Max Speed:  ");
    lcd.at(1, 12, TILT_MAX_JOG_STEPS_PER_SEC);
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
      case 1:  // Up
        TILT_MAX_JOG_STEPS_PER_SEC += 500;
        if (TILT_MAX_JOG_STEPS_PER_SEC > 65000) TILT_MAX_JOG_STEPS_PER_SEC = 1000;
        redraw = true;
        break;
  
      case -1:  // Down
        TILT_MAX_JOG_STEPS_PER_SEC -= 500;
        if (TILT_MAX_JOG_STEPS_PER_SEC < 1000) TILT_MAX_JOG_STEPS_PER_SEC = 65000;
        redraw = true;
        break;
    }
    
    switch(HandleButtons())
    {
      case C_Pressed:
        progstep = 900;
        redraw = true;
        break;
  
      case Z_Pressed:
        progstep = 900;
        redraw = true;
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
      case 1: // Up
        if (!PAUSE_ENABLED) {
          PAUSE_ENABLED = true;
          redraw = true;
        }
        break;
  
      case -1: // Down
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
        progstep = 900;
        redraw = true;
        break;
  
      case Z_Pressed:
        progstep = 900;
        redraw = true;
        break;
    }
  }
}
