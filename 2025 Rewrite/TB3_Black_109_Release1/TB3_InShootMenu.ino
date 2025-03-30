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

Menu returnProgstep;

void SMS_In_Shoot_Paused_Menu()  //this runs once and is quick - not persistent
{
  CZ_Button_Read_Count = 0;
  UpdatePowersave(Powersave::Always); // Only keep powered if Always on
  inprogtype = Inprog_Options::INPROG_RESUME;  //default this to the first option, Resume
  returnProgstep = progstep;  // Capture where we came from
  goto_shot = config.camera_fired; // Capture what shot we where at
  progstep_goto(Menu::PAUSE_Menu);         //send us to a loop where we can select options
}

void SMS_Resume()  //this runs once and is quick - not persistent
{
  CZ_Button_Read_Count = 0;
  lcd.empty();
  draw(F_STR("Resuming"), 1, 1);
  delay(screen_delay);
  UpdatePowersave(Powersave::Shoot_Minimum); // Only keep powered if Always on
  progstep_goto(returnProgstep);  //send us back to the main SMS Loop
}

void InProg_Select_Option() {
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    switch (inprogtype) {
      case Inprog_Options::Count:             // Error case, fall through
      case Inprog_Options::INPROG_RESUME:      { draw(F_STR("Resume"), 1, 6); break; }
      case Inprog_Options::INPROG_RESTART:     { draw(F_STR("Restart"), 1, 6); break; }
      case Inprog_Options::INPROG_GOTO_END:    { draw(F_STR("Go to End"), 1, 5); break; }
      case Inprog_Options::INPROG_GOTO_FRAME:  { draw(F_STR("Go to Frame"), 1, 1);  DisplayGoToShot(goto_shot, 1, 12);   break; }
      case Inprog_Options::INPROG_EXPOSURE:    { draw(F_STR("Intval:"), 1, 1); DisplayInterval(config.shot_interval_time, 1, 8); break; } // 0.1s
      case Inprog_Options::INPROG_STOPMOTION:  { draw(F_STR("StopMo R+C / L+C"), 1, 1); break;}  //Hold Right, then C to advance a frame with static time, left C goes back
    }
    draw(F_STR("UpDown  C-Select"), 2, 1);
  }  //end first time

  NunChuckjoybuttons();

  if (inprogtype == Inprog_Options::INPROG_GOTO_FRAME) {  //read leftright values for the goto frames
    int16_t x_val = joy_capture_x3();
    if (x_val != 0) {
      goto_shot = static_cast<uint16_t>(updateWithWraparound(static_cast<int16_t>(goto_shot), 1, static_cast<int16_t>(config.camera_total_shots), x_val));
      DisplayGoToShot(goto_shot, 1, 12);
      delay(scroll_delay);
    }
  }

  if (inprogtype == Inprog_Options::INPROG_EXPOSURE) {  //read leftright values for the goto frames
    int16_t x_val = joy_capture_x3();
    if (x_val != 0) {
      config.shot_interval_time = static_cast<uint16_t>(updateWithWraparound(static_cast<int16_t>(config.shot_interval_time), static_cast<int16_t>(EXTTRIG_EXPVAL), 6000, x_val));
      DisplayInterval(config.shot_interval_time, 1, 8);  // 0.1s
      delay(scroll_delay);
    }
  }

  int16_t yUpDown = joy_capture_y1();
  if (yUpDown != 0) {
    inprogtype = static_cast<Inprog_Options>(updateWithWraparound(static_cast<int16_t>(inprogtype), 0, static_cast<int16_t>(Inprog_Options::Count) - 1, yUpDown));
    delay(update_delay);
    first_ui = true;
  }

  button_actions_InProg_Select_Option();
  delay(scroll_delay);
}

void button_actions_InProg_Select_Option() {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    lcd.empty();
    switch(inprogtype) {
      case Inprog_Options::Count:
      case Inprog_Options::INPROG_RESUME: { SMS_Resume(); break;} // Unpause and Return
      case Inprog_Options::INPROG_RESTART: {  //Return to restart the shot  - send to review screen of relative move
        config.camera_fired = 0;
        lcd.bright(config.LCD_BRIGHTNESS_DURING_MENU);
        draw(F_STR("Going to Start"), 1, 2);

        switch (progtype) {
          case Menu_Options::PORTRAITPANO:
          case Menu_Options::PANOGIGA: {
            PanoState = ShootMoveState::Waiting_Camera;
            UpdatePowersave(Powersave::Shoot_Minimum); // Only keep powered if Always on
            progstep_goto(returnProgstep);  //send us back to the main Loop
            break;
          }
          case Menu_Options::REG2POINTMOVE:
          case Menu_Options::REV2POINTMOVE: {
            config.camera_fired = 0;
            goto_position();
            progstep_goto(Menu::PT2_ShotRepeat);
            break;
          }
          case Menu_Options::REG3POINTMOVE:
          case Menu_Options::REV3POINTMOVE: {
            config.camera_fired = 0;
            goto_position();
            progstep_goto(Menu::PT3_Reveiw);
            break;
          }
          case Menu_Options::AUXDISTANCE: {
            config.camera_fired = 0;
            goto_position();
            progstep_goto(Menu::PT2_ShotRepeat);
            break;
          }
          case Menu_Options::DFSLAVE:   progstep_goto(Menu::PT2_MainMenu); break;
          case Menu_Options::SETUPMENU: progstep_goto(Menu::PT2_MainMenu); break;
          case Menu_Options::Count:     progstep_goto(Menu::PT2_MainMenu); break;
        }
        break;
      }
      case Inprog_Options::INPROG_GOTO_END: { //Go to end point - basically a reverse move setup from wherever we are.
        REVERSE_PROG_ORDER = true;
        config.camera_fired = 0;
        lcd.bright(config.LCD_BRIGHTNESS_DURING_MENU);
        draw(F_STR("Going to End"), 1, 3);

        if (progtype == Menu_Options::REG2POINTMOVE || progtype == Menu_Options::REV2POINTMOVE) {
          config.camera_fired = 0;
          goto_position();
          progstep_goto(Menu::PT2_ShotRepeat);
        }
        else if (progtype == Menu_Options::REG3POINTMOVE || progtype == Menu_Options::REV3POINTMOVE) {
          config.camera_fired = 0;
          goto_position();
          progstep_goto(Menu::PT3_Reveiw);
        }
        else if (progtype == Menu_Options::AUXDISTANCE) {
          config.camera_fired = 0;
          goto_position();
          progstep_goto(Menu::PT2_ShotRepeat);
        }
        break;
      }
      case Inprog_Options::INPROG_GOTO_FRAME: {  // Go to specific frame
        first_ui = true;
        draw(F_STR("Going to"), 1, 4);
        draw(F_STR("Frame:"), 2, 4);
        lcd.at(2, 11, goto_shot);
        config.camera_fired = goto_shot; // Capture what shot we where at
        goto_position();
        inprogtype = Inprog_Options::INPROG_RESUME;
        break;
      }
      case Inprog_Options::INPROG_EXPOSURE: {  // Change Interval and static time
        first_ui = true;
        //look at current gap between interval and static time = available move time.
        uint16_t available_move_time = static_cast<uint16_t>(IntervalTime() / 100) - static_cast<uint16_t>(ShutterTime() / 100);  //this is the gap we keep interval isn't live
        if (available_move_time <= MIN_INTERVAL_STATIC_GAP) available_move_time = MIN_INTERVAL_STATIC_GAP;  //enforce min gap between static and interval
        IntervalTime(static_cast<int32_t>(config.shot_interval_time) * 100);                                                     //set the new ms timer for SMS
        if (config.shot_interval_time > available_move_time) {                                                          //we can apply the gap
          ShutterTime(static_cast<uint32_t>(config.shot_interval_time - available_move_time) * 100);
        }
        else  //squished it too much, go with minimum static time
        {
          ShutterTime(100);
        }
        inprogtype = Inprog_Options::INPROG_RESUME;
        break;
      }
      case Inprog_Options::INPROG_STOPMOTION:
        inprogtype = Inprog_Options::INPROG_RESUME;
        break;
    }
  }
}

template<uint8_t maxWidth>
void DrawPaddedValue(const uint32_t value, const uint8_t row, const uint8_t column) {
    static_assert(maxWidth > 0 && maxWidth <= 16, "maxWidth must be between 1 and 16");

    char buffer[maxWidth + 1]; // Extra space for null terminator
    uint32_t temp = value;
    uint8_t index = maxWidth;

    // Fill the buffer with digits from right to left
    while (temp > 0) {
        if (index == 0) break; // Overflow case, handle as needed
        buffer[--index] = '0' + (temp % 10);
        temp /= 10;
    }

    // Fill the remaining space with spaces
    for (; index > 0; --index) {
        buffer[index - 1] = ' ';
    }
    buffer[maxWidth] = '\0'; // Null terminator

    // Set the cursor position and print the entire buffer in one go
    lcd.pos(row, column);
    lcd.print(buffer);
}

template<uint8_t maxWidth>
void DrawPaddedDecimalValue(const uint32_t value, const uint8_t row, const uint8_t column, const uint16_t scale) {
    static_assert(maxWidth > 0 && maxWidth <= 16, "maxWidth must be between 1 and 16");

    char buffer[maxWidth + 1]; // Extra space for null terminator
    uint32_t temp = value;
    uint16_t tempscale = scale;
    uint8_t index = maxWidth;

    // Fill the buffer with digits from right to left
    while (temp > 0 || tempscale > 1) {
        if (index == 0) break; // Overflow case, handle as needed

        if (tempscale > 1) {
            buffer[--index] = '0' + (temp % 10);
            temp /= 10;
            tempscale /= 10;
        } else {
            buffer[--index] = '.';
        }
    }

    // Fill the remaining space with spaces
    for (; index > 0; --index) {
        buffer[index - 1] = ' ';
    }
    buffer[maxWidth] = '\0'; // Null terminator

    // Set the cursor position and print the entire buffer in one go
    lcd.pos(row, column);
    lcd.print(buffer);
}

void DisplayGoToShot(uint16_t value, uint8_t row, uint8_t col) {
  DrawPaddedValue<5>(value, row, col); // maxWidth is 5 for values up to 65535
}
