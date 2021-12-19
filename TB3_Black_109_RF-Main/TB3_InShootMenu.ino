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
  void Program_Engaged_Toggle()	{  //used for pausing
	  ButtonState = ReadAgain; //to prevent entry into this method until CZ button release again
	  EEPROM_STORED.Program_Engaged=!EEPROM_STORED.Program_Engaged; //toggle off the loop
  }
*/


void SMS_In_Shoot_Paused_Menu() //this runs once and is quick - not persistent
{
  EEPROM_STORED.Program_Engaged = false; //toggle off the loop
  if (SETTINGS.POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
  if (SETTINGS.POWERSAVE_AUX > PWR_PROGRAM_ON)   disable_AUX();
  inprogtype = 0; //default this to the first option, Resume
  progstep_goto(1001); //send us to a loop where we can select options
}


void SMS_Resume() //this runs once and is quick - not persistent
{
  EEPROM_STORED.Program_Engaged = true; //toggle off the loop
  lcd.empty();
  lcd.at(1, 1, "Resuming");
  delay (1000);
  progstep_goto(50); //send us back to the main SMS Loop
}


void InProg_Select_Option()
{
  if (FLAGS.redraw)
  {
    lcd.empty();
    switch(inprogtype)
    {
      case INPROG_RESUME:
        draw(86, 1, 6); //lcd.at(1,6,"Resume");
        break;

      case INPROG_RTS:
        draw(87, 1, 6); //lcd.at(1,6,"Restart");
        break;

      case INPROG_GOTO_END:
        draw(89, 1, 5); //lcd.at(1,5,"Go to End");
        break;

      case INPROG_GOTO_FRAME:
        draw(88, 1, 1); //lcd.at(1,1,"GoTo Frame:");
        GLOBAL.goto_shot = EEPROM_STORED.camera_fired;
        DisplayGoToShot();
        break;

      case INPROG_INTERVAL:
        //draw(18,1,1);//lcd.at(1,1,"Intval:   .  sec"); //having issue with this command and some overflow issue??
        EEPROM_STORED.intval = EEPROM_STORED.interval / 100;  // Convert from milliseconds to 0.1 second increments
        lcd.at(1, 1, "Intval:   .  sec");
        DisplayInterval();
        break;

      case INPROG_STOPMOTION:
        //Hold Right, then C to advance a frame with static time, left C goes back
        lcd.at(1, 1, "StopMo R+C / L+C");
        break;
    }

    lcd.at(2, 1, "UpDown  C-Select");
    FLAGS.redraw = false;
    if (SETTINGS.POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
    if (SETTINGS.POWERSAVE_AUX > PWR_PROGRAM_ON)   disable_AUX();
    delay(GLOBAL.prompt_time);

  } //end first time

  if ((millis() - GLOBAL.NClastread) > 50)
  {
    GLOBAL.NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
  }
    
  switch(inprogtype)
  {
    case INPROG_GOTO_FRAME:
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

    case INPROG_INTERVAL:
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


  switch(joy_capture_y_map())
  {
    case -1: // Up
      inprogtype++;
      if (inprogtype > (INPROG_OPTIONS - 1)) 	inprogtype = (INPROG_OPTIONS - 1);
      else
      {
        FLAGS.redraw = true;
      }
      break;

    case 1: // Down
      inprogtype--;
      if (inprogtype > (INPROG_OPTIONS - 1))	inprogtype = 0;
      else
      {
        FLAGS.redraw = true;
      }
      break;
  }
  button_actions_InProg_Select_Option();
}


void button_actions_InProg_Select_Option()
{
  switch (HandleButtons())
  {
    case C_Pressed:

      lcd.empty();
      if (inprogtype == INPROG_RESUME) { // Resume (unpause)
        SMS_Resume();
      }
      else if (inprogtype == INPROG_RTS) { //Return to restart the shot  - send to review screen of relative move
        EEPROM_STORED.REVERSE_PROG_ORDER = false;
        EEPROM_STORED.camera_fired = 0;
        lcd.bright(8);
        lcd.at(1, 2, "Going to Start");

        if (EEPROM_STORED.progtype == REG2POINTMOVE || EEPROM_STORED.progtype == REV2POINTMOVE) {
          go_to_start_new();
          progstep_goto(8);
        }
        else if (EEPROM_STORED.progtype == REG3POINTMOVE || EEPROM_STORED.progtype == REV3POINTMOVE) {
          go_to_start_new();
          progstep_goto(109);
        }
        else if (EEPROM_STORED.progtype == AUXDISTANCE) {
          go_to_start_new();
          progstep_goto(8);
        }
      }
      else if  (inprogtype == INPROG_GOTO_END) { //Go to end point - basically a reverse move setup from wherever we are.
        EEPROM_STORED.REVERSE_PROG_ORDER = true;
        EEPROM_STORED.camera_fired = 0;
        lcd.bright(8);
        lcd.at(1, 3, "Going to End");

        if (EEPROM_STORED.progtype == REG2POINTMOVE || EEPROM_STORED.progtype == REV2POINTMOVE) {
          go_to_start_new();
          progstep_goto(8);
        }
        else if (EEPROM_STORED.progtype == REG3POINTMOVE || EEPROM_STORED.progtype == REV3POINTMOVE) {
          go_to_start_new();
          progstep_goto(109);
        }
        else if (EEPROM_STORED.progtype == AUXDISTANCE) {
          go_to_start_new();
          progstep_goto(8);
        }
      }
      else if  (inprogtype == INPROG_GOTO_FRAME) { //Go to specific frame
        FLAGS.redraw = true;
        lcd.at(1, 4, "Going to");
        lcd.at(2, 4, "Frame:");
        lcd.at(2, 11, GLOBAL.goto_shot);
        goto_position(GLOBAL.goto_shot);
        inprogtype = INPROG_RESUME;
      }
      else if  (inprogtype == INPROG_INTERVAL) { //Change Interval and static time
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
        inprogtype = INPROG_RESUME;
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
  if	    (GLOBAL.goto_shot < 10)	  lcd.at(1, 14, "   "); //clear extra if goes from 3 to 2 or 2 to 1
  else if (GLOBAL.goto_shot < 100)   lcd.at(1, 15, "  ");  //clear extra if goes from 3 to 2 or 2 to 1
  else if (GLOBAL.goto_shot < 1000)  lcd.at(1, 16, " ");   //clear extra if goes from 3 to 2 or 2 to 1
}
