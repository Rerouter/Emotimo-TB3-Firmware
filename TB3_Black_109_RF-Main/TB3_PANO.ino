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


int8_t OnionArray9 [10][2] =  {{0, 0}, {0, -1}, { -1, -1}, { -1, 0}, { -1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, 0}};

int8_t OnionArray25 [26][2] = {{0, 0}, {0, -1}, { -1, -1}, { -1, 0}, { -1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -2},
  { -1, -2}, { -2, -2}, { -2, -1}, { -2, -0}, { -2, 1}, { -2, 2}, { -1, 2}, {0, 2}, {1, 2}, {2, 2},
  {2, 1}, {2, 0}, {2, -1}, {2, -2}, {1, -2}, {0, 0}
};

int8_t SevenByThree[22][2] =  {{0, 0}, {0, -1}, { -1, -1}, { -1, 0}, { -1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {2, -1},
  {2, 0}, {2, 1}, {3, 1}, {3, 0}, {3, -1}, { -2, -1}, { -2, 0}, { -2, 1}, { -3, 1}, { -3, 0},
  { -3, -1}, {0, 0}
};
int8_t NineByFive_1[46][2] =  {{0, 0}, {0, -1}, { -1, -1}, { -1, 0}, { -1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {2, -1},
  {2, 0}, {2, 1}, {3, 1}, {3, 0}, {3, -1}, {4, -1}, {4, 0}, {4, 1}, {4, 2}, {3, 2}, {2, 2},
  {1, 2}, {0, 2}, { -1, 2}, { -2, 2}, { -2, 1}, { -2, 0}, { -2, -1}, { -3, -1}, { -3, 0}, { -3, 1},
  { -3, 2}, { -4, 2}, { -4, 1}, { -4, 0}, { -4, -1}, { -4, -2}, { -3, -2}, { -2, -2}, { -1, -2},
  {0, -2}, {1, -2}, {2, -2}, {3, -2}, {4, -2}, {0, 0}
};
int8_t NineByFive_2[46][2] =  {{0, 0}, {0, -1}, { -1, -1}, { -1, 0}, { -1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {2, -1},
  {2, 0}, {2, 1}, {2, 2}, {1, 2}, {0, 2}, { -1, 2}, { -2, 2}, { -2, 1}, { -2, 0}, { -2, -1},
  { -2, -2}, { -1, -2}, {0, -2}, {1, -2}, {2, -2}, {3, -2}, {3, -1}, {3, 0}, {3, 1}, {3, 2},
  {4, 2}, {4, 1}, {4, 0}, {4, -1}, {4, -2}, { -3, -2}, { -3, -1}, { -3, 0}, { -3, 1}, { -3, 2},
  { -4, 2}, { -4, 1}, { -4, 0}, { -4, -1}, { -4, -2}, {0, 0}
};
int8_t TopThird25 [26][2] =  {{0, 0}, {0, -1}, {0, -2}, { -1, -2}, { -1, -1}, { -1, 0}, { -1, 1}, {0, 1}, {1, 1}, {1, 0},
  {1, -1}, {1, -2}, {2, 1}, {2, 0}, {2, -1}, {2, -2}, {2, -3}, {1, -3}, {0, -3}, { -1, -3},
  { -2, -3}, { -2, -2}, { -2, -1}, { -2, 0}, { -2, 1}, {0, 0}
};
int8_t TopThird7by5 [36][2] = {{0, 0}, {0, -1}, {0, -2}, { -1, -2}, { -1, -1}, { -1, 0}, { -1, 1}, {0, 1}, {1, 1}, {1, 0},
  {1, -1}, {1, -2}, {2, -2}, {2, -1}, {2, 0}, {2, 1}, {3, 1}, {3, 0}, {3, -1}, {3, -2},
  {3, -3}, {2, -3}, {1, -3}, {0, -3}, { -1, -3}, { -2, -3}, { -2, -2}, { -2, -1}, { -2, 0}, { -2, 1},
  { -3, 1}, { -3, 0}, { -3, -1}, { -3, -2}, { -3, -3}, {0, 0}
};


void Set_angle_of_view()
{
  if (FLAGS.redraw)
  {
    SETTINGS.AUX_ON = false; // turn off Aux as only PT Needed for this step

    lcd.empty();
    draw(75, 1, 1); //lcd.at(1,1,"Set Angle o'View");
    draw(76, 2, 2); //lcd.at(2,2,"C-Set, Z-Reset");
    delay(GLOBAL.prompt_time);
    
    EEPROM_STORED.current_steps.x = 0;
    EEPROM_STORED.current_steps.y = 0;
    
    lcd.empty();
    draw(77, 1, 1); //lcd.at(1,1,"Pan AOV: ");
    lcd.at(1, 11, steps_to_deg_decimal(0));
    draw(78, 2, 1); //lcd.at(2,1,"Tilt AOV: ");
    lcd.at(2, 11, steps_to_deg_decimal(0));
    
    FLAGS.redraw = false;

    //   Velocity Engine update
    DFSetup(); //setup the ISR
    //int32_t *ramValues = (int32_t *)malloc(sizeof(int32_t) * MOTOR_COUNT);
    //int32_t *ramNotValues = (int32_t *)malloc(sizeof(int32_t) * MOTOR_COUNT);
  }

  //Velocity Engine update
  if (!nextMoveLoaded)
  {
    NunChuckRequestData();
    NunChuckProcessData();
    updateMotorVelocities2();

    lcd.at(1, 11, steps_to_deg_decimal(abs(EEPROM_STORED.current_steps.x)));
    lcd.at(2, 11, steps_to_deg_decimal(abs(EEPROM_STORED.current_steps.y)));
    button_actions_move_x(1);
  }
}


void Define_Overlap_percentage()
{
  if (FLAGS.redraw)
  {
    lcd.empty();
    draw(79, 1, 3); //lcd.at(1,2,"   % Overlap");
    draw(3, 2, 1); //lcd.at(2,1,CZ1);
    //olpercentage=20;
    Display_olpercentage();
    FLAGS.redraw = false;
    delay(GLOBAL.prompt_time);
  }

  uint8_t olpercentage_last = olpercentage;
  NunChuckRequestData();
  NunChuckProcessData();
  if (olpercentage < 20) GLOBAL.joy_y_lock_count = 0;
  olpercentage += joy_capture3();
  if (!olpercentage || olpercentage > 250) {
    olpercentage = 1;
  }
  else if (olpercentage > 99)					{
    olpercentage = 99;
  }

  if (olpercentage_last != olpercentage)
  {
    Display_olpercentage();
  }
  button_actions_olpercentage();  //read buttons, look for c button press to set interval
  delay (GLOBAL.prompt_delay);
}


void Display_olpercentage()
{
  if (olpercentage < 10)
  {
    lcd.at(1, 3, " "); //clear extra if goes from 3 to 2 or 2 to  1
    lcd.at(1, 4, olpercentage);
  }
  else
  {
    lcd.at(1, 3, olpercentage);
  }
  // if (olGLOBAL.percentage<100)  lcd.at(1,9," ");  //clear extra if goes from 3 to 2 or 2 to  1
}


void button_actions_olpercentage()
{
  switch (HandleButtons())
  {
    case C_Pressed: // looking for c button press
      //perform all calcs based on Angle of view and GLOBAL.percentage overlap
      Pan_AOV_steps  = abs(EEPROM_STORED.current_steps.x); //Serial.println(Pan_AOV_steps);
      Tilt_AOV_steps = abs(EEPROM_STORED.current_steps.y); //Serial.println(Tilt_AOV_steps);
  
      steps_per_shot_max_x = Pan_AOV_steps  * (100 - olpercentage) / 100; //Serial.println(steps_per_shot_max_x);
      steps_per_shot_max_y = Tilt_AOV_steps * (100 - olpercentage) / 100; //Serial.println(steps_per_shot_max_y);
      
      #if DEBUG_PANO
      Serial.print("steps_per_shot_max_x: "); Serial.println(steps_per_shot_max_x);
      Serial.print("steps_per_shot_max_y: "); Serial.println(steps_per_shot_max_y);
      #endif
      
      lcd.empty();
      draw(80, 1, 3); //lcd.at(1,3,"Overlap Set");
      delay(GLOBAL.prompt_time);
      progstep_forward();
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}


void Set_PanoArrayType()
{
  /*
    3x3,
    7x3, 5x5 top third, 7x5 top third
    int PanoArrayTypes=1; // 1 is 9 shot center, 2 is 25 shot center, 3 is SevenbyThree, 4 is NineByFive1, 5 is NineByFive2
  */

  if (FLAGS.redraw)
  {
    lcd.empty();
    lcd.at(1, 2, "Set Array Type");
    switch(PanoArrayType)
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
    EEPROM_STORED.camera_total_shots = total_pano_shots + 1; //set this to allow us to compare in main loops
    
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
        PanoArrayType++;
        if (PanoArrayType > PanoArrayTypeOptions)  { PanoArrayType = PanoArrayTypeOptions; }
        else                                       { FLAGS.redraw = true; }
        break;
  
      case 1: // Down
        PanoArrayType--;
        if (PanoArrayType < 1) { PanoArrayType = 1; }
        else                   { FLAGS.redraw = true; }
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


String steps_to_deg_decimal(int32_t steps)
{
  //Function is tested for 0 to 99.99 degrees - at 100 degrees and over, the decimal place moves.
  if (steps < 0) steps = 0 - steps;
  String Degree_display = "";

  int32_t temp_degs = (steps * 9L) / 4000L; //  9/4000 = 1/444.4444
  int32_t temp_mod = (steps * 9L) % 4000L;
  int32_t temp_tenths = (temp_mod * 10) / 4000L;
  int32_t temp_hundredths = ((temp_mod * 100) / 4000L) - (temp_tenths * 10);
  int32_t temp_thousandths = ((temp_mod * 1000) / 4000L) - (temp_tenths * 100) - (temp_hundredths * 10);
  //Serial.println(temp_degs);
  //Serial.println(temp_mod);
  //Serial.println(temp_decimal);

  if (temp_degs < 10) Degree_display += " ";
  Degree_display += temp_degs;
  Degree_display += ".";
  Degree_display += temp_tenths;
  Degree_display += temp_hundredths;
  Degree_display += temp_thousandths;
  return Degree_display;
}


void Pano_Review_Confirm()
{
  if (FLAGS.redraw)
  {
    lcd.empty();
    draw(41, 1, 4); //lcd.at(1,4,"Review and");
    draw(42, 2, 2); //lcd.at(2,2,"Confirm Setting");
    delay(GLOBAL.prompt_time*2);
    
    reviewprog = 1;
    EEPROM_STORED.camera_fired = 0;
    
    local_total_pano_move_time = total_shots_x * total_shots_y * sqrt(steps_per_shot_max_x / (SETTINGS.PAN_MAX_JOG_STEPS_PER_SEC / 2.0)) * 2;
    local_total_pano_move_time += total_shots_y * sqrt(steps_per_shot_max_y / (SETTINGS.TILT_MAX_JOG_STEPS_PER_SEC / 2.0)) * 2;
    GLOBAL.total_pano_move_time = local_total_pano_move_time;
    
    lcd.empty();
    Pano_DisplayReviewProg();
    GLOBAL.display_last_tm = millis();
    FLAGS.redraw = false;
  }

  if ((millis() - GLOBAL.display_last_tm) > 1000)
  { //test for display update
    reviewprog ++;
    GLOBAL.display_last_tm = millis();
    if (reviewprog > 4) reviewprog = 1;
    Pano_DisplayReviewProg();
  } //end test for display update

  NunChuckRequestData();
  NunChuckProcessData();

  if (abs(GLOBAL.joy_y_axis) > 20)
  { //do read time updates to delay program
    reviewprog = 4;
    Pano_DisplayReviewProg();
    GLOBAL.display_last_tm = millis();
  }

  pano_button_actions_review();
  delay(100);
}


void pano_button_actions_review()
{
  switch(HandleButtons())
  {
    case C_Pressed:
      lcd.empty();
  
      if (GLOBAL.start_delay_sec)
      {
        lcd.at(1, 2, "Delaying Start");
        //delay (GLOBAL.start_delay_sec*60L*1000L);
        lcd.empty();
      }
  
      disable_AUX();  //
  
      draw(49, 1, 1); //lcd.at(1,1,"Program Running");
      delay(GLOBAL.prompt_time / 3);
  
      //EEPROM_STORED.static_tm = 1; //use a tenth of a second
      EEPROM_STORED.intval = EEPROM_STORED.static_tm + 3; // calc interval based on static time only
      //intval = EEPROM_STORED.static_tm; // calc interval based on static time only
      EEPROM_STORED.interval = EEPROM_STORED.intval * 100; //tenths of a second to ms
      GLOBAL.interval_tm = 0; //set this to 0 to immediately trigger the first shot
  
      if (EEPROM_STORED.intval > 3)
      { //SMS Mode
        lcd.empty(); //clear for non video
        EEPROM_STORED.progstep = 250; //  move to the main programcamera_real_fire
      }
  
      EEPROM_STORED.camera_fired = 0; //reset the counter
      EEPROM_STORED.Program_Engaged = true; //leave this for pano
      FLAGS.Interrupt_Fire_Engaged = true; //just to start off first shot immediately
      FLAGS.redraw = false;
      lcd.bright(20);
      if (move_with_acceleration)
      {
        interrupts();
        DFSetup(); //setup the ISR
        //Set the motor position between standard and dragonframes
        motors[0].position = EEPROM_STORED.current_steps.x;
        motors[1].position = EEPROM_STORED.current_steps.y;
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
      lcd.at(1, 1, "Cam Shots:");
      lcd.at(1, 12, total_pano_shots);
      calc_time_remain(local_total_pano_move_time);
      display_time(2, 6);
      break;

    case 3:   //
      lcd.empty();
      draw(47, 1, 6); //lcd.at(1,6,"Ready?");
      draw(48, 2, 2); //lcd.at(2,2,"Press C Button");
      break;

    case 4:   //
      //lcd.empty();
      lcd.at(1, 1, "Delay:        ");
      lcd.at(2, 2, "Press C Button");
      if (GLOBAL.start_delay_sec < 20) GLOBAL.joy_y_lock_count = 0;

      GLOBAL.start_delay_sec += joy_capture3();
      if (GLOBAL.start_delay_sec > 1800) {
        GLOBAL.start_delay_sec = 0;
      }
      if (GLOBAL.start_delay_sec == 0) {
        GLOBAL.start_delay_sec = 1800;
      }
      lcd.at(1, 7, GLOBAL.start_delay_sec);
      // if (start_delay_min <10)  lcd.at(1,8,"  ");  //clear extra if goes from 3 to 2 or 2 to  1
      // if (start_delay_min <100)  lcd.at(1,9," ");  //clear extra if goes from 3 to 2 or 2 to  1
      break;

  }//end switch
}


void move_motors_pano_basic()
{
  uint16_t index_x; // Row Position of Pano
  uint16_t index_y; // Column Position of Pano
  uint16_t x_mod_pass_1; // Photos per Row
  bool even_odd_row;
  int16_t slope_adjustment;

  //Figure out which row we are in

  index_y = EEPROM_STORED.camera_fired / total_shots_x;

  x_mod_pass_1 = EEPROM_STORED.camera_fired % total_shots_x;
  even_odd_row = index_y % 2;
  slope_adjustment = even_odd_row * ((total_shots_x - 1) - 2 * x_mod_pass_1);
  index_x = x_mod_pass_1 + slope_adjustment;

  int32_t x = EEPROM_STORED.motor_steps_pt[1][0] - step_per_pano_shot_x * index_x;
  int32_t y = EEPROM_STORED.motor_steps_pt[1][1] - step_per_pano_shot_y * index_y;

#if DEBUG_PANO
  Serial.print("camera_fired;"); Serial.println(EEPROM_STORED.camera_fired);
  Serial.print("x_mod_pass_1;"); Serial.println(x_mod_pass_1);

  Serial.print("slope_adjustment;"); Serial.println(slope_adjustment);

  Serial.print("Motor Steps X;"); Serial.println(EEPROM_STORED.motor_steps_pt[1][0]);
  Serial.print("Motor Steps Y;"); Serial.println(EEPROM_STORED.motor_steps_pt[1][1]);

  Serial.print("index_x;"); Serial.println(index_x);
  Serial.print("index_y;"); Serial.println(index_y);

  Serial.print("x;"); Serial.println(x);
  Serial.print("y;"); Serial.println(y);

  Serial.print("even_odd_row;"); Serial.println(even_odd_row);
#endif

  set_target(x, y, 0); //we are in incremental mode to start abs is false

  dda_move(50);
  FLAGS.Move_Engauged = false; //clear move engaged flag

  return;
} //end move motors basic


void move_motors_pano_accel()
{
  uint16_t index_x; // Assuming max 65535 photos wide max
  uint16_t index_y; // Assuming max 65535 photos high max
  int16_t x_mod_pass_1;
  bool even_odd_row;
  int16_t slope_adjustment;

  //Figure out which row we are in

  index_y = EEPROM_STORED.camera_fired / total_shots_x;
  x_mod_pass_1 = EEPROM_STORED.camera_fired % total_shots_x;
  even_odd_row = index_y % 2;
  slope_adjustment = even_odd_row * ((total_shots_x - 1) - 2 * x_mod_pass_1);
  index_x = x_mod_pass_1 + slope_adjustment;

  int32_t x = EEPROM_STORED.motor_steps_pt[1][0] - (step_per_pano_shot_x * index_x);
  int32_t y = EEPROM_STORED.motor_steps_pt[1][1] - (step_per_pano_shot_y * index_y);

#if DEBUG_PANO
  Serial.print("camera_fired;"); Serial.println(EEPROM_STORED.camera_fired);
  Serial.print("x_mod_pass_1;"); Serial.println(x_mod_pass_1);
  Serial.print("even_odd_row;"); Serial.println(even_odd_row);
  Serial.print("slope_adjustment;"); Serial.println(slope_adjustment);

  Serial.print("motor_steps_pt"); Serial.println(EEPROM_STORED.motor_steps_pt[1][1]);
  
  Serial.print("index_x;"); Serial.println(index_x);
  Serial.print("index_y;"); Serial.println(index_y);

  Serial.print("x;"); Serial.println(x);
  Serial.print("y;"); Serial.println(y);
#endif

  //set_target(fp.x,fp.y,0.0); //we are in incremental mode to start abs is false
  //dda_move(100);
  setPulsesPerSecond(0, SETTINGS.PAN_MAX_JOG_STEPS_PER_SEC); //this is now pusing through d
  setPulsesPerSecond(1, SETTINGS.TILT_MAX_JOG_STEPS_PER_SEC);
  setupMotorMove(0, x);
  setupMotorMove(1, y);

  updateMotorVelocities();

  //FLAGS.Move_Engauged=false; //clear move engaged flag

  return;
}//end move motors accel


void move_motors_accel_array()
{ //this is for the PORTRAITPANO array method

  //Figure out which row we are in program
#if DEBUG_PANO
  Serial.print("camera_fired;"); Serial.println(EEPROM_STORED.camera_fired);
#endif
  //load from array

  int32_t x = 0;
  int32_t y = 0;
  
  if (PanoArrayType == PANO_9ShotCenter) {
    x = steps_per_shot_max_x * OnionArray9[EEPROM_STORED.camera_fired][0];
    y = int32_t(steps_per_shot_max_y * OnionArray9[EEPROM_STORED.camera_fired][1]) * -1;
  }
  else if (PanoArrayType == PANO_25ShotCenter) {
    x = steps_per_shot_max_x * OnionArray25[EEPROM_STORED.camera_fired][0];
    y = int32_t(steps_per_shot_max_y * OnionArray25[EEPROM_STORED.camera_fired][1]) * -1;
  }
  else if (PanoArrayType == PANO_7X3) {
    x = steps_per_shot_max_x * SevenByThree[EEPROM_STORED.camera_fired][0];
    y = int32_t(steps_per_shot_max_y * SevenByThree[EEPROM_STORED.camera_fired][1]) * -1;
  }
  else if (PanoArrayType == PANO_9X5Type1) {
    x = steps_per_shot_max_x * NineByFive_1[EEPROM_STORED.camera_fired][0];
    y = int32_t(steps_per_shot_max_y * NineByFive_1[EEPROM_STORED.camera_fired][1]) * -1;
  }
  else if (PanoArrayType == PANO_9X5Type2) {
    x = steps_per_shot_max_x * NineByFive_2[EEPROM_STORED.camera_fired][0];
    y = int32_t(steps_per_shot_max_y * NineByFive_2[EEPROM_STORED.camera_fired][1]) * -1;
  }
  else if (PanoArrayType == PANO_5x5TopThird) {
    x = steps_per_shot_max_x * TopThird25[EEPROM_STORED.camera_fired][0];
    y = int32_t(steps_per_shot_max_y * TopThird25[EEPROM_STORED.camera_fired][1]) * -1;
  }
  else if (PanoArrayType == PANO_7X5TopThird) {
    x = steps_per_shot_max_x * TopThird7by5[EEPROM_STORED.camera_fired][0];
    y = int32_t(steps_per_shot_max_y * TopThird7by5[EEPROM_STORED.camera_fired][1]) * -1;
  }

  // x= EEPROM_STORED.motor_steps_pt[1][0] - step_per_pano_shot_x * index_x;
  // y= EEPROM_STORED.motor_steps_pt[1][1] - step_per_pano_shot_y * index_y;

#if DEBUG_PANO
  Serial.print("x;"); Serial.println(x);
  Serial.print("y;"); Serial.println(y);
#endif

  setPulsesPerSecond(0, 20000);
  setPulsesPerSecond(1, 20000);
  setupMotorMove(0, x);
  setupMotorMove(1, y);

  updateMotorVelocities();

  //FLAGS.Move_Engauged=false; //clear move engaged flag
  return;
}//end move motors accel


void Move_to_Origin()
{ //this is for the PORTRAITPANO array method
  //Figure out which row we are in program
#if DEBUG_PANO
  Serial.print("camera_fired;"); Serial.println(EEPROM_STORED.camera_fired);
#endif
  //load from array

  setPulsesPerSecond(0, 20000);
  setPulsesPerSecond(1, 20000);
  setupMotorMove(0, 0);
  setupMotorMove(1, 0);

  updateMotorVelocities();

  //FLAGS.Move_Engauged=false; //clear move engaged flag
  return;
}//end move motors accel


void calc_pano_move() //pano - calculate other values
{
  //unsigned long total_shots_x; //calulated value for to divide up scene evenly  ABS(current steps)/max steps per shot+1 = just use integer math
  //unsigned long total_shots_y; //calulated value for to divide up scene evenly
  //unsigned long total_pano_shots; //rows x columns for display
  //unsigned int step_per_pano_shot_x;
  //unsigned int step_per_pano_shot_y;
  total_shots_x = (abs(EEPROM_STORED.current_steps.x) / steps_per_shot_max_x) + 2;
  total_shots_y = (abs(EEPROM_STORED.current_steps.y) / steps_per_shot_max_y) + 2;
  if (abs(EEPROM_STORED.current_steps.y) < 444.0) { //do a test to see if the tilt angle is very small - indating pano
    total_shots_y = 1;
  }
  total_pano_shots = total_shots_x * total_shots_y;
  EEPROM_STORED.camera_total_shots = total_pano_shots;//set this to allow us to compare in main loops
  step_per_pano_shot_x = EEPROM_STORED.current_steps.x / int32_t(total_shots_x - 1);
  step_per_pano_shot_y = EEPROM_STORED.current_steps.y / int32_t(total_shots_y - 1);
}

void button_actions290()
{ //repeat
  switch (HandleButtons())
  {
    case C_Pressed:
      if (SETTINGS.POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
      if (SETTINGS.POWERSAVE_AUX > PWR_PROGRAM_ON)  disable_AUX();
      //EEPROM_STORED.Program_Engaged=true;
      EEPROM_STORED.camera_fired = 0;
      EEPROM_STORED.current_steps.x = motors[0].position; //get our motor position variable synced
      EEPROM_STORED.current_steps.y = motors[1].position; //get our motor position variable synced
      //noInterrupts(); //turn this off while programming for now
      lcd.bright(100);
      if      (EEPROM_STORED.progtype == PANOGIGA)     EEPROM_STORED.progstep = 206; //  move to the main program at the interval setting - UD050715
      else if (EEPROM_STORED.progtype == PORTRAITPANO) EEPROM_STORED.progstep = 306; //  move to the main program at the interval setting UD050715
      FLAGS.redraw = false;
      break;
  }
}
