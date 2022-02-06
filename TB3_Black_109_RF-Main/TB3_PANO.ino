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


uint32_t	motor_steps_pov[2][2];  // 2 total points.  Used for Pano Calcs and other hard targets
uint8_t		olpercentage = 20;
uint32_t	steps_per_shot_max_x;
uint32_t	steps_per_shot_max_y;
uint32_t	total_shots_x; //calulated value for to divide up scene evenly
uint32_t	total_shots_y; //calulated value for to divide up scene evenly
uint32_t	total_pano_shots; //rows x columns for display
int32_t		step_per_pano_shot_x = 0; // Calculated value for how many steps in a pano move
int32_t		step_per_pano_shot_y = 0; // Calculated value for how many steps in a pano move
float     local_total_pano_move_time = 0;
int32_t   AOV_Offset_X = 1;
int32_t   AOV_Offset_Y = 1;
int32_t   Corner2_x = 0;
int32_t   Corner2_y = 0;

// Should belong in Pano, need to shift out
uint32_t  Pan_AOV_steps;
uint32_t  Tilt_AOV_steps;

//---------------------------------------------------------------------------------------------------

//Portrait Pano
#define PanoArrayTypeOptions 5

uint8_t PanoArrayType = 1;
enum PanoArrayType : uint8_t {
  PANO_9ShotCenter  = 1,
  PANO_25ShotCenter = 3,
  PANO_7X3          = 2,
  PANO_9X5Type1     = 6,
  PANO_9X5Type2     = 7,
  PANO_5x5TopThird  = 4,
  PANO_7X5TopThird  = 5
};

//PORTRAITPANO Method Variables -  3x3, 7x3, 5x5 top third, 7x5 top third

//  2,-2 |  1,-2 |  0,-2 | -1,-2 | -2,-2
//  2,-1 |  1,-1 |  0,-1 | -1,-1 | -2,-1
//  2, 0 |  1, 0 |  0, 0 | -1, 0 | -2, 0
//  2, 1 |  1, 1 |  0, 1 | -1, 1 | -2, 1
//  2, 2 |  1, 2 |  0, 2 | -1, 2 | -2, 2

int8_t OnionArray9 [10][2] =  {{ 0, 0}, { 0, -1}, { -1, -1}, { -1, 0}, { -1, 1}, { 0, 1}, { 1, 1}, { 1, 0}, { 1, -1}, {0, 0}};

int8_t OnionArray25 [26][2] = {{ 0, 0}, { 0, -1}, { -1, -1}, { -1, 0}, { -1, 1}, { 0, 1}, { 1, 1}, { 1, 0}, { 1, -1}, {1, -2},
  { 0, -2}, { -1, -2}, { -2, -2}, { -2, -1}, { -2, -0}, { -2, 1}, { -2, 2}, { -1, 2}, { 0, 2}, {1, 2},
  { 2, 2}, { 2, 1}, { 2, 0}, { 2, -1}, { 2, -2}, {0, 0}
};

int8_t SevenByThree[22][2] =  {{ 0, 0}, { 0, -1}, { -1, -1}, { -1, 0}, { -1, 1}, { 0, 1}, { 1, 1}, { 1, 0}, { 1, -1}, {2, -1},
  { 2, 0}, { 2, 1}, { 3, 1}, { 3, 0}, { 3, -1}, { -2, -1}, { -2, 0}, { -2, 1}, { -3, 1}, { -3, 0},
  { -3, -1}, { 0, 0}
};

int8_t NineByFive_1[46][2] =  {{ 0, 0}, { 0, -1}, { -1, -1}, { -1, 0}, { -1, 1}, { 0, 1}, { 1, 1}, { 1, 0}, { 1, -1}, { 2, -1},
  { 2, 0}, { 2, 1}, { 3, 1}, { 3, 0}, { 3, -1}, { 4, -1}, { 4, 0}, { 4, 1}, { 4, 2}, { 3, 2},
  { 2, 2}, { 1, 2}, { 0, 2}, { -1, 2}, { -2, 2}, { -2, 1}, { -2, 0}, { -2, -1}, { -3, -1}, { -3, 0},
  { -3, 1}, { -3, 2}, { -4, 2}, { -4, 1}, { -4, 0}, { -4, -1}, { -4, -2}, { -3, -2}, { -2, -2}, { -1, -2},
  { 0, -2}, { 1, -2}, { 2, -2}, { 3, -2}, { 4, -2}, { 0, 0}
};

int8_t NineByFive_2[46][2] =  {{ 0, 0}, { 0, -1}, { -1, -1}, { -1, 0}, { -1, 1}, { 0, 1}, { 1, 1}, { 1, 0}, { 1, -1}, { 2, -1},
  { 2, 0}, { 2, 1}, { 2, 2}, { 1, 2}, { 0, 2}, { -1, 2}, { -2, 2}, { -2, 1}, { -2, 0}, { -2, -1},
  { -2, -2}, { -1, -2}, { 0, -2}, { 1, -2}, { 2, -2}, { 3, -2}, { 3, -1}, { 3, 0}, { 3, 1}, { 3, 2},
  { 4, 2}, { 4, 1}, { 4, 0}, { 4, -1}, { 4, -2}, { -3, -2}, { -3, -1}, { -3, 0}, { -3, 1}, { -3, 2},
  { -4, 2}, { -4, 1}, { -4, 0}, { -4, -1}, { -4, -2}, { 0, 0}
};

int8_t TopThird25 [26][2] =   {{ 0, 0}, { 0, -1}, { 0, -2}, { -1, -2}, { -1, -1}, { -1, 0}, { -1, 1}, { 0, 1}, { 1, 1}, { 1, 0},
  { 1, -1}, { 1, -2}, { 2, 1}, { 2, 0}, { 2, -1}, { 2, -2}, { 2, -3}, { 1, -3}, { 0, -3}, { -1, -3},
  { -2, -3}, { -2, -2}, { -2, -1}, { -2, 0}, { -2, 1}, { 0, 0}
};

int8_t TopThird7by5 [36][2] = {{ 0, 0}, { 0, -1}, { 0, -2}, { -1, -2}, { -1, -1}, { -1, 0}, { -1, 1}, { 0, 1}, { 1, 1}, { 1, 0},
  { 1, -1}, { 1, -2}, { 2, -2}, { 2, -1}, { 2, 0}, { 2, 1}, { 3, 1}, { 3, 0}, { 3, -1}, { 3, -2},
  { 3, -3}, { 2, -3}, { 1, -3}, { 0, -3}, { -1, -3}, { -2, -3}, { -2, -2}, { -2, -1}, { -2, 0}, { -2, 1},
  { -3, 1}, { -3, 0}, { -3, -1}, { -3, -2}, { -3, -3}, { 0, 0}
};




#define Gigapano_Options  10    //up this when code for gotoframe
enum Gigapano : uint8_t {
  GIGAPANO_AOV_SET            = 0,
  GIGAPANO_CORNER_1_SET       = 1,
  GIGAPANO_CORNER_2_SET       = 2,
  GIGAPANO_OVERLAP_SET        = 3,
  GIGAPANO_CAMERA_FIRE_TIME   = 4,
  GIGAPANO_MINIMUM_MOVE_TIME  = 5,
  GIGAPANO_TRIGGER_TYPE       = 6,
  GIGAPANO_SERPENTINE         = 7,
  GIGAPANO_LOOP               = 8,
  GIGAPANO_START              = 9
};
uint8_t Gigapano_Item = 0;


void GigaPano_Menu()
{
  if (redraw)
  {
    lcd.at(1, 1, " Gigapano Menu ");
    switch (Gigapano_Item)
    { //0123456789ABCDEF
      case GIGAPANO_AOV_SET:            lcd.at(2, 1, " Set Image AOV  ");  break;
      case GIGAPANO_CORNER_1_SET:       lcd.at(2, 1, " Set Corner 1   ");  break;
      case GIGAPANO_CORNER_2_SET:       lcd.at(2, 1, " Set Corner 2   ");  break;
      case GIGAPANO_OVERLAP_SET:        lcd.at(2, 1, " Set Overlap%   ");  break;
      case GIGAPANO_CAMERA_FIRE_TIME:   lcd.at(2, 1, " Set Static Time");  break;
      case GIGAPANO_MINIMUM_MOVE_TIME:  lcd.at(2, 1, "Set Minimum Time");  break;
      case GIGAPANO_TRIGGER_TYPE:       lcd.at(2, 1, "Set Trigger Type");  break;
      case GIGAPANO_SERPENTINE:         lcd.at(2, 1, " Serpentine     ");  break;
      case GIGAPANO_LOOP:               lcd.at(2, 1, " Loop Setting   ");  break;
      case GIGAPANO_START:              lcd.at(2, 1, " Start Shooting ");  break;
    }
    delay(prompt_time);
    redraw = false;
  }

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch (joy_capture_y_map())
    {
      case 1: // Up
        if (Gigapano_Item == (Gigapano_Options - 1)) Gigapano_Item = 0;
        else                                         Gigapano_Item++;
        redraw = true;
        break;

      case -1: // Down
        if (Gigapano_Item) Gigapano_Item--;
        else               Gigapano_Item = Gigapano_Options - 1;
        redraw = true;
        break;
    }

    switch (HandleButtons())
    {
      case C_Pressed:
        switch (Gigapano_Item)
        {
          case GIGAPANO_AOV_SET:           progstep_goto(201);  break;
          case GIGAPANO_CORNER_1_SET:      progstep_goto(202);  break;
          case GIGAPANO_CORNER_2_SET:      progstep_goto(203);  break;
          case GIGAPANO_OVERLAP_SET:       progstep_goto(204);  break;
          case GIGAPANO_CAMERA_FIRE_TIME:  progstep_goto(205);  break;
          case GIGAPANO_MINIMUM_MOVE_TIME: progstep_goto(206);  break;
          case GIGAPANO_TRIGGER_TYPE:      progstep_goto(207);  break;
          case GIGAPANO_SERPENTINE:        progstep_goto(208);  break;
          case GIGAPANO_LOOP:              progstep_goto(209);  break;
          case GIGAPANO_START:             progstep_goto(210);  break;
        }
        break;

      case Z_Pressed:
        ReturnToMenu();
        break;
    }
  }
}


void Set_angle_of_view()
{
  if (redraw)
  {
    if (POWERSAVE_PT <= PWR_MOVEONLY_ON)   enable_PT(); // Turn on power to the motors if move only
    lcd.at(1, 1, " Set AOV Size  ");
    lcd.at(2, 1, " C-Set  Z-Exit ");
    delay(prompt_time);
    lcd.at(2, 1, "    CZ-Clear   ");
    delay(prompt_time);

    if (AOV_Offset_X == 1) AOV_Offset_X = current_steps.x;
    else                   AOV_Offset_X = current_steps.x + Pan_AOV_steps;

    if (AOV_Offset_Y == 1) AOV_Offset_Y = current_steps.y;
    else                   AOV_Offset_Y = current_steps.y + Tilt_AOV_steps;

    lcd.empty();
    draw(77, 1, 1); //lcd.at(1, 1, "Pan AOV: ");
    lcd.at(1, 11, steps_to_deg_decimal(current_steps.x - AOV_Offset_X));
    draw(78, 2, 1); //lcd.at(2, 1, "Tilt AOV: ");
    lcd.at(2, 11, steps_to_deg_decimal(current_steps.y - AOV_Offset_Y));

    //   Velocity Engine update
    DFSetup(); //setup the ISR
    redraw = false;
  }

  //Velocity Engine update
  if (!nextMoveLoaded && (millis() - NClastread) > NCdelay)
  {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    updateMotorVelocities2();

    lcd.at(1, 11, steps_to_deg_decimal(current_steps.x - AOV_Offset_X));
    lcd.at(2, 11, steps_to_deg_decimal(current_steps.y - AOV_Offset_Y));
    button_actions_set_aov();
  }
}


void button_actions_set_aov()
{
  switch (HandleButtons())
  {
    case CZ_Held:
      AOV_Offset_X = current_steps.x;
      AOV_Offset_Y = current_steps.y;
      Pan_AOV_steps = 0;
      Tilt_AOV_steps = 0;
      break;

    case C_Pressed:
      Motor_Stop();
      Pan_AOV_steps  = (current_steps.x - AOV_Offset_X); //Serial.println(Pan_AOV_steps);
      Tilt_AOV_steps = (current_steps.y - AOV_Offset_Y); //Serial.println(Tilt_AOV_steps);
      motors[0].position = current_steps.x;
      motors[1].position = current_steps.y;
      if (Corner2_x || Corner2_y)
      {
        lcd.empty();
        lcd.at (1, 1, "Position Reset ");
        lcd.at (2, 1, "   Corner 2    ");
        delay(prompt_time);
        setupMotorMove(0, Corner2_x);
        setupMotorMove(1, Corner2_y);
        updateMotorVelocities();
        do
        {
          if (!nextMoveLoaded)  updateMotorVelocities();  //finished up the interrupt routine
        }
        while (motorMoving);
      }
      lcd.empty();
      lcd.at (1, 5, "AOV Set");
      delay(prompt_time);
      if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
      progstep_goto(200);
      break;

    case Z_Held:
      Motor_Stop();
      motors[0].position = current_steps.x;
      motors[1].position = current_steps.y;
      if (Corner2_x || Corner2_y)
      {
        lcd.empty();
        lcd.at (1, 1, "Position Reset ");
        lcd.at (2, 1, "   Corner 2    ");
        delay(prompt_time);
        setupMotorMove(0, Corner2_x);
        setupMotorMove(1, Corner2_y);
        updateMotorVelocities();
        do
        {
          if (!nextMoveLoaded)  updateMotorVelocities();  //finished up the interrupt routine
        }
        while (motorMoving);
      }
      if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
      progstep_goto(200);
      break;
  }
}


String steps_to_deg_decimal(int32_t steps)
{
  //Function is tested for 0 to 99.99 degrees - at 100 degrees and over, the decimal place moves.
  if (steps < 0) steps = -steps;
  String Degree_display = "";

  uint32_t temp_degs = (steps * 9L) / 4000L; //  9/4000 = 1/444.4444
  uint16_t temp_mod =  (steps * 9L) % 4000L;
  uint8_t  temp_tenths      = temp_mod / 400L;
  uint8_t  temp_hundredths  = temp_mod / 40L - temp_tenths * 10;
  uint8_t  temp_thousandths = temp_mod / 4L - temp_tenths * 100 - temp_hundredths * 10;

  if (temp_degs < 10) Degree_display += " ";
  Degree_display += temp_degs;
  Degree_display += ".";
  Degree_display += temp_tenths;
  Degree_display += temp_hundredths;
  Degree_display += temp_thousandths;
  return Degree_display;
}


void Motor_Stop()
{
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
}


void Move_to_Point_Pano_X(uint8_t Point)
{
  if (redraw)
  {
    if (POWERSAVE_PT <= PWR_MOVEONLY_ON)   enable_PT(); // Turn on power to the motors if move only
    motors[0].position = current_steps.x;
    motors[1].position = current_steps.y;
    if (Point == 0)
    {
      if (Corner2_x || Corner2_y)
      {
        lcd.empty(); //123456789ABCDEF
        lcd.at (1, 1, "Position Reset ");
        lcd.at (2, 1, "   Corner 1    ");
        delay(prompt_time);
        setupMotorMove(0, 0);
        setupMotorMove(1, 0);
        updateMotorVelocities();
        do
        {
          if (!nextMoveLoaded)  updateMotorVelocities();  //finished up the interrupt routine
        }
        while (motorMoving);
      }
      lcd.at(1, 1, " Set Corner 1  ");
      lcd.at(2, 1, " C-Set  Z-Exit ");
      delay(prompt_time);
    }
    else if (Point == 1)
    { //123456789ABCDEF
      lcd.empty();
      lcd.at(1, 1, " Set Corner 2  ");
      lcd.at(2, 1, " C-Set  Z-Exit ");
      delay(prompt_time);
    }

    //   Velocity Engine update
    DFSetup(); //setup the ISR
    redraw = false;
  }

  //  Velocity Engine update
  if (!nextMoveLoaded && (millis() - NClastread) > NCdelay)
  {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    updateMotorVelocities2();
    button_actions_move_pano_x(Point); //check buttons
  }
}


void button_actions_move_pano_x(uint8_t Point)
{
  switch (HandleButtons())
  {
    case C_Pressed:
      Motor_Stop();
      motors[0].position = current_steps.x;
      motors[1].position = current_steps.y;
      if (Point == 0)
      {
        if (Corner2_x || Corner2_y)
        {
          Corner2_x -= current_steps.x; // We moved corner 1, need to add on the change to preserve corner 2
          Corner2_y -= current_steps.y;
          set_position(0, 0, 0); //reset for home position
          lcd.empty(); //123456789ABCDEF
          lcd.at (1, 1, "Position Reset ");
          lcd.at (2, 1, "   Corner 2    ");
          delay(prompt_time);
          setupMotorMove(0, Corner2_x);
          setupMotorMove(1, Corner2_y);
          updateMotorVelocities();
          do
          {
            if (!nextMoveLoaded)  updateMotorVelocities();  //finished up the interrupt routine
          }
          while (motorMoving);
        }
        else
        {
          set_position(0, 0, 0); //reset for home position
        }
        lcd.at (1, 1, " Corner 1 Set ");
        delay(prompt_time);
      }

      else if (Point == 1)
      {
        calc_pano_move();
        lcd.at (1, 1, " Corner 2 Set ");
        Corner2_x = current_steps.x;
        Corner2_y = current_steps.y;
        delay(prompt_time);
      }
      if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
      progstep_goto(200);
      break;

    case Z_Pressed:
      Motor_Stop();
      motors[0].position = current_steps.x;
      motors[1].position = current_steps.y;
      if (Corner2_x || Corner2_y)
      {
        lcd.empty();
        lcd.at (1, 1, "Position Reset ");
        lcd.at (2, 1, "   Corner 2    ");
        delay(prompt_time);
        setupMotorMove(0, Corner2_x + current_steps.x);
        setupMotorMove(1, Corner2_y + current_steps.y);
        updateMotorVelocities();
        do
        {
          if (!nextMoveLoaded)  updateMotorVelocities();  //finished up the interrupt routine
        }
        while (motorMoving);
      }
      if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
      progstep_goto(200);
      break;
  }
}


void Define_Overlap_Percentage()
{
  if (redraw)
  {
    lcd.empty();
    draw(79, 1, 3); //lcd.at(1,2,"   % Overlap");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    Display_olpercentage(olpercentage);
    redraw = false;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > NCdelay)
  {
    uint8_t olpercentage_last = olpercentage;
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    olpercentage -= joy_capture3(1);
    if (!olpercentage || olpercentage > 200) {
      olpercentage = 99;
    }
    else if (olpercentage > 99)              {
      olpercentage = 1;
    }

    if (olpercentage_last != olpercentage)
    {
      Display_olpercentage(olpercentage);
      delay(prompt_delay);
    }
    button_actions_olpercentage();  //read buttons, look for c button press to set interval
  }
}


void Display_olpercentage(uint8_t overlap_percent)
{
  if (overlap_percent < 10)
  {
    lcd.at(1, 3, " "); //clear extra if goes from 3 to 2 or 2 to  1
    lcd.at(1, 4, overlap_percent);
  }
  else
  {
    lcd.at(1, 3, overlap_percent);
  }
  // if (overlap_percent<100)  lcd.at(1,9," ");  //clear extra if goes from 3 to 2 or 2 to  1
}


void button_actions_olpercentage()
{
  switch (HandleButtons())
  {
    case C_Pressed: // perform all calcs based on Angle of view and percentage overlap
      steps_per_shot_max_x = abs(Pan_AOV_steps)  * (100 - olpercentage) / 100;
      steps_per_shot_max_y = abs(Tilt_AOV_steps) * (100 - olpercentage) / 100;

#if DEBUG_PANO
      Serial.print("steps_per_shot_max_x: "); Serial.println(steps_per_shot_max_x);
      Serial.print("steps_per_shot_max_y: "); Serial.println(steps_per_shot_max_y);
#endif

      lcd.empty();
      draw(80, 1, 3); // lcd.at(1,3,"Overlap Set");
      delay(prompt_time);

      if (progstep == 204) progstep_goto(200);
      else                 progstep_forward();
      break;

    case Z_Pressed:
      if (progstep == 204) progstep_goto(200);
      else                 progstep_backward();
      break;
  }
}


void Set_Static_Time_Pano()
{
  if (redraw)
  {
    lcd.empty();
    draw(22, 1, 1); //lcd.at(1,1,"Set Static Time");
    delay(prompt_time);
    lcd.empty();
    draw(23, 1, 1); //lcd.at(1,1,"Stat_T:   .  sec");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    max_shutter = 36000; //pano modes = 3600.0 Seconds
    DisplayStatic_tm_Pano(static_tm);
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
      DisplayStatic_tm_Pano(static_tm);
      delay(prompt_delay);
    }
    button_actions_stat_time_pano();  //read buttons, look for c button press to set interval
  }
}


void DisplayStatic_tm_Pano(uint16_t static_time)
{                                                     //89ABCDEF
  if      (Trigger_Type == Flash_Trigger)     lcd.at(1,8," Flash  ");
  else if (Trigger_Type == Video_Trigger)     lcd.at(1,8," Video  ");
  else if (Trigger_Type == External_Trigger)  lcd.at(1,8," Extern ");
  else if (Trigger_Type == Button_Trigger)    lcd.at(1,8," Button ");
  else
  {
    if (static_time < 10)
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
}


void button_actions_stat_time_pano()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      lcd.empty();
      draw(25, 1, 1); //lcd.at(1,1,"Static Time Set"); // try this to push correct character
      delay(prompt_time);
      progstep_goto(200);
      break;

    case Z_Pressed:
      progstep_goto(200);
      break;

    case CZ_Pressed:
      static_tm = 1;
      break;
  }
}


void Set_PanoArrayType()
{
  if (redraw)
  {
    lcd.empty();
    lcd.at(1, 2, "Set Array Type");
    switch (PanoArrayType)
    {
      case PANO_9ShotCenter:  lcd.at(2, 3, "9-Shot Center");  total_shots_x = 3;  total_shots_y = 3;  break;
      case PANO_25ShotCenter: lcd.at(2, 2, "25-Shot Center"); total_shots_x = 5;  total_shots_y = 5;  break;
      case PANO_7X3:          lcd.at(2, 4, "7x3 Matrix");     total_shots_x = 7;  total_shots_y = 3;  break;
      case PANO_9X5Type1:     lcd.at(2, 4, "9x5 Type 1");     total_shots_x = 9;  total_shots_y = 5;  break;
      case PANO_9X5Type2:     lcd.at(2, 4, "9x5 Type 2");     total_shots_x = 9;  total_shots_y = 5;  break;
      case PANO_5x5TopThird:  lcd.at(2, 4, "5x5 Top1/3");     total_shots_x = 5;  total_shots_y = 5;  break;
      case PANO_7X5TopThird:  lcd.at(2, 4, "7x5 Top1/3");     total_shots_x = 7;  total_shots_y = 5;  break;
    }

    total_pano_shots = total_shots_x * total_shots_y;
    camera_total_shots = total_pano_shots + 1; //set this to allow us to compare in main loops

    delay(prompt_time);
    redraw = false;
  }

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    switch (joy_capture_y_map())
    {
      case 1: // Up
        PanoArrayType++;
        if (PanoArrayType > PanoArrayTypeOptions)  {
          PanoArrayType = 1;
        }
        else                                       {
          redraw = true;
        }
        break;

      case -1: // Down
        PanoArrayType--;
        if (PanoArrayType < 1) {
          PanoArrayType = PanoArrayTypeOptions;
        }
        else                   {
          redraw = true;
        }
        break;
    }

    switch (HandleButtons())
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


void Pano_Review_Confirm()
{
  if (redraw)
  {
    calc_pano_move();
    lcd.empty();
    draw(41, 1, 4); //lcd.at(1,4,"Review and");
    draw(42, 2, 2); //lcd.at(2,2,"Confirm Setting");
    delay(prompt_time);

    reviewprog = 1;
    camera_fired = 0;

    local_total_pano_move_time = total_shots_x * total_shots_y * sqrt(steps_per_shot_max_x / (PAN_MAX_JOG_STEPS_PER_SEC / 2.0)) * 2;
    local_total_pano_move_time += total_shots_y * sqrt(steps_per_shot_max_y / (TILT_MAX_JOG_STEPS_PER_SEC / 2.0)) * 2;
    total_pano_move_time = local_total_pano_move_time;

    lcd.empty();
    Pano_DisplayReviewProg();
    display_last_tm = millis();
    redraw = false;
  }

  if ((millis() - display_last_tm) > 1000)
  { //test for display update
    reviewprog ++;
    display_last_tm = millis();
    if (reviewprog > 4) reviewprog = 1;
    Pano_DisplayReviewProg();
  } //end test for display update

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    if (abs(joy_y_axis) > 20)
    { //do read time updates to delay program
      reviewprog = 4;
      Pano_DisplayReviewProg();
      display_last_tm = millis();
    }
    pano_button_actions_review();
  }
}


void pano_button_actions_review()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      lcd.empty();

      if (start_delay_sec)
      {
        lcd.at(1, 2, "Delaying Start");
        //delay (start_delay_sec*60L*1000L);
        lcd.empty();
      }
      disable_AUX();  //

      draw(49, 1, 1); //lcd.at(1,1,"Program Running");
      delay(prompt_time);

      //static_tm = 1; //use a tenth of a second
      //Trigger_Type = ; // calc interval based on static time only
      //Trigger_Type = static_tm; // calc interval based on static time only
      interval = (static_tm + 3) * 100; //tenths of a second to ms
      interval_tm = 0; //set this to 0 to immediately trigger the first shot

      if (Trigger_Type > External_Trigger)
      { //SMS Mode
        lcd.empty(); //clear for non video
        progstep = 250; //  move to the main programcamera_real_fire
      }

      camera_fired = 0; //reset the counter
      Program_Engaged = true; //leave this for pano
      Flag_Wait_For_Trigger = true; //just to start off first shot immediately
      redraw = true;
      lcd.bright(LCD_BRIGHTNESS_RUNNING);
      if (move_with_acceleration)
      {
        interrupts();
        DFSetup(); //setup the ISR
        //Set the motor position between standard and dragonframes
        motors[0].position = current_steps.x;
        motors[1].position = current_steps.y;
        display_status();
        //DFloop();
      }
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void Pano_DisplayReviewProg()
{
  switch (reviewprog)
  {
    case 1:   //
      lcd.empty();
      lcd.at(1, 2, "Columns:");
      lcd.at(2, 5, "Rows:");
      lcd.at(1, 11, total_shots_x);
      lcd.at(2, 11, total_shots_y);
      break;

    case 2:   //
      lcd.empty();
      lcd.at(1, 4, "Total:");
      lcd.at(1, 11, total_pano_shots);
      calc_time_remain(local_total_pano_move_time);
      display_time(2, 6);
      break;

    case 3:   //
      lcd.empty();
      draw(47, 1, 6); // lcd.at(1,6,"Ready?");
      draw(48, 2, 2); // lcd.at(2,2,"Press C Button");
      break;

    case 4:   //
      lcd.empty();
      lcd.at(1, 3, "Delay:");
      lcd.at(2, 2, "Press C Button");

      start_delay_sec += joy_capture3(1);
      if (start_delay_sec > 60000)      {
        start_delay_sec = 1800;
      }
      else if (start_delay_sec > 1800)  {
        start_delay_sec = 0;
      }
      delay(prompt_delay);
      lcd.at(1, 10, start_delay_sec);
      break;
  }//end switch
}


void move_motors_pano_basic()
{
  uint16_t index_x; // Assuming max 65535 Row Position of Pano
  uint16_t index_y; // Assuming max 65535 Columns Position of Pano
  int16_t x_mod_pass_1; // Photos per Row
  bool even_odd_row;  // Used to change direction every row.
  int16_t slope_adjustment;

  //Figure out which row we are in

  index_y = camera_fired / total_shots_x;
  x_mod_pass_1 = camera_fired % total_shots_x;
  if (SERPENTINE) {
    even_odd_row = index_y % 2;
  }
  else           {
    even_odd_row = 0;
  }
  slope_adjustment = even_odd_row * ((total_shots_x - 1) - 2 * x_mod_pass_1);
  index_x = x_mod_pass_1 + slope_adjustment;

  int32_t x = motor_steps_pt[1][0] - step_per_pano_shot_x * index_x;
  int32_t y = motor_steps_pt[1][1] - step_per_pano_shot_y * index_y;

  set_target(x, y, 0); //we are in incremental mode to start abs is false

  dda_move(50);

  return;
} // end move motors basic


void move_motors_pano_accel()
{
  uint16_t index_x; // Assuming max 65535 Row Position of Pano
  uint16_t index_y; // Assuming max 65535 Columns Position of Pano
  int16_t x_mod_pass_1; // Photos per Row
  bool even_odd_row;  // Used to change direction every row.
  int16_t slope_adjustment;

  //Figure out which row we are in

  index_y = camera_fired / total_shots_x;
  x_mod_pass_1 = camera_fired % total_shots_x;
  if (SERPENTINE) {
    even_odd_row = index_y % 2;
  }
  else           {
    even_odd_row = 0;
  }
  slope_adjustment = even_odd_row * ((total_shots_x - 1) - 2 * x_mod_pass_1);
  index_x = x_mod_pass_1 + slope_adjustment;

  int32_t x = motor_steps_pt[1][0] - (step_per_pano_shot_x * index_x);
  int32_t y = motor_steps_pt[1][1] - (step_per_pano_shot_y * index_y);

  setPulsesPerSecond(0, PAN_MAX_JOG_STEPS_PER_SEC); //this is now pusing through d
  setPulsesPerSecond(1, TILT_MAX_JOG_STEPS_PER_SEC);
  setupMotorMove(0, x);
  setupMotorMove(1, y);

  updateMotorVelocities();
}//end move motors accel


void move_motors_accel_array()
{ //this is for the PORTRAITPANO array method

  //Figure out which row we are in program
  //load from array

  int32_t x = 0;
  int32_t y = 0;

  switch (PanoArrayType)
  {
    case PANO_9ShotCenter:
      x = steps_per_shot_max_x * OnionArray9[camera_fired][0];
      y = int32_t(steps_per_shot_max_y * OnionArray9[camera_fired][1]) * -1;
      break;

    case PANO_25ShotCenter:
      x = steps_per_shot_max_x * OnionArray25[camera_fired][0];
      y = int32_t(steps_per_shot_max_y * OnionArray25[camera_fired][1]) * -1;
      break;

    case PANO_7X3:
      x = steps_per_shot_max_x * SevenByThree[camera_fired][0];
      y = int32_t(steps_per_shot_max_y * SevenByThree[camera_fired][1]) * -1;
      break;

    case PANO_9X5Type1:
      x = steps_per_shot_max_x * NineByFive_1[camera_fired][0];
      y = int32_t(steps_per_shot_max_y * NineByFive_1[camera_fired][1]) * -1;
      break;

    case PANO_9X5Type2:
      x = steps_per_shot_max_x * NineByFive_2[camera_fired][0];
      y = int32_t(steps_per_shot_max_y * NineByFive_2[camera_fired][1]) * -1;
      break;

    case PANO_5x5TopThird:
      x = steps_per_shot_max_x * TopThird25[camera_fired][0];
      y = int32_t(steps_per_shot_max_y * TopThird25[camera_fired][1]) * -1;
      break;

    case PANO_7X5TopThird:
      x = steps_per_shot_max_x * TopThird7by5[camera_fired][0];
      y = int32_t(steps_per_shot_max_y * TopThird7by5[camera_fired][1]) * -1;
      break;
  }

  setPulsesPerSecond(0, PAN_MAX_JOG_STEPS_PER_SEC);
  setPulsesPerSecond(1, TILT_MAX_JOG_STEPS_PER_SEC);
  setupMotorMove(0, x);
  setupMotorMove(1, y);

  updateMotorVelocities();
}//end move motors accel


void Move_to_Origin()
{ //this is for the PORTRAITPANO array method
  //Figure out which row we are in program
#if DEBUG_PANO
  Serial.print("camera_fired;"); Serial.println(camera_fired);
#endif
  //load from array

  setPulsesPerSecond(0, PAN_MAX_JOG_STEPS_PER_SEC);
  setPulsesPerSecond(1, TILT_MAX_JOG_STEPS_PER_SEC);
  setupMotorMove(0, 0);
  setupMotorMove(1, 0);

  updateMotorVelocities();
  //Move_Engaged = false; //clear move engaged flag
}//end move motors accel


void calc_pano_move() //pano - calculate other values
{
  // Assumes there is always some remainder from the division adding 1 image,
  // Also need a second extra image as it doesnt account for the width of the image
  total_shots_x = (abs(current_steps.x) / steps_per_shot_max_x) + 2;
  total_shots_y = (abs(current_steps.y) / steps_per_shot_max_y) + 2;

  if (abs(current_steps.x) < (steps_per_shot_max_x >> 3)) { //test if the angle is very small - Implying single Column
    total_shots_x = 1;
  }
  if (abs(current_steps.y) < (steps_per_shot_max_y >> 3)) { //test if the angle is very small - Implying single Row
    total_shots_y = 1;
  }

  total_pano_shots = total_shots_x * total_shots_y;
  camera_total_shots = total_pano_shots;//set this to allow us to compare in main loops

  // Right now it adds extra overlap if the spacing doesnt add up to a full image to preserve the corners.
  // Think I may change it later to instead offset the position to include the extra margin with a fixed overlap
  step_per_pano_shot_x = current_steps.x / int32_t(total_shots_x - 1);
  step_per_pano_shot_y = current_steps.y / int32_t(total_shots_y - 1);
}


void button_actions290()
{ //repeat
  switch (HandleButtons())
  {
    case C_Pressed:
      camera_fired = 0; //reset the counter
      Program_Engaged = true; //leave this for pano
      redraw = true;
      progstep = 250;
      current_steps.x = motors[0].position; //get our motor position variable synced
      current_steps.y = motors[1].position; //get our motor position variable synced
      updateMotorVelocities();
      do
      {
        if (!nextMoveLoaded)  updateMotorVelocities();  //finished up the interrupt routine
      }
      while (motorMoving);
      break;

    case Z_Pressed:
      if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
      if (POWERSAVE_AUX > PWR_PROGRAM_ON)  disable_AUX();
      camera_fired = 0;
      current_steps.x = motors[0].position; //get our motor position variable synced
      current_steps.y = motors[1].position; //get our motor position variable synced

      lcd.bright(LCD_BRIGHTNESS_RUNNING);
      if      (progtype == PANOGIGA)     progstep = 206; //  move to the main program at the interval setting - UD050715
      else if (progtype == PORTRAITPANO) progstep = 306; //  move to the main program at the interval setting UD050715
      redraw = true;
      break;
  }
}
