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

template <typename T>
T uabs(T value) {
    if (T(-1) < T(0)) { // Signed types
        return (value < 0) ? T(-value) : value;
    } else { // Unsigned types
        return value;
    }
}


__attribute__ ((warn_unused_result))
constexpr int16_t umap(const int16_t x, const int16_t in_min, const int16_t in_max, const int16_t out_min, const int16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


template <typename T, typename BitType>
constexpr T ubitSet(T value, BitType bit) {
    return value | (static_cast<T>(1) << static_cast<T>(bit));
}



template <typename T, typename BitType>
constexpr T ubitClear(T value, BitType bit) {
    static_assert(sizeof(T) <= sizeof(uint32_t) && sizeof(BitType) <= sizeof(uint32_t),
                  "Parameters must be integral types or compatible with bit operations.");
    // Perform the shift and mask within the type of `T`
    return value & ~(static_cast<T>(1) << static_cast<BitType>(bit));
}


__attribute__ ((warn_unused_result))
int32_t updateWithWraparound(int32_t current, int32_t min, int32_t max, int32_t increment) {
  int32_t range = max - min + 1; // Ensure inclusive range
  current += increment;

  // Handle wraparound
  if (current > max) {
    joy_y_lock_count = 0;
    current = min + (current - max - 1) % range;
  }
  else if (current < min) {
    joy_y_lock_count = 0;
    current = max - (min - current - 1) % range;
  }
  return current;
}


__attribute__ ((warn_unused_result))
uint32_t calculateMoveTimeMs(const int32_t distance, const uint16_t maxVelocity) {
  // Calculate max acceleration
  uint32_t dist = abs(distance);
  uint32_t d_acc = maxVelocity;

  if (dist > (2 * d_acc)) { // Trapezoidal profile
    uint32_t d_const = dist - (2 * d_acc); // Constant velocity distance
    uint32_t t_const = (d_const * 1000) / maxVelocity;        // Time at max velocity (ms)
    return 4000 + t_const;            // Total time (ms)
  }
  else { // Triangular profile
    return static_cast<uint32_t>(sqrt(static_cast<float>(dist) / (static_cast<float>(d_acc) / 2.0f)) * 1000.0f * 2.0f); // Adjust acceleration time (ms)
  }
}


void Choose_Program() {
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    Display_Main_Menu();
    draw(F_STR("UpDown  C-Select"), 2, 1);
    UpdatePowersave(Powersave::Always); // Only keep powered if Always on
    delay(update_delay);
  }

  NunChuckjoybuttons();

  int16_t yUpDown = joy_capture_y1();
  if (yUpDown != 0) {
    progtype = static_cast<Menu_Options>(updateWithWraparound(static_cast<int16_t>(progtype), 0, static_cast<int16_t>(Menu_Options::Count) - 1, yUpDown));
    Display_Main_Menu();
    delay(update_delay);
  }
  button_actions_choose_program();
}

void Display_Main_Menu() {
  switch (progtype) {
    case Menu_Options::Count:
    case Menu_Options::REG2POINTMOVE:    { draw(F_STR("New 2 Point Move"), 1, 1); break; }
    case Menu_Options::REV2POINTMOVE:    { draw(F_STR("Rev 2 Point Move"), 1, 1); break; }
    case Menu_Options::REG3POINTMOVE:    { draw(F_STR("New 3 Point Move"), 1, 1); break; }
    case Menu_Options::REV3POINTMOVE:    { draw(F_STR("Rev 3 Point Move"), 1, 1); break; }
    case Menu_Options::DFSLAVE:          { draw(F_STR(" DF Slave Mode  "), 1, 1); break; }
    case Menu_Options::SETUPMENU:        { draw(F_STR("   Setup Menu   "), 1, 1); break; }
    case Menu_Options::PANOGIGA:         { draw(F_STR("    Panorama    "), 1, 1); break; }
    case Menu_Options::PORTRAITPANO:     { draw(F_STR(" Portrait Pano  "), 1, 1); break; }
    case Menu_Options::AUXDISTANCE:      { draw(F_STR("  Aux Distance  "), 1, 1); break;}
  }
}


void button_actions_choose_program() {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    REVERSE_PROG_ORDER = false;
    switch (progtype) {
      case Menu_Options::Count: 
      case Menu_Options::REV2POINTMOVE: { REVERSE_PROG_ORDER = true; }
      // fall through
      case Menu_Options::REG2POINTMOVE: { progstep_goto(Menu::PT2_StartPoint); break; }
      case Menu_Options::REV3POINTMOVE: { REVERSE_PROG_ORDER = true; }
      // fall through
      case Menu_Options::REG3POINTMOVE: { progstep_goto(Menu::PT3_Point0); break; }
      case Menu_Options::SETUPMENU:     { progstep_goto(Menu::SET_AuxOn); break; }
      case Menu_Options::PANOGIGA:      { progstep_goto(Menu::GIGA_AOV); break; }
      case Menu_Options::PORTRAITPANO:  { progstep_goto(Menu::PANO_AOV); break; }
      case Menu_Options::AUXDISTANCE:   { progstep_goto(Menu::AUX_StartPoint); break; }
      case Menu_Options::DFSLAVE:   //DFMode
      {
        if (PINOUT_VERSION == 4) draw(F_STR("eMotimo TB3Black"), 1, 1);
        if (PINOUT_VERSION == 3) draw(F_STR("eMotimoTB3Orange"), 1, 1);
        draw(F_STR("Dragonframe 1.26"), 2, 1);
        DFSetup();
        DFloop();
        break;
      }
    }
  }
}

//Move to Start Point
void Move_to_Startpoint() {
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    if (!REVERSE_PROG_ORDER)  //normal programing, start first
    {
      draw(F_STR("Move to Start Pt"), 1, 1);
      draw(F_STR("C-Next"), 2, 6);
    }
    if (REVERSE_PROG_ORDER) {
      draw(F_STR("Move to End Pt."), 1, 1);
      draw(F_STR("C-Next Z-Go Back"), 2, 1);
    }
    UpdatePowersave(Powersave::Shoot_Minimum); // Need to Move
    startISR1();  //setup the ISR
  }

  NunChuckjoybuttons();
  button_actions_move_start();  //check buttons

  //Velocity Engine update
  if (!nextMoveLoaded) {
    updateMotorVelocities2();
  }
}  //end move to start point

void button_actions_move_start() {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    WaitForMotorStop();
    UpdatePowersave(Powersave::Programs);

    set_home();  //sets current steps to 0
    lcd.empty();
    if (!REVERSE_PROG_ORDER) draw(F_STR("Start Pt. Set"), 1, 3);
    if (REVERSE_PROG_ORDER) draw(F_STR("End Point Set"), 1, 3);
    delay(screen_delay);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
    WaitForMotorStop();
    UpdatePowersave(Powersave::Programs);
    delay(screen_delay);
  }
}

void Move_to_Endpoint() {
  if (first_ui) {
    first_ui = false;
    lcd.empty();

    if (!REVERSE_PROG_ORDER)  //mormal programming, end point last
    {
      draw(F_STR("Move to End Pt."), 1, 1);
      draw(F_STR("C-Next Z-Go Back"), 2, 1);
    }

    if (REVERSE_PROG_ORDER)  //reverse programing, start last
    {
      draw(F_STR("Move to Start Pt"), 1, 1);
      draw(F_STR("C-Next"), 2, 6);
    }
    UpdatePowersave(Powersave::Shoot_Minimum); // Need to Move
    startISR1();  //setup the ISR
  }

  NunChuckjoybuttons();
  button_actions_move_end();  //check buttons

  //Velocity Engine update
  if (!nextMoveLoaded) {
    updateMotorVelocities2();
  }
}

void button_actions_move_end() {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    WaitForMotorStop();
    UpdatePowersave(Powersave::Programs);

    for (uint8_t i = 0; i < MOTORS; i++) {
      config.motor_steps_pt[1][i] = motors[i].position; 
    }

    lcd.empty();
    if (REVERSE_PROG_ORDER) { draw(F_STR("Start Pt. Set"), 1, 3); }
    else                    { draw(F_STR("End Point Set"), 1, 3); }
    delay(screen_delay);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
    WaitForMotorStop();
    UpdatePowersave(Powersave::Programs);
  }
}

void Move_to_Point_X(const uint8_t Point) {
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    if (progstep == Menu::PANO_Point0) {  // Programming center for Menu_Options::PORTRAITPANO
      draw(F_STR("Move to Center"), 1, 1);
    }
    else {
      draw(F_STR("Move to Point"), 1, 1);
      if (REVERSE_PROG_ORDER) { lcd.at(1, 15, 3 - Point); }
      else                    { lcd.at(1, 15, Point + 1); }  
    }
    draw(F_STR("C-Next Z-Go Back"), 2, 1);
    UpdatePowersave(Powersave::Shoot_Minimum); // Need to Move
    startISR1();  //setup the ISR
  }

  NunChuckjoybuttons();
  if ((millis() - display_last_tm) > screen_delay) {
    button_actions_move_x(Point);  //check buttons
  }
  if ((millis() - display_last_tm) > screen_delay) {
    display_last_tm = millis();
    DisplayDebug();
  }

  //Velocity Engine update
  if (!nextMoveLoaded) {
    updateMotorVelocities2();
  }
}


void button_actions_move_x(const uint8_t Point) {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    WaitForMotorStop();
    UpdatePowersave(Powersave::Programs);

    if (Point == 0)  set_home();  //reset for home position

    config.motor_steps_pt[Point][0] = motors[0].position; 
    config.motor_steps_pt[Point][1] = motors[1].position;
    config.motor_steps_pt[Point][2] = motors[2].position;

    lcd.empty();
    if (progstep == Menu::PANO_Point0)  // Menu_Options::PORTRAITPANO Method
    {
      draw(F_STR("Center Set"), 1, 4);
    }
    else {
      draw(F_STR("Point X Set"), 1, 3);
      if (REVERSE_PROG_ORDER) { lcd.at(1, 9, 3 - Point); } // reverse programming
      else                    { lcd.at(1, 9, Point + 1); } // Normal     
    }
    delay(screen_delay);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
    WaitForMotorStop();
    UpdatePowersave(Powersave::Programs);
  }
}

//Set Camera Interval
void Set_Shot_Interval() {
  UpdatePowersave(Powersave::Shoot_Minimum); // Need to Move
  DFSetup();
  startISR1();  //setup the ISR
  while (true) {
    int32_t Target[MOTORS] = { config.motor_steps_pt[0][0], config.motor_steps_pt[0][1], config.motor_steps_pt[0][2] };
    setupMotorMove(Target);
    while (motorMoving) {
      if (!nextMoveLoaded) {
        updateMotorVelocities();  //finished up the interrupt routine
      }
    }
    int32_t Target2[MOTORS] = { config.motor_steps_pt[1][0], config.motor_steps_pt[1][1], config.motor_steps_pt[1][2] };
    setupMotorMove(Target2);
    while (motorMoving) {
      if (!nextMoveLoaded) {
        updateMotorVelocities();  //finished up the interrupt routine
      }
    }
  }


  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("Shot Interval"), 1, 2);
    delay(update_delay);
  }

  if (second_ui && (millis() - display_last_tm) > screen_delay) {
    second_ui = false;
    lcd.empty();
    draw(F_STR("Intval:"), 1, 1);
    DisplayInterval(config.shot_interval_time, 1, 9);  // 0.1s
    draw(F_STR("C-Next Z-Go Back"), 2, 1);
  }

  NunChuckjoybuttons();

  int16_t delta = joy_capture_y3();
  if (delta != 0) {
    config.shot_interval_time = static_cast<uint16_t>(updateWithWraparound(static_cast<int16_t>(config.shot_interval_time), 0, 65535, delta));
    DisplayInterval(config.shot_interval_time, 1, 9);  // 0.1s
    delay(scroll_delay);
  }
  button_actions_shot_interval_time(config.shot_interval_time);  //read buttons, look for c button press to set interval
}


void DisplayInterval(uint16_t shot_interval_time_val, const uint8_t row, const uint8_t col) {
  if (shot_interval_time_val == VIDEO_EXPVAL) {
    draw(F_STR("Vid.Trig"), row, col);
  }
  else if (shot_interval_time_val == EXTTRIG_EXPVAL) {
    draw(F_STR("Ext.Trig"), row, col);
  } 
  else {
    draw(F_STR(" "), row, col); // the draw command covers 6 chars
    DrawPaddedDecimalValue<6>(shot_interval_time_val - EXTTRIG_EXPVAL, row, col + 1, 10); // Dynamically handle scaled value
    draw(F_STR("s"), row, col + 7); // the draw command covers 6 chars
  }
}


void button_actions_shot_interval_time(const uint16_t shot_interval_time_val) {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    lcd.empty();
    draw(F_STR("Interval Set"), 1, 3);  // try this to push correct character
    delay(screen_delay);
    
    if (shot_interval_time_val == VIDEO_EXPVAL)  //means video, set very small exposure time
    {
      IntervalTime(0);  //we overwrite this later based on move length currently 100
      ShutterTime(0);
    }
    else if (shot_interval_time_val == EXTTRIG_EXPVAL)  //means ext trigger
    {
      IntervalTime(6000);        //Hardcode this to 6.0 seconds are 10 shots per minute to allow for max shot selection
      ShutterTime(0);
    }
    else {
      IntervalTime(static_cast<uint32_t>(shot_interval_time_val - EXTTRIG_EXPVAL) * 100);
      ShutterTime(static_cast<uint32_t>(shot_interval_time_val - EXTTRIG_EXPVAL) * 100);
    }
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
  }
}


void Set_Duration()  //This is really setting frames
{
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("Set Move"), 1, 5);
    draw(F_STR("Duration"), 2, 5);
    if (config.camera_total_shots < 2) { config.camera_total_shots = 200; }
    delay(update_delay);
  }

  if (second_ui && (millis() - display_last_tm) > screen_delay) {
    second_ui = false;
    draw(F_STR("H:MM:SS"), 1, 10);
    if (config.shot_interval_time >= EXTTRIG_EXPVAL) {
      draw(F_STR("Frames"), 2, 11);  //SMS
      Display_Duration();
    } 
    else {
      draw(F_STR("C-Next Z-Go Back"), 2, 1);  //Video
      display_time(config.overaldur, 1, 1);
    }
  }

  NunChuckjoybuttons();

  if (config.shot_interval_time == VIDEO_EXPVAL) {  //video
    int16_t delta = joy_capture_y3();
    if (delta != 0) {
      config.overaldur = static_cast<uint16_t>(updateWithWraparound(static_cast<int16_t>(config.overaldur), 1, 10000, delta));
      display_time(config.overaldur, 1, 1);
      delay(scroll_delay);
    }
  }
  else {  //sms
    int16_t delta = joy_capture_y3();
    if (delta != 0) {
      config.camera_total_shots = static_cast<uint16_t>(updateWithWraparound(static_cast<int16_t>(config.camera_total_shots), 2, 10000, delta));
      Display_Duration();
      delay(scroll_delay);
    }
  }                                                          //end sms
  button_actions_overaldur();  //read buttons, look for c button press to set overall time
}

void Display_Duration() {

  uint32_t dominant_motor = max(uabs(config.motor_steps_pt[1][0]), uabs(config.motor_steps_pt[1][1]));
  config.move_time = calculateMoveTimeMs(dominant_motor / config.camera_total_shots, config.MAX_JOG_STEPS_PER_SEC[0]);

  uint32_t intervalms = static_cast<uint32_t>(config.shot_interval_time - EXTTRIG_EXPVAL) * static_cast<uint32_t>(config.camera_total_shots) * 100;
  uint32_t movems = config.move_time * static_cast<uint32_t>(config.camera_total_shots - 1);

  display_time((intervalms + movems) / 1000, 1, 1);

  if (config.shot_interval_time >= EXTTRIG_EXPVAL) {
      DrawPaddedValue<5>(config.camera_total_shots, 2, 4);
  }
}

void button_actions_overaldur() {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    lcd.empty();
    draw(F_STR("Duration Set"), 1, 3);
    delay(screen_delay);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
  }
}

void Set_Static_Time() {
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("Set Static Time"), 1, 1);
    delay(update_delay);
  }

  if (second_ui && (millis() - display_last_tm) > screen_delay) {
    second_ui = false;
    lcd.empty();
    draw(F_STR("Stat_T:"), 1, 1);
    DisplayStatic(ShutterTime(), 1, 8);  // 0.1s
    draw(F_STR("C-Next Z-Go Back"), 2, 1);
  }

  NunChuckjoybuttons();

  int16_t delta = joy_capture_y3();
  if (delta != 0) {
    uint32_t interval_time = 0;
    if (config.shot_interval_time >= EXTTRIG_EXPVAL) { interval_time = (config.shot_interval_time - EXTTRIG_EXPVAL) * 100; }
    ShutterTime(static_cast<uint32_t>(updateWithWraparound(static_cast<int16_t>(ShutterTime()), 100, interval_time, delta * 100) / 100 * 100));
    DisplayStatic(ShutterTime(), 1, 8);  // 0.1s
    delay(scroll_delay);
  }
  button_actions_stat_time();  //read buttons, look for c button press to set interval
}


void DisplayStatic(uint16_t static_val, const uint8_t row, const uint8_t col) {
  DrawPaddedDecimalValue<6>(static_val / 100, row, col + 1, 10); // Dynamically handle scaled value
 draw(F_STR("s"), row, col + 7); // the draw command covers 6 chars
}


void button_actions_stat_time() {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    lcd.empty();
    draw(F_STR("Static Time Set"), 1, 1);  // try this to push correct character
    delay(screen_delay);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
  }
}

void Set_Ramp() {
  if (first_ui) {
    lcd.empty();
    draw(F_STR("Set Ramp"), 1, 5);
    first_ui = false;
    delay(update_delay);
  }

  if (second_ui && (millis() - display_last_tm) > screen_delay) {
    second_ui = false;
    lcd.empty();
    draw(F_STR("Ramp:     Frames"), 1, 1);
    if (config.shot_interval_time == VIDEO_EXPVAL) {
      lcd.at(1, 10, "Percent");
    }
    draw(F_STR("C-Next Z-Go Back"), 2, 1);
    DisplayRampval();
  }

  NunChuckjoybuttons();

  int16_t delta = joy_capture_y3();
  if (delta != 0) {
    config.rampval = static_cast<uint8_t>(updateWithWraparound(static_cast<int16_t>(config.rampval), 0, static_cast<int16_t>(config.camera_total_shots) / 2, delta));
    DisplayRampval();
    delay(scroll_delay);
  }
  button_actions_rampval();  //read buttons, look for c button press to set interval
}

void DisplayRampval() {
    DrawPaddedValue<3>(config.rampval, 1, 7);
}


void button_actions_rampval() {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    lcd.empty();
    draw(F_STR("Ramp Set"), 1, 5);
    delay(screen_delay);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
  }
}

void Set_LeadIn_LeadOut() {
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("Set Static Lead"), 1, 1);
    draw(F_STR("In/Out Frames"), 2, 2);
    delay(update_delay);
  }

  if (second_ui && (millis() - display_last_tm) > screen_delay) {
    second_ui = false;
    lcd.empty();
    draw(F_STR("In"), 1, 6);
    draw(F_STR("Out"), 1, 14);
    draw(F_STR("C-Next Z-Go Back"), 2, 1);
    DisplayLeadIn_LeadOut(0);
  }

  NunChuckjoybuttons();

  int16_t x_val = joy_capture_x1();
  if (x_val != 0) {
    cursorpos = static_cast<uint8_t>(updateWithWraparound(cursorpos, 1, 2, x_val));
    delay(screen_delay);
  }
  int16_t delta = joy_capture_y3();
  if (delta != 0 || x_val != 0) {
    DisplayLeadIn_LeadOut(delta);
  }
  button_actions_lead_in_out();  //read buttons, look for c button press to ramp
}

void DisplayLeadIn_LeadOut(int16_t delta_val) {
  if (cursorpos == 1) {
    lcd.at(1, 1, '>');
    lcd.at(1, 8, ' ');
    config.lead_in = static_cast<uint16_t>(updateWithWraparound(static_cast<int16_t>(config.lead_in), 0, config.camera_total_shots - config.lead_out, delta_val));
    DrawPaddedValue<4>(config.lead_in, 1, 2);
  }
  else {
    lcd.at(1, 1, ' ');
    lcd.at(1, 8, '>');
    config.lead_out = static_cast<uint16_t>(updateWithWraparound(static_cast<int16_t>(config.lead_out), 0, config.camera_total_shots - config.lead_in, delta_val));
    DrawPaddedValue<4>(config.lead_out, 1, 10);
  }
  delay(scroll_delay);
}

void button_actions_lead_in_out() {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    lcd.empty();
    draw(F_STR("Lead Frames Set"), 1, 1);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
  }
}

void Set_Shot_Repeat() { 
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("Select Shot Type"), 1, 1);
    DisplaySequenceRepeatType(sequence_repeat_type);
    delay(update_delay);
  }

  NunChuckjoybuttons();

  int16_t yUpDown = joy_capture_y1();
  if (yUpDown != 0) {
    sequence_repeat_type = static_cast<Repeat>(updateWithWraparound(static_cast<int16_t>(sequence_repeat_type), 0, static_cast<int16_t>(Repeat::Count) - 1, yUpDown));
    DisplaySequenceRepeatType(sequence_repeat_type);
    delay(update_delay);
  }

  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
  }
}

void DisplaySequenceRepeatType(const enum Repeat value) {
  lcd.clearRegion(2,9,16);
  switch(value) {
    case Repeat::Count: // Error Condition
    case Repeat::Run_Once:                { draw(F_STR("Run Once"), 2, 1); break; }
    case Repeat::Continuous_Alternate:    { draw(F_STR("Continuous_Loop Loop"), 2, 1); break; }
    case Repeat::Continuous_Loop:         { draw(F_STR("Repeat Forward"), 2, 1); break; }
  }
}

void Calculate_Shot()
{
  uint16_t camera_moving_shots = config.camera_total_shots - config.lead_in - config.lead_out;

  if (config.shot_interval_time == VIDEO_EXPVAL) {
    //really only need this for 3 point moves now - this could screw up 2 points, be careful
    camera_moving_shots = config.video_segments;                                                //hardcode this and back into proper interval - this is XXX segments per sequence
    IntervalTime(config.overaldur * 1000L / camera_moving_shots);                 //This gives us ms for our delays
  }

  //fix issues with ramp that is too big
  config.rampval = min(config.rampval, camera_moving_shots / 2);  // makes sure the ramp up + ramp down doesnt exceed out move

  //Set keyframe points for program to help with runtime calcs
  config.keyframe[0][0] = 0;                                                              //start frame
  config.keyframe[0][1] = config.lead_in;                                                 //beginning of ramp
  config.keyframe[0][2] = config.lead_in + config.rampval;                                //end of ramp, beginning of linear
  config.keyframe[0][3] = config.lead_in + camera_moving_shots - config.rampval;   //end or linear, beginning of rampdown
  config.keyframe[0][4] = config.lead_in + camera_moving_shots;                    //end of rampdown, beginning of leadout
  config.keyframe[0][5] = config.lead_in + camera_moving_shots + config.lead_out;  //end of leadout, end of program
}

void Review_Confirm() {
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("Review and"), 1, 4);
    draw(F_STR("Confirm Setting"), 2, 2);
    reviewprog = 2;
    start_delay_sec = 0;
    delay(update_delay);
  }

  if ((millis() - display_last_tm) > screen_delay) {
    DisplayDebug();
    //reviewprog = static_cast<uint8_t>(updateWithWraparound(static_cast<int16_t>(reviewprog), 1, 4, 1));
    //DisplayReviewProg();
  }

  NunChuckjoybuttons();

  if (uabs(joy_y_axis) > 20) {  //do read time updates to delay program
    reviewprog = 4;
    DisplayReviewProg();
  }
  button_actions_review();
  delay(scroll_delay);
}

void DisplayReviewProg() {
  lcd.empty();
  switch (reviewprog) {
    case 1:  //
      draw(F_STR("Pan Steps:"), 1, 1);
      draw(F_STR("Tilt Steps:"), 2, 1);
      //lcd.at(1,12,motor_steps[0]);
      lcd.at(1, 12, config.motor_steps_pt[0][0]);
      lcd.at(2, 12, config.motor_steps_pt[0][1]);
      break;

    case 2:  //
      draw(F_STR("Cam Shots:"), 1, 1);
      draw(F_STR("Time:"), 2, 1);
      lcd.at(1, 12, config.camera_total_shots);
      display_time(calc_time_remain(), 2, 6);
      break;

    case 3:  //
      draw(F_STR("Ready?"), 1, 6);
      draw(F_STR("Press C Button"), 2, 2);
      break;
    case 4:  //
      draw(F_STR("Set Start Delay"), 1, 1);
      draw(F_STR("H:MM:SS"), 2, 10);

      int16_t delta = joy_capture_y3();
      if (delta != 0) {
        start_delay_sec = static_cast<uint16_t>(updateWithWraparound(static_cast<int16_t>(start_delay_sec), 0, 32767, delta));
        DisplayAUX_Dist();
        delay(scroll_delay);
      }
      display_time(config.overaldur, 2, 1);
      break;

  }  //end switch
}


void DisplayDebug() {
  static uint8_t review = 0;
  lcd.empty();
  review = static_cast<uint8_t>(updateWithWraparound(static_cast<int16_t>(review), 1, 4, 1));
  switch (review) {
    case 1:
      draw(F_STR("PT0:0:"), 1, 1);
      draw(F_STR("PT0:1:"), 2, 1);
      lcd.at(1, 10, config.motor_steps_pt[0][0]);
      lcd.at(2, 10, config.motor_steps_pt[0][1]);
      break;

    case 2:
      draw(F_STR("PT1:0"), 1, 1);
      draw(F_STR("PT1:1:"), 2, 1);
      lcd.at(1, 10, config.motor_steps_pt[1][0]);
      lcd.at(2, 10, config.motor_steps_pt[1][1]);
      break;

    case 3:
      draw(F_STR("MP:0"), 1, 1);
      draw(F_STR("MP:1:"), 2, 1);
      lcd.at(1, 10, motors[0].position);
      lcd.at(2, 10, motors[1].position);
      break;

    case 4:
      draw(F_STR("DST:0"), 1, 1);
      draw(F_STR("DST:1:"), 2, 1);
      lcd.at(1, 10, motors[0].destination);
      lcd.at(2, 10, motors[1].destination);
      break;
  }  //end switch
}


void button_actions_review() {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();

    lcd.empty();
    draw(F_STR("H:MM:SS"), 2, 10);
    draw(F_STR("Delaying Start"), 1, 2);

    MOVE_REVERSED_FOR_RUN = false;                            //Reset This
    start_delay_tm = ((millis() / 1000L) + start_delay_sec);  //system seconds in the future - use this as big value to compare against
    
    CZ_Button_Read_Count = 0;  //reset this to zero to start
    Calculate_Shot();
    config.camera_fired = 0;
    compute_s_curve();

    // while (start_delay_tm > millis() / 1000L) {
    //   //enter delay routine
    //   if ((millis() - display_last_tm) > 1000) {
    //     display_time(calc_time_remain_start_delay(), 2, 1);
    //   }
    //   NunChuckjoybuttons();
        
    //   if (CZ_Button_Read_Count > CZ_Count_Threshold) {
    //     start_delay_tm = ((millis() / 1000L) + 5);  //start right away by lowering this to 5 seconds.
    //     CZ_Button_Read_Count = 0;                   //reset this to zero to start
    //   }
    // }

    UpdatePowersave(Powersave::Shoot_Minimum); // Need to Move

    if (config.shot_interval_time == VIDEO_EXPVAL) {
      lcd.empty();            //clear for non video
      progstep_goto(Menu::SMS_Loop);  //  move to the main programcamera_real_fire
      lcd.bright(config.LCD_BRIGHTNESS_DURING_RUN);
      PanoState = ShootMoveState::Waiting_Camera;
      config.camera_fired = 0;
      interval_tm_last = micros();
    }
    else if (config.shot_interval_time == EXTTRIG_EXPVAL) {  //manual trigger/ext trigger
      lcd.empty();                                 //clear for non video
      progstep_goto(Menu::PT2_TriggerLoop);            //  move to the external interrupt loop
      ext_shutter_count = 0;                       //set this to avoid any prefire shots or cable insertions.
      draw(F_STR("Waiting for Trig"), 1, 1);
    }
    else if (config.shot_interval_time > EXTTRIG_EXPVAL) {  //SMS Mode
      lcd.empty();        //SMS_Loop for non video
      progstep_goto(Menu::SMS_Loop);  //  move to the main programcamera_real_fire

      lcd.bright(config.LCD_BRIGHTNESS_DURING_RUN);

      PanoState = ShootMoveState::Waiting_Camera;
      config.camera_fired = 0;

      if (P2PAccelMove) {
        startISR1();  //setup the ISR
        display_status();
      }
    }
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
  }
}


void progstep_forward() {
  progstep_forward_dir = true;
  progstep_goto(static_cast<Menu>(static_cast<uint16_t>(progstep) + 1));
}

void progstep_backward() {
  progstep_forward_dir = false;
  if (static_cast<uint16_t>(progstep) > 0) {
    progstep_goto(static_cast<Menu>(static_cast<uint16_t>(progstep) -1));
  }
}

void progstep_goto(enum Menu prgstp) {
  first_ui = true;
  second_ui = true;
  C_Button_Read_Count = 0;
  Z_Button_Read_Count = 0;
  CZ_Button_Read_Count = 0;
  progstep = prgstp;
}

void button_actions_end_of_program() {  //repeat - need to turn off the REVERSE_PROG_ORDER flag
  if (C_Button_Read_Count > CZ_Count_Threshold) {       // looking for c button for Repeat
    // Normal
    UpdatePowersave(Powersave::Always); // Only keep powered if Always on
    config.camera_fired = 0;
    lcd.bright(config.LCD_BRIGHTNESS_DURING_MENU);
    if (progtype == Menu_Options::REG2POINTMOVE || progtype == Menu_Options::REV2POINTMOVE) {
      config.camera_fired = 0;
      goto_position();
      progstep_goto(Menu::PT2_ShotRepeat);
    } else if (progtype == Menu_Options::REG3POINTMOVE || progtype == Menu_Options::REV3POINTMOVE) {
      config.camera_fired = 0;
      goto_position();
      progstep_goto(Menu::PT3_Reveiw);
    } else if (progtype == Menu_Options::AUXDISTANCE) {
      config.camera_fired = 0;
      goto_position();
      progstep_goto(Menu::PT2_ShotRepeat);
    }
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    UpdatePowersave(Powersave::Always); // Only keep powered if Always on
    config.camera_fired = 0;
    lcd.bright(config.LCD_BRIGHTNESS_DURING_MENU);
    if (progtype == Menu_Options::REG2POINTMOVE || progtype == Menu_Options::REV2POINTMOVE) {
      config.camera_fired = 0;
      goto_position();
      progstep_goto(Menu::PT2_ShotRepeat);
    } else if (progtype == Menu_Options::REG3POINTMOVE || progtype == Menu_Options::REV3POINTMOVE) {
      config.camera_fired = 0;
      goto_position();
      progstep_goto(Menu::PT3_Reveiw);
    } else if (progtype == Menu_Options::AUXDISTANCE) {
      config.camera_fired = 0;
      goto_position();
      progstep_goto(Menu::PT2_ShotRepeat);
    }
  }
}

void Auto_Repeat_Video() {  //Auto Reverse
  sequence_repeat_count++;
  config.camera_fired = 0;
  if (progtype == Menu_Options::REG2POINTMOVE || progtype == Menu_Options::REV2POINTMOVE) {
    config.camera_fired = 0;
    goto_position();
  }


  lcd.empty();
  MOVE_REVERSED_FOR_RUN = false;  //Reset This
  //start_delay_tm=((millis()/1000L)+start_delay_sec); //system seconds in the future - use this as big value to compare against
  //draw(F_STR("H:MM:SS"),2,10);
  //draw(F_STR("Delaying Start"), 1,2);
  CZ_Button_Read_Count = 0;  //reset this to zero to start

  /*
                   while (start_delay_tm>millis()/1000L) {
                     //enter delay routine
                     calc_time_remain_start_delay ();
                     if ((millis()-display_last_tm) > 1000) display_time(2,1);
                     NunChuckjoybuttons();
                     if (CZ_Button_Read_Count>CZ_Count_Threshold) {
                           start_delay_tm=((millis()/1000L)+5); //start right away by lowering this to 5 seconds.
                           CZ_Button_Read_Count=0; //reset this to zero to start
                     }

                   }
           */

  UpdatePowersave(Powersave::Shoot_Minimum); // Need to Move

  draw(F_STR("Program Running"), 1, 1);

  interval_tm = 0;  //set this to 0 to immediately trigger the first shot

  if (config.shot_interval_time > EXTTRIG_EXPVAL) {  //SMS Mode
    lcd.empty();            //clear for non video
    progstep_goto(Menu::PT2_ShootLoop);         //  move to the main programcamera_real_fire
  } 
  else if (config.shot_interval_time == VIDEO_EXPVAL) {
    //lcd.empty(); //leave "program running up for video
    progstep_goto(Menu::PT2_VideoLoop);
  }
  /*
           else if (config.shot_interval_time==EXTTRIG_EXPVAL)  { //manual trigger/ext trigge
                      lcd.empty(); //clear for non video
                      progstep=52; //  move to the external interrup loop
                      ext_shutter_count=0; //set this to avoid any prefire shots or cable insertions.
                      draw(F_STR("Waiting for Trig"), 1,1);
                   }
                   */
  first_ui = true;
  lcd.bright(config.LCD_BRIGHTNESS_DURING_RUN);

}  //end of 91


void display_status() {
  //1234567890123456
  //1234567890123456
  //XXXX/XXXX LeadIn      LeadOT Rampup RampDn, Pause
  //HH:MM:SS  XX.XXV
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    lcd.at(2, 16, 'v');
  }

  if (progtype == Menu_Options::REG2POINTMOVE || progtype == Menu_Options::REV2POINTMOVE || progtype == Menu_Options::AUXDISTANCE) {
    DrawPaddedValue<4>(config.camera_fired + 1, 1, 1);
    lcd.at(1, 5, '/');
    DrawPaddedValue<4>(config.camera_total_shots, 1, 6);
    switch (program_progress_2PT) {
      case MoveProgress2PT::Lead_In:   { draw(F_STR("LeadIn"), 1, 11); break; }
      case MoveProgress2PT::Ramp_Up:   { draw(F_STR("RampUp"), 1, 11); break; }
      case MoveProgress2PT::Linear:    { draw(F_STR("Linear"), 1, 11); break; }
      case MoveProgress2PT::Ramp_Down: { draw(F_STR("RampDn"), 1, 11); break; }
      case MoveProgress2PT::Lead_Out:  { draw(F_STR("LeadOT"), 1, 11); break; }
      case MoveProgress2PT::Finished:  { draw(F_STR("Finish"), 1, 11); break; }
    }
  }

  else if (progtype == Menu_Options::REG3POINTMOVE || progtype == Menu_Options::REV3POINTMOVE) {
    DrawPaddedValue<4>(config.camera_fired + 1, 1, 1);
    lcd.at(1, 5, '/');
    DrawPaddedValue<4>(config.camera_total_shots, 1, 6);
    switch (program_progress_3PT) {
      case MoveProgress3PT::Lead_In:  { draw(F_STR("LeadIn"), 1, 11); break; }
      case MoveProgress3PT::Leg:      { draw(F_STR("Leg X "), 1, 11); break; }
      case MoveProgress3PT::Lead_Out: { draw(F_STR("LeadOT"), 1, 11); break; }
      case MoveProgress3PT::Finished: { draw(F_STR("Finish"), 1, 11); break; }
    }
  }

  else if (progtype == Menu_Options::PANOGIGA || progtype == Menu_Options::PORTRAITPANO) {
    draw(F_STR("Pano "), 1, 13);
    DrawPaddedValue<5>(config.camera_fired + 1, 1, 1);
    lcd.at(1, 6, '/');
    DrawPaddedValue<5>(config.camera_total_shots, 1, 7);
  }

  //Update Run Clock
  if (timer2.count > 10 || IsCameraIdle()) {
    display_time(calc_time_remain(), 2, 1);
  }

  // Perform multiple reads of the battery and sum them
  uint32_t batteryread = 0;
  for (uint8_t i = 0; i < 3; i++) {
      batteryread += analogRead(0); // Sum without dividing
  }

  // Adjust the scale factor to incorporate the number of samples
  uint16_t scaledBattery = ((batteryread) * 100) / (3 * 51); //  51 point per volt, and 3 samples

  // Use the DrawPaddedDecimalValue function to display the result
  DrawPaddedDecimalValue<5>(scaledBattery, 2, 10, 100); // 6 characters wide, scale of 100 for two decimals

  if (POWERDOWN_LV) {
    static uint8_t batt_low_cnt = 0;
    if (scaledBattery < 900) { // 9 V
      draw(F_STR("Low Power"), 2, 1);
      batt_low_cnt++;

      if (batt_low_cnt > 20) {
        //Stop the program and go to low power state
        disable_PT();
        disable_AUX();
        lcd.empty();
        draw(F_STR("Battery too low"), 1, 1);
        draw(F_STR("to continue"), 2, 3);
      }

      first_ui = true;
    } else batt_low_cnt = 0;
  }  //end of powerdown if
}  //end of display


__attribute__ ((warn_unused_result))
uint32_t calc_time_remain() {
  uint32_t shots = (config.camera_total_shots - config.camera_fired); // How many shots are left
  uint32_t interval_ms = 0;
  if (config.shot_interval_time >= EXTTRIG_EXPVAL) { interval_ms = static_cast<uint32_t>(config.shot_interval_time - EXTTRIG_EXPVAL) * 100; }
  if (interval_ms < (ShutterTime() + PrefireTime() + PostfireTime())) {
    interval_ms = (ShutterTime() + PrefireTime() + PostfireTime());
  }
  uint32_t time_ms = (shots * interval_ms) + (shots * config.move_time) - timer2.count; 
  return time_ms / 1000;  
}


__attribute__ ((warn_unused_result))
uint32_t calc_time_remain_start_delay() {
  uint32_t current_sec = millis() / 1000;  // Convert millis to seconds
  return (start_delay_tm > current_sec) ? (start_delay_tm - current_sec) : 0;
}


void display_time(uint32_t total_seconds, const uint8_t row, const uint8_t col) {
  // Calculate hours, minutes, and seconds
  uint32_t hours = total_seconds / 3600;
  total_seconds %= 3600;
  uint32_t minutes = total_seconds / 60;
  uint32_t seconds = total_seconds % 60;

  // Helper lambda to print a value with a leading zero if needed
  auto print_with_leading_zero = [&](uint8_t offset, uint32_t value) {
    uint8_t target_col = static_cast<uint8_t>(col + offset);  // Safely cast the result
    if (value < 10) {
      lcd.at(row, target_col, '0');          // Print leading zero
      lcd.at(row, target_col + 1, value);   // Print the value
    } else {
      lcd.at(row, target_col, value);       // Print the value directly
    }
  };

  // Print colons at fixed positions
  lcd.at(row, static_cast<uint8_t>(col + 2), ':');
  lcd.at(row, static_cast<uint8_t>(col + 5), ':');

  // Print hours, minutes, and seconds
  print_with_leading_zero(0, hours);
  print_with_leading_zero(3, minutes);
  print_with_leading_zero(6, seconds);
}


void draw(const char* progmemStr, const uint8_t col, const uint8_t row) {
  strlcpy_P(lcdbuffer1, progmemStr, sizeof(lcdbuffer1));
  lcd.at(col, row, lcdbuffer1);
  display_last_tm = millis();
}

///START OF BETA CODE

void Enter_Aux_Endpoint() {
  if (first_ui) {
    first_ui = false;
    stopISR1();
    //routine for just moving to end point if nothing was stored.
    lcd.empty();
    draw(F_STR("Enter Aux End Pt"), 1, 1);
    //move the aux motor 0.5 inches in the positive direction
    int32_t Target[MOTORS] = { 0, 0, STEPS_PER_INCH_AUX / 2 };
    set_target(Target);
    dda_move(20);
  }

  if (second_ui && (millis() - display_last_tm) > screen_delay) {
    second_ui = false;
    lcd.empty();
    draw(F_STR("AuxDist:   .  In"), 1, 1);
    draw(F_STR("C-Next Z-Go Back"), 2, 1);
    aux_dist = static_cast<int16_t>((motors[2].position * 10) / STEPS_PER_INCH_AUX); 
    DisplayAUX_Dist();
  }

  NunChuckjoybuttons();

  int16_t delta = joy_capture_y3();
  if (delta != 0) {
    aux_dist = (updateWithWraparound(aux_dist, -MAX_AUX_MOVE_DISTANCE, MAX_AUX_MOVE_DISTANCE, delta * 500));
    DisplayAUX_Dist();
    delay(scroll_delay);
  }

  //lcd.at(1,1,aux_dist/10);
  //lcd.at(1,7,aux_dist%10);
  //config.motor_steps_pt[2][2]=(aux_dist*47812L)/10;
  //lcd.at(2,1,static_cast<int32_t>config.motor_steps_pt[2][2]);
  button_actions_Enter_Aux_Endpoint();  //read buttons, look for c button press to set interval
}

void DisplayAUX_Dist() {
    // Handle negative sign separately
    if (aux_dist < 0) {
        draw(F_STR("-"), 1, 9); // Draw the negative sign at the starting position
        DrawPaddedDecimalValue<4>(uabs(aux_dist), 1, 10, 10); // Adjust column for negative numbers
    }
    else {
        lcd.clearRegion(1, 9, 9); // Draw a space for positive numbers
        DrawPaddedDecimalValue<4>(aux_dist, 1, 10, 10); // Standard position for positive numbers
    }
}

void button_actions_Enter_Aux_Endpoint() {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    lcd.empty();
    draw(F_STR("Aux End Pt. Set"), 1, 2);
    delay(screen_delay);
    
    config.motor_steps_pt[0][2] = 0;                                                   //this sets the end point
    config.motor_steps_pt[1][2] = 0;                                                   //this sets the end point
    config.motor_steps_pt[2][2] = static_cast<int32_t>((static_cast<float>(aux_dist) * static_cast<float>(STEPS_PER_INCH_AUX)) / 10.0f);  //this sets the end point
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
  }
}

//END BETA CODE