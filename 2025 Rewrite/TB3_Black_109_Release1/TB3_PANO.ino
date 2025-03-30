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

uint32_t Pan_AOV_steps;
uint32_t Tilt_AOV_steps;
uint8_t olpercentage = 20;

Pano_Array_Type PanoArrayType = Pano_Array_Type::PANO_9ShotCenter;  //

//constexpr int8_t SevenByThree[22][2] = { { 0, 0 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, {  1, -1 }, { 1,  0 }, { 1,  1 }, { 2, 1 }, { 2, 0 }, { 2, -1 }, { 3, -1 }, { 3,  0 }, { 3,  1 }, { -2,  1 }, { -2,  0 }, { -2, -1 }, { -3, -1 }, { -3,  0 }, { -3,  1 }, {  0,  0 } };
//constexpr int8_t NineByFive_1[46][2] = { { 0, 0 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, {  1, -1 }, { 1,  0 }, { 1,  1 }, { 2, 1 }, { 2, 0 }, { 2, -1 }, { 3, -1 }, { 3,  0 }, { 3,  1 }, {  4,  1 }, {  4,  0 }, {  4, -1 }, {  4, -2 }, {  3, -2 }, {  2, -2 }, {  1, -2 }, {  0, -2 }, { -1, -2 }, { -2, -2 }, { -2, -1 }, { -2, 0 }, { -2, 1 }, { -3,  1 }, { -3,  0 }, { -3, -1 }, { -3, -2 }, { -4, -2 }, { -4, -1 }, { -4,  0 }, { -4, 1 }, { -4, 2 }, { -3, 2 }, { -2, 2 }, { -1, 2 }, { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 }, { 4, 2 }, { 0, 0 } };
//constexpr int8_t NineByFive_2[46][2] = { { 0, 0 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, {  1, -1 }, { 1,  0 }, { 1,  1 }, { 2, 1 }, { 2, 0 }, { 2, -1 }, { 2, -2 }, { 1, -2 }, { 0, -2 }, { -1, -2 }, { -2, -2 }, { -2, -1 }, { -2,  0 }, { -2,  1 }, { -2,  2 }, { -1,  2 }, {  0,  2 }, {  1,  2 }, {  2,  2 }, {  3,  2 }, {  3, 1 }, {  3, 0 }, {  3, -1 }, {  3, -2 }, {  4, -2 }, {  4, -1 }, {  4,  0 }, {  4,  1 }, {  4,  2 }, { -3, 2 }, { -3, 1 }, { -3, 0 }, { -3, -1 }, { -3, -2 }, { -4, -2 }, { -4, -1 }, { -4, 0 }, { -4, 1 }, { -4, 2 }, { 0, 0 } };
constexpr int8_t TopThird25[26][2]   = { { 0, 0 }, { 0, 1 }, {  0, 2 }, { -1, 2 }, { -1,  1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 1,  2 }, { 2, -1 }, { 2,  0 }, { 2,  1 }, {  2,  2 }, {  2,  3 }, {  1,  3 }, {  0,  3 }, { -1,  3 }, { -2,  3 }, { -2,  2 }, { -2,  1 }, { -2,  0 }, { -2, -1 }, {  0,  0 } };
constexpr int8_t TopThird7by5[36][2] = { { 0, 0 }, { 0, 1 }, {  0, 2 }, { -1, 2 }, { -1,  1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 1,  2 }, { 2,  2 }, { 2,  1 }, { 2,  0 }, {  2, -1 }, {  3, -1 }, {  3,  0 }, {  3,  1 }, {  3,  2 }, {  3,  3 }, {  2,  3 }, {  1,  3 }, {  0,  3 }, { -1,  3 }, { -2,  3 }, { -2, 2 }, { -2, 1 }, { -2,  0 }, { -2, -1 }, { -3, -1 }, { -3,  0 }, { -3,  1 }, { -3,  2 }, { -3,  3 }, { 0, 0 } };


void find_nth_point_spiral(uint16_t n, int32_t target[3]) {
    const uint16_t w = config.total_shots_x;
    const uint16_t h = config.total_shots_y;
    const int32_t steps[] = { config.linear_steps_per_shot[0], config.linear_steps_per_shot[1] };
    
   
    // Direction vectors: East, North, West, South
    int8_t directions[4][2] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
    uint8_t direction_index = (w >= h) ? 0 : 1;
    uint8_t direction_offset = (w < h && (w % 2) && (h % 2)) ? 0 : direction_index;

    int8_t repeat_count[2] = { static_cast<int8_t>(w >= h ? (w - h > 0 ? w - h : 1) : 1),
                               static_cast<int8_t>(w >= h ? 1 : (h - w > 0 ? h - w : 1)) };

    int8_t x = -((repeat_count[0] - 1) / 2 + 1) * (w > h) + direction_offset;
    int8_t y = -((repeat_count[1] - 1) / 2 + 1) * (h > w) + direction_offset;

    while (true) {
        // Calculate steps in the current direction
        int8_t segment_length = repeat_count[direction_index % 2];

        // If remaining steps (n) fit in this segment
        if (n < segment_length) {
            x += n * directions[direction_index][0];
            y += n * directions[direction_index][1];

            target[0] = x * steps[0] - (w % 2 == 0 ? steps[0] / 2 : 0);
            target[1] = y * steps[1] - (h % 2 == 0 ? steps[1] / 2 : 0);
            target[2] = 0; // For potential future use
            return;
        }

        // Subtract the full segment and move position
        n -= segment_length;
        x += segment_length * directions[direction_index][0];
        y += segment_length * directions[direction_index][1];

        // Update direction and increment repeat count
        repeat_count[direction_index % 2] += 1;
        direction_index = (direction_index + 1) % 4;
    }
}



void get_nth_point_onion(const uint8_t n, int8_t *x, int8_t *y) {
    // Direction vectors: D, L, U, R (Down, Left, Up, Right)
    int8_t directions[4][2] = { { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 0 } };
    int8_t repeat_count = 1;
    uint8_t step = 0;
    uint8_t direction_index = 0;
    uint8_t current_position = 0;

    // Calculate position directly based on repeat counts
    while (current_position + repeat_count <= n) {
        // Determine current direction
        int8_t dx = directions[direction_index][0];
        int8_t dy = directions[direction_index][1];

        // Move full repeat count
        *x += dx * repeat_count;
        *y += dy * repeat_count;
        current_position += repeat_count;

        // Update direction and step
        direction_index = (direction_index + 1) % 4;
        step++;

        // Increase repeat count every two directions
        if (step % 2 == 0) {
            repeat_count++;
        }
    }

    // Calculate remaining steps, if any
    if (current_position < n) {
        int8_t dx = directions[direction_index][0];
        int8_t dy = directions[direction_index][1];
        int8_t remaining_steps = n - current_position;

        *x += dx * remaining_steps;
        *y += dy * remaining_steps;
    }
}

void Set_angle_of_view() {
  if (first_ui) {
    first_ui = false;
    set_home();
    lcd.empty();
    draw(F_STR("Set Angle o'View"), 1, 1);
    draw(F_STR("C-Set, Z-Reset"), 2, 2);
  
    UpdatePowersave(Powersave::Shoot_Minimum); // Need to Move
    startISR1();  //setup the ISR
  }

  if (second_ui && (millis() - display_last_tm) > screen_delay) {
    second_ui = false;
    lcd.empty();
    draw(F_STR("Pan  AOV:"), 1, 1); DisplayStepsToDegDecimal(0, 1, 10);
    draw(F_STR("Tilt AOV:"), 2, 1); DisplayStepsToDegDecimal(0, 2, 10);
  }

  NunChuckjoybuttons();
  button_actions_aov();

  //Velocity Engine update
  if (!nextMoveLoaded) {
    updateMotorVelocities2();

    if (!second_ui) {
      DisplayStepsToDegDecimal(uabs(motors[0].position), 1, 10); // Row 1, Column 10
      DisplayStepsToDegDecimal(uabs(motors[1].position), 2, 10); // Row 2, Column 10
    }
  }
}


void DisplayStepsToDegDecimal(const uint32_t steps, const uint8_t row, const uint8_t column) {
    // Scale the value to thousandths directly
    uint32_t scaledValue = (steps * 9L) / 4L; // Scale by 9/4 to get to degrees
    DrawPaddedDecimalValue<7>(scaledValue, row, column, 1000); // Scale of 1000 for 3 decimal places
}


void button_actions_aov() {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    WaitForMotorStop();
    UpdatePowersave(Powersave::Programs);

    lcd.empty();
    Pan_AOV_steps = uabs(motors[0].position);
    Tilt_AOV_steps = uabs(motors[1].position);
    draw(F_STR("AOV Set"), 1, 5);
    delay(screen_delay);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
    WaitForMotorStop();
    UpdatePowersave(Powersave::Programs);
  }
}


void Define_Overlap_Percentage() {
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("% Overlap"), 1, 5);
    draw(F_STR("C-Next Z-Go Back"), 2, 1);
    DrawPaddedValue<2>(olpercentage, 1, 2); // 1-99% for overlap
    delay(update_delay);
  }

  NunChuckjoybuttons();

  int16_t delta = joy_capture_y3();
  if (delta != 0) {
    olpercentage = static_cast<uint8_t>(updateWithWraparound(static_cast<int16_t>(olpercentage), 1, 99, delta));
    DrawPaddedValue<2>(olpercentage, 1, 2); // 1-99% for overlap
    delay(update_delay);
  }
  button_actions_olpercentage();  //read buttons, look for c button press to set interval
}


void button_actions_olpercentage() {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_forward();
    lcd.empty();
    draw(F_STR("Overlap Set"), 1, 3);
    delay(screen_delay);
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
  }
}

void Set_PanoArrayType() {
  if (first_ui) {
    first_ui = false;
    DisplayPanoArrayType(PanoArrayType);
    delay(update_delay);
  }

  NunChuckjoybuttons();

  int16_t yUpDown = joy_capture_y1();
  if (yUpDown != 0) {
    PanoArrayType = static_cast<Pano_Array_Type>(updateWithWraparound(static_cast<int16_t>(PanoArrayType), 0, static_cast<int16_t>(Pano_Array_Type::Count) - 1, yUpDown));
    DisplayPanoArrayType(PanoArrayType);
    delay(update_delay);
  }

  if (C_Button_Read_Count > CZ_Count_Threshold) {
    switch (PanoArrayType) {
      case Pano_Array_Type::Count:             // Error Condition, should not be hit
      case Pano_Array_Type::PANO_9ShotCenter:  { config.total_shots_x = 3; config.total_shots_y = 3; break; }
      case Pano_Array_Type::PANO_25ShotCenter: { config.total_shots_x = 5; config.total_shots_y = 5; break; }
      case Pano_Array_Type::PANO_5x5TopThird:  { config.total_shots_x = 5; config.total_shots_y = 5; break; }
      case Pano_Array_Type::PANO_7X3:          { config.total_shots_x = 7; config.total_shots_y = 3; break; }
      case Pano_Array_Type::PANO_7X5TopThird:  { config.total_shots_x = 7; config.total_shots_y = 5; break; }
      case Pano_Array_Type::PANO_9X5:          { config.total_shots_x = 9; config.total_shots_y = 5; break; }
    }
    config.camera_total_shots = config.total_shots_x * config.total_shots_y;
    progstep_forward(); 
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
  }
}

void DisplayPanoArrayType(const enum Pano_Array_Type typeval) {
    lcd.empty();
    draw(F_STR("Set Array Type"), 1, 2);
    switch (typeval) {
      case Pano_Array_Type::Count:             // Error Condition, should not be hit
      case Pano_Array_Type::PANO_9ShotCenter:  { draw(F_STR("9-Shot Center"), 2, 3); break; }
      case Pano_Array_Type::PANO_25ShotCenter: { draw(F_STR("25-Shot Center"), 2, 2); break; }
      case Pano_Array_Type::PANO_5x5TopThird:  { draw(F_STR("5x5 Top1/3"), 2, 4); break; }
      case Pano_Array_Type::PANO_7X3:          { draw(F_STR("7x3 Center"), 2, 4); break; }
      case Pano_Array_Type::PANO_7X5TopThird:  { draw(F_STR("7x5 Top1/3"), 2, 4); break; }
      case Pano_Array_Type::PANO_9X5     :     { draw(F_STR("9x5 Center"), 2, 4); break; }
    }
}


void Pano_Review_Confirm() {
  if (first_ui) {
    first_ui = false;
    lcd.empty();
    draw(F_STR("Review and"), 1, 4);
    draw(F_STR("Confirm Setting"), 2, 2);
    calc_pano_move();
    delay(update_delay);
    reviewprog = 1;
  }

  if ((millis() - display_last_tm) > screen_delay) {  //test for display update
    reviewprog = static_cast<uint8_t>(updateWithWraparound(static_cast<int16_t>(reviewprog), 1, 4, 1));
    Pano_DisplayReviewProg();
  }  //end test for display update

  NunChuckjoybuttons();

  if (uabs(joy_y_axis) > 20) {  //do read time updates to delay program
    reviewprog = 4;
    Pano_DisplayReviewProg();
  }
  pano_button_actions_review();
}

void pano_button_actions_review() {
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    lcd.empty();
    if (start_delay_sec > 0) {
      draw(F_STR("Delaying Start"), 1, 2);
      lcd.empty();
    }
    disable_AUX();  //
    draw(F_STR("Program Running"), 1, 1);
    delay(screen_delay);

    lcd.empty();            //clear for non video
    progstep_goto(Menu::SMS_Loop);  //  move to the main programcamera_real_fire

    lcd.bright(config.LCD_BRIGHTNESS_DURING_RUN);
    PanoState = ShootMoveState::Waiting_Camera;
    config.camera_fired = 0;

    if (P2PAccelMove) {
      startISR1();  //setup the ISR
    }
    else {
      stopISR1();
    }
    display_status();
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    progstep_backward();
  }
}

void Pano_DisplayReviewProg() {
  lcd.empty();
  switch (reviewprog) {
    case 1:  //
      draw(F_STR("Columns:"), 1, 2);
      draw(F_STR("Rows:"), 2, 5);
      DrawPaddedValue<5>(config.total_shots_x, 1, 11);
      DrawPaddedValue<5>(config.total_shots_y, 2, 11);
      break;

    case 2:  //
      draw(F_STR("Cam Shots:"), 1, 1);
      DrawPaddedValue<5>(config.camera_total_shots, 1, 12);
      draw(F_STR("Time:"), 2, 1);
      display_time(calc_time_remain(), 2, 9);
      break;

    case 3:  //
      draw(F_STR("Ready?"), 1, 6);
      draw(F_STR("Press C Button"), 2, 2);
      break;
    case 4:  //
      draw(F_STR("StartDly:    min"), 1, 1);
      draw(F_STR("Press C Button"), 2, 2);

      int16_t delta = joy_capture_y3();
      if (delta != 0) {
        start_delay_sec = static_cast<uint16_t>(updateWithWraparound(static_cast<int16_t>(start_delay_sec), 0, 500, delta));
        delay(scroll_delay);
      }
      DrawPaddedValue<3>(start_delay_sec, 1, 11); // Display seconds with padding
      break;

  }  //end switch
}


void calc_pano_move()  //pano - calculate other values
{
  config.linear_steps_per_shot[0] = static_cast<int32_t>((1.0f - (static_cast<float>(olpercentage) / 100.0f)) * static_cast<float>(Pan_AOV_steps));
  config.linear_steps_per_shot[1] = static_cast<int32_t>((1.0f - (static_cast<float>(olpercentage) / 100.0f)) * static_cast<float>(Tilt_AOV_steps));

  if (progstep == Menu::GIGA_Reveiw) {
    config.total_shots_x = static_cast<uint32_t>(fabs(static_cast<float>(uabs(config.motor_steps_pt[1][0])) / static_cast<float>(config.linear_steps_per_shot[0]) + 2));
    config.total_shots_y = static_cast<uint32_t>(fabs(static_cast<float>(uabs(config.motor_steps_pt[1][1])) / static_cast<float>(config.linear_steps_per_shot[1]) + 2));

    if (uabs(config.motor_steps_pt[1][0]) < static_cast<int32_t>(STEPS_PER_DEG)) { config.total_shots_x = 1; }// If the angle is too small, set it to 1 shot
    if (uabs(config.motor_steps_pt[1][1]) < static_cast<int32_t>(STEPS_PER_DEG)) { config.total_shots_y = 1; }// If the angle is too small, set it to 1 shot

    config.camera_total_shots = config.total_shots_x * config.total_shots_y; 

    config.linear_steps_per_shot[0] = static_cast<int32_t>(static_cast<float>(config.motor_steps_pt[1][0]) / static_cast<float>(config.total_shots_x - 1));
    config.linear_steps_per_shot[1] = static_cast<int32_t>(static_cast<float>(config.motor_steps_pt[1][1]) / static_cast<float>(config.total_shots_y - 1));
  }

  uint32_t x_time;
  uint32_t y_time;
  if (config.shot_interval_time == VIDEO_EXPVAL) {
    x_time = (config.linear_steps_per_shot[0] * 50);
    y_time = (config.linear_steps_per_shot[1] * 50);
    config.move_time = ((x_time * (config.total_shots_x - 1) * config.total_shots_y + y_time * (config.total_shots_y - 1)) / 1000) / config.camera_total_shots;
  }
  else {
    x_time = calculateMoveTimeMs(uabs(config.linear_steps_per_shot[0]), config.MAX_JOG_STEPS_PER_SEC[0]);
    y_time = calculateMoveTimeMs(uabs(config.linear_steps_per_shot[1]), config.MAX_JOG_STEPS_PER_SEC[1]);
    config.move_time = (x_time * (config.total_shots_x - 1) * config.total_shots_y + y_time * (config.total_shots_y - 1)) / config.camera_total_shots;
  }
}


void move_motors_pano()
{
   uint16_t currentImage = config.camera_fired;
  if (REVERSE_PROG_ORDER) {
    currentImage = config.camera_total_shots - currentImage;
  }
  uint16_t primaryIndex = currentImage / (config.panoRowFirst ? config.total_shots_x : config.total_shots_y);
  uint16_t secondaryIndex = currentImage % (config.panoRowFirst ? config.total_shots_x : config.total_shots_y);

  if (config.panoSerpentine == true && primaryIndex % 2 == 1) {
    secondaryIndex = (config.panoRowFirst ? config.total_shots_x : config.total_shots_y) - 1 - secondaryIndex;
  }

  uint16_t row = config.panoRowFirst ? primaryIndex : secondaryIndex;
  uint16_t column = config.panoRowFirst ? secondaryIndex : primaryIndex;

  int32_t Target[MOTORS] = { 0, 0, 0 };
  Target[0] = config.motor_steps_pt[1][0] - config.linear_steps_per_shot[0] * column;
  Target[1] = config.motor_steps_pt[1][1] - config.linear_steps_per_shot[1] * row;

  if (P2PAccelMove) {
    setPulsesPerSecond(config.MAX_JOG_STEPS_PER_SEC);
    synched3PtMove_max(Target);
    updateMotorVelocities();
  }
  else {
    stopISR1();
    set_target(Target);
    dda_move(50);
  }
  return;
}


void move_motors_accel_array()
{ 
  uint16_t image_num = config.camera_fired;
  if (REVERSE_PROG_ORDER) {
    image_num = config.camera_total_shots - image_num;
  }

  int32_t Target[MOTORS] = { 0, 0, 0 };
  switch(PanoArrayType) {
    case Pano_Array_Type::Count:
    case Pano_Array_Type::PANO_9ShotCenter:
    case Pano_Array_Type::PANO_25ShotCenter:
    case Pano_Array_Type::PANO_7X3:
    case Pano_Array_Type::PANO_9X5: {
      find_nth_point_spiral(image_num, Target);
      break; }
    case Pano_Array_Type::PANO_5x5TopThird: {
      Target[0] = config.linear_steps_per_shot[0] * TopThird25[image_num][0];
      Target[1] = config.linear_steps_per_shot[1] * TopThird25[image_num][1];
      break; }
    case Pano_Array_Type::PANO_7X5TopThird: {
      Target[0] = config.linear_steps_per_shot[0] * TopThird7by5[image_num][0];
      Target[1] = config.linear_steps_per_shot[1] * TopThird7by5[image_num][1];
      break; }
  }

  if (P2PAccelMove) {
    setPulsesPerSecond(config.MAX_JOG_STEPS_PER_SEC);
    synched3PtMove_max(Target);
    updateMotorVelocities();
  }
  else {
    stopISR1();
    set_target(Target);
    dda_move(50);
  }

  return;
}  //end move motors accel


void button_actions290() {  //repeat
  if (C_Button_Read_Count > CZ_Count_Threshold) {
    progstep_goto(Menu::SMS_Loop);
    lcd.empty();
              //1234567890123456
    draw(F_STR(" Repeating Prog "), 1, 1);
    draw(F_STR(" Moving To Start"), 1, 1);

    PanoState = ShootMoveState::Waiting_Camera;
    config.camera_fired = 0;
  }
  if (Z_Button_Read_Count > CZ_Count_Threshold) {
    UpdatePowersave(Powersave::Always); // Only keep powered if Always on
    config.camera_fired = 0;

    switch(progtype) {
      case Menu_Options::PANOGIGA:      { progstep_goto(Menu::GIGA_CamInterval); break; }
      case Menu_Options::PORTRAITPANO:  { progstep_goto(Menu::PANO_CamInterval); break; }
      case Menu_Options::REG2POINTMOVE:
      case Menu_Options::REV2POINTMOVE: { progstep_goto(Menu::PT2_ShotRepeat);   break; }
      case Menu_Options::REG3POINTMOVE:
      case Menu_Options::REV3POINTMOVE: { progstep_goto(Menu::PT3_Reveiw);       break; }
      default:                          { progstep_goto(Menu::PT2_MainMenu);     break; }
    }
  }
}
