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
  if (first_time)
  {
    // Clean Up any previous states
    if (POWERSAVE_PT > 2)    disable_PT();  //  Put the motors back to idle
    if (POWERSAVE_AUX > 2)   disable_AUX(); //
    lcd.empty();                            //  Clear the LCD

    // Select what menu item we want.
    switch (progtype)
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
    delay(prompt_time/2);
    first_time = 0;
  }

  if ((millis() - NClastread) > 50) {
    NClastread = millis();

    NunChuckRequestData();
    NunChuckProcessData();
    
    int8_t joyYDown = joy_capture_y_map();
    if (joyYDown == 1) 
    {
      first_time = 1;
      if (progtype == (MENU_ITEMS - 1)) { progtype = 0; }
      else                              { progtype++;   }
    }
    else if (joyYDown == -1)
    {
      first_time = 1;
      if (progtype)  { progtype--;               }
      else           { progtype = (MENU_ITEMS - 1);  // accomodating rollover
      }
    }
  }


//#define MENU_ITEMS  8
//enum progtype : uint8_t {
//  REG2POINTMOVE = 0,
//  REV2POINTMOVE = 1,
//  REG3POINTMOVE = 2,
//  REV3POINTMOVE = 3,
//  PANOGIGA      = 4,
//  PORTRAITPANO  = 5,
//  DFSLAVE       = 6,
//  SETUPMENU     = 7,
//  ASCOMSLAVE    = 98,
//  AUXDISTANCE   = 99
//};

  
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
          intval = 5;//default this for static time selection
          interval = 100;//default this to low value to insure we don't have left over values from old progam delaying shots.
          progstep_goto(201);
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
          progstep_goto(901);
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


//Move to Start Point
void Move_to_Startpoint()
{
  if (first_time) {
    lcd.empty();
    lcd.bright(6);
    
    if (!REVERSE_PROG_ORDER) //normal programing, start first
    {
      draw(8, 1, 1); //lcd.at(1,1,"Move to Start Pt");
      draw(14, 2, 6); //lcd.at(2,6,"C-Next");
    }
    else
    {
      draw(15, 1, 1); //lcd.at(1,1,"Move to End Pt.");
      draw(3, 2, 1); //lcd.at(2,1,CZ1);
    }

    first_time = 0;
    delay(prompt_time);

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
        //Serial.print("motorMoving:");Serial.println(motorMoving);
        if (!nextMoveLoaded)
        {
          updateMotorVelocities2();
        }
      } while (motorMoving);

      lcd.empty();
      set_position(0, 0, 0); //sets current steps to 0
#if DEBUG
      Serial.print("current_steps_start.x: "); Serial.println(current_steps.x);
      Serial.print("current_steps_start.y: "); Serial.println(current_steps.y);
      Serial.print("current_steps_start.z: "); Serial.println(current_steps.z);
#endif
      if (!REVERSE_PROG_ORDER) draw(9, 1, 3);  //lcd.at(1,3,"Start Pt. Set");
      else					           draw(16, 1, 3); //lcd.at(1,3,"End Point Set");
      delay(prompt_time);
      progstep_forward();
      break;

    case Z_Pressed:
      progstep_goto(0);
      break;
  }
}

void Move_to_Endpoint()
{
  if (first_time) {

    lcd.empty();

    if (!REVERSE_PROG_ORDER) //mormal programming, end point last
    {
      draw(15, 1, 1); //lcd.at(1,1,"Move to End Pt.");
      draw(3, 2, 1); //lcd.at(2,1,CZ1);
    }
    else //reverse programing, start last
    {
      draw(8, 1, 1); //lcd.at(1,1,"Move to Start Pt");
      draw(14, 2, 6); //lcd.at(2,6,"C-Next");
    }

    first_time = 0;
    startISR1 ();
    delay(prompt_time);
    enable_PanTilt();
    if (AUX_ON) enable_AUX();  //
  }
  /*
    NunChuckRequestData();
    NunChuckProcessData();
    applyjoymovebuffer_exponential();
    dda_move(feedrate_micros);
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
        //Serial.print("motorMoving:");Serial.println(motorMoving);
        if (!nextMoveLoaded)
        {
          updateMotorVelocities2();
        }
      }
      while (motorMoving);

      //end stop the motors

      motors[0].position = current_steps.x;
      motors[1].position = current_steps.y;
      motors[2].position = current_steps.z;

#if DEBUG
      Serial.print("motors[0].position:"); Serial.println( motors[0].position);
      Serial.print("motors[1].position:"); Serial.println( motors[1].position);
      Serial.print("motors[2].position:"); Serial.println( motors[2].position);
#endif

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

      if (!REVERSE_PROG_ORDER) draw(16, 1, 3); //lcd.at(1,3,"End Point Set");
      else					             draw(9, 1, 3); //lcd.at(1,3,"Start Pt. Set");

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
  if (first_time)
  {
    lcd.empty();
    if (progstep == 201 || progstep == 301)
    {
      // Programming center for PANOGIGA AND PORTRAITPANO UD010715
      lcd.at(1, 2, "Set AOV Corner");
      set_position(0, 0, 0);
    }
    else if (progstep == 304)
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
    first_time = 0;
    delay(prompt_time);
    

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
          //Serial.print("motorMoving:");Serial.println(motorMoving);
          if (!nextMoveLoaded)
          {
            updateMotorVelocities2();
          }
        }
        while (motorMoving);
        //end stop the motors
  
        if (Point == 0) set_position(0, 0, 0);		 //reset for home position
  
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
  
        if (progstep != 201 && progstep != 301)
        {
          lcd.empty();
          draw(63, 1, 3); //lcd.at(1,3,"Point X Set";);
          if (!REVERSE_PROG_ORDER)  lcd.at(1, 9, (Point + 1)); //Normal
          else //reverse programming
          {
            if (Point == 0) lcd.at(1, 9, "3");
            else if (Point == 1) lcd.at(1, 9, "2");
            else if (Point == 2) lcd.at(1, 9, "1");
          }
        }
  
        if (progstep == 202 || progstep == 302) //set angle of view UD050715
        {
          Pan_AOV_steps  = current_steps.x; //Serial.println(Pan_AOV_steps);
          Tilt_AOV_steps = current_steps.y; //Serial.println(Tilt_AOV_steps);
          lcd.empty();
          lcd.at (1, 5, "AOV Set");
        }
        if (progstep == 205) //pano - calculate other values UD050715
        {
          calc_pano_move();
        }
        if (progstep == 304) // PORTRAITPANO Method UD050715
        {
          lcd.empty();
          lcd.at (1, 4, "Center Set");
        }
  
        delay(prompt_time);
        progstep_forward();
        break;
     

    case Z_Pressed:
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
        progstep_backward();
      break;
  }
}


//Set Camera Interval
void Set_Cam_Interval()
{
  if (first_time)
  {
    lcd.empty();
    draw(17, 1, 1); //lcd.at(1,1,"Set Sht Interval");
    delay(prompt_time);
    lcd.empty();
    draw(18, 1, 1); //lcd.at(1,1,"Intval:   .  sec");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    DisplayInterval();
    first_time = 0;
  }

  uint16_t intval_last = intval;
  NunChuckRequestData();
  NunChuckProcessData();
  if (intval < 20) joy_y_lock_count = 0;
  intval += joy_capture3();
  intval  = constrain(intval, 2, 6000); //remove the option for Ext Trigger right now
  if (intval_last != intval) {
    DisplayInterval();
  }

  button_actions_intval(intval);  //read buttons, look for c button press to set interval
  delay (70);
  //delay (prompt_delay);
}


void DisplayInterval()
{
  if (intval == EXTTRIG_INTVAL) //run the Ext
  {
    draw(19, 1, 8); //lcd.at(1,8," Ext.Trig");
  }
  else if (intval == VIDEO_INTVAL) //run the video routine
  {
    draw(20, 1, 8); //lcd.at(1,8," Video   ");
  }
  else if (intval < 10)
  {
    lcd.at(1, 13, " sec");
    lcd.at(1, 8, "  0.");
    lcd.at(1, 10, intval / 10);
    lcd.at(1, 12, intval % 10);
  }
  else if (intval < 100)
  {
    lcd.at(1, 8, "  ");
    lcd.at(1, 10, intval / 10);
    lcd.at(1, 12, intval % 10);
  }
  else if (intval < 1000)
  {
    lcd.at(1, 8, " ");
    lcd.at(1, 9, intval / 10);
    lcd.at(1, 12, intval % 10);
  }
  else
  {
    lcd.at(1, 8, intval / 10);
    lcd.at(1, 12, intval % 10);
  }
}


void button_actions_intval(uint16_t intval)
{
  switch (HandleButtons())
  {
    case C_Pressed:
    {
      uint16_t video_sample_ms = 100; //
      interval = intval * 100; //tenths of a second to ms
      //camera_exp_tm=100;
      if (intval == EXTTRIG_INTVAL) //means ext trigger
      {
        interval = 6000; //Hardcode this to 6.0 seconds are 10 shots per minute to allow for max shot selection
      }
      else if (intval == VIDEO_INTVAL) //means video, set very small exposure time
      {
        //camera_exp_tm=5; //doesn't matter, we never call the shutter
        interval = video_sample_ms; //we overwrite this later based on move length currently 100
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
  if (first_time)
  {
    lcd.empty();
    //			1234567890123456
    draw(32, 1, 5); //lcd.at(1,5,	"Set Move");
    draw(33, 2, 5); //lcd.at(2,5,	"Duration");
    delay(prompt_time * 1.5);
    lcd.empty();
    draw(34, 1, 10); //lcd.at(1,9,"H:MM:SS");
    if ((intval > 3) || (intval == EXTTRIG_INTVAL))
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
    //Display_Duration();
    first_time = 0;
  }
  NunChuckRequestData();
  NunChuckProcessData();

  if (intval == VIDEO_INTVAL)
  { //video
    uint16_t overaldur_last = overaldur;
    overaldur += joy_capture3();
    if (overaldur <= 0) {
      overaldur = 10000;
    }
    if (overaldur > 10000) {
      overaldur = 1;
    }
    if (overaldur_last != overaldur)
    {
      calc_time_remain_dur_sec (overaldur);
      display_time(1, 1);
    }
  }
  else
  { //sms
    uint32_t camera_moving_shots_last = camera_moving_shots;
    camera_moving_shots += joy_capture3();
    if (camera_moving_shots <= 9) {
      camera_moving_shots = 10000;
    }
    if (camera_moving_shots > 10000) {
      camera_moving_shots = 10;
    }
    //camera_moving_shots=constrain(camera_moving_shots,10,10000);
    camera_total_shots = camera_moving_shots; //we add in lead in lead out later
    if (camera_moving_shots_last != camera_moving_shots) {
      Display_Duration();
    } //end update time and shots
  } //end sms
  button_actions_overaldur();  //read buttons, look for c button press to set overall time
  delay (prompt_delay);
}


void Display_Duration()
{
  calc_time_remain(total_pano_move_time);
  display_time(1, 1);

  if ((intval > 3) || (intval == EXTTRIG_INTVAL))
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
  if (first_time)
  {
    lcd.empty();
    draw(22, 1, 1); //lcd.at(1,1,"Set Static Time");
    delay(prompt_time);
    lcd.empty();
    draw(23, 1, 1); //lcd.at(1,1,"Stat_T:   .  sec");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    max_shutter = intval - MIN_INTERVAL_STATIC_GAP; //max static is .3 seconds less than interval (leaves 0.3 seconds for move)
    if (intval == EXTTRIG_INTVAL) max_shutter = 600; //external trigger
    if (progtype == PANOGIGA || progtype == PORTRAITPANO) max_shutter = 36000; //pano mode - allows
    DisplayStatic_tm();
    first_time = 0;
  }

  uint16_t static_tm_last = static_tm;
  NunChuckRequestData();
  NunChuckProcessData();

  if (static_tm < 20) joy_y_lock_count = 0;
  static_tm += joy_capture3();
  if (static_tm > 60000) static_tm = max_shutter;
  static_tm = constrain(static_tm, 1, max_shutter);

  if (static_tm_last != static_tm) DisplayStatic_tm();
  button_actions_stat_time();  //read buttons, look for c button press to set interval
  delay (prompt_delay);
}


void DisplayStatic_tm()
{
  if (intval == VIDEO_INTVAL)	draw(24, 1, 8); //lcd.at(1,8," Video   ");

  else if (static_tm < 10)
  {
    lcd.at(1, 8, "  0.");
    lcd.at(1, 10, static_tm / 10);
    lcd.at(1, 12, static_tm % 10);
  }
  else if (static_tm < 100)
  {
    lcd.at(1, 8, "   .");
    lcd.at(1, 10, static_tm / 10);
    lcd.at(1, 12, static_tm % 10);
  }
  else if (static_tm < 1000)
  {
    lcd.at(1, 8, "   .");
    lcd.at(1, 9, static_tm / 10);
    lcd.at(1, 12, static_tm % 10);
  }
  else if (static_tm < 10000)
  {
    lcd.at(1, 8, "   .  ");
    lcd.at(1, 8, static_tm / 10);
    lcd.at(1, 12, static_tm % 10);
  }
}


void button_actions_stat_time()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      //if (intval!=3)  camera_exp_tm=(long)static_tm*100; //want to get to ms, but this is already multipled by 10 // 3 is used for video mode - this is likely obsolete-remove later
      lcd.empty();
      draw(25, 1, 1); //lcd.at(1,1,"Static Time Set"); // try this to push correct character
      delay(prompt_time);
      progstep_forward();
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void Set_Ramp()
{
  if (first_time)
  {
    lcd.empty();
    draw(29, 1, 1); //lcd.at(1,1,"	Set Ramp");
    delay(prompt_time);
    lcd.empty();
    draw(30, 1, 1); //lcd.at(1,1,"Ramp:	 Frames");
    if (intval == VIDEO_INTVAL) {
      camera_moving_shots = 147; //allow for up to 49 % ramp
      lcd.at(1, 10, "Percent");
    }

    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    DisplayRampval();
    first_time = 0;
  }

  uint16_t rampval_last = rampval;
  NunChuckRequestData();
  NunChuckProcessData();
  if (rampval < 20) joy_y_lock_count = 0;
  rampval += joy_capture3();
  if (rampval < 1) {
    rampval = 1;
    delay(prompt_time / 2);
  }
  //if (rampval>500) {rampval=2;}

  if ( rampval * 3 > camera_moving_shots ) {	// we have an issue where the ramp is to big can be 2/3 of total move, but not more.
    rampval = camera_moving_shots / 3; //set to 1/3 up and 1/3 down (2/3) of total move)
    delay(prompt_time / 2);
  }

  //rampval=constrain(rampval,1,500); //
  if (rampval_last != rampval) {
    DisplayRampval();
  }
  //delay(50);
  button_actions_rampval();  //read buttons, look for c button press to set interval
  delay (prompt_delay);
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

      if (intval == VIDEO_INTVAL) {
        delay(1);
      }

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

  if (first_time)
  {
    lcd.empty();
    draw(36, 1, 1); //lcd.at(1,1,"Set Static Lead");
    draw(37, 2, 2); //lcd.at(2,2,"In/Out Frames");
    delay(prompt_time);
    lcd.empty();
    draw(38, 1, 6); //lcd.at(1,1,"IN -	Out");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    first_time = 0;
    lcd.at(1, 9, lead_out);
    cursorpos = cursorleft;
    DisplayLeadIn_LeadOut();
  }

  int16_t joyxysum_last = joy_x_axis + joy_y_axis; //figure out if changing

  NunChuckRequestData();
  NunChuckProcessData();
  cursorpos += joy_capture_x_map();

  if (joyxysum_last != (joy_x_axis + joy_y_axis) || abs(joy_x_axis + joy_y_axis) > 10) { //check to see if there is an input, otherwise don't update display
    DisplayLeadIn_LeadOut();
  }

  button_actions_lead_in_out();  //read buttons, look for c button press to ramp
  delay (prompt_delay);
}


void DisplayLeadIn_LeadOut()
{
  if (cursorpos == cursorright) lcd.pos(1, 13);

  if (cursorpos == cursorleft)
  { //update lead in
    lcd.cursorOff();
    lead_in += joy_capture3();
    if (lead_in < 1)    {
      lead_in = 5000;
    }
    if (lead_in > 5000) {
      lead_in = 1;
    }
    lcd.at(1, 1, lead_in);

    if (lead_in < 10)   {
      lcd.at(1, 2, "   ");
    }
    if (lead_in < 100)  {
      lcd.at(1, 3, "  ");
    }
    if (lead_in < 1000) {
      lcd.at(1, 4, " ");
    }
    lcd.cursorBlock();
  }
  else
  { //update leadout
    lcd.cursorOff();
    lead_out += joy_capture3();
    if (lead_out < 1)    {
      lead_out = 5000;
    }
    if (lead_out > 5000) {
      lead_out = 1;
    }
    lcd.at(1, 9, lead_out);

    if (lead_out < 10)   {
      lcd.at(1, 10, "   ");
    }
    if (lead_out < 100)  {
      lcd.at(1, 11, "  ");
    }
    if (lead_out < 1000) {
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

        if (intval==VIDEO_INTVAL) {
      	//if (intval==VIDEO_INTVAL) camera_moving_shots=(overaldur*1000L)/interval; //figure out moving shots based on duration for video only
      	//camera_moving_shots=video_segments; //hardcode this and back into proper interval - this is XXX segments per sequence
      	//camera_moving_shots=100; //new method to allow for easy ramp %
      	//interval=overaldur*1000L/camera_moving_shots;   //This gives us ms
      	camera_total_shots=camera_moving_shots+lead_in+lead_out; //good temp variable for display
        }

        //fix issues with ramp that is too big
        if( rampval*3 > camera_moving_shots )
        {	// we have an issue where the ramp is to big can be 2/3 of total move, but not more.
        rampval=camera_moving_shots/3; //set to 1/3 up and 1/3 down (2/3) of total move)
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
        if (POWERSAVE_PT>2)   disable_PT();
        if (POWERSAVE_AUX>2)   disable_AUX();
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

  if (intval == VIDEO_INTVAL)
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
  if (POWERSAVE_PT > 2)   disable_PT();
  if (POWERSAVE_AUX > 2)   disable_AUX();
  //end of code block pulled from LeadinLeadOut
}


void Review_Confirm()
{
  if (first_time)
  {
    lcd.empty();
    draw(41, 1, 4); //lcd.at(1,4,"Review and");
    draw(42, 2, 2); //lcd.at(2,2,"Confirm Setting");
    //delay(prompt_time);
    delay(prompt_time);
    //delay(100);
    lcd.empty();
    first_time = 0;
    diplay_last_tm = millis();
    DisplayReviewProg();
    reviewprog = 2;
    start_delay_sec = 0;
  }

  if ((millis() - diplay_last_tm) > (prompt_time * 4))
  { //test for display update
    //if ((millis()-diplay_last_tm) >(700)){ //test for display update

    reviewprog ++;
    diplay_last_tm = millis();
    if (reviewprog > 4) reviewprog = 2;
    first_time2 = true;
    DisplayReviewProg();
  } //end test for display update

  NunChuckRequestData();
  NunChuckProcessData();

  if (abs(joy_y_axis) > 20)
  { //do read time updates to delay program
    reviewprog = 4;
    DisplayReviewProg();
    diplay_last_tm = millis();
  }

  button_actions_review();
  delay (prompt_delay);
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
      lcd.at(1, 12, (int)linear_steps_per_shot[0]);
      lcd.at(2, 12, (int)linear_steps_per_shot[1]);
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
      if (first_time2)
      {
        lcd.at(1, 1, "Set Start Delay");
        lcd.at(2, 1, "			");
        draw(34, 2, 10); //lcd.at(2,10,"H:MM:SS");
        first_time2 = false;
      }

      if (start_delay_sec < 20) joy_y_lock_count = 0; //this is an unsigned int
      start_delay_sec += joy_capture3();
      //if (start_delay_sec<0) {start_delay_sec=0;} //this statement doesn't do anything as this is an unsigned int
      if (start_delay_sec > 43200) {
        start_delay_sec = 0;
      }
      calc_time_remain_dur_sec (start_delay_sec);
      display_time(2, 1);
      //lcd.at(1,11,start_delay_sec);
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
      MOVE_REVERSED_FOR_RUN = false; //Reset This
      start_delay_tm = ((millis() / 1000L) + start_delay_sec); //system seconds in the future - use this as big value to compare against
      draw(34, 2, 10); //lcd.at(2,10,"H:MM:SS");
      lcd.at(1, 2, "Delaying Start");
      CZ_Button_Read_Count = 0; //reset this to zero to start

      while (start_delay_tm > millis() / 1000L)
      {
        //enter delay routine
        calc_time_remain_start_delay ();
        if ((millis() - diplay_last_tm) > 1000) display_time(2, 1);
        NunChuckRequestData();
        NunChuckProcessData();
        Check_Prog(); //look for long button press
        if (CZ_Button_Read_Count > 20 && !Program_Engaged)
        {
          start_delay_tm = ((millis() / 1000L) + 5); //start right away by lowering this to 5 seconds.
          CZ_Button_Read_Count = 0; //reset this to zero to start
        }
      }

      enable_PanTilt();
      if (AUX_ON) enable_AUX();  //

      //draw(49,1,1);//lcd.at(1,1,"Program Running");
      //delay(prompt_time/3);

      if (intval == EXTTRIG_INTVAL)  lcd.at(2, 1, "Waiting for Man.");

      Program_Engaged = true;
      Interrupt_Fire_Engaged = true; //just to start off first shot immediately
      interval_tm = 0; //set this to 0 to immediately trigger the first shot
      sequence_repeat_count = 0; //this is zeroed out every time we start a new shot

      if (intval > 3) { //SMS Mode
        lcd.empty(); //clear for non video
        progstep = 50; //  move to the main programcamera_real_fire
      }
      else if (intval == VIDEO_INTVAL)
      {
        lcd.empty();
        draw(49, 1, 1); //lcd.at(1,1,"Program Running");
        progstep = 51;
      }
      else if (intval == EXTTRIG_INTVAL)  { //manual trigger/ext trigger
        lcd.empty(); //clear for non video
        progstep = 52; //  move to the external interrupt loop
        ext_shutter_count = 0; //set this to avoid any prefire shots or cable insertions.
        lcd.at(1, 1, "Waiting for Trig");
      }
      //lcd_backlight_cur= 100;
      first_time = 1;
      lcd.bright(LCD_BRIGHTNESS_DURING_RUN);
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void progstep_forward()
{
  lcd.empty();
  first_time = 1;
  progstep_forward_dir = true;
  if (progstep < 65535) progstep++;
  delay(100);
  NunChuckClearData(); //  Use this to clear out any button registry from the last step
}


void progstep_backward()
{
  lcd.empty();
  first_time = 1;
  progstep_forward_dir = false;
  if (progstep) progstep--;
  delay(100);
  NunChuckClearData(); //  Use this to clear out any button registry from the last step
}


void progstep_goto(uint16_t prgstp)
{
  lcd.empty();
  first_time = 1;
  progstep = prgstp;
  delay(100);
  NunChuckClearData(); //  Use this to clear out any button registry from the last step
}


void button_actions_end_of_program()
{ //repeat - need to turn off the REVERSE_PROG_ORDER flag
  switch (HandleButtons())
  {
    case C_Pressed:
      // Normal
      REVERSE_PROG_ORDER = false;
      if (POWERSAVE_PT > 2)   disable_PT();
      if (POWERSAVE_AUX > 2)   disable_AUX();
      //Program_Engaged=true;
      camera_fired = 0;
      lcd.bright(8);
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
      if (POWERSAVE_PT > 2)   disable_PT();
      if (POWERSAVE_AUX > 2)   disable_AUX();
      //Program_Engaged=true;
      camera_fired = 0;
      lcd.bright(8);
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
  lcd.bright(8);
  if (progtype == REG2POINTMOVE || progtype == REV2POINTMOVE) {
    go_to_start_new();
    //progstep_goto(8);
  }

  lcd.empty();
  MOVE_REVERSED_FOR_RUN = false; //Reset This
  //start_delay_tm=((millis()/1000L)+start_delay_sec); //system seconds in the future - use this as big value to compare against
  //draw(34,2,10);//lcd.at(2,10,"H:MM:SS");
  //lcd.at(1,2,"Delaying Start");
  CZ_Button_Read_Count = 0; //reset this to zero to start

  /*
    while (start_delay_tm>millis()/1000L) {
  	//enter delay routine
  	calc_time_remain_start_delay ();
  	if ((millis()-diplay_last_tm) > 1000) display_time(2,1);
  	NunChuckRequestData();
  	NunChuckProcessData();
  	Check_Prog(); //look for long button press
  	if (CZ_Button_Read_Count>20 && !Program_Engaged) {
  		start_delay_tm=((millis()/1000L)+5); //start right away by lowering this to 5 seconds.
  		CZ_Button_Read_Count=0; //reset this to zero to start
  	}
    }
  */
  enable_PanTilt();
  enable_AUX();  //

  draw(49, 1, 1); //lcd.at(1,1,"Program Running");
  //delay(prompt_time/3);

  Program_Engaged = true;
  Interrupt_Fire_Engaged = true; //just to start off first shot immediately

  interval_tm = 0; //set this to 0 to immediately trigger the first shot

  if (intval > 3) { //SMS Mode
    lcd.empty(); //clear for non video
    progstep = 50; //  move to the main programcamera_real_fire
  }

  else if (intval == VIDEO_INTVAL)  {
    //lcd.empty(); //leave "program running up for video
    progstep = 51;
  }
  /*
    else if (intval==EXTTRIG_INTVAL)  { //manual trigger/ext trigge
  	lcd.empty(); //clear for non video
  	progstep=52; //  move to the external interrup loop
  	ext_shutter_count=0; //set this to avoid any prefire shots or cable insertions.
  	lcd.at(1,1,"Waiting for Trig");
    }
  */
  //lcd_backlight_cur= 100;
  first_time = 1;
  lcd.bright(LCD_BRIGHTNESS_DURING_RUN);
}//end of 91


void Pause_Prog()
{
  CZ_Button_Read_Count = 0;
  Program_Engaged = !Program_Engaged; //turn off the loop
  if (!Program_Engaged) {  //program turned off
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
  if (first_time)
  {
    lcd.empty();
    lcd.at(1, 6, "/"); //Add to one time display update
    lcd.at(2, 13, ".");
    lcd.at(2, 16, "v");
    first_time = 0;
  }
  //update upper left camera fired/total shots
  unsigned int camera_fired_display = camera_fired + 1;
  if		  (camera_fired_display < 10)    lcd.at(1, 5, camera_fired_display);
  else if (camera_fired_display < 100)   lcd.at(1, 4, camera_fired_display);
  else if (camera_fired_display < 1000)  lcd.at(1, 3, camera_fired_display);
  else if (camera_fired_display < 10000) lcd.at(1, 2, camera_fired_display);
  else								                   lcd.at(1, 1, camera_fired_display);

  lcd.at(1, 7, camera_total_shots);

  //Update program progress secion - upper right

  if (progtype == REG2POINTMOVE || progtype == REV2POINTMOVE || progtype == AUXDISTANCE) {
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

  if (progtype == REG3POINTMOVE || progtype == REV3POINTMOVE)
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

  if (progtype == PANOGIGA || progtype == PORTRAITPANO) {
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
      first_time = 1;
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
  if (first_time)
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

    NunChuckRequestData(); //  Use this to clear out any button registry from the last step
    lcd.empty();
    lcd.at(1, 1, "AuxDist:   .  In");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    //delay(prompt_time)
    aux_dist = current_steps.z * 10 / STEPS_PER_INCH_AUX; //t
    DisplayAUX_Dist();
    first_time = 0;
  }

  int32_t aux_dist_last = aux_dist;
  NunChuckRequestData();
  NunChuckProcessData();

  aux_dist += joy_capture3();
  //aux_dist=constrain(aux_dist,-999,999);
  aux_dist = constrain(aux_dist, -MAX_AUX_MOVE_DISTANCE, MAX_AUX_MOVE_DISTANCE);

  //STEPS_PER_INCH_AUX

  if (aux_dist_last != aux_dist) {
    DisplayAUX_Dist();
  }

  //lcd.at(1,1,aux_dist/10);
  //lcd.at(1,7,aux_dist%10);
  //motor_steps_pt[2][2]=(aux_dist*47812L)/10;
  //lcd.at(2,1,(long)motor_steps_pt[2][2]);
  button_actions_Enter_Aux_Endpoint();  //read buttons, look for c button press to set interval
  delay (prompt_delay);

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
