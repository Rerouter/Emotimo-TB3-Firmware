//init our variables
//float motor_steps_pt[MAX_MOVE_POINTS][MOTORS];  // 3 total points.   Start point is always 0.0
unsigned long motor_steps_pov[2][2];  // 2 total points.  Used for Pano Calcs and other hard targets
unsigned long Pan_AOV_steps;
unsigned long Tilt_AOV_steps;
unsigned long olpercentage;
unsigned long steps_per_shot_max_x;
unsigned long steps_per_shot_max_y;
unsigned long total_shots_x; //calulated value for to divide up scene evenly
unsigned long total_shots_y; //calulated value for to divide up scene evenly
unsigned long total_pano_shots; //rows x columns for display
long step_per_pano_shot_x;
long step_per_pano_shot_y;
float     local_total_pano_move_time = 0;




void Set_angle_of_view()
{
  if (redraw)
  {
    AUX_ON = false; // turn off Aux as only PT Needed for this step

    lcd.empty();
    set_position(0.0, 0.0, 0.0);
    draw(75, 1, 1); //lcd.at(1, 1, "Set Angle o'View");
    draw(76, 2, 2); //lcd.at(2, 2, "C-Set, Z-Reset");
    delay(prompt_time);
    lcd.empty();
    draw(77, 1, 1); //lcd.at(1, 1, "Pan AOV: ");
    lcd.at(1, 11, steps_to_deg_decimal(0));
    draw(78, 2, 1); //lcd.at(2, 1, "Tilt AOV: ");
    lcd.at(2, 11, steps_to_deg_decimal(0));
    redraw = false;
    delay(prompt_time);
    NunChuckRequestData(); //  Use this to clear out any button registry from the last step
    //motor_steps_pt[2][0];
  }

  NunChuckRequestData();
  NunChuckProcessData();
  applyjoymovebuffer_exponential();

  dda_move(feedrate_micros);
  button_actions_move_x(1);  //read buttons - set first position
  if (feedrate_micros > 500) { //only move if almost stopped
    lcd.at(1, 11, steps_to_deg_decimal(abs(current_steps.x)));
    lcd.at(2, 11, steps_to_deg_decimal(abs(current_steps.y)));
  }
  else delayMicroseconds(200);
  //delay(1);
}


void Define_Overlap_Percentage()
{
  if (redraw)
  {
    lcd.empty();
    draw(79, 1, 3); //lcd.at(1, 2, "   % Overlap");
    draw(3, 2, 1); //lcd.at(2, 1, CZ1);
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

    olpercentage += joy_capture3(1);
    if (!olpercentage || olpercentage > 200) { olpercentage = 99; }
    else if (olpercentage > 99)              { olpercentage = 1;  }
  
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
      Pan_AOV_steps  = abs(current_steps.x);
      Tilt_AOV_steps = abs(current_steps.y);
  
      steps_per_shot_max_x = Pan_AOV_steps  * (100 - olpercentage) / 100;
      steps_per_shot_max_y = Tilt_AOV_steps * (100 - olpercentage) / 100;
      
      #if DEBUG_PANO
      Serial.print("steps_per_shot_max_x: "); Serial.println(steps_per_shot_max_x);
      Serial.print("steps_per_shot_max_y: "); Serial.println(steps_per_shot_max_y);
      #endif

      lcd.empty();
      draw(80, 1, 3); //lcd.at(1, 3, "Overlap Set");
      delay(prompt_time);
      progstep_forward();
      break;

    case Z_Pressed:
      progstep_backward();
      break;
  }
}


String steps_to_deg_decimal(unsigned long steps)
{
  //Function is tested for 0 to 99.99 degrees - at 100 degrees and over, the decimal place moves.
  String Degree_display = "";
  int temp_degs = (steps * 9L) / 4000L; //  9/4000 = 1/444.4444
  int temp_mod = (steps * 9L) % 4000L;
  int temp_tenths = (temp_mod * 10) / 4000L;
  int temp_hundredths = ((temp_mod * 100) / 4000L) - (temp_tenths * 10);
  int temp_thousandths = ((temp_mod * 1000) / 4000L) - (temp_tenths * 100) - (temp_hundredths * 10);
  //Serial.println(temp_degs);
  //Serial.println(temp_mod);
  //Serial.println(temp_decimal);

  if (temp_degs < 10) Degree_display += " ";
  Degree_display += temp_degs;
  Degree_display += ".";
  Degree_display += temp_tenths;
  Degree_display += temp_hundredths;
  Degree_display += temp_thousandths;
  Serial.println(Degree_display);
  return Degree_display;
}


void Pano_Review_Confirm()
{
  if (redraw)
  {
    lcd.empty();
    draw(41, 1, 4); //lcd.at(1, 4, "Review and");
    draw(42, 2, 2); //lcd.at(2, 2, "Confirm Setting");
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

  if ((millis() - display_last_tm) > 1000) { //test for display update
    reviewprog ++;
    display_last_tm = millis();
    if (reviewprog > 4) reviewprog = 1;
    Pano_DisplayReviewProg();
  } //end test for display update

  NunChuckRequestData();
  NunChuckProcessData();

  if (abs(joy_y_axis) > 20) { //do read time updates to delay program
    reviewprog = 4;
    Pano_DisplayReviewProg();
    display_last_tm = millis();
  }

  pano_button_actions_review();
  delay(100);
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
        //delay (start_delay_sec * 60L * 1000L);
        lcd.empty();
      }
      disable_AUX();  //

      draw(49, 1, 1); //lcd.at(1, 1, "Program Running");
      delay(prompt_time);

      //static_tm = 1; //use a tenth of a second
      //Trigger_Type = ; // calc interval based on static time only
      //Trigger_Type = static_tm; // calc interval based on static time only
      interval = (static_tm + 3) * 100; //tenths of a second to ms
      interval_tm = 0; //set this to 0 to immediately trigger the first shot

      if (intval > EXTTRIG_INTVAL)
      { //SMS Mode
        lcd.empty(); //clear for non video
        progstep = 250; //  move to the main programcamera_real_fire
      }

      camera_fired = 0; //reset the counter
      Program_Engaged = true; //leave this for pano
      Interrupt_Fire_Engaged = true; //just to start off first shot immediately
      redraw = true;
      lcd.bright(LCD_BRIGHTNESS_DURING_RUN);

      interrupts();
      DFSetup(); //setup the ISR
      //Set the motor position between standard and dragonframes
      motors[0].position = current_steps.x;
      motors[1].position = current_steps.y;
      display_status();
      //DFloop();

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
      draw(47, 1, 6); //lcd.at(1, 6, "Ready?");
      draw(48, 2, 2); //lcd.at(2, 2, "Press C Button");
      break;

    case 4:   //
      lcd.empty();
      lcd.at(1, 3, "Delay:");
      lcd.at(2, 2, "Press C Button");

      start_delay_sec += joy_capture3(1);
      if (start_delay_sec > 60000)      { start_delay_sec = 1800; }
      else if (start_delay_sec > 1800)  { start_delay_sec = 0;    }
      delay(prompt_delay);
      lcd.at(1, 10, start_delay_sec);
      break;
  }//end switch
}


void move_motors_pano_basic()
{
  int index_x;
  int index_y;
  int x_mod_pass_1;
  int even_odd_row;
  int slope_adjustment;

  //Figure out which row we are in

  index_y = camera_fired / total_shots_x;
  x_mod_pass_1 = camera_fired % total_shots_x;
  even_odd_row = index_y % 2;
  slope_adjustment = even_odd_row * ((total_shots_x - 1) - 2 * x_mod_pass_1);
  index_x = x_mod_pass_1 + slope_adjustment;

  fp.x = motor_steps_pt[1][0] - step_per_pano_shot_x * index_x;
  fp.y = motor_steps_pt[1][1] - step_per_pano_shot_y * index_y;

  set_target(fp.x, fp.y, 0.0); //we are in incremental mode to start abs is false

  dda_move(50);
  Move_Engaged = false; // clear move engaged flag

  return;
} // end move motors basic


void move_motors_pano_accel()
{
  int index_x;
  int index_y;
  int x_mod_pass_1;
  int even_odd_row;
  int slope_adjustment;

  //Figure out which row we are in

  index_y = camera_fired / total_shots_x;
  x_mod_pass_1 = camera_fired % total_shots_x;
  even_odd_row = index_y % 2;
  slope_adjustment = even_odd_row * ((total_shots_x - 1) - 2 * x_mod_pass_1);
  index_x = x_mod_pass_1 + slope_adjustment;

  FloatPoint fp;

  fp.x = motor_steps_pt[1][0] - step_per_pano_shot_x * index_x;
  fp.y = motor_steps_pt[1][1] - step_per_pano_shot_y * index_y;

  //set_target(fp.x,fp.y,0.0); //we are in incremental mode to start abs is false
  //dda_move(100);
  setPulsesPerSecond(0, 20000);
  setPulsesPerSecond(1, 20000);
  setupMotorMove(0, long(fp.x));
  //setupMotorMove(0, 50000);
  setupMotorMove(1, long(fp.y));

  updateMotorVelocities();

  //Move_Engaged = false; //clear move engaged flag
}//end move motors accel


void calc_pano_move() //pano - calculate other values
{
  //unsigned long total_shots_x; //calulated value for to divide up scene evenly  ABS(current steps)/max steps per shot+1 = just use integer math
  //unsigned long total_shots_y; //calulated value for to divide up scene evenly
  //unsigned long total_pano_shots; //rows x columns for display
  //unsigned int step_per_pano_shot_x;
  //unsigned int step_per_pano_shot_y;
  total_shots_x = float(abs(current_steps.x) / steps_per_shot_max_x) + 2.0; //Serial.print("total_shots_x = ");Serial.println(total_shots_x);
  total_shots_y = float(abs(current_steps.y) / steps_per_shot_max_y) + 2.0; //Serial.print("total_shots_y = ");Serial.println(total_shots_y);
  if (abs(current_steps.y) < 444.0) { //do a test to see if the tilt angle is very small - indating pano
    total_shots_y = 1;
  }

  total_pano_shots = total_shots_x * total_shots_y;  //Serial.print("total_pano_shots = ");Serial.println(total_pano_shots);
  camera_total_shots = total_pano_shots;//set this to allow us to compare in main loops
  step_per_pano_shot_x = float((current_steps.x) / (total_shots_x - 1.0));  //Serial.print("step_per_pano_shot_x = ");Serial.println(step_per_pano_shot_x);
  step_per_pano_shot_y = float((current_steps.y) / (total_shots_y - 1.0));  //Serial.print("step_per_pano_shot_y = ");Serial.println(step_per_pano_shot_y);
}


void button_actions290()
{ //repeat
  switch (HandleButtons())
  {
    case C_Pressed:
      if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
      if (POWERSAVE_AUX > PWR_PROGRAM_ON)  disable_AUX();
      //Program_Engaged=true;
      camera_fired = 0;
      current_steps.x = motors[0].position; //get our motor position variable synced
      current_steps.y = motors[1].position; //get our motor position variable synced

      lcd.bright(LCD_BRIGHTNESS_DURING_RUN);
      if      (progtype == PANOGIGA)     progstep = 206; //  move to the main program at the interval setting - UD050715
      else if (progtype == PORTRAITPANO) progstep = 306; //  move to the main program at the interval setting UD050715
      redraw = true;
      break;
  }
}
