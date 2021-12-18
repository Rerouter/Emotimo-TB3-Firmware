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

  =========================================
  PT_LCD_Buttons - Code for Menus and User Interface
  =========================================

*/

int32_t	aux_dist;

// Should belong in Pano, need to shift out
uint32_t  Pan_AOV_steps;
uint32_t  Tilt_AOV_steps;

bool cursorpos = 0;
enum cursorpos : bool {
  cursorleft = 0,
  cursorright = 1
};


uint8_t HandleButtons()
{
  static bool armed = false;
  switch (ButtonState)
  {
    case Released:
      armed = true;
      return 0;
      break;
      
    case C_Pressed:
      if (armed) { armed = false;  return C_Pressed;  }
      else       {                 return Read_Again; }
      break;

    case Z_Pressed:
      if (armed) { armed = false;  return Z_Pressed;  }
      else       {                 return Read_Again; }
      break;

    case CZ_Pressed:
      if (armed) { armed = false;  return CZ_Pressed; }
      else       {                 return Read_Again; }
      break;

    default:
      return Read_Again;
  }
}


void Choose_Program()
{
  if (FLAGS.redraw)
  {
    // Clean Up any previous states
    if (SETTINGS.POWERSAVE_PT > 2)    disable_PT();  //  Put the motors back to idle
    if (SETTINGS.POWERSAVE_AUX > 2)   disable_AUX(); //
    lcd.empty();                            //  Clear the LCD

    // Select what menu item we want.
    switch (EEPROM_STORED.progtype)
    {
      case REG2POINTMOVE:
        draw(66, 1, 1);  //lcd.at(1,1,"New   Point Move");
        lcd.at(1, 5, "2");
        break;

      case REV2POINTMOVE:
        draw(81, 1, 1);  //lcd.at(1,1,"Rev   Point Move");
        lcd.at(1, 5, "2");
        break;

      case REG3POINTMOVE:
        draw(66, 1, 1);  //lcd.at(1,1,"New   Point Move");
        lcd.at(1, 5, "3");
        break;

      case REV3POINTMOVE:
        draw(81, 1, 1);  //lcd.at(1,1,"Rev   Point Move");
        lcd.at(1, 5, "3");
        break;

      case DFSLAVE:
        draw(82, 1, 2);  //lcd.at(1,2,"DF Slave Mode");
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
    delay(GLOBAL.prompt_time/2);
    FLAGS.redraw = false;
  }

  if ((millis() - GLOBAL.NClastread) > 50) {
    GLOBAL.NClastread = millis();

    NunChuckRequestData();
    NunChuckProcessData();
    
    switch(joy_capture_y_map())
    {
      case -1: // Up
        FLAGS.redraw = true;
        if (EEPROM_STORED.progtype)  { EEPROM_STORED.progtype--;               }
        else                  { EEPROM_STORED.progtype = (MENU_ITEMS - 1);  // accomodating rollover
        }
        break;

      case 1: // Down
        FLAGS.redraw = false;
        if (EEPROM_STORED.progtype == (MENU_ITEMS - 1)) { EEPROM_STORED.progtype = 0; }
        else                                     { EEPROM_STORED.progtype++;   }
    }
  }

  switch (HandleButtons())
  {
    case C_Pressed:
      lcd.empty();

      switch (EEPROM_STORED.progtype)
      {
        case REG2POINTMOVE: //new 2 point move
          EEPROM_STORED.REVERSE_PROG_ORDER = false;
          GLOBAL.reset_prog = 1;
          set_defaults_in_ram(); //put all basic parameters in RAM
          draw(6, 1, 3); //lcd.at(1,3,"Params Reset");
          delay(GLOBAL.prompt_time);
          lcd.empty();
          progstep_goto(1);
          break;

        case REV2POINTMOVE:   //reverse beta 2Pt
          EEPROM_STORED.REVERSE_PROG_ORDER = true;
          GLOBAL.reset_prog = 1;
          set_defaults_in_ram(); //put all basic paramters in RAM
          draw(6, 1, 3); //lcd.at(1,3,"Params Reset");
          delay(GLOBAL.prompt_time);
          lcd.empty();
          progstep_goto(1);
          break;

        case REG3POINTMOVE: //new 3 point move
          EEPROM_STORED.REVERSE_PROG_ORDER = false;
          GLOBAL.reset_prog = 1;
          set_defaults_in_ram(); //put all basic parameters in RAM
          draw(6, 1, 3); //lcd.at(1,3,"Params Reset");
          delay(GLOBAL.prompt_time);
          lcd.empty();
          progstep_goto(101); //three point move
          break;

        case REV3POINTMOVE: //new 3 point move reverse
          EEPROM_STORED.REVERSE_PROG_ORDER = true;
          GLOBAL.reset_prog = 1;
          set_defaults_in_ram(); //put all basic paramters in RAM
          draw(6, 1, 3); //lcd.at(1,3,"Params Reset");
          delay(GLOBAL.prompt_time);
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
          EEPROM_STORED.REVERSE_PROG_ORDER = false;
          EEPROM_STORED.intval = 5;//default this for static time selection
          EEPROM_STORED.interval = 100;//default this to low value to insure we don't have left over values from old progam delaying shots.
          progstep_goto(201);
          break;

        case PORTRAITPANO:  //Pano Beta
          EEPROM_STORED.REVERSE_PROG_ORDER = false;
          GLOBAL.reset_prog = 1;
          set_defaults_in_ram(); //put all basic parameters in RAM
          draw(6, 1, 3); //lcd.at(1,3,"Params Reset");
          delay(GLOBAL.prompt_time);
          lcd.empty();
          progstep_goto(301);
          break;

        case SETUPMENU:   //setup menu
          progstep_goto(901);
          break;

        case AUXDISTANCE:   //
          EEPROM_STORED.REVERSE_PROG_ORDER = false;
          GLOBAL.reset_prog = 1;
          set_defaults_in_ram(); //put all basic paramters in RAM
          draw(6, 1, 3); //lcd.at(1,3,"Params Reset");
          delay(GLOBAL.prompt_time);
          lcd.empty();
          progstep_goto(401);
          break;
      }
      break;
  }
}


//Move to Start Point
void Move_to_Startpoint()
{
  if (FLAGS.redraw) {
    lcd.empty();
    lcd.bright(6);
    
    if (!EEPROM_STORED.REVERSE_PROG_ORDER) //normal programing, start first
    {
      draw(8, 1, 1); //lcd.at(1,1,"Move to Start Pt");
      draw(14, 2, 6); //lcd.at(2,6,"C-Next");
    }
    else
    {
      draw(15, 1, 1); //lcd.at(1,1,"Move to End Pt.");
      draw(3, 2, 1); //lcd.at(2,1,CZ1);
    }

    FLAGS.redraw = false;
    delay(GLOBAL.prompt_time);

    //   Velocity Engine update
    DFSetup(); //setup the ISR
    //int32_t *ramValues = (int32_t *)malloc(sizeof(int32_t) * MOTOR_COUNT);
    //int32_t *ramNotValues = (int32_t *)malloc(sizeof(int32_t) * MOTOR_COUNT);
  } //end of first time

  //Velocity Engine update
  if (!nextMoveLoaded)
  {
    NunChuckRequestData();
    axis_button_deadzone();
    updateMotorVelocities2();
    button_actions_move_start(); //check buttons
  }
} //end move to start point


void button_actions_move_start()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      //this puts input to zero to allow a stop
      NunChuckClearData();

      do //run this loop until the motors stop
      {
        //Serial.print("FLAGS.motorMoving:");Serial.println(FLAGS.motorMoving);
        if (!nextMoveLoaded)
        {
          updateMotorVelocities2();
        }
      } while (FLAGS.motorMoving);

      lcd.empty();
      set_position(0, 0, 0); //sets current steps to 0
#if DEBUG
      Serial.print("current_steps_start.x: "); Serial.println(EEPROM_STORED.current_steps.x);
      Serial.print("current_steps_start.y: "); Serial.println(EEPROM_STORED.current_steps.y);
      Serial.print("current_steps_start.z: "); Serial.println(EEPROM_STORED.current_steps.z);
#endif
      if (!EEPROM_STORED.REVERSE_PROG_ORDER) draw(9, 1, 3);  //lcd.at(1,3,"Start Pt. Set");
      else					           draw(16, 1, 3); //lcd.at(1,3,"End Point Set");
      delay(GLOBAL.prompt_time);
      progstep_forward();
      break;

    case Z_Pressed:
      progstep_goto(0);
      break;
  }
}

void Move_to_Endpoint()
{
  if (FLAGS.redraw) {

    lcd.empty();

    if (!EEPROM_STORED.REVERSE_PROG_ORDER) //mormal programming, end point last
    {
      draw(15, 1, 1); //lcd.at(1,1,"Move to End Pt.");
      draw(3, 2, 1); //lcd.at(2,1,CZ1);
    }
    else //reverse programing, start last
    {
      draw(8, 1, 1); //lcd.at(1,1,"Move to Start Pt");
      draw(14, 2, 6); //lcd.at(2,6,"C-Next");
    }

    FLAGS.redraw = false;
    startISR1 ();
    delay(GLOBAL.prompt_time);
    enable_PanTilt();
    if (SETTINGS.AUX_ON) enable_AUX();  //
  }
  /*
    NunChuckRequestData();
    NunChuckProcessData();
    applyjoymovebuffer_exponential();
    dda_move(GLOBAL.feedrate_micros);
    button_actions_move_end();  //read buttons, look for home set on c
    delayMicroseconds(200);
    //delay(1);
  */
  //Velocity Engine update
  if (!nextMoveLoaded)
  {
    NunChuckRequestData();
    axis_button_deadzone();
    updateMotorVelocities2();
    button_actions_move_end(); //check buttons
  }
}


void button_actions_move_end()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      //begin to stop the motors
      //this puts input to zero to allow a stop
      NunChuckClearData();
      do //run this loop until the motors stop
      {
        //Serial.print("FLAGS.motorMoving:");Serial.println(FLAGS.motorMoving);
        if (!nextMoveLoaded)
        {
          updateMotorVelocities2();
        }
      }
      while (FLAGS.motorMoving);

      //end stop the motors

      motors[0].position = EEPROM_STORED.current_steps.x;
      motors[1].position = EEPROM_STORED.current_steps.y;
      motors[2].position = EEPROM_STORED.current_steps.z;

#if DEBUG
      Serial.print("motors[0].position:"); Serial.println( motors[0].position);
      Serial.print("motors[1].position:"); Serial.println( motors[1].position);
      Serial.print("motors[2].position:"); Serial.println( motors[2].position);
#endif

      EEPROM_STORED.motor_steps_pt[2][0] = EEPROM_STORED.current_steps.x; //now signed storage
      EEPROM_STORED.motor_steps_pt[2][1] = EEPROM_STORED.current_steps.y;
      EEPROM_STORED.motor_steps_pt[2][2] = EEPROM_STORED.current_steps.z;

#if DEBUG_MOTOR
      Serial.println("motor_steps_end");
      Serial.print(EEPROM_STORED.motor_steps_pt[2][0]); Serial.print(",");
      Serial.print(EEPROM_STORED.motor_steps_pt[2][1]); Serial.print(",");
      Serial.print(EEPROM_STORED.motor_steps_pt[2][2]); Serial.println();
#endif

      lcd.empty();

      if (!EEPROM_STORED.REVERSE_PROG_ORDER) draw(16, 1, 3); //lcd.at(1,3,"End Point Set");
      else					             draw(9, 1, 3); //lcd.at(1,3,"Start Pt. Set");

      delay(GLOBAL.prompt_time);
      progstep_forward();
      break;
    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void Move_to_Point_X(uint8_t Point)
{
  if (FLAGS.redraw)
  {
    lcd.empty();
    if (EEPROM_STORED.progstep == 201 || EEPROM_STORED.progstep == 301)
    {
      // Programming center for PANOGIGA AND PORTRAITPANO UD010715
      lcd.at(1, 2, "Set AOV Corner");
      set_position(0, 0, 0);
    }
    else if (EEPROM_STORED.progstep == 304)
    {
      lcd.at(1, 1, "Move to Subject ");
    }
    else
    {
      draw(10, 1, 1); //lcd.at(1,1,"Move to Point");
      if (!EEPROM_STORED.REVERSE_PROG_ORDER)
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
    FLAGS.redraw = false;
    delay(GLOBAL.prompt_time);
    

    //   Velocity Engine update
    DFSetup(); //setup the ISR
    //int32_t *ramValues = (int32_t *)malloc(sizeof(int32_t) * MOTOR_COUNT);
    //int32_t *ramNotValues = (int32_t *)malloc(sizeof(int32_t) * MOTOR_COUNT);
  }

  //  Velocity Engine update
  if (!nextMoveLoaded)
  {
    NunChuckRequestData();
    axis_button_deadzone();
    updateMotorVelocities2();
    button_actions_move_x(Point); //check buttons
  }
}


void button_actions_move_x(uint8_t Point)
{
  switch (HandleButtons())
  {
    case C_Pressed:
        //begin to stop the motors
        //this puts input to zero to allow a stop
        NunChuckClearData();
  
        do //run this loop until the motors stop
        {
          //Serial.print("FLAGS.motorMoving:");Serial.println(FLAGS.motorMoving);
          if (!nextMoveLoaded)
          {
            updateMotorVelocities2();
          }
        }
        while (FLAGS.motorMoving);
        //end stop the motors
  
        if (Point == 0) set_position(0, 0, 0);		 //reset for home position
  
        EEPROM_STORED.motor_steps_pt[Point][0] = EEPROM_STORED.current_steps.x; //now signed storage
        EEPROM_STORED.motor_steps_pt[Point][1] = EEPROM_STORED.current_steps.y;
        EEPROM_STORED.motor_steps_pt[Point][2] = EEPROM_STORED.current_steps.z;
  #if DEBUG_MOTOR
        Serial.print("motor_steps_point ");
        Serial.print(Point); Serial.print(";");
        Serial.print(EEPROM_STORED.motor_steps_pt[Point][0]); Serial.print(",");
        Serial.print(EEPROM_STORED.motor_steps_pt[Point][1]); Serial.print(",");
        Serial.print(EEPROM_STORED.motor_steps_pt[Point][2]); Serial.println();
  #endif
  
        if (EEPROM_STORED.progstep != 201 && EEPROM_STORED.progstep != 301)
        {
          lcd.empty();
          draw(63, 1, 3); //lcd.at(1,3,"Point X Set";);
          if (!EEPROM_STORED.REVERSE_PROG_ORDER)  lcd.at(1, 9, (Point + 1)); //Normal
          else //reverse programming
          {
            if (Point == 0) lcd.at(1, 9, "3");
            else if (Point == 1) lcd.at(1, 9, "2");
            else if (Point == 2) lcd.at(1, 9, "1");
          }
        }
  
        if (EEPROM_STORED.progstep == 202 || EEPROM_STORED.progstep == 302) //set angle of view UD050715
        {
          Pan_AOV_steps  = EEPROM_STORED.current_steps.x; //Serial.println(Pan_AOV_steps);
          Tilt_AOV_steps = EEPROM_STORED.current_steps.y; //Serial.println(Tilt_AOV_steps);
          lcd.empty();
          lcd.at (1, 5, "AOV Set");
        }
        if (EEPROM_STORED.progstep == 205) //pano - calculate other values UD050715
        {
          calc_pano_move();
        }
        if (EEPROM_STORED.progstep == 304) // PORTRAITPANO Method UD050715
        {
          lcd.empty();
          lcd.at (1, 4, "Center Set");
        }
  
        delay(GLOBAL.prompt_time);
        progstep_forward();
        break;
     

    case Z_Pressed:
        //this puts input to zero to allow a stop
        NunChuckClearData();
  
        do //run this loop until the motors stop
        {
          //Serial.print("FLAGS.motorMoving:");Serial.println(FLAGS.motorMoving);
          if (!nextMoveLoaded)
          {
            updateMotorVelocities2();
          }
        }
        while (FLAGS.motorMoving); // Wait for the motors to stop
        progstep_backward();
      break;
  }
}


//Set Camera Interval
void Set_Cam_Interval()
{
  if (FLAGS.redraw)
  {
    lcd.empty();
    draw(17, 1, 1); //lcd.at(1,1,"Set Sht Interval");
    delay(GLOBAL.prompt_time);
    lcd.empty();
    draw(18, 1, 1); //lcd.at(1,1,"Intval:   .  sec");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    DisplayInterval();
    FLAGS.redraw = false;
  }

  uint16_t intval_last = EEPROM_STORED.intval;
  NunChuckRequestData();
  NunChuckProcessData();
  if (EEPROM_STORED.intval < 20) GLOBAL.joy_y_lock_count = 0;
  EEPROM_STORED.intval += joy_capture3();
  EEPROM_STORED.intval  = constrain(EEPROM_STORED.intval, 2, 6000); //remove the option for Ext Trigger right now
  if (intval_last != EEPROM_STORED.intval) {
    DisplayInterval();
  }

  button_actions_intval(EEPROM_STORED.intval);  //read buttons, look for c button press to set interval
  delay (GLOBAL.prompt_delay);
}


void DisplayInterval()
{
  if (EEPROM_STORED.intval == EXTTRIG_INTVAL) //run the Ext
  {
    draw(19, 1, 8); //lcd.at(1,8," Ext.Trig");
  }
  else if (EEPROM_STORED.intval == VIDEO_INTVAL) //run the video routine
  {
    draw(20, 1, 8); //lcd.at(1,8," Video   ");
  }
  else if (EEPROM_STORED.intval < 10)
  {
    lcd.at(1, 13, " sec");
    lcd.at(1, 8, "  0.");
    lcd.at(1, 10, EEPROM_STORED.intval / 10);
    lcd.at(1, 12, EEPROM_STORED.intval % 10);
  }
  else if (EEPROM_STORED.intval < 100)
  {
    lcd.at(1, 8, "  ");
    lcd.at(1, 10, EEPROM_STORED.intval / 10);
    lcd.at(1, 12, EEPROM_STORED.intval % 10);
  }
  else if (EEPROM_STORED.intval < 1000)
  {
    lcd.at(1, 8, " ");
    lcd.at(1, 9, EEPROM_STORED.intval / 10);
    lcd.at(1, 12, EEPROM_STORED.intval % 10);
  }
  else
  {
    lcd.at(1, 8, EEPROM_STORED.intval / 10);
    lcd.at(1, 12, EEPROM_STORED.intval % 10);
  }
}


void button_actions_intval(uint16_t intval)
{
  switch (HandleButtons())
  {
    case C_Pressed:
    {
      uint16_t video_sample_ms = 100; //
      EEPROM_STORED.interval = intval * 100; //tenths of a second to ms
      //camera_exp_tm=100;
      if (intval == EXTTRIG_INTVAL) //means ext trigger
      {
        EEPROM_STORED.interval = 6000; //Hardcode this to 6.0 seconds are 10 shots per minute to allow for max shot selection
      }
      else if (intval == VIDEO_INTVAL) //means video, set very small exposure time
      {
        //camera_exp_tm=5; //doesn't matter, we never call the shutter
        EEPROM_STORED.interval = video_sample_ms; //we overwrite this later based on move length currently 100
      }

      lcd.empty();
      draw(21, 1, 3); //lcd.at(1,3,"Interval Set"); // try this to push correct character
      delay(GLOBAL.prompt_time);
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
  if (FLAGS.redraw)
  {
    lcd.empty();
    //			1234567890123456
    draw(32, 1, 5); //lcd.at(1,5,	"Set Move");
    draw(33, 2, 5); //lcd.at(2,5,	"Duration");
    delay(GLOBAL.prompt_time * 1.5);
    lcd.empty();
    draw(34, 1, 10); //lcd.at(1,9,"H:MM:SS");
    if ((EEPROM_STORED.intval > 3) || (EEPROM_STORED.intval == EXTTRIG_INTVAL))
    {
      lcd.at(2, 11, "Frames"); //SMS
      EEPROM_STORED.camera_total_shots = EEPROM_STORED.camera_moving_shots;
      Display_Duration();
    }
    else
    {
      draw(3, 2, 1); //lcd.at(2,1,CZ1); //Video
      calc_time_remain_dur_sec (EEPROM_STORED.overaldur);
      display_time(1, 1);
    }
    EEPROM_STORED.camera_total_shots = EEPROM_STORED.camera_moving_shots;
    EEPROM_STORED.camera_fired = 0;
    //Display_Duration();
    FLAGS.redraw = false;
  }
  NunChuckRequestData();
  NunChuckProcessData();

  if (EEPROM_STORED.intval == VIDEO_INTVAL)
  { //video
    uint16_t overaldur_last = EEPROM_STORED.overaldur;
    EEPROM_STORED.overaldur += joy_capture3();
    if (EEPROM_STORED.overaldur <= 0) {
      EEPROM_STORED.overaldur = 10000;
    }
    if (EEPROM_STORED.overaldur > 10000) {
      EEPROM_STORED.overaldur = 1;
    }
    if (overaldur_last != EEPROM_STORED.overaldur)
    {
      calc_time_remain_dur_sec (EEPROM_STORED.overaldur);
      display_time(1, 1);
    }
  }
  else
  { //sms
    uint32_t camera_moving_shots_last = EEPROM_STORED.camera_moving_shots;
    EEPROM_STORED.camera_moving_shots += joy_capture3();
    if (EEPROM_STORED.camera_moving_shots <= 9) {
      EEPROM_STORED.camera_moving_shots = 10000;
    }
    if (EEPROM_STORED.camera_moving_shots > 10000) {
      EEPROM_STORED.camera_moving_shots = 10;
    }
    //EEPROM_STORED.camera_moving_shots=constrain(EEPROM_STORED.camera_moving_shots,10,10000);
    EEPROM_STORED.camera_total_shots = EEPROM_STORED.camera_moving_shots; //we add in lead in lead out later
    if (camera_moving_shots_last != EEPROM_STORED.camera_moving_shots) {
      Display_Duration();
    } //end update time and shots
  } //end sms
  button_actions_overaldur();  //read buttons, look for c button press to set overall time
  delay (GLOBAL.prompt_delay);
}


void Display_Duration()
{
  calc_time_remain(GLOBAL.total_pano_move_time);
  display_time(1, 1);

  if ((EEPROM_STORED.intval > 3) || (EEPROM_STORED.intval == EXTTRIG_INTVAL))
  {
    if (EEPROM_STORED.camera_total_shots < 100)
    {
      lcd.at(2, 7, EEPROM_STORED.camera_total_shots);
      lcd.at(2, 4, "   ");
    }
    else if (EEPROM_STORED.camera_total_shots < 1000)
    {
      lcd.at(2, 6, EEPROM_STORED.camera_total_shots);
      lcd.at(2, 4, "  ");
    }
    else if (EEPROM_STORED.camera_total_shots < 10000)
    {
      lcd.at(2, 5, EEPROM_STORED.camera_total_shots);
      lcd.at(2, 4, " ");
    }
    else
    {
      lcd.at(2, 4, EEPROM_STORED.camera_total_shots);
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
      delay (GLOBAL.prompt_time);
      progstep_forward();
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void Set_Static_Time()
{
  if (FLAGS.redraw)
  {
    lcd.empty();
    draw(22, 1, 1); //lcd.at(1,1,"Set Static Time");
    delay(GLOBAL.prompt_time);
    lcd.empty();
    draw(23, 1, 1); //lcd.at(1,1,"Stat_T:   .  sec");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    GLOBAL.max_shutter = EEPROM_STORED.intval - MIN_INTERVAL_STATIC_GAP; //max static is .3 seconds less than interval (leaves 0.3 seconds for move)
    if (EEPROM_STORED.intval == EXTTRIG_INTVAL) GLOBAL.max_shutter = 600; //external trigger
    if (EEPROM_STORED.progtype == PANOGIGA || EEPROM_STORED.progtype == PORTRAITPANO) GLOBAL.max_shutter = 36000; //pano mode - allows
    DisplayStatic_tm();
    FLAGS.redraw = false;
  }

  uint16_t static_tm_last = EEPROM_STORED.static_tm;
  NunChuckRequestData();
  NunChuckProcessData();

  if (EEPROM_STORED.static_tm < 20) GLOBAL.joy_y_lock_count = 0;
  EEPROM_STORED.static_tm += joy_capture3();
  if (EEPROM_STORED.static_tm > 60000) EEPROM_STORED.static_tm = GLOBAL.max_shutter;
  EEPROM_STORED.static_tm = constrain(EEPROM_STORED.static_tm, 1, GLOBAL.max_shutter);

  if (static_tm_last != EEPROM_STORED.static_tm) DisplayStatic_tm();
  button_actions_stat_time();  //read buttons, look for c button press to set interval
  delay (GLOBAL.prompt_delay);
}


void DisplayStatic_tm()
{
  if (EEPROM_STORED.intval == VIDEO_INTVAL)	draw(24, 1, 8); //lcd.at(1,8," Video   ");

  else if (EEPROM_STORED.static_tm < 10)
  {
    lcd.at(1, 8, "  0.");
    lcd.at(1, 10, EEPROM_STORED.static_tm / 10);
    lcd.at(1, 12, EEPROM_STORED.static_tm % 10);
  }
  else if (EEPROM_STORED.static_tm < 100)
  {
    lcd.at(1, 8, "   .");
    lcd.at(1, 10, EEPROM_STORED.static_tm / 10);
    lcd.at(1, 12, EEPROM_STORED.static_tm % 10);
  }
  else if (EEPROM_STORED.static_tm < 1000)
  {
    lcd.at(1, 8, "   .");
    lcd.at(1, 9, EEPROM_STORED.static_tm / 10);
    lcd.at(1, 12, EEPROM_STORED.static_tm % 10);
  }
  else if (EEPROM_STORED.static_tm < 10000)
  {
    lcd.at(1, 8, "   .  ");
    lcd.at(1, 8, EEPROM_STORED.static_tm / 10);
    lcd.at(1, 12, EEPROM_STORED.static_tm % 10);
  }
}


void button_actions_stat_time()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      //if (EEPROM_STORED.intval!=3)  camera_exp_tm=(long)EEPROM_STORED.static_tm*100; //want to get to ms, but this is already multipled by 10 // 3 is used for video mode - this is likely obsolete-remove later
      lcd.empty();
      draw(25, 1, 1); //lcd.at(1,1,"Static Time Set"); // try this to push correct character
      delay(GLOBAL.prompt_time);
      progstep_forward();
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void Set_Ramp()
{
  if (FLAGS.redraw)
  {
    lcd.empty();
    draw(29, 1, 1); //lcd.at(1,1,"	Set Ramp");
    delay(GLOBAL.prompt_time);
    lcd.empty();
    draw(30, 1, 1); //lcd.at(1,1,"Ramp:	 Frames");
    if (EEPROM_STORED.intval == VIDEO_INTVAL) {
      EEPROM_STORED.camera_moving_shots = 147; //allow for up to 49 % ramp
      lcd.at(1, 10, "GLOBAL.percent");
    }

    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    DisplayRampval();
    FLAGS.redraw = false;
  }

  uint16_t rampval_last = EEPROM_STORED.rampval;
  NunChuckRequestData();
  NunChuckProcessData();
  if (EEPROM_STORED.rampval < 20) GLOBAL.joy_y_lock_count = 0;
  EEPROM_STORED.rampval += joy_capture3();
  if (EEPROM_STORED.rampval < 1) {
    EEPROM_STORED.rampval = 1;
    delay(GLOBAL.prompt_time / 2);
  }
  //if (EEPROM_STORED.rampval>500) {EEPROM_STORED.rampval=2;}

  if ( EEPROM_STORED.rampval * 3 > EEPROM_STORED.camera_moving_shots ) {	// we have an issue where the ramp is to big can be 2/3 of total move, but not more.
    EEPROM_STORED.rampval = EEPROM_STORED.camera_moving_shots / 3; //set to 1/3 up and 1/3 down (2/3) of total move)
    delay(GLOBAL.prompt_time / 2);
  }

  //EEPROM_STORED.rampval=constrain(EEPROM_STORED.rampval,1,500); //
  if (rampval_last != EEPROM_STORED.rampval) {
    DisplayRampval();
  }
  //delay(50);
  button_actions_rampval();  //read buttons, look for c button press to set interval
  delay (GLOBAL.prompt_delay);
}


void DisplayRampval()
{
  lcd.at(1, 7, EEPROM_STORED.rampval);
  if		  (EEPROM_STORED.rampval < 10)  lcd.at(1, 8, "  "); //clear extra if goes from 3 to 2 or 2 to  1
  else if (EEPROM_STORED.rampval < 100)  lcd.at(1, 9, " "); //clear extra if goes from 3 to 2 or 2 to  1
}


void button_actions_rampval()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      //we actually don't store anything here - wait for final confirmation to set everything

      if (EEPROM_STORED.intval == VIDEO_INTVAL) {
        delay(1);
      }

      lcd.empty();
      draw(31, 1, 5); //lcd.at(1,5,"Ramp Set");
      delay(GLOBAL.prompt_time);
      progstep_forward();
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void Set_LeadIn_LeadOut()
{

  if (FLAGS.redraw)
  {
    lcd.empty();
    draw(36, 1, 1); //lcd.at(1,1,"Set Static Lead");
    draw(37, 2, 2); //lcd.at(2,2,"In/Out Frames");
    delay(GLOBAL.prompt_time);
    lcd.empty();
    draw(38, 1, 6); //lcd.at(1,1,"IN -	Out");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    FLAGS.redraw = false;
    lcd.at(1, 9, EEPROM_STORED.lead_out);
    cursorpos = cursorleft;
    DisplayLeadIn_LeadOut();
  }

  int16_t joyxysum_last = GLOBAL.joy_x_axis + GLOBAL.joy_y_axis; //figure out if changing

  NunChuckRequestData();
  NunChuckProcessData();
  cursorpos += joy_capture_x_map();

  if (joyxysum_last != (GLOBAL.joy_x_axis + GLOBAL.joy_y_axis) || abs(GLOBAL.joy_x_axis + GLOBAL.joy_y_axis) > 10) { //check to see if there is an input, otherwise don't update display
    DisplayLeadIn_LeadOut();
  }

  button_actions_lead_in_out();  //read buttons, look for c button press to ramp
  delay (GLOBAL.prompt_delay);
}


void DisplayLeadIn_LeadOut()
{
  if (cursorpos == cursorright) lcd.pos(1, 13);

  if (cursorpos == cursorleft)
  { //update lead in
    lcd.cursorOff();
    EEPROM_STORED.lead_in += joy_capture3();
    if (EEPROM_STORED.lead_in < 1)    {
      EEPROM_STORED.lead_in = 5000;
    }
    if (EEPROM_STORED.lead_in > 5000) {
      EEPROM_STORED.lead_in = 1;
    }
    lcd.at(1, 1, EEPROM_STORED.lead_in);

    if (EEPROM_STORED.lead_in < 10)   {
      lcd.at(1, 2, "   ");
    }
    if (EEPROM_STORED.lead_in < 100)  {
      lcd.at(1, 3, "  ");
    }
    if (EEPROM_STORED.lead_in < 1000) {
      lcd.at(1, 4, " ");
    }
    lcd.cursorBlock();
  }
  else
  { //update leadout
    lcd.cursorOff();
    EEPROM_STORED.lead_out += joy_capture3();
    if (EEPROM_STORED.lead_out < 1)    {
      EEPROM_STORED.lead_out = 5000;
    }
    if (EEPROM_STORED.lead_out > 5000) {
      EEPROM_STORED.lead_out = 1;
    }
    lcd.at(1, 9, EEPROM_STORED.lead_out);

    if (EEPROM_STORED.lead_out < 10)   {
      lcd.at(1, 10, "   ");
    }
    if (EEPROM_STORED.lead_out < 100)  {
      lcd.at(1, 11, "  ");
    }
    if (EEPROM_STORED.lead_out < 1000) {
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
        EEPROM_STORED.camera_total_shots=EEPROM_STORED.camera_moving_shots+EEPROM_STORED.lead_in+EEPROM_STORED.lead_out;

        if (EEPROM_STORED.intval==VIDEO_INTVAL) {
      	//if (EEPROM_STORED.intval==VIDEO_INTVAL) EEPROM_STORED.camera_moving_shots=(EEPROM_STORED.overaldur*1000L)/EEPROM_STORED.interval; //figure out moving shots based on duration for video only
      	//EEPROM_STORED.camera_moving_shots=video_segments; //hardcode this and back into proper interval - this is XXX segments per sequence
      	//EEPROM_STORED.camera_moving_shots=100; //new method to allow for easy ramp %
      	//EEPROM_STORED.interval=EEPROM_STORED.overaldur*1000L/EEPROM_STORED.camera_moving_shots;   //This gives us ms
      	EEPROM_STORED.camera_total_shots=EEPROM_STORED.camera_moving_shots+EEPROM_STORED.lead_in+EEPROM_STORED.lead_out; //good temp variable for display
        }

        //fix issues with ramp that is too big
        if( EEPROM_STORED.rampval*3 > EEPROM_STORED.camera_moving_shots )
        {	// we have an issue where the ramp is to big can be 2/3 of total move, but not more.
        EEPROM_STORED.rampval=EEPROM_STORED.camera_moving_shots/3; //set to 1/3 up and 1/3 down (2/3) of total move)
        }

        //Set keyframe points for program to help with runtime calcs
        EEPROM_STORED.keyframe[0][0]=0; //start frame
        EEPROM_STORED.keyframe[0][1]=EEPROM_STORED.lead_in; //beginning of ramp
        EEPROM_STORED.keyframe[0][2]=EEPROM_STORED.lead_in+EEPROM_STORED.rampval; //end of ramp, beginning of linear
        EEPROM_STORED.keyframe[0][3]=EEPROM_STORED.lead_in+EEPROM_STORED.camera_moving_shots-EEPROM_STORED.rampval;  //end or linear, beginning of rampdown
        EEPROM_STORED.keyframe[0][4]=EEPROM_STORED.lead_in+EEPROM_STORED.camera_moving_shots; //end of rampdown, beginning of leadout
        EEPROM_STORED.keyframe[0][5]=EEPROM_STORED.lead_in+EEPROM_STORED.camera_moving_shots+EEPROM_STORED.lead_out; //end of leadout, end of program

        if (DEBUG_MOTOR) {
      	for (int i=0; i < 6; i++){
      		Serial.print("Keyframe");Serial.print(i);Serial.print("_");Serial.println(EEPROM_STORED.keyframe[0][i]);
      	}
        }

        //go_to_origin_slow();
        go_to_start_new();

        //write_all_ram_to_eeprom(); //set this here to allow us to rerun this program from this point if we just want to turn it on review and go
        //restore_from_eeprom_memory();
        if (DEBUG) review_RAM_Contents();

        //delay(GLOBAL.prompt_time);
        if (SETTINGS.POWERSAVE_PT>2)   disable_PT();
        if (SETTINGS.POWERSAVE_AUX>2)   disable_AUX();
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
  EEPROM_STORED.camera_total_shots = EEPROM_STORED.camera_moving_shots + EEPROM_STORED.lead_in + EEPROM_STORED.lead_out;

  if (EEPROM_STORED.intval == VIDEO_INTVAL)
  {
    //really only need this for 3 point moves now - this could screw up 2 points, be careful
    EEPROM_STORED.camera_moving_shots = video_segments; //hardcode this and back into proper interval - this is XXX segments per sequence
    EEPROM_STORED.interval = EEPROM_STORED.overaldur * 1000L / EEPROM_STORED.camera_moving_shots;   //This gives us ms for our delays
    EEPROM_STORED.camera_total_shots = EEPROM_STORED.camera_moving_shots + EEPROM_STORED.lead_in + EEPROM_STORED.lead_out; //good temp variable for display
  }

  //fix issues with ramp that is too big
  if ( EEPROM_STORED.rampval * 3 > EEPROM_STORED.camera_moving_shots ) {	// we have an issue where the ramp is to big can be 2/3 of total move, but not more.
    EEPROM_STORED.rampval = EEPROM_STORED.camera_moving_shots / 3; //set to 1/3 up and 1/3 down (2/3) of total move)
  }

  //Set keyframe points for program to help with runtime calcs
  EEPROM_STORED.keyframe[0][0] = 0; //start frame
  EEPROM_STORED.keyframe[0][1] = EEPROM_STORED.lead_in; //beginning of ramp
  EEPROM_STORED.keyframe[0][2] = EEPROM_STORED.lead_in + EEPROM_STORED.rampval; //end of ramp, beginning of linear
  EEPROM_STORED.keyframe[0][3] = EEPROM_STORED.lead_in + EEPROM_STORED.camera_moving_shots - EEPROM_STORED.rampval; //end or linear, beginning of rampdown
  EEPROM_STORED.keyframe[0][4] = EEPROM_STORED.lead_in + EEPROM_STORED.camera_moving_shots; //end of rampdown, beginning of leadout
  EEPROM_STORED.keyframe[0][5] = EEPROM_STORED.lead_in + EEPROM_STORED.camera_moving_shots + EEPROM_STORED.lead_out; //end of leadout, end of program

#if DEBUG_MOTOR
  for (uint8_t i = 0; i < 6; i++) {
    Serial.print("Keyframe");
    Serial.print(i);
    Serial.print("_");
    Serial.println(EEPROM_STORED.keyframe[0][i]);
  }
#endif

  //go_to_origin_slow();
  go_to_start_new();

  //write_all_ram_to_eeprom(); //set this here to allow us to rerun this program from this point if we just want to turn it on review and go
  //restore_from_eeprom_memory();
#if DEBUG
  review_RAM_Contents();
#endif

  //delay(GLOBAL.prompt_time);
  if (SETTINGS.POWERSAVE_PT > 2)   disable_PT();
  if (SETTINGS.POWERSAVE_AUX > 2)   disable_AUX();
  //end of code block pulled from LeadinLeadOut
}


void Review_Confirm()
{
  if (FLAGS.redraw)
  {
    lcd.empty();
    draw(41, 1, 4); //lcd.at(1,4,"Review and");
    draw(42, 2, 2); //lcd.at(2,2,"Confirm Setting");
    //delay(GLOBAL.prompt_time);
    delay(GLOBAL.prompt_time);
    //delay(100);
    lcd.empty();
    FLAGS.redraw = false;
    GLOBAL.display_last_tm = millis();
    DisplayReviewProg();
    reviewprog = 2;
    GLOBAL.start_delay_sec = 0;
  }

  if ((millis() - GLOBAL.display_last_tm) > (GLOBAL.prompt_time * 4))
  { //test for display update
    //if ((millis()-GLOBAL.display_last_tm) >(700)){ //test for display update

    reviewprog ++;
    GLOBAL.display_last_tm = millis();
    if (reviewprog > 4) reviewprog = 2;
    GLOBAL.first_time2 = true;
    DisplayReviewProg();
  } //end test for display update

  NunChuckRequestData();
  NunChuckProcessData();

  if (abs(GLOBAL.joy_y_axis) > 20)
  { //do read time updates to delay program
    reviewprog = 4;
    DisplayReviewProg();
    GLOBAL.display_last_tm = millis();
  }

  button_actions_review();
  delay (GLOBAL.prompt_delay);
}


void DisplayReviewProg()
{
  switch (reviewprog)
  {
    case 1:   //
      lcd.empty();
      draw(43, 1, 1); //lcd.at(1,1,"Pan Steps:");
      draw(44, 2, 1); //lcd.at(2,1,"Tilt Steps:");
      //lcd.at(1,12,motor_steps[0]);
      lcd.at(1, 12, (int)EEPROM_STORED.linear_steps_per_shot[0]);
      lcd.at(2, 12, (int)EEPROM_STORED.linear_steps_per_shot[1]);
      break;

    case 2:   //
      lcd.empty();
      draw(45, 1, 1); //lcd.at(1,1,"Cam Shots:");
      draw(46, 2, 1); //lcd.at(2,1,"Time:");
      lcd.at(1, 12, EEPROM_STORED.camera_total_shots);
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
      if (GLOBAL.first_time2)
      {
        lcd.at(1, 1, "Set Start Delay");
        lcd.at(2, 1, "			");
        draw(34, 2, 10); //lcd.at(2,10,"H:MM:SS");
        GLOBAL.first_time2 = false;
      }

      if (GLOBAL.start_delay_sec < 20) GLOBAL.joy_y_lock_count = 0; //this is an unsigned int
      GLOBAL.start_delay_sec += joy_capture3();
      //if (GLOBAL.start_delay_sec<0) {GLOBAL.start_delay_sec=0;} //this statement doesn't do anything as this is an unsigned int
      if (GLOBAL.start_delay_sec > 43200) {
        GLOBAL.start_delay_sec = 0;
      }
      calc_time_remain_dur_sec (GLOBAL.start_delay_sec);
      display_time(2, 1);
      //lcd.at(1,11,GLOBAL.start_delay_sec);
      // if (start_delay_min <10)  lcd.at(1,8,"  ");  //clear extra if goes from 3 to 2 or 2 to  1
      // if (start_delay_min <100)  lcd.at(1,9," ");  //clear extra if goes from 3 to 2 or 2 to  1
      break;
  }//end switch
}


void button_actions_review()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      lcd.empty();
      EEPROM_STORED.MOVE_REVERSED_FOR_RUN = false; //Reset This
      GLOBAL.start_delay_tm = ((millis() / 1000L) + GLOBAL.start_delay_sec); //system seconds in the future - use this as big value to compare against
      draw(34, 2, 10); //lcd.at(2,10,"H:MM:SS");
      lcd.at(1, 2, "Delaying Start");
      GLOBAL.CZ_Button_Read_Count = 0; //reset this to zero to start

      while (GLOBAL.start_delay_tm > millis() / 1000L)
      {
        //enter delay routine
        calc_time_remain_start_delay ();
        if ((millis() - GLOBAL.display_last_tm) > 1000) display_time(2, 1);
        NunChuckRequestData();
        NunChuckProcessData();
        Check_Prog(); //look for long button press
        if (GLOBAL.CZ_Button_Read_Count > 20 && !EEPROM_STORED.Program_Engaged)
        {
          GLOBAL.start_delay_tm = ((millis() / 1000L) + 5); //start right away by lowering this to 5 seconds.
          GLOBAL.CZ_Button_Read_Count = 0; //reset this to zero to start
        }
      }

      enable_PanTilt();
      if (SETTINGS.AUX_ON) enable_AUX();  //

      //draw(49,1,1);//lcd.at(1,1,"Program Running");
      //delay(GLOBAL.prompt_time/3);

      if (EEPROM_STORED.intval == EXTTRIG_INTVAL)  lcd.at(2, 1, "Waiting for Man.");

      EEPROM_STORED.Program_Engaged = true;
      FLAGS.Interrupt_Fire_Engaged = true; //just to start off first shot immediately
      GLOBAL.interval_tm = 0; //set this to 0 to immediately trigger the first shot
      GLOBAL.sequence_repeat_count = 0; //this is zeroed out every time we start a new shot

      if (EEPROM_STORED.intval > 3) { //SMS Mode
        lcd.empty(); //clear for non video
        EEPROM_STORED.progstep = 50; //  move to the main programcamera_real_fire
      }
      else if (EEPROM_STORED.intval == VIDEO_INTVAL)
      {
        lcd.empty();
        draw(49, 1, 1); //lcd.at(1,1,"Program Running");
        EEPROM_STORED.progstep = 51;
      }
      else if (EEPROM_STORED.intval == EXTTRIG_INTVAL)  { //manual trigger/ext trigger
        lcd.empty(); //clear for non video
        EEPROM_STORED.progstep = 52; //  move to the external interrupt loop
        ext_shutter_count = 0; //set this to avoid any prefire shots or cable insertions.
        lcd.at(1, 1, "Waiting for Trig");
      }
      FLAGS.redraw = true;
      lcd.bright(SETTINGS.LCD_BRIGHTNESS_DURING_RUN);
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void progstep_forward()
{
  lcd.empty();
  FLAGS.redraw = true;
  FLAGS.progstep_forward_dir = true;
  if (EEPROM_STORED.progstep) EEPROM_STORED.progstep++;
  delay(100);
  NunChuckClearData(); //  Use this to clear out any button registry from the last step
}


void progstep_backward()
{
  lcd.empty();
  FLAGS.redraw = true;
  FLAGS.progstep_forward_dir = false;
  if (EEPROM_STORED.progstep) EEPROM_STORED.progstep--;
  delay(100);
  NunChuckClearData(); //  Use this to clear out any button registry from the last step
}


void progstep_goto(uint16_t prgstp)
{
  lcd.empty();
  FLAGS.redraw = true;
  EEPROM_STORED.progstep = prgstp;
  delay(100);
  NunChuckClearData(); //  Use this to clear out any button registry from the last step
}


void button_actions_end_of_program()
{ //repeat - need to turn off the REVERSE_PROG_ORDER flag
  switch (HandleButtons())
  {
    case C_Pressed:
      // Normal
      EEPROM_STORED.REVERSE_PROG_ORDER = false;
      if (SETTINGS.POWERSAVE_PT > 2)   disable_PT();
      if (SETTINGS.POWERSAVE_AUX > 2)   disable_AUX();
      //EEPROM_STORED.Program_Engaged=true;
      EEPROM_STORED.camera_fired = 0;
      lcd.bright(8);
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
      break;

    case Z_Pressed:
      EEPROM_STORED.REVERSE_PROG_ORDER = true;
      if (SETTINGS.POWERSAVE_PT > 2)   disable_PT();
      if (SETTINGS.POWERSAVE_AUX > 2)   disable_AUX();
      //EEPROM_STORED.Program_Engaged=true;
      EEPROM_STORED.camera_fired = 0;
      lcd.bright(8);
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
      break;
  }
}


void Auto_Repeat_Video()
{ //Auto Reverse
  GLOBAL.sequence_repeat_count++;
  EEPROM_STORED.REVERSE_PROG_ORDER = true;
  EEPROM_STORED.camera_fired = 0;
  lcd.bright(8);
  if (EEPROM_STORED.progtype == REG2POINTMOVE || EEPROM_STORED.progtype == REV2POINTMOVE) {
    go_to_start_new();
    //progstep_goto(8);
  }

  lcd.empty();
  EEPROM_STORED.MOVE_REVERSED_FOR_RUN = false; //Reset This
  //GLOBAL.start_delay_tm=((millis()/1000L)+GLOBAL.start_delay_sec); //system seconds in the future - use this as big value to compare against
  //draw(34,2,10);//lcd.at(2,10,"H:MM:SS");
  //lcd.at(1,2,"Delaying Start");
  GLOBAL.CZ_Button_Read_Count = 0; //reset this to zero to start

  /*
    while (GLOBAL.start_delay_tm>millis()/1000L) {
  	//enter delay routine
  	calc_time_remain_start_delay ();
  	if ((millis()-GLOBAL.display_last_tm) > 1000) display_time(2,1);
  	NunChuckRequestData();
  	NunChuckProcessData();
  	Check_Prog(); //look for long button press
  	if (GLOBAL.CZ_Button_Read_Count>20 && !EEPROM_STORED.Program_Engaged) {
  		GLOBAL.start_delay_tm=((millis()/1000L)+5); //start right away by lowering this to 5 seconds.
  		GLOBAL.CZ_Button_Read_Count=0; //reset this to zero to start
  	}
    }
  */
  enable_PanTilt();
  enable_AUX();  //

  draw(49, 1, 1); //lcd.at(1,1,"Program Running");
  //delay(GLOBAL.prompt_time/3);

  EEPROM_STORED.Program_Engaged = true;
  FLAGS.Interrupt_Fire_Engaged = true; //just to start off first shot immediately

  GLOBAL.interval_tm = 0; //set this to 0 to immediately trigger the first shot

  if (EEPROM_STORED.intval > 3) { //SMS Mode
    lcd.empty(); //clear for non video
    EEPROM_STORED.progstep = 50; //  move to the main programcamera_real_fire
  }

  else if (EEPROM_STORED.intval == VIDEO_INTVAL)  {
    //lcd.empty(); //leave "program running up for video
    EEPROM_STORED.progstep = 51;
  }
  /*
    else if (EEPROM_STORED.intval==EXTTRIG_INTVAL)  { //manual trigger/ext trigge
  	lcd.empty(); //clear for non video
  	EEPROM_STORED.progstep=52; //  move to the external interrup loop
  	ext_shutter_count=0; //set this to avoid any prefire shots or cable insertions.
  	lcd.at(1,1,"Waiting for Trig");
    }
  */
  FLAGS.redraw = true;
  lcd.bright(SETTINGS.LCD_BRIGHTNESS_DURING_RUN);
}//end of 91


void Pause_Prog()
{
  GLOBAL.CZ_Button_Read_Count = 0;
  EEPROM_STORED.Program_Engaged = !EEPROM_STORED.Program_Engaged; //turn off the loop
  if (!EEPROM_STORED.Program_Engaged) {  //program turned off
    write_all_ram_to_eeprom(); //capture current steps too!
    lcd.at(1, 11, " Pause");
    delay(2000);
    NunChuckRequestData();
  }
  else { //turn it on
    lcd.at(1, 11, "Resume");
    delay(500);
    NunChuckRequestData();
  }
}


void display_status()
{
  static uint8_t   batt_low_cnt = 0;
  //1234567890123456
  //1234567890123456
  //XXXX/XXXX LeadIn		LeadOT Rampup RampDn, Pause
  //HH:MM:SS  XX.XXV
  if (FLAGS.redraw)
  {
    lcd.empty();
    lcd.at(1, 6, "/"); //Add to one time display update
    lcd.at(2, 13, ".");
    lcd.at(2, 16, "v");
    FLAGS.redraw = false;
  }
  //update upper left camera fired/total shots
  unsigned int camera_fired_display = EEPROM_STORED.camera_fired + 1;
  if		  (camera_fired_display < 10)    lcd.at(1, 5, camera_fired_display);
  else if (camera_fired_display < 100)   lcd.at(1, 4, camera_fired_display);
  else if (camera_fired_display < 1000)  lcd.at(1, 3, camera_fired_display);
  else if (camera_fired_display < 10000) lcd.at(1, 2, camera_fired_display);
  else								                   lcd.at(1, 1, camera_fired_display);

  lcd.at(1, 7, EEPROM_STORED.camera_total_shots);

  //Update program progress secion - upper right

  if (EEPROM_STORED.progtype == REG2POINTMOVE || EEPROM_STORED.progtype == REV2POINTMOVE || EEPROM_STORED.progtype == AUXDISTANCE) {
    switch (Move_State_2PT)
    {
      case LeadIn2PT:
        draw(51, 1, 11); //lcd.at(1,11,"LeadIn");
        break;
      case RampUp2PT:
        draw(52, 1, 11); //lcd.at(1,11,"RampUp");
        break;
      case Linear2PT:
        draw(53, 1, 11); //lcd.at(1,11,"Linear");
        break;
      case RampDown2PT:
        draw(54, 1, 11); //lcd.at(1,11,"RampDn");
        break;
      case LeadOut2PT:
        draw(55, 1, 11); //lcd.at(1,11,"LeadOT");
        break;
      case Finished2PT:
        draw(56, 1, 11); //lcd.at(1,11,"Finish");
        break;
    }
  }

  if (EEPROM_STORED.progtype == REG3POINTMOVE || EEPROM_STORED.progtype == REV3POINTMOVE)
  {
    switch (Move_State_3PT)
    {
      case LeadIn3PT: //3PT Lead In
        draw(51, 1, 11); //lcd.at(1,11,"LeadIn");
        break;
      case FirstLeg3PT: //3PT leg 1
        lcd.at(1, 11, "Leg 1 ");
        break;
      case SecondLeg3PT: //3PT leg 2
        lcd.at(1, 11, "Leg 2 ");
        break;
      case LeadOut3PT: //3PT Lead Out
        draw(55, 1, 11); //lcd.at(1,11,"LeadOT");
        break;
      case Finished3PT: //3PT Finish
        draw(56, 1, 11); //lcd.at(1,11,"Finish");
        break;
    }
  }

  if (EEPROM_STORED.progtype == PANOGIGA || EEPROM_STORED.progtype == PORTRAITPANO) {
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

  if (SETTINGS.POWERDOWN_LV) {
    if (batt1 < 9) {
      draw(7, 2, 1); //lcd.at(2,1,"Low Power");

      batt_low_cnt++;
      if (batt_low_cnt > 20) {
        //Stop the program and go to low power state
        disable_PT();
        disable_AUX();
        EEPROM_STORED.Program_Engaged = false;
        lcd.empty();
        draw(60, 1, 1); //lcd.at(1,1,"Battery too low");
        draw(61, 2, 1); //lcd.at(2,1,"  to continue");
      }
      FLAGS.redraw = true;
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
  unsigned long timetotal = ((EEPROM_STORED.camera_total_shots - EEPROM_STORED.camera_fired) * EEPROM_STORED.interval) / 1000;

  timeh =  timetotal / 3600;	// hours
  timem =  timetotal / 60 % 60; // minutes
  time_s = timetotal % 60;		// seconds
}

void calc_time_remain(float totalmovetime)
{
  unsigned long timetotal = ((EEPROM_STORED.camera_total_shots - EEPROM_STORED.camera_fired) * EEPROM_STORED.static_tm * 100) / 1000 + totalmovetime * (EEPROM_STORED.camera_total_shots - EEPROM_STORED.camera_fired) / EEPROM_STORED.camera_total_shots;
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
  timeh =  (GLOBAL.start_delay_tm - current_sec) / 3600;	// hours
  timem =  (GLOBAL.start_delay_tm - current_sec) / 60 % 60; // minutes
  time_s = (GLOBAL.start_delay_tm - current_sec) % 60;		// seconds
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
  if (FLAGS.redraw)
  {
    //routine for just moving to end point if nothing was stored.
    lcd.empty();
    lcd.at(1, 1, "Enter Aux End Pt");
    delay(GLOBAL.prompt_time);

    //move the aux motor 0.5 inches in the positive direction
    int32_t z = SETTINGS.STEPS_PER_INCH_AUX / 2;
    set_target(0, 0, z);

    dda_move(20);

    //Serial.println(EEPROM_STORED.current_steps.z);
    // Serial.println(int(EEPROM_STORED.current_steps.z/STEPS_PER_INCH_AUX));

    NunChuckRequestData(); //  Use this to clear out any button registry from the last step
    lcd.empty();
    lcd.at(1, 1, "AuxDist:   .  In");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    //delay(GLOBAL.prompt_time)
    aux_dist = EEPROM_STORED.current_steps.z * 10 / SETTINGS.STEPS_PER_INCH_AUX; //t
    DisplayAUX_Dist();
    FLAGS.redraw = false;
  }

  int32_t aux_dist_last = aux_dist;
  NunChuckRequestData();
  NunChuckProcessData();

  aux_dist += joy_capture3();
  aux_dist = constrain(aux_dist, int32_t(-SETTINGS.MAX_AUX_MOVE_DISTANCE), int32_t(SETTINGS.MAX_AUX_MOVE_DISTANCE));

  //STEPS_PER_INCH_AUX

  if (aux_dist_last != aux_dist) {
    DisplayAUX_Dist();
  }

  //lcd.at(1,1,aux_dist/10);
  //lcd.at(1,7,aux_dist%10);
  //EEPROM_STORED.motor_steps_pt[2][2]=(aux_dist*47812L)/10;
  //lcd.at(2,1,(long)EEPROM_STORED.motor_steps_pt[2][2]);
  button_actions_Enter_Aux_Endpoint();  //read buttons, look for c button press to set interval
  delay (GLOBAL.prompt_delay);

  //delay(1);
}


void DisplayAUX_Dist()
{
  if (abs(aux_dist) < 10)
  {
    if (aux_dist < 0)   lcd.at(1, 9, " -0.");
    else				lcd.at(1, 9, "  0.");
    //lcd.at(1,10,aux_dist/10);
    lcd.at(1, 13, abs(aux_dist % 10));
  }
  else if (abs(aux_dist) < 100)
  {
    lcd.at(1, 9, "  ");
    if (aux_dist < 0)   lcd.at(1, 10, aux_dist / 10);
    else				lcd.at(1, 11, aux_dist / 10);
    lcd.at(1, 13, abs(aux_dist % 10));
  }
  else if (abs(aux_dist) < 1000)
  {
    lcd.at(1, 9, " ");
    if (aux_dist < 0)   lcd.at(1, 9, aux_dist / 10);
    else				lcd.at(1, 10, aux_dist / 10);
    lcd.at(1, 13, abs(aux_dist % 10));
  }
}


void button_actions_Enter_Aux_Endpoint()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      //calculate_deltas();

      lcd.empty();
      EEPROM_STORED.motor_steps_pt[0][2] = 0;//this sets the end point
      EEPROM_STORED.motor_steps_pt[1][2] = 0;//this sets the end point
      EEPROM_STORED.motor_steps_pt[2][2] = aux_dist * SETTINGS.STEPS_PER_INCH_AUX / 10;  //this sets the end point
#if DEBUG
      Serial.println(EEPROM_STORED.motor_steps_pt[2][2]);
#endif
      lcd.at(1, 2, "Aux End Pt. Set");
      delay(GLOBAL.prompt_time);
      progstep_forward();
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}

//END BETA CODE
