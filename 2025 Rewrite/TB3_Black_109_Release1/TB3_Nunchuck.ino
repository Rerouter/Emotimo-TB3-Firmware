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

//remote and interface variables

int8_t joy_x_axis, joy_y_axis;
int8_t joy_x_axis_Threshold, joy_y_axis_Threshold;
uint8_t joy_x_axis_Offset, joy_y_axis_Offset;

int16_t accel_x_axis;
int16_t accel_x_axis_Threshold;
uint16_t accel_x_axis_Offset;

constexpr int8_t joy_deadband = 10;
constexpr int16_t acc_deadband = 100;

__attribute__ ((const))
__attribute__ ((warn_unused_result))
int16_t applyDeadband(const int16_t value, const int16_t deadband) {
  if (value > deadband) {
    return value - deadband;
  } else if (value < -deadband) {
    return value + deadband;
  } else {
    return 0;
  }
}

__attribute__ ((const))
__attribute__ ((warn_unused_result))
uint8_t JoystickLock(const int16_t axis_value, const uint8_t lock_count) {
  uint8_t int_lock_count = lock_count;
  if (uabs(axis_value) > 83) {
    int_lock_count++;
    if (int_lock_count > 250) int_lock_count = 250;  // Prevent overflow
  }
  else {
    int_lock_count = 0;  // Reset lock count if below threshold
  }
  return int_lock_count;
}


void calibrate_joystick(const uint8_t tempx, const uint8_t tempy) {
  joy_x_axis_Offset = tempx;
  joy_x_axis_Threshold = 127 - abs(static_cast<int>(tempx) - 127);

  joy_y_axis_Offset = tempy;
  joy_y_axis_Threshold = 127 - abs(static_cast<int>(tempy) - 127);

  accel_x_axis_Offset = 500;
  accel_x_axis_Threshold = 200;  //hardcode this, don't calibrate
}

void ClearNunChuck() {
  joy_x_axis = 0;
  joy_y_axis = 0;
  accel_x_axis = 0;

  c_button = 0;
  z_button = 0;
}


void NunChuckQuerywithEC()  // Error correction and reinit on disconnect - takes about 1050 microseconds
{
  while (Nunchuck.getData()) {}   // keep retrying until no error is returned, 0 = success, 1 = failure
}


void NunChuckjoybuttons() {
  NunChuckQuerywithEC();
  joy_x_axis = constrain(static_cast<int16_t>(Nunchuck.joyx()) - static_cast<int16_t>(joy_x_axis_Offset), -joy_x_axis_Threshold, joy_x_axis_Threshold);
  joy_y_axis = constrain(static_cast<int16_t>(Nunchuck.joyy()) - static_cast<int16_t>(joy_y_axis_Offset), -joy_y_axis_Threshold, joy_y_axis_Threshold);
  accel_x_axis = constrain(static_cast<int16_t>(Nunchuck.accelx()) - static_cast<int16_t>(accel_x_axis_Offset), -accel_x_axis_Threshold, accel_x_axis_Threshold);
  if (config.AUX_REV) accel_x_axis *= -1;

  joy_x_axis = applyDeadband(joy_x_axis, joy_deadband);
  joy_y_axis = -1 * applyDeadband(joy_y_axis, joy_deadband);
  accel_x_axis = -1 * applyDeadband(accel_x_axis, acc_deadband);

  joy_x_lock_count = JoystickLock(joy_x_axis, joy_x_lock_count);
  joy_y_lock_count = JoystickLock(joy_y_axis, joy_y_lock_count);

  c_button = Nunchuck.cbutton();
  z_button = Nunchuck.zbutton();

  Check_Prog();
}


void Check_Prog() {  // Routine for button presses
    static uint32_t input_last_tm = 0;  // Static to retain value across calls

    // Get current time
    uint32_t current_time = millis();

    // Reset counts if more than 2000ms have passed since the last input
    if ((current_time - input_last_tm) > 2000) {
        C_Button_Read_Count = 0;
        Z_Button_Read_Count = 0;
        CZ_Button_Read_Count = 0;
    }

    // Handle button press combinations
    if (c_button && z_button) {
      CZ_Button_Read_Count++;
      C_Button_Read_Count++;
      Z_Button_Read_Count++;
    }
    else if (c_button) { C_Button_Read_Count++; }
    else if (z_button) { Z_Button_Read_Count++; }

    // Update last input time
    input_last_tm = current_time;
}

__attribute__ ((warn_unused_result))
int joy_capture_x1() { return constrain(umap(joy_x_axis, -35, 35, -1, 1), -1, 1);  }

__attribute__ ((warn_unused_result))
int joy_capture_y1() { return constrain(umap(-joy_y_axis, -35, 35, -1, 1), -1, 1); }

__attribute__ ((warn_unused_result))
int joy_capture_x3()  //captures joystick input and conditions it for UI
{
  if (joy_x_lock_count > 245) {  //really really fast
    scroll_delay = 0;
    return -1 * umap(-joy_x_axis, -55, 55, -40, 40);
  }
  if (joy_x_lock_count > 50) {  //really fast
    scroll_delay = 10;
    return -1 * umap(-joy_x_axis, -100, 100, -10, 10);
  }
  else if (joy_x_lock_count > 20) {  //pretty fast
    scroll_delay = 70;
    return -1 * umap(-joy_x_axis, -80, 80, -10, 10);
  }
  else if (uabs(joy_x_axis) > 10) {  //go variable add delay which we run later
    scroll_delay = (500 - 6 * uabs(joy_x_axis));
    if (scroll_delay < 15) scroll_delay = 15;
    return -1 * constrain(umap(-joy_x_axis, -35, 35, -1, 1), -1, 1);
  }
  else {  //null loop
    scroll_delay = 0;
    return 0;
  }
}

__attribute__ ((warn_unused_result))
int16_t joy_capture_y3()  //captures joystick input and conditions it for UI
{
  if (joy_y_lock_count > 245) {  //really really fast
    scroll_delay = 0;
    return -1 * umap(joy_y_axis, -55, 55, -40, 40);
  }
  if (joy_y_lock_count > 50) {  //really fast
    scroll_delay = 10;
    return -1 * umap(joy_y_axis, -100, 100, -10, 10);
  }
  else if (joy_y_lock_count > 20) {  //pretty fast
    scroll_delay = 70;
    return -1 * umap(joy_y_axis, -80, 80, -10, 10);
  }
  else if (uabs(joy_y_axis) > 10) {  //go variable add delay which we run later
    scroll_delay = (500 - 6 * uabs(joy_y_axis));
    if (scroll_delay < 15) scroll_delay = 15;
    return -1 * constrain(umap(joy_y_axis, -35, 35, -1, 1), -1, 1);
  }
  else {  //null loop
    scroll_delay = 0;
    return 0;
  }
}


void updateMotorVelocities2()
{
  int32_t maxJoyValue = joy_y_axis_Threshold - joy_deadband;
  int32_t scaleFactor = (maxJoyValue * maxJoyValue * maxJoyValue * 64L) / (65535/2); // max joystick value and a scale factor for division accuacy 

  //exponential curve

  int32_t intAxisX = static_cast<int32_t>(-joy_x_axis);
  int32_t intAxisY = static_cast<int32_t>(joy_y_axis);
  int32_t intAxisZ = static_cast<int32_t>(accel_x_axis);

  int32_t int_axis[3] = { intAxisX, intAxisY, intAxisY };
  int32_t speedTarget[3];
  for (uint8_t mot = 0; mot < MOTORS; mot++) {
    Motor *motor = &motors[mot];
    int_axis[mot] = (int_axis[mot] * int_axis[mot] * int_axis[mot]);
    speedTarget[mot] = int_axis[mot] * 64L / scaleFactor;
  }
  setMotorSpeedTarget(speedTarget);
}


void setMotorSpeedTarget(int32_t speedTarget[3])  //Happens  20 times a second
{
  //accelerations - accumulator limit is is 65553. Loop is 20Hz.   If we want zero to max to be 1 sec, we choose
  //example 1 If we want zero to max to be 1 sec, we choose (65535/20)/1.0 =3276.75 this is the max per cycle.
  //example 2 If we want zero to max to be 2 sec,(65535/20)/2.0 =1638.375 this is the max per cycle.

  constexpr int32_t accelmax_axis = (65535 / (1000 / TIME_CHUNK)) / 1;

  constexpr int32_t accelmax[3] = { accelmax_axis, accelmax_axis, accelmax_axis };
  //could also make accel dynamic based on velocity - decelerate faster when going fast - have to make sure we don't create hyperbole

  for (uint8_t mot = 0; mot < MOTORS; mot++) {
    Motor *motor = &motors[mot];

    //record last speed for compare, multiply by direction to get signed value
    int32_t lastMoveSpeed = motor->nextMotorMoveSpeed;
    if (motor->dir == 0) lastMoveSpeed *= -1;  //0 is reverse

    //set the accumulator value for the 1/20th second move - this is our accumulator value

    //accel
    if (speedTarget[mot] != lastMoveSpeed) {
      if ((speedTarget[mot] > lastMoveSpeed) && ((speedTarget[mot] - lastMoveSpeed) > accelmax[mot]))  //accel
      {
        speedTarget[mot] = lastMoveSpeed + accelmax[mot];
      } else if ((speedTarget[mot] < lastMoveSpeed) && ((lastMoveSpeed - speedTarget[mot]) > accelmax[mot]))  //decel
      {
        speedTarget[mot] = lastMoveSpeed - accelmax[mot];
      }
    }
    motor->nextMotorMoveSpeed = static_cast<uint16_t>(constrain(uabs(speedTarget[mot]),0, uint16_t(65535)));  //top is 65535
    if (motor->nextMotorMoveSpeed > 0) motorMoving = ubitSet(motorMoving, mot);
    else                               motorMoving = ubitClear(motorMoving, mot);

    motor->dir = (speedTarget[mot] > 0);
    motor->motorAccumulator = 65535;
    motor->motorMoveSteps = 32767;
  }
  nextMoveLoaded = true;
}
