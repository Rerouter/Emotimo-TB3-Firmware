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


/*
  void Program_Engaged_Toggle() {  //used for pausing
    ButtonState = ReadAgain; //to prevent entry into this method until CZ button release again
    FLAGS.Program_Engaged=!FLAGS.Program_Engaged; //toggle off the loop
  }
*/
uint16_t panoprogtype = 0;
uint16_t restore_progstep = 0;

//In Program Menu Ordering

#define PANO_OPTIONS  11
enum panoprogtype : uint8_t {
  PANO_RESUME       = 0,
  PANO_GOTO_START   = 1,
  PANO_GOTO_END     = 2,
  PANO_GOTO_FRAME   = 3,
  PANO_INTERVAL     = 4,
  PANO_FOCUS_INT    = 5,
  PANO_EXT_TRIGGER  = 6,
  PANO_DETOUR       = 7,
  PANO_AOV          = 8,
  PANO_OVERLAP      = 9,
  PANO_START_POINT  = 10,
  PANO_END_POINT    = 11
};

void Pano_Pause() //this runs once and is quick - not persistent
{
  lcd.empty();
  lcd.at(1, 1, "Pausing");
  delay(GLOBAL.prompt_time);
  FLAGS.Program_Engaged = false; //toggle off the loop
  if (SETTINGS.POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
  if (SETTINGS.POWERSAVE_AUX > PWR_PROGRAM_ON)   disable_AUX();
  panoprogtype = 0;   //default this to the first option
  restore_progstep = EEPROM_STORED.progstep; // Grab the progstep so we can mess with enums without needing to worry
  progstep_goto(299); //go to the Pano_Paused_Menu item and loop
}


void Pano_Resume() //this runs once and is quick - not persistent
{
  FLAGS.Program_Engaged = true; //toggle off the loop
  lcd.empty();
  lcd.at(1, 1, "Resuming");
  delay(GLOBAL.prompt_time);
  progstep_goto(restore_progstep); //send us back to the main Panorama Loop
}


void Pano_Paused_Menu()
{
  if (FLAGS.redraw)
  {
    lcd.empty();
    switch(panoprogtype)
    {
      case PANO_RESUME:
        draw(86, 1, 6); //lcd.at(1,6,"Resume");
        break;

      case PANO_GOTO_START:
        draw(87, 1, 6); //lcd.at(1,6,"Restart");
        break;

      case PANO_GOTO_END:
        draw(89, 1, 5); //lcd.at(1,5,"Go to End");
        break;

      case PANO_GOTO_FRAME:
        draw(88, 1, 1); //lcd.at(1,1,"GoTo Frame:");
        GLOBAL.goto_shot = EEPROM_STORED.camera_fired;
        DisplayGoToShot();
        break;

      case PANO_INTERVAL:
        //draw(18,1,1);//lcd.at(1,1,"Intval:   .  sec"); //having issue with this command and some overflow issue??
        EEPROM_STORED.intval = EEPROM_STORED.interval / 100;  // Convert from milliseconds to 0.1 second increments
        lcd.at(1, 1, "Intval:   .  sec");
        DisplayInterval();
        break;

      case PANO_FOCUS_INT:
        lcd.at(1,1,"Change FocusTime");
        break;

      case PANO_EXT_TRIGGER:
        lcd.at(1,1,"Change Ext_Trig");
        break;

      case PANO_DETOUR:
        lcd.at(1,1,"Detour Mid Shot");
        break;

      case PANO_AOV:
        lcd.at(1,1,"Change AOV");
        break;

      case PANO_OVERLAP:
        lcd.at(1,1,"Change Overlap");
        break;

      case PANO_START_POINT:
        lcd.at(1,1,"Change Start Pos");
        break;

      case PANO_END_POINT:
        lcd.at(1,1,"Change End Pos");
        break;

      default:
        panoprogtype = 0;
    }

    lcd.at(2, 1, "UpDown  C-Select");
    FLAGS.redraw = false;
    if (SETTINGS.POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
    if (SETTINGS.POWERSAVE_AUX > PWR_PROGRAM_ON)   disable_AUX();
    delay(GLOBAL.prompt_time);

  } //end first time
    
  switch(panoprogtype)
  {
    case PANO_GOTO_FRAME:
    {
      //read leftright values for the goto frames
      uint32_t goto_shot_last = GLOBAL.goto_shot;
  
      if (GLOBAL.goto_shot < 20) GLOBAL.joy_x_lock_count = 0;
      GLOBAL.goto_shot += joy_capture_x3();
      if (GLOBAL.goto_shot < 1) {
        GLOBAL.goto_shot = 1;
        delay(GLOBAL.prompt_time / 2);
      }
      else if (GLOBAL.goto_shot > EEPROM_STORED.camera_total_shots) {
        GLOBAL.goto_shot = EEPROM_STORED.camera_total_shots;
        delay(GLOBAL.prompt_time / 2);
      }
      if (goto_shot_last != GLOBAL.goto_shot) {
        DisplayGoToShot();
      }
      break;
    }

    case PANO_INTERVAL:
      //read leftright values for the goto frames
      uint32_t intval_last = EEPROM_STORED.intval;
  
      if (EEPROM_STORED.intval < 20) GLOBAL.joy_x_lock_count = 0;
      EEPROM_STORED.intval += joy_capture_x3();
      EEPROM_STORED.intval = constrain(EEPROM_STORED.intval, 5, 6000); //no limits, you can crunch static time
      if (intval_last != EEPROM_STORED.intval) {
        DisplayInterval();
      }
      break;
  }

  if ((millis() - GLOBAL.NClastread) > 50)
  {
    GLOBAL.NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    
    switch(joy_capture_y_map())
    {
      case -1: // Up
        panoprogtype++;
        if (panoprogtype > (PANO_OPTIONS - 1))  panoprogtype = (PANO_OPTIONS - 1);
        else
        {
          FLAGS.redraw = true;
        }
        break;
  
      case 1: // Down
        panoprogtype--;
        if (panoprogtype > (PANO_OPTIONS - 1))  panoprogtype = 0;
        else
        {
          FLAGS.redraw = true;
        }
        break;
    }

     button_actions_Pano_Paused_Menu();
  }
}


void button_actions_Pano_Paused_Menu()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      lcd.empty();
      switch(panoprogtype)
      {
        case PANO_RESUME:
          Pano_Resume();
          break;

        case PANO_GOTO_START: //Return to restart the shot  - send to review screen of relative move
          EEPROM_STORED.REVERSE_PROG_ORDER = false;
          EEPROM_STORED.camera_fired = 0;
          lcd.bright(8);
          lcd.at(1, 2, "Going to Start");
          go_to_start_new();
          progstep_goto(207);
          break;

        case PANO_GOTO_END:
          EEPROM_STORED.REVERSE_PROG_ORDER = true;
          EEPROM_STORED.camera_fired = 0;
          lcd.bright(8);
          lcd.at(1, 3, "Going to End");
          go_to_start_new();
          progstep_goto(207);
          break;

        case PANO_GOTO_FRAME:
          FLAGS.redraw = true;
          lcd.at(1, 4, "Going to");
          lcd.at(2, 4, "Frame:");
          lcd.at(2, 11, GLOBAL.goto_shot);
          goto_position(GLOBAL.goto_shot);
          panoprogtype = PANO_RESUME;
          break;

        case PANO_FOCUS_INT:
          FLAGS.redraw = true;
          //look at current gap between interval and static time = available move time.
          uint32_t available_move_time = EEPROM_STORED.interval / 100 - EEPROM_STORED.static_tm; //this is the gap we keep interval isn't live
          //Serial.print("AMT:");Serial.println(available_move_time);
          if (available_move_time <= MIN_INTERVAL_STATIC_GAP) available_move_time = MIN_INTERVAL_STATIC_GAP; //enforce min gap between static and interval
          EEPROM_STORED.interval = EEPROM_STORED.intval * 100; //set the new ms timer for SMS
          if (EEPROM_STORED.intval > available_move_time)
          { //we can apply the gap
            //Serial.print("intval-available_move_time pos: ");Serial.println(EEPROM_STORED.intval-available_move_time);
            EEPROM_STORED.static_tm = EEPROM_STORED.intval - available_move_time;
            //Serial.print("static_tm= ");Serial.println(EEPROM_STORED.static_tm);
          }
          else  //squished it too much, go with minimum static time
          {
            EEPROM_STORED.static_tm = 1;
          }
          panoprogtype = PANO_RESUME;
          break;
  
        case PANO_EXT_TRIGGER:
          lcd.at(1,1,"Change Ext_Trig");
          break;
  
        case PANO_DETOUR:
          lcd.at(1,1,"Detour Mid Shot");
          break;
  
        case PANO_AOV:
          Set_angle_of_view();
          break;

        case PANO_OVERLAP:
          Set_angle_of_view();
          break;
  
        case PANO_START_POINT:
          lcd.at(1,1,"Change Start Pos");
          break;
  
        case PANO_END_POINT:
          lcd.at(1,1,"Change End Pos");
          break;
      }
      break;
    case Z_Pressed:
      //progtype=0;
      //progstep_goto(0);
      delay(1);
      break;
  }
}


void DisplayGoToShot()
{
  lcd.at(1, 13, GLOBAL.goto_shot);
  if      (GLOBAL.goto_shot < 10)    lcd.at(1, 14, "   "); //clear extra if goes from 3 to 2 or 2 to 1
  else if (GLOBAL.goto_shot < 100)   lcd.at(1, 15, "  ");  //clear extra if goes from 3 to 2 or 2 to 1
  else if (GLOBAL.goto_shot < 1000)  lcd.at(1, 16, " ");   //clear extra if goes from 3 to 2 or 2 to 1
  else if (GLOBAL.goto_shot < 10000) lcd.at(1, 16, "");    //clear extra if goes from 3 to 2 or 2 to 1
}
