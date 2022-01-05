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
/*

  =========================================
  PT_LCD_Buttons - Code for Menus and User Interface
  =========================================

*/
int32_t  aux_dist;

bool cursorpos = 0;
enum cursorpos : bool {
  cursorleft = 0,
  cursorright = 1
};


void ReturnToMenu()
{
  progstep_goto(0);
  lcd.empty();
  lcd.at(1, 1, "Return Main Menu");
  delay(prompt_time);
}


void Choose_Program()
{
  if (redraw)
  {
    // Clean Up any previous states
    if (POWERSAVE_PT > PWR_PROGRAM_ON)    disable_PT();  //  Put the motors back to idle
    if (POWERSAVE_AUX > PWR_PROGRAM_ON)   disable_AUX(); //
    lcd.empty();                            //  Clear the LCD

    // Select what menu item we want.
    switch (progtype)
    {
      case REG2POINTMOVE:
        draw(66, 1, 1); //lcd.at(1, 1, "New   Point Move");
        lcd.at(1, 5, "2");
        break;

      case REV2POINTMOVE:
        draw(81, 1, 1); //lcd.at(1, 1, "Rev   Point Move");
        lcd.at(1, 5, "2");
        break;

      case REG3POINTMOVE:
        draw(66, 1, 1); //lcd.at(1, 1, "New   Point Move");
        lcd.at(1, 5, "3");
        break;

      case REV3POINTMOVE:
        draw(81, 1, 1); //lcd.at(1, 1, "Rev   Point Move");
        lcd.at(1, 5, "3");
        break;

      case DFSLAVE:
        draw(82, 1, 2); //lcd.at(1, 2, "DF Slave Mode");
        break;

      case SETUPMENU:
        draw(83, 1, 4);  //lcd.at(1,4,"Setup Menu");
        break;

      case PANOGIGA:
        draw(84, 1, 5);  //lcd.at(1,5,"Panorama");
        break;

      case PORTRAITPANO:
        lcd.at(1, 2, "Portrait Pano");
        break;

      case AUXDISTANCE:
        draw(85, 1, 3);  //lcd.at(1,3,"AuxDistance");
        break;

      default:
        lcd.at(1, 2, "Menu Error");
        break;
    }

    draw(65, 2, 1);  //lcd.at(2,1,"UpDown  C-Select");
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
        redraw = true;
        if (progtype)  { progtype--;                  }
        else           { progtype = (MENU_ITEMS - 1); } // accomodating rollover
        break;

      case -1: // Down
        redraw = true;
        if (progtype == (MENU_ITEMS - 1)) { progtype = 0; }
        else                              { progtype++;   }
        break;
    }

    switch (HandleButtons())
    {
      case C_Pressed:
        lcd.empty();
  
        switch (progtype)
        {
          case REG2POINTMOVE: //new 2 point move
            REVERSE_PROG_ORDER = false;
            reset_prog = 1;
            set_defaults_in_ram(); //put all basic parameters in RAM
            draw(6, 1, 3); //lcd.at(1,3,"Params Reset");
            delay(prompt_time);
            lcd.empty();
            progstep_goto(1);
            break;
  
          case REV2POINTMOVE:   //reverse beta 2Pt
            REVERSE_PROG_ORDER = true;
            reset_prog = 1;
            set_defaults_in_ram(); //put all basic paramters in RAM
            draw(6, 1, 3); //lcd.at(1,3,"Params Reset");
            delay(prompt_time);
            lcd.empty();
            progstep_goto(1);
            break;
  
          case REG3POINTMOVE: //new 3 point move
            REVERSE_PROG_ORDER = false;
            reset_prog = 1;
            set_defaults_in_ram(); //put all basic parameters in RAM
            draw(6, 1, 3); //lcd.at(1,3,"Params Reset");
            delay(prompt_time);
            lcd.empty();
            progstep_goto(101); //three point move
            break;
  
          case REV3POINTMOVE: //new 3 point move reverse
            REVERSE_PROG_ORDER = true;
            reset_prog = 1;
            set_defaults_in_ram(); //put all basic paramters in RAM
            draw(6, 1, 3); //lcd.at(1,3,"Params Reset");
            delay(prompt_time);
            lcd.empty();
            progstep_goto(101); //three point move
            break;
  
          case DFSLAVE:  //DFMode
            lcd.empty();
            if (PINOUT_VERSION == 4)  lcd.at(1, 1, "eMotimo TB3Black");
            if (PINOUT_VERSION == 3)  lcd.at(1, 1, "eMotimoTB3Orange");
            lcd.at(2, 1, "Dragonframe 1.26");
            DFSetup();
            DFloop();
            break;
  
          case PANOGIGA:   //Pano Beta
            REVERSE_PROG_ORDER = false;
            Trigger_Type = Static_Time_Trigger; // default this for static time selection
            interval = 100;//default this to low value to insure we don't have left over values from old progam delaying shots.
            progstep_goto(200);
            break;
  
          case PORTRAITPANO:  //Pano Beta
            REVERSE_PROG_ORDER = false;
            reset_prog = 1;
            set_defaults_in_ram(); //put all basic parameters in RAM
            draw(6, 1, 3); //lcd.at(1,3,"Params Reset");
            delay(prompt_time);
            lcd.empty();
            progstep_goto(301);
            break;
  
          case SETUPMENU:   //setup menu
            progstep_goto(900);
            break;
  
          case AUXDISTANCE:   //
            REVERSE_PROG_ORDER = false;
            reset_prog = 1;
            set_defaults_in_ram(); //put all basic paramters in RAM
            draw(6, 1, 3); //lcd.at(1,3,"Params Reset");
            delay(prompt_time);
            lcd.empty();
            progstep_goto(401);
            break;
        }
        break;
    }
  }
}


//Move to Start Point
void Move_to_Startpoint()
{
  if (redraw) {
    lcd.empty();
    lcd.bright(LCD_BRIGHTNESS_RUNNING);
    if (!REVERSE_PROG_ORDER) //normal programing, start first
    {
      draw(8, 1, 1); //lcd.at(1, 1, "Move to Start Pt");
      draw(14, 2, 6); //lcd.at(2, 6, "C-Next");
    }
    else
    {
      draw(15, 1, 1); //lcd.at(1,1,"Move to End Pt.");
      draw(3, 2, 1); //lcd.at(2,1,CZ1);
    }
    delay(prompt_time);

	  prev_joy_x_reading = 0; //prevents buffer from moving axis from previous input
    prev_joy_y_reading = 0;
    redraw = false;
  } //end of first time

  if ((millis() - NClastread) > NCdelay)
  {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    applyjoymovebuffer_exponential();
    dda_move(feedrate_micros);
    button_actions_move_start(); //read buttons, look for home set on c
  }
} //end move to start point


void button_actions_move_start()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      lcd.empty();
      set_position(0, 0, 0); //sets current steps to 0
#if DEBUG
      Serial.print("current_steps_start.x: "); Serial.println(current_steps.x);
      Serial.print("current_steps_start.y: "); Serial.println(current_steps.y);
      Serial.print("current_steps_start.z: "); Serial.println(current_steps.z);
#endif
      if (!REVERSE_PROG_ORDER) draw(9, 1, 3); //lcd.at(1,3,"Start Pt. Set");
      else                     draw(16, 1, 3); //lcd.at(1,3,"End Point Set");
      delay(prompt_time);
      progstep_forward();
      break;

    case Z_Pressed:
      ReturnToMenu();
      break;
  }
}


void Move_to_Endpoint()
{
  if (redraw)
  {
    prev_joy_x_reading = 0; //prevents buffer from moving axis from previous input
    prev_joy_y_reading = 0;

    if ( reset_prog == 0 ) {
      lcd.empty();
      draw(11, 1, 1);//lcd.at(1, 1, "Moving to stored");
      draw(12, 2, 4);//lcd.at(2, 4, "end point");

      int32_t x = motor_steps_pt[2][0];
      int32_t y = motor_steps_pt[2][1];
      int32_t z;
      if (AUX_ON) z = motor_steps_pt[2][2];
      else z = 0.0;

      set_target(x, y, z);
      if (AUX_ON) dda_move(10); //send the motors home
      else dda_move(30);

      lcd.empty();
      draw(13, 1, 1);//lcd.at(1, 1, "Confirm or Move");
      draw(3, 2, 1);//lcd.at(2, 1, CZ1);

      delay(prompt_time);
      redraw = false;
    }
    else { //routine for just moving to end point if nothing was stored.
      lcd.empty();
      if (!REVERSE_PROG_ORDER) //mormal programming, end point last
      {
        draw(15, 1, 1);//lcd.at(1, 1, "Move to End Pt.");
        draw(3, 2, 1);//lcd.at(2, 1, CZ1);
      }

      if (REVERSE_PROG_ORDER) //reverse programing, start last
      {
        draw(8, 1, 1);//lcd.at(1, 1, "Move to Start Pt");
        draw(14, 2, 6);//lcd.at(2, 6, "C-Next");
      }
      delay(prompt_time);
      enable_PT();
      if (AUX_ON) enable_AUX();  //
      redraw = false;
    }
  }

  if ((millis() - NClastread) > NCdelay)
  {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    applyjoymovebuffer_exponential();
    dda_move(feedrate_micros);
    button_actions_move_end();  //read buttons, look for home set on c
  }
}


void button_actions_move_end()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      motor_steps_pt[2][0] = current_steps.x; //now signed storage
      motor_steps_pt[2][1] = current_steps.y;
      motor_steps_pt[2][2] = current_steps.z;

#if DEBUG_MOTOR
      Serial.println("motor_steps_end");
      Serial.print(motor_steps_pt[2][0]); Serial.print(",");
      Serial.print(motor_steps_pt[2][1]); Serial.print(",");
      Serial.print(motor_steps_pt[2][2]); Serial.println();
#endif

      lcd.empty();

      if (!REVERSE_PROG_ORDER) draw(16, 1, 3); //lcd.at(1, 3, "End Point Set");
      else                     draw(9, 1, 3); //lcd.at(1, 3, "Start Pt. Set");

      delay(prompt_time);
      progstep_forward();
      break;
      
    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void Move_to_Point_X(uint8_t Point)
{
  if (redraw)
  {
    lcd.empty();
    if (progstep == 201 || progstep == 301)
    {
      // Programming center for PANOGIGA AND PORTRAITPANO UD010715
      lcd.at(1, 2, "Set AOV Corner");
      set_position(0, 0, 0);
    }
    else if (progstep == 303)
    {
      lcd.at(1, 1, "Move to Subject ");
    }
    else
    {
      draw(10, 1, 1); //lcd.at(1,1,"Move to Point");
      if (!REVERSE_PROG_ORDER)
      { //normal programming
        lcd.at(1, 15, (Point + 1));
      }
      else
      { //reverse programming
        if      (Point == 0) lcd.at(1, 15, "3");
        else if (Point == 1) lcd.at(1, 15, "2");
        else if (Point == 2) lcd.at(1, 15, "1");
      }
    }

    if (Point == 0) draw(14, 2, 6); //lcd.at(2,6,"C-Next");
    else draw(3, 2, 1); //lcd.at(2,1,CZ1);

    //   Velocity Engine update
    DFSetup(); //setup the ISR
    delay(prompt_time);
    redraw = false;
  }

  //  Velocity Engine update
  if (!nextMoveLoaded && (millis() - NClastread) > NCdelay)
  {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    updateMotorVelocities2();
    button_actions_move_x(Point); //check buttons
  }
}


void button_actions_move_x(uint8_t Point)
{
  switch (HandleButtons())
  {
    case CZ_Held:
        if (progstep == 201 || progstep == 301)  set_position(0, 0, 0);
        break;
            
    case C_Pressed:
        //begin to stop the motors
        //this puts input to zero to allow a stop
        NunChuckClearData();
  
        do //run this loop until the motors stop
        {
          //Serial.print("motorMoving:");Serial.println(motorMoving);
          if (!nextMoveLoaded)
          {
            updateMotorVelocities2();
          }
        }
        while (motorMoving);
        //end stop the motors
  
        if (Point == 0) set_position(0, 0, 0); //reset for home position
  
        motor_steps_pt[Point][0] = current_steps.x; //now signed storage
        motor_steps_pt[Point][1] = current_steps.y;
        motor_steps_pt[Point][2] = current_steps.z;
  #if DEBUG_MOTOR
        Serial.print("motor_steps_point ");
        Serial.print(Point); Serial.print(";");
        Serial.print(motor_steps_pt[Point][0]); Serial.print(",");
        Serial.print(motor_steps_pt[Point][1]); Serial.print(",");
        Serial.print(motor_steps_pt[Point][2]); Serial.println();
  #endif
  
        if (progstep == 202) //pano - calculate other values UD050715
        {
          lcd.at (1, 1, " Corner 1 Set ");
          delay(prompt_time);
          progstep_goto(200);
          break;
        }
        else if (progstep == 203) //pano - calculate other values UD050715
        {
          calc_pano_move();
          lcd.at (1, 1, " Corner 2 Set ");
          delay(prompt_time);
          progstep_goto(200);
          break;
        }
        else if (progstep == 303) // PORTRAITPANO Method UD050715
        {
          lcd.empty();
          lcd.at (1, 4, "Center Set");
          delay(prompt_time);
        }
        progstep_forward();
        break;
     

    case Z_Held:
        //this puts input to zero to allow a stop
        NunChuckClearData();
  
        do //run this loop until the motors stop
        {
          //Serial.print("motorMoving:");Serial.println(motorMoving);
          if (!nextMoveLoaded)
          {
            updateMotorVelocities2();
          }
        }
        while (motorMoving); // Wait for the motors to stop
        if (progstep == 201 || progstep == 202 || progstep == 203) progstep_goto(200);
        else                                                       progstep_backward();
      break;
  }
}


//Set Camera Interval
void Set_Cam_Interval()
{
  if (redraw)
  {
    lcd.empty();
    draw(17, 1, 1); //lcd.at(1, 1, "Set Sht Interval");
    delay(prompt_time);
    lcd.empty();
    draw(18, 1, 1); //lcd.at(1, 1, "Intval:   .  sec");
    draw(3, 2, 1); //lcd.at(2, 1, CZ1);
    DisplayInterval(Trigger_Type);
    redraw = false;
  }

  if ((millis() - NClastread) > NCdelay) {
    uint16_t Trigger_Type_last = Trigger_Type;
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    
    Trigger_Type += joy_capture3(1);
    uint16_t maxval = 9000; // 15 minutes for now
    uint16_t overflowval = max(maxval + 100, 65400);
    if (!Trigger_Type || Trigger_Type > overflowval) { Trigger_Type = maxval; }
    else if (Trigger_Type > maxval)                  { Trigger_Type = 1; }
    
    if (Trigger_Type_last != Trigger_Type) {
      DisplayInterval(Trigger_Type);
      delay(prompt_delay);
    }
    button_actions_intval();  //read buttons, look for c button press to set interval
  }
}


void DisplayInterval(uint16_t shot_time)
{
  switch(shot_time)
  {
    case External_Trigger: draw(19, 1, 8);          break; //lcd.at(1,8," Ext.Trig");
    case Button_Trigger:   lcd.at(1, 8, " Button"); break;
    case Video_Trigger:    draw(20, 1, 8);          break; //lcd.at(1,8," Video   ");
    default: shot_time -= 3;
  }

  if (shot_time < 10)
  {
    lcd.at(1, 13, " sec");
    lcd.at(1, 8, "  0.");
    lcd.at(1, 10, shot_time / 10);
    lcd.at(1, 12, shot_time % 10);
  }
  else if (shot_time < 100)
  {
    lcd.at(1, 8, "  ");
    lcd.at(1, 10, shot_time / 10);
    lcd.at(1, 12, shot_time % 10);
  }
  else if (shot_time < 1000)
  {
    lcd.at(1, 8, " ");
    lcd.at(1, 9, shot_time / 10);
    lcd.at(1, 12, shot_time % 10);
  }
  else
  {
    lcd.at(1, 8, shot_time / 10);
    lcd.at(1, 12, shot_time % 10);
  }
}


void button_actions_intval()
{
  switch (HandleButtons())
  {
    case C_Pressed:
    {
      switch (Trigger_Type)
      {
        case External_Trigger:  interval = max_shutter * 100;  break;  // Hardcode this to 60 seconds are 1 shots per minute to allow for max shot selection
        case Video_Trigger:     interval = 100;                break;  // means video, set very small exposure time
        default:                interval = (Trigger_Type - 3) * 100;   // tenths of a second to ms, subtracting the trigger modes so time between moves can go down to 0.1 seconds
      }

      lcd.empty();
      draw(21, 1, 3); //lcd.at(1,3,"Interval Set"); // try this to push correct character
      delay(prompt_time);
      progstep_forward();
      break;
    }

    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void Set_Duration() //This is really setting frames
{
  if (redraw)
  {
    lcd.empty();
    //			1234567890123456
    draw(32, 1, 5); //lcd.at(1,5,	"Set Move");
    draw(33, 2, 5); //lcd.at(2,5,	"Duration");
    delay(prompt_time);
    lcd.empty();
    draw(34, 1, 10); //lcd.at(1,9,"H:MM:SS");
    if (Trigger_Type >= External_Trigger)
    {
      lcd.at(2, 11, "Frames"); //SMS
      camera_total_shots = camera_moving_shots;
      Display_Duration();
    }
    else
    {
      draw(3, 2, 1); //lcd.at(2,1,CZ1); //Video
      calc_time_remain_dur_sec (overaldur);
      display_time(1, 1);
    }
    camera_total_shots = camera_moving_shots;
    camera_fired = 0;
    redraw = false;
  }
  
  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    if (Trigger_Type == Video_Trigger) // video
    {
      uint16_t overaldur_last = overaldur;
      overaldur += joy_capture3(1);
      uint16_t maxval = 10000;
      uint16_t overflowval = max(maxval + 100, 65400);
      if (!overaldur || overaldur > overflowval) { overaldur = maxval; }
      else if (overaldur > maxval)               { overaldur = 1;  }
      
      if (overaldur_last != overaldur)
      {
        calc_time_remain_dur_sec (overaldur);
        display_time(1, 1);
        delay(prompt_delay);
      }
    }
    else // sms
    {
      uint32_t camera_moving_shots_last = camera_moving_shots;
      camera_moving_shots += joy_capture3(1);
      uint16_t maxval = 10000;
      uint16_t overflowval = max(maxval + 100, 65400);
      if (camera_moving_shots < 10 || camera_moving_shots > overflowval) { camera_moving_shots = maxval; }
      else if (camera_moving_shots > maxval)                             { camera_moving_shots = 10;  }
      
      camera_total_shots = camera_moving_shots; //we add in lead in lead out later
      if (camera_moving_shots_last != camera_moving_shots) {
        Display_Duration();
        delay(prompt_delay);
      } //end update time and shots
    } //end sms

    button_actions_overaldur();  //read buttons, look for c button press to set overall time
  }
}


void Display_Duration()
{
  calc_time_remain(total_pano_move_time);
  display_time(1, 1);

  if (Trigger_Type >= External_Trigger)
  {
    if (camera_total_shots < 100)
    {
      lcd.at(2, 7, camera_total_shots);
      lcd.at(2, 4, "   ");
    }
    else if (camera_total_shots < 1000)
    {
      lcd.at(2, 6, camera_total_shots);
      lcd.at(2, 4, "  ");
    }
    else if (camera_total_shots < 10000)
    {
      lcd.at(2, 5, camera_total_shots);
      lcd.at(2, 4, " ");
    }
    else
    {
      lcd.at(2, 4, camera_total_shots);
    }
  }
}


void button_actions_overaldur()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      lcd.empty();
      draw(35, 1, 3); //lcd.at(1,3,"Duration Set");
      delay (prompt_time);
      progstep_forward();
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void Set_Static_Time()
{
  if (redraw)
  {
    lcd.empty();
    draw(22, 1, 1); //lcd.at(1,1,"Set Static Time");
    delay(prompt_time);
    lcd.empty();
    draw(23, 1, 1); //lcd.at(1,1,"Stat_T:   .  sec");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    max_shutter = Trigger_Type - MIN_INTERVAL_STATIC_GAP; //max static is .3 seconds less than interval (leaves 0.3 seconds for move)
    if (Trigger_Type == External_Trigger) max_shutter = 600; //external trigger = 60.0 Seconds7
    if (progtype == PANOGIGA || progtype == PORTRAITPANO) max_shutter = 36000; //pano modes = 3600.0 Seconds
    DisplayStatic_tm(static_tm);
    redraw = false;
  }

  if ((millis() - NClastread) > NCdelay) {
    uint16_t static_tm_last = static_tm;
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    static_tm += joy_capture3(1);
    uint16_t maxval = max_shutter;
    uint16_t overflowval = max(maxval + 100, 65400);
    if (!static_tm || static_tm > overflowval) { static_tm = maxval; }
    else if (static_tm > maxval)               { static_tm = 1; }
    
    if (static_tm_last != static_tm) {
      DisplayStatic_tm(static_tm);
      delay(prompt_delay);
    }
    button_actions_stat_time();  //read buttons, look for c button press to set interval
  }
}


void DisplayStatic_tm(uint16_t static_time)
{
  if (Trigger_Type == Video_Trigger)	draw(24, 1, 8); //lcd.at(1,8," Video   ");

  else if (static_time < 10)
  {
    lcd.at(1, 8, " 0.   ");
    lcd.at(1, 11, static_time % 10);
  }
  else if (static_tm < 100)
  {
    lcd.at(1, 8, "  .   ");
    lcd.at(1, 9, static_time / 10);
    lcd.at(1, 11, static_time % 10);
  }
  else if (static_tm < 1000)
  {
    lcd.at(1, 8, "   .  ");
    lcd.at(1, 9, static_time / 10);
    lcd.at(1, 12, static_time % 10);
  }
  else if (static_tm < 10000)
  { 
    lcd.at(1, 8, "    . ");
    lcd.at(1, 9, static_tm / 10);
    lcd.at(1, 13, static_tm % 10);
  }
  else
  {
    lcd.at(1, 8, "    . ");
    lcd.at(1, 8, static_time / 10);
    lcd.at(1, 13, static_time % 10);
  }
}


void button_actions_stat_time()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      //if (Trigger_Type != 3)  camera_exp_tm = (long)static_tm*100; //want to get to ms, but this is already multipled by 10 // 3 is used for video mode - this is likely obsolete-remove later
      lcd.empty();
      draw(25, 1, 1); //lcd.at(1,1,"Static Time Set"); // try this to push correct character
      delay(prompt_time);
      if (progstep == 205) progstep_goto(200);
      else                 progstep_forward();
      break;

    case Z_Pressed:
      if (progstep == 205) progstep_goto(200);
      else                 progstep_backward();
      break;

    case CZ_Pressed:
      static_tm = 1;
      break;
  }
}


void Set_Ramp()
{
  if (redraw)
  {
    lcd.empty();
    draw(29, 1, 1); //lcd.at(1,1,"	Set Ramp");
    delay(prompt_time);
    lcd.empty();
    draw(30, 1, 1); //lcd.at(1,1,"Ramp:	 Frames");
    if (Trigger_Type == Video_Trigger) {
      camera_moving_shots = 147; //allow for up to 49 % ramp
      lcd.at(1, 10, "Percent");
    }
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    DisplayRampval();
    redraw = false;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > NCdelay) {
    uint16_t rampval_last = rampval;
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    
    rampval += joy_capture3(1);
    uint16_t maxval = 10000;
    uint16_t overflowval = max(maxval + 100, 65400);
    if (!rampval || rampval > overflowval) { rampval = maxval; }
    else if (rampval > maxval)             { rampval = 1;  }
  
    if ( rampval * 3 > camera_moving_shots ) {	// we have an issue where the ramp is to big can be 2/3 of total move, but not more.
      rampval = camera_moving_shots / 3; //set to 1/3 up and 1/3 down (2/3) of total move)
    }
  
    if (rampval_last != rampval) {
      DisplayRampval();
      delay (prompt_delay);
    }
    button_actions_rampval();  //read buttons, look for c button press to set interval
  }
}


void DisplayRampval()
{
  lcd.at(1, 7, rampval);
  if		  (rampval < 10)  lcd.at(1, 8, "  "); //clear extra if goes from 3 to 2 or 2 to  1
  else if (rampval < 100)  lcd.at(1, 9, " "); //clear extra if goes from 3 to 2 or 2 to  1
}


void button_actions_rampval()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      //we actually don't store anything here - wait for final confirmation to set everything
      lcd.empty();
      draw(31, 1, 5); //lcd.at(1,5,"Ramp Set");
      delay(prompt_time);
      progstep_forward();
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void Set_LeadIn_LeadOut()
{
  if (redraw)
  {
    lcd.empty();
    draw(36, 1, 1); //lcd.at(1,1,"Set Static Lead");
    draw(37, 2, 2); //lcd.at(2,2,"In/Out Frames");
    delay(prompt_time);
    lcd.empty();
    draw(38, 1, 6); //lcd.at(1,1,"IN -	Out");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    redraw = false;
    lcd.at(1, 9, lead_out);
    cursorpos = cursorleft;
    DisplayLeadIn_LeadOut();
  }


  if ((millis() - NClastread) > NCdelay) {
    int8_t joyxysum_last = joy_x_axis ^ joy_y_axis; //figure out if changing
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    cursorpos += joy_capture_x_map();
  
    if (joyxysum_last != (joy_x_axis ^ joy_y_axis)) { //check to see if there is an input, otherwise don't update display
      DisplayLeadIn_LeadOut();
      delay(prompt_delay);
    }
 
    button_actions_lead_in_out();  //read buttons, look for c button press to ramp
  }
}


void DisplayLeadIn_LeadOut()
{
  if (cursorpos == cursorright) lcd.pos(1, 13);

  if (cursorpos == cursorleft) { //update lead in
    lcd.cursorOff();

    lead_in += joy_capture3(1);
    uint16_t maxval = 5000;
    uint16_t overflowval = max(maxval + 100, 65400);
    if (!lead_in || lead_in > overflowval) { lead_in = maxval; }
    else if (lead_in > maxval)             { lead_in = 1;    }
    delay(prompt_delay);
    
    lcd.at(1, 1, lead_in);

    if (lead_in < 10)  {
      lcd.at(1, 2, "   ");
    }
    else if (lead_in < 100)  {
      lcd.at(1, 3, "  ");
    }
    else if (lead_in < 1000) {
      lcd.at(1, 4, " ");
    }
    lcd.cursorBlock();
  }
  else
  { //update leadout
    lcd.cursorOff();
    
    lead_out += joy_capture3(1);
    uint16_t maxval = 5000;
    uint16_t overflowval = max(maxval + 100, 65400);
    if (!lead_out || lead_out > overflowval) { lead_out = maxval; }
    else if (lead_out > maxval)              { lead_out = 1;    }
    delay(prompt_delay);

    lcd.at(1, 9, lead_out);

    if (lead_out < 10)   {
      lcd.at(1, 10, "   ");
    }
    else if (lead_out < 100)  {
      lcd.at(1, 11, "  ");
    }
    else if (lead_out < 1000) {
      lcd.at(1, 12, " ");
    }
    lcd.cursorBlock();
  }
}


void button_actions_lead_in_out()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      lcd.cursorOff();
      lcd.empty();
      draw(39, 1, 1); //lcd.at(1,1,"Lead Frames Set");
      Calculate_Shot();

      /*		//start  of code block to be moved from LeadinLeadOut.  Replaced with Calculate_Shot()
        camera_total_shots=camera_moving_shots+lead_in+lead_out;

        if (Trigger_Type == Video_Trigger) {
      	  //if (Trigger_Type == Video_Trigger) camera_moving_shots=(overaldur*1000L)/interval; //figure out moving shots based on duration for video only
      	  //camera_moving_shots = video_segments; //hardcode this and back into proper interval - this is XXX segments per sequence
      	  //camera_moving_shots = 100; //new method to allow for easy ramp %
      	  //interval = overaldur * 1000L / camera_moving_shots;   //This gives us ms
      	  camera_total_shots = camera_moving_shots + lead_in + lead_out; //good temp variable for display
        }

        //fix issues with ramp that is too big
        if( rampval * 3 > camera_moving_shots )
        {	// we have an issue where the ramp is to big can be 2/3 of total move, but not more.
          rampval = camera_moving_shots / 3; //set to 1/3 up and 1/3 down (2/3) of total move)
        }

        //Set keyframe points for program to help with runtime calcs
        keyframe[0][0]=0; //start frame
        keyframe[0][1]=lead_in; //beginning of ramp
        keyframe[0][2]=lead_in+rampval; //end of ramp, beginning of linear
        keyframe[0][3]=lead_in+camera_moving_shots-rampval;  //end or linear, beginning of rampdown
        keyframe[0][4]=lead_in+camera_moving_shots; //end of rampdown, beginning of leadout
        keyframe[0][5]=lead_in+camera_moving_shots+lead_out; //end of leadout, end of program

        if (DEBUG_MOTOR) {
      	  for (int i=0; i < 6; i++){
      		Serial.print("Keyframe");Serial.print(i);Serial.print("_");Serial.println(keyframe[0][i]);
      	  }
        }

        //go_to_origin_slow();
        go_to_start_new();

        //write_all_ram_to_eeprom(); //set this here to allow us to rerun this program from this point if we just want to turn it on review and go
        //restore_from_eeprom_memory();
        if (DEBUG) review_RAM_Contents();

        //delay(prompt_time);
        if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
        if (POWERSAVE_AUX > PWR_PROGRAM_ON)   disable_AUX();
      */  //end of code block to be moved from LeadinLeadOut

      progstep_forward();
      break;

    case Z_Pressed:
      lcd.cursorOff();
      progstep_backward();
      break;
  }
}


void Calculate_Shot() //this used to reside in LeadInLeadout, but now pulled.
{ //start  of code block to be moved from LeadinLeadOut
  unsigned int video_segments = 150; //arbitrary
  camera_total_shots = camera_moving_shots + lead_in + lead_out;

  if (Trigger_Type == Video_Trigger)
  {
    //really only need this for 3 point moves now - this could screw up 2 points, be careful
    camera_moving_shots = video_segments; //hardcode this and back into proper interval - this is XXX segments per sequence
    interval = overaldur * 1000L / camera_moving_shots;   //This gives us ms for our delays
    camera_total_shots = camera_moving_shots + lead_in + lead_out; //good temp variable for display
  }

  //fix issues with ramp that is too big
  if ( rampval * 3 > camera_moving_shots ) {	// we have an issue where the ramp is to big can be 2/3 of total move, but not more.
    rampval = camera_moving_shots / 3; //set to 1/3 up and 1/3 down (2/3) of total move)
  }

  //Set keyframe points for program to help with runtime calcs
  keyframe[0][0] = 0; //start frame
  keyframe[0][1] = lead_in; //beginning of ramp
  keyframe[0][2] = lead_in + rampval; //end of ramp, beginning of linear
  keyframe[0][3] = lead_in + camera_moving_shots - rampval; //end or linear, beginning of rampdown
  keyframe[0][4] = lead_in + camera_moving_shots; //end of rampdown, beginning of leadout
  keyframe[0][5] = lead_in + camera_moving_shots + lead_out; //end of leadout, end of program

#if DEBUG_MOTOR
  for (uint8_t i = 0; i < 6; i++) {
    Serial.print("Keyframe");
    Serial.print(i);
    Serial.print("_");
    Serial.println(keyframe[0][i]);
  }
#endif

  //go_to_origin_slow();
  go_to_start_new();

  //write_all_ram_to_eeprom(); //set this here to allow us to rerun this program from this point if we just want to turn it on review and go
  //restore_from_eeprom_memory();
#if DEBUG
  review_RAM_Contents();
#endif

  //delay(prompt_time);
  if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
  if (POWERSAVE_AUX > PWR_PROGRAM_ON)   disable_AUX();
  //end of code block pulled from LeadinLeadOut
}


void Review_Confirm()
{
  if (redraw) {
    lcd.empty();
    draw(41, 1, 4); //lcd.at(1,4,"Review and");
    draw(42, 2, 2); //lcd.at(2,2,"Confirm Setting");
    delay(prompt_time);
    display_last_tm = millis();
    DisplayReviewProg();
    reviewprog = 1;
    start_delay_sec = 0;
    redraw = false;
  }

  if ((millis() - display_last_tm) > (prompt_time * 4)) { //test for display update
    //if ((millis()-diplay_last_tm) >(700)){ //test for display update

    reviewprog ++;
    display_last_tm = millis();
    if (reviewprog > 4) reviewprog = 1;
    redraw2 = true;
    DisplayReviewProg();
  } //end test for display update

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    if (joy_y_axis) { //do read time updates to delay program
      reviewprog = 4;
      DisplayReviewProg();
      delay (prompt_delay);
      display_last_tm = millis();
    }
    button_actions_review();
  }
}


void DisplayReviewProg()
{
  switch (reviewprog)
  {
    case 1:   //
      lcd.empty();
      draw(43, 1, 1); //lcd.at(1,1,"Pan Steps:");
      draw(44, 2, 1); //lcd.at(2,1,"Tilt Steps:");
      lcd.at(1, 12, linear_steps_per_shot[0]);
      lcd.at(2, 12, linear_steps_per_shot[1]);
      break;

    case 2:   //
      lcd.empty();
      draw(45, 1, 1); //lcd.at(1,1,"Cam Shots:");
      draw(46, 2, 1); //lcd.at(2,1,"Time:");
      lcd.at(1, 12, camera_total_shots);
      calc_time_remain();
      display_time(2, 6);
      break;

    case 3:   //
      lcd.empty();
      draw(47, 1, 6); //lcd.at(1,6,"Ready?");
      draw(48, 2, 2); //lcd.at(2,2,"Press C Button");
      break;

    case 4:   //
      //lcd.empty();
      if (redraw2) {
        lcd.at(1, 1, "Set Start Delay");
        lcd.at(2, 1, "			");
        draw(34, 2, 10); //lcd.at(2,10,"H:MM:SS");
        redraw2 = false;
      }
      // Clip it down to a uint16_t
      start_delay_sec += joy_capture3(1);
      uint16_t maxval = max_shutter;
      uint16_t overflowval = max(maxval + 100, 65400);
      if (start_delay_sec > overflowval) { start_delay_sec = 36000; }
      else if (start_delay_sec > 36000)  { start_delay_sec = 0; }
      delay(prompt_delay);

      calc_time_remain_dur_sec (start_delay_sec);
      display_time(2, 1);
      break;
  }//end switch
}


void button_actions_review()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      lcd.empty();
      MOVE_REVERSED_FOR_RUN = false; //Reset This
      start_delay_tm = ((millis() / 1000L) + start_delay_sec); //system seconds in the future - use this as big value to compare against
      draw(34, 2, 10); //lcd.at(2,10,"H:MM:SS");
      lcd.at(1, 2, "Delaying Start");

      while (start_delay_tm > millis() / 1000L)
      {
        //enter delay routine
        calc_time_remain_start_delay ();
        if ((millis() - display_last_tm) > 1000) {
          display_time(2, 1);
        }

        if ((millis() - NClastread) > NCdelay) {
          NClastread = millis();
          NunChuckRequestData();
          NunChuckProcessData();
          if (HandleButtons() == CZ_Held && !Program_Engaged) {
            start_delay_tm = ((millis() / 1000L) + 5); //start right away by lowering this to 5 seconds.
          }
        }
      }

      enable_PT();
      if (AUX_ON) enable_AUX();  //

      Program_Engaged = true;
      Flag_Wait_For_Trigger = true; //just to start off first shot immediately
      interval_tm = 0; //set this to 0 to immediately trigger the first shot
      sequence_repeat_count = 0; //this is zeroed out every time we start a new shot

      if (Trigger_Type > External_Trigger) { //SMS Mode
        lcd.empty(); //clear for non video
        progstep = 50; //  move to the main programcamera_real_fire
      }
      else if (Trigger_Type == Video_Trigger)  {
        lcd.empty();
        draw(49, 1, 1); //lcd.at(1,1,"Program Running");
        progstep = 51;
      }
      else if (Trigger_Type == External_Trigger)  { //manual trigger/ext trigger
        lcd.empty(); //clear for non video
        progstep = 52; //  move to the external interrupt loop
        ext_shutter_count = 0; //set this to avoid any prefire shots or cable insertions.
        lcd.at(1, 1, "Waiting for Trig");
      }
      redraw = true;
      lcd.bright(LCD_BRIGHTNESS_RUNNING);
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void progstep_forward()
{
  redraw = true;
  progstep_forward_dir = true;
  if (progstep < 65535) progstep++;
}


void progstep_backward()
{
  redraw = true;
  progstep_forward_dir = false;
  if (progstep) progstep--;
}


void progstep_goto(uint16_t prgstp)
{
  redraw = true;
  progstep = prgstp;
}


void button_actions_end_of_program()
{ //repeat - need to turn off the REVERSE_PROG_ORDER flag
  switch (HandleButtons())
  {
    case C_Pressed:
      // Normal
      REVERSE_PROG_ORDER = false;
      if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
      if (POWERSAVE_AUX > PWR_PROGRAM_ON)   disable_AUX();
      //Program_Engaged=true;
      camera_fired = 0;
      lcd.bright(LCD_BRIGHTNESS_MENU);
      if (progtype == REG2POINTMOVE || progtype == REV2POINTMOVE) {
        go_to_start_new();
        progstep_goto(8);
      }
      else if (progtype == REG3POINTMOVE || progtype == REV3POINTMOVE) {
        go_to_start_new();
        progstep_goto(109);
      }
      else if (progtype == AUXDISTANCE) {
        go_to_start_new();
        progstep_goto(8);
      }
      break;

    case Z_Pressed:
      REVERSE_PROG_ORDER = true;
      if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
      if (POWERSAVE_AUX > PWR_PROGRAM_ON)   disable_AUX();
      //Program_Engaged=true;
      camera_fired = 0;
      lcd.bright(LCD_BRIGHTNESS_MENU);
      if (progtype == REG2POINTMOVE || progtype == REV2POINTMOVE) {
        go_to_start_new();
        progstep_goto(8);
      }
      else if (progtype == REG3POINTMOVE || progtype == REV3POINTMOVE) {
        go_to_start_new();
        progstep_goto(109);
      }
      else if (progtype == AUXDISTANCE) {
        go_to_start_new();
        progstep_goto(8);
      }
      break;
  }
}


void Auto_Repeat_Video()
{ //Auto Reverse
  sequence_repeat_count++;
  REVERSE_PROG_ORDER = true;
  camera_fired = 0;
  lcd.bright(LCD_BRIGHTNESS_RUNNING);
  if (progtype == REG2POINTMOVE || progtype == REV2POINTMOVE) {
    go_to_start_new();
    //progstep_goto(8);
  }

  lcd.empty();
  MOVE_REVERSED_FOR_RUN = false; //Reset This
  //start_delay_tm=((millis()/1000L)+start_delay_sec); //system seconds in the future - use this as big value to compare against
  //draw(34,2,10);//lcd.at(2,10,"H:MM:SS");
  //lcd.at(1,2,"Delaying Start");

  /*
    while (start_delay_tm>millis()/1000L) {
  	//enter delay routine
  	calc_time_remain_start_delay ();
  	if ((millis()-display_last_tm) > 1000) display_time(2,1);
  	if ((millis() - NClastread) > NCdelay) {
      NClastread = millis();
      NunChuckRequestData();
      NunChuckProcessData();
    	if (HandleButtons() == CZ_Held && !Program_Engaged) {
    		start_delay_tm=((millis()/1000L)+5); //start right away by lowering this to 5 seconds.
    	}
     }
    }
  */
  enable_PT();
  enable_AUX();  //

  draw(49, 1, 1); //lcd.at(1,1,"Program Running");
  //delay(prompt_time/3);

  Program_Engaged = true;
  Flag_Wait_For_Trigger = true; //just to start off first shot immediately

  interval_tm = 0; //set this to 0 to immediately trigger the first shot

  if (Trigger_Type > External_Trigger) { //SMS Mode
    lcd.empty(); //clear for non video
    progstep = 50; //  move to the main programcamera_real_fire
  }

  else if (Trigger_Type == Video_Trigger)  {
    //lcd.empty(); //leave "program running up for video
    progstep = 51;
  }
  /*
    else if (Trigger_Type==External_Trigger)  { //manual trigger/ext trigge
  	lcd.empty(); //clear for non video
  	progstep=52; //  move to the external interrup loop
  	ext_shutter_count=0; //set this to avoid any prefire shots or cable insertions.
  	lcd.at(1,1,"Waiting for Trig");
    }
  */
  redraw = true;
  lcd.bright(LCD_BRIGHTNESS_RUNNING);
}//end of 91


void Pause_Prog()
{
  Program_Engaged = !Program_Engaged; //turn off the loop
  if (!Program_Engaged) {  //program turned off
    write_all_ram_to_eeprom(); //capture current steps too!
    lcd.at(1, 11, " Pause");
    delay(prompt_time);
  }
  else { //turn it on
    lcd.at(1, 11, "Resume");
    delay(prompt_time);
  }
}


void display_status()
{
  static uint8_t   batt_low_cnt = 0;
  //1234567890123456
  //1234567890123456
  //XXXX/XXXX LeadIn		LeadOT Rampup RampDn, Pause
  //HH:MM:SS  XX.XXV
  if (redraw)
  {
    lcd.empty();
    lcd.at(1, 6, "/"); //Add to one time display update
    lcd.at(2, 13, ".");
    lcd.at(2, 16, "v");
    redraw = false;
  }
  //update upper left camera fired/total shots
  if      (camera_fired < 10)    lcd.at(1, 5, camera_fired);
  else if (camera_fired < 100)   lcd.at(1, 4, camera_fired);
  else if (camera_fired < 1000)  lcd.at(1, 3, camera_fired);
  else if (camera_fired < 10000) lcd.at(1, 2, camera_fired);
  else								           lcd.at(1, 1, camera_fired);

  lcd.at(1, 7, camera_total_shots);

  //Update program progress secion - upper right

  if (progtype == REG2POINTMOVE || progtype == REV2POINTMOVE || progtype == AUXDISTANCE) {
    switch (Move_State_2PT)
    {
      case LeadIn2PT:    draw(51, 1, 11);  break;  // lcd.at(1,11,"LeadIn");
      case RampUp2PT:    draw(52, 1, 11);  break;  // lcd.at(1,11,"RampUp");
      case Linear2PT:    draw(53, 1, 11);  break;  // lcd.at(1,11,"Linear");
      case RampDown2PT:  draw(54, 1, 11);  break;  // lcd.at(1,11,"RampDn");
      case LeadOut2PT:   draw(55, 1, 11);  break;  // lcd.at(1,11,"LeadOT");
      case Finished2PT:  draw(56, 1, 11);  break;  // lcd.at(1,11,"Finish");
    }
  }

  else if (progtype == REG3POINTMOVE || progtype == REV3POINTMOVE)
  {
    switch (Move_State_3PT)
    {
      case LeadIn3PT:    draw(51, 1, 11);  break;  // lcd.at(1,11,"LeadIn");
      case FirstLeg3PT:  lcd.at(1, 11, "Leg 1 ");  break;
      case SecondLeg3PT: lcd.at(1, 11, "Leg 2 ");  break;
      case LeadOut3PT:   draw(55, 1, 11);  break;  // lcd.at(1,11,"LeadOT");
      case Finished3PT:  draw(56, 1, 11);  break;  // lcd.at(1,11,"Finish");
    }
  }

  else if (progtype == PANOGIGA || progtype == PORTRAITPANO) {
    lcd.at(1, 11, "  Pano");
  }

  //Update Run Clock
  calc_time_remain();
  display_time(2, 1);

  //Do multiple reads of the battery and average
  uint16_t batteryread = 0;
  for (uint8_t i = 0; i < 3; i++) {
    batteryread += analogRead(0); //
  }
  batteryread = batteryread / 3;

  uint8_t batt1 = (batteryread / 51); //  51 point per volt
  uint8_t batt2 = ((batteryread % 51) * 100) / 51; //3 places off less the full decimal

  if (batt1 < 10) {
    lcd.at(2, 11, "0");
    lcd.at(2, 12, batt1);
  }
  else lcd.at(2, 11, batt1);

  if (batt2 < 10) {
    lcd.at(2, 14, "0");
    lcd.at(2, 15, batt2);
  }
  else lcd.at(2, 14, batt2);

  if (POWERDOWN_LV) {
    if (batt1 < 9) {
      draw(7, 2, 1); //lcd.at(2,1,"Low Power");

      batt_low_cnt++;
      if (batt_low_cnt > 20) {
        //Stop the program and go to low power state
        disable_PT();
        disable_AUX();
        Program_Engaged = false;
        lcd.empty();
        draw(60, 1, 1); //lcd.at(1,1,"Battery too low");
        draw(61, 2, 1); //lcd.at(2,1,"  to continue");
      }
      redraw = true;
    }
    else batt_low_cnt = 0;
  }//end of powerdown if
}//end of display

// Time Functions ###############################################################

uint8_t timeh;
uint8_t timem;
uint8_t time_s;

void calc_time_remain()
{
  unsigned long timetotal = ((camera_total_shots - camera_fired) * interval) / 1000;

  timeh =  timetotal / 3600;	// hours
  timem =  timetotal / 60 % 60; // minutes
  time_s = timetotal % 60;		// seconds
}

void calc_time_remain(float totalmovetime)
{
  unsigned long timetotal = ((camera_total_shots - camera_fired) * static_tm * 100) / 1000 + totalmovetime * (camera_total_shots - camera_fired) / camera_total_shots;
  timeh =  timetotal / 3600;  // hours
  timem =  timetotal / 60 % 60; // minutes
  time_s = timetotal % 60;    // seconds
}


void calc_time_remain_dur_sec (unsigned int sec)
{
  timeh =  sec / 3600;	// hours
  timem =  sec / 60 % 60; // minutes
  time_s = sec % 60;		// seconds
}


void calc_time_remain_start_delay ()
{
  unsigned long current_sec = (millis() / 1000);
  timeh =  (start_delay_tm - current_sec) / 3600;	// hours
  timem =  (start_delay_tm - current_sec) / 60 % 60; // minutes
  time_s = (start_delay_tm - current_sec) % 60;		// seconds
}


void display_time (uint8_t row, uint8_t col)
{
  lcd.at(row, col + 2, ":");
  lcd.at(row, col + 5, ":");

  if (timeh < 10) {
    lcd.at(row, col, " ");
    lcd.at(row, col + 1, timeh);
  }
  else lcd.at(row, col, timeh);

  if (timem < 10) {
    lcd.at(row, col + 3, "0");
    lcd.at(row, col + 4, timem);
  }
  else lcd.at(row, col + 3, timem);

  if (time_s < 10) {
    lcd.at(row, col + 6, "0");
    lcd.at(row, col + 7, time_s);
  }
  else lcd.at(row, col + 6, time_s);
}

void draw(uint8_t array_num, uint8_t col, uint8_t row)
{
  strcpy_P(lcdbuffer1, (PGM_P)pgm_read_word(&(setup_str[array_num]))); // Necessary casts and dereferencing, just copy.
  lcd.at(col, row, lcdbuffer1);
}


///START OF BETA CODE

void Enter_Aux_Endpoint()
{
  if (redraw)
  {
    //routine for just moving to end point if nothing was stored.
    lcd.empty();
    lcd.at(1, 1, "Enter Aux End Pt");
    delay(prompt_time);

    //move the aux motor 0.5 inches in the positive direction
    int32_t z = STEPS_PER_INCH_AUX / 2;
    set_target(0, 0, z);

    dda_move(20);

    //Serial.println(current_steps.z);
    // Serial.println(int(current_steps.z/STEPS_PER_INCH_AUX));

    lcd.empty();
    lcd.at(1, 1, "AuxDist:   .  In");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    aux_dist = current_steps.z * 10 / STEPS_PER_INCH_AUX; //t
    DisplayAUX_Dist(aux_dist);
    redraw = false;
  }

  if ((millis() - NClastread) > NCdelay) {
    int32_t aux_dist_last = aux_dist;
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    aux_dist += joy_capture3(1);
    aux_dist = constrain(aux_dist, -MAX_AUX_MOVE_DISTANCE, MAX_AUX_MOVE_DISTANCE);

    //STEPS_PER_INCH_AUX

    if (aux_dist_last != aux_dist) {
      DisplayAUX_Dist(aux_dist);
      delay (prompt_delay);
    }

    //lcd.at(1,1,aux_dist/10);
    //lcd.at(1,7,aux_dist%10);
    //motor_steps_pt[2][2]=(aux_dist*47812L)/10;
    //lcd.at(2,1,(long)motor_steps_pt[2][2]);
    button_actions_Enter_Aux_Endpoint();  //read buttons, look for c button press to set interval
  }
}


void DisplayAUX_Dist(int32_t distance)
{
  uint32_t abs_distance = abs(distance);
  if (abs_distance < 10)
  {
    if (aux_dist < 0)   lcd.at(1, 9, " -0.");
    else				        lcd.at(1, 9, "  0.");
    lcd.at(1, 13, abs_distance % 10);
  }
  else if (abs_distance < 100)
  {
    lcd.at(1, 9, "  ");
    if (aux_dist < 0)   lcd.at(1, 10, distance / 10);
    else				        lcd.at(1, 11, distance / 10);
    lcd.at(1, 13, abs_distance % 10);
  }
  else if (abs_distance < 1000)
  {
    lcd.at(1, 9, " ");
    if (aux_dist < 0)   lcd.at(1, 9, distance / 10);
    else				        lcd.at(1, 10, distance / 10);
    lcd.at(1, 13, abs_distance % 10);
  }
}


void button_actions_Enter_Aux_Endpoint()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      //calculate_deltas();

      lcd.empty();
      motor_steps_pt[0][2] = 0;//this sets the end point
      motor_steps_pt[1][2] = 0;//this sets the end point
      motor_steps_pt[2][2] = aux_dist * STEPS_PER_INCH_AUX / 10;  //this sets the end point
#if DEBUG
      Serial.println(motor_steps_pt[2][2]);
#endif
      lcd.at(1, 2, "Aux End Pt. Set");
      delay(prompt_time);
      progstep_forward();
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}

//END BETA CODE
