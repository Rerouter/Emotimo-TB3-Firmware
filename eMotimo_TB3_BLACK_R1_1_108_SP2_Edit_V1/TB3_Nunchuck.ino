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
uint8_t  joy_x_axis_Offset,    joy_y_axis_Offset;
uint16_t acc_x_axis_Offset,    acc_y_axis_Offset;

unsigned int joy_y_lock_count = 0;
unsigned int joy_x_lock_count = 0;

uint8_t HandleButtons()
{
  static uint8_t held;
  static uint8_t last;
  switch (ButtonState)
  {
    case Released:
      held = 0;
      last = Released;
      return Released;
      
    case C_Pressed:
      switch(last)
      {
        case Released:
          last = C_Pressed;
          return C_Pressed;

        case C_Pressed:
          last = C_Pressed;
          if (held < 250) held++;
          if (held > HeldThreshold) return C_Held;
          else                      return Read_Again;

        default:
          held = 0;
          last = C_Pressed;
          return Read_Again;
          
      }
    case Z_Pressed:
      switch(last)
      {
        case Released:
          last = Z_Pressed;
          return Z_Pressed;

        case Z_Pressed:
          last = Z_Pressed;
          if (held < 250) held++;
          if (held > HeldThreshold) return Z_Held;
          else                      return Read_Again;
          
        default:
          held = 0;
          last = Z_Pressed;
          return Read_Again;
          
      }
    case CZ_Pressed:
      switch(last)
      {
        case Released:
          last = CZ_Pressed;
          return CZ_Pressed;

        case CZ_Pressed:
          last = CZ_Pressed;
          if (held < 250) held++;
          if (held > HeldThreshold) return CZ_Held;
          else                      return Read_Again;

       default:
          held = 0;
          last = CZ_Pressed;
          return Read_Again;
      }
    default:
      return Read_Again;
  }
}


void calibrate_joystick(uint8_t tempx, uint8_t tempy)
{
  joy_x_axis_Offset = tempx;  //int joy_x_axis_map=180;
  joy_y_axis_Offset = tempy;  //int joy_y_axis_map=180;
  acc_x_axis_Offset = 512;    //hardcode this, don't calibrate
  acc_y_axis_Offset = 512;
}


void NunChuckRequestData() //error correction and reinit on disconnect  - takes about 1050 microsecond
{
  uint8_t NCReadStatus = 0; //control variable for NC error handling
  uint8_t failcount;
  do
  {
    failcount = Nunchuck.getData();

    if (!Nunchuck.joyx() && !Nunchuck.joyy() )
    { //error condition //throw this out and read again
      NCReadStatus ++;
    }
    else if (Nunchuck.joyx() == 255 && Nunchuck.joyy() == 255 && Nunchuck.accelx() == 1023)
    { //nunchuck disconnected, then reconnected  - needs initializing
      Nunchuck.init(0);
      NCReadStatus ++;
    }
    else if (!Nunchuck.accelx() && !Nunchuck.accely() && !Nunchuck.accelz())
    { //nunchuck just reintialized - needs a few more reads before good
      NCReadStatus ++;
    }
    else break;
  }
  while (NCReadStatus);
  if (failcount > 5   && Move_Engaged)  Nunchuck.clearData();
  if (failcount > 250 && !Move_Engaged) Nunchuck.clearData();
}


void NunChuckProcessData()
{
  // These need to be constained to +-101 to correcting scale for the exponential move functions
  joy_x_axis = constrain(int16_t(Nunchuck.joyx() - joy_x_axis_Offset), -101, 101); //gets us to +- 101
  joy_y_axis = constrain(int16_t(Nunchuck.joyy() - joy_y_axis_Offset), -101, 101); //gets us to +- 101
  acc_x_axis = constrain(int16_t((Nunchuck.accelx() - acc_x_axis_Offset) >> 2), -101, 101); //gets us to +- 101
  //acc_y_axis = constrain(int16_t((Nunchuck.accely() - acc_y_axis_Offset) >> 2), -101, 101); //gets us to +- 101

  if (AUX_REV) acc_x_axis = -acc_x_axis;
  if (joy_x_axis) joy_x_axis = -joy_x_axis;
  if (joy_y_axis) joy_y_axis = -joy_y_axis;

  //create a deadband
  const uint8_t deadband  = 7;   // results in 100-7 or +-93 - this is for the joystick
  const uint8_t deadband2 = 25; //  this is for the accelerometer

  joy_x_axis = NunchuckDeadband(joy_x_axis, deadband);
  joy_y_axis = NunchuckDeadband(joy_y_axis, deadband);
  acc_x_axis = NunchuckDeadband(acc_x_axis, deadband2);
  //acc_y_axis = NunchuckDeadband(acc_y_axis, deadband2);

  //check for joystick y lock for more than one second
  if (abs(joy_y_axis) > 50) {  
    if (joy_y_lock_count < 250) { joy_y_lock_count++;   }
  }
  else                          { joy_y_lock_count = 0; }

  //check for joystick x lock for more than one second
  if (abs(joy_x_axis) > 50) {
    if (joy_x_lock_count < 250) { joy_x_lock_count++; }
  }
  else                          { joy_x_lock_count = 0; }

  ButtonState = (Nunchuck.zbutton() << 1) | Nunchuck.cbutton();
}


int8_t NunchuckDeadband(int8_t input, int8_t deadband)
{
  if      (input > deadband)  return input -= deadband;
  else if (input < -deadband) return input += deadband;
  else                        return 0;
}


void NunChuckClearData()
{
  joy_x_axis = 0;
  joy_y_axis = 0;
  acc_x_axis = 0;
  //acc_y_axis = 0;
  ButtonState = Read_Again;
}


void applyjoymovebuffer_exponential()  //exponential stuff
{
  //scale based on read frequency  base is 500 reads per second  - now 20 reads per second = 25x
  int32_t z;
  int32_t x = current_steps.x + ((int32_t(joy_x_axis) * joy_x_axis * joy_x_axis) >> 8);
  int32_t y = current_steps.y + ((int32_t(joy_y_axis) * joy_y_axis * joy_y_axis) >> 8);
  if (AUX_ON) z = current_steps.z + ((int32_t(acc_x_axis) * acc_x_axis * acc_x_axis) >> 8);
  else        z = current_steps.z;

  set_target(x, y, z);
  feedrate_micros = calculate_feedrate_delay_2();
}


void applyjoymovebuffer_linear()
{
  int32_t x = joy_x_axis + current_steps.x;
  int32_t y = joy_y_axis + current_steps.y;
  int32_t z = acc_x_axis + current_steps.z;

  set_target(x, y, z);
  feedrate_micros = calculate_feedrate_delay_2();
}


void nc_sleep()
{
  if (abs(joy_x_axis) > 15 || abs(joy_y_axis) > 15)  digitalWrite(MOTOR_EN, LOW);
  else                                               digitalWrite(MOTOR_EN, HIGH);
}


int8_t joy_capture2(uint8_t axis) //captures joystick input
{
  uint8_t axisval;
  switch (axis)
  {
    case 0:  axisval = joy_x_axis;  break;
    case 1:  axisval = joy_y_axis;  break;
    default: return 0;
  }
  prompt_delay = (500 - 5 * abs(axisval)); // The joystick is constrained to +-100 so this should never equal below 0
  return -1 * map(axisval, -20, 20, -1, 1);
}


int8_t joy_capture3(uint8_t axis) //captures joystick input and conditions it for UI
{
  int8_t  axisval;
  uint8_t axislock;
  switch (axis)
  {
    case 0:  axisval = joy_x_axis;  axislock = joy_x_lock_count;  break;
    case 1:  axisval = joy_y_axis;  axislock = joy_y_lock_count;  break;
    default: return 0;
  }
  
  if (axislock > 245 && abs(axisval > 90)) { //really really fast
    prompt_delay = 0;
    return -1 * map(axisval, -55, 55, -40, 40);
  }
  else if (axislock > 50 && abs(axisval > 75)) { //really fast
    prompt_delay = (100 - abs(axisval));
    return -1 * map(axisval, -100, 100, -10, 10);
  }
  else if (axislock > 20) { //pretty fast
    prompt_delay = (200 - 2 * abs(axisval));
    return -1 * map(axisval, -100, 100, -5, 5);
  }
  else if (abs(axislock) > 10) { //go variable add delay which we run later
    prompt_delay = (500 - 5 * abs(axisval)); // The joystick is constrained to +-100 so this should never equal below 0
    return -1 * map(axisval, -50, 50, -1, 1);
  }
  else { //null loop
    prompt_delay = 0;
    return 0;
  }
}


int joy_capture_x_map() //captures joystick input for left and right
{
  return map(joy_x_axis, -85, 85, -1, 1);
}


int8_t joy_capture_y_map() //captures joystick input for up down
{
  return map(joy_y_axis, -85, 85, -1, 1);
}


void updateMotorVelocities2()   //Happens  20 times a second
{
  //accelerations - accumulator limit is is 65553. Loop is 20Hz.   If we want zero to max to be 1 sec, we choose
  //example 1 If we want zero to max to be 1 sec, we choose (65535/2)/10 =3276 this is the max per cycle.
  //example 2 If we want zero to max to be 2 sec, we choose (65535/2)/20 =1638 this is the max per cycle.

  uint8_t acceleration_time = 5;  // 0.1x Seconds time to reach maximum acceration

  uint16_t accelmax0 = (65535 / 2) / acceleration_time;
  uint16_t accelmax1 = (65535 / 2) / acceleration_time;
  uint16_t accelmax2 = (65535 / 2) / acceleration_time;
  //could also make accel dynamic based on velocity - decelerate faster when going fast - have to make sure we don't create hyperbole

  //record last speed for compare, multiply by direction to get signed value
  int32_t signedlastMotorMoveSpeed0 = motors[0].nextMotorMoveSpeed;
  if (!motors[0].dir) signedlastMotorMoveSpeed0 *= -1; //0 is reverse
  int32_t signedlastMotorMoveSpeed1 = motors[1].nextMotorMoveSpeed;
  if (!motors[1].dir) signedlastMotorMoveSpeed1 *= -1;
  int32_t signedlastMotorMoveSpeed2 = motors[2].nextMotorMoveSpeed;
  if (!motors[2].dir) signedlastMotorMoveSpeed2 *= -1;

  //set the accumulator value for the 1/20th second move - this is our accumulator value
  //axis_button_deadzone constrains the axis's to max of 101, so max of 64393 out of 65535
  int32_t signedMotorMoveSpeedTarget0 = (int32_t(joy_x_axis) * joy_x_axis * joy_x_axis) >> 4;
  int32_t signedMotorMoveSpeedTarget1 = (int32_t(joy_y_axis) * joy_y_axis * joy_y_axis) >> 4;
  int32_t signedMotorMoveSpeedTarget2 = (int32_t(acc_x_axis) * acc_x_axis * acc_x_axis) >> 4;

  // Speed Limiting
  if (signedMotorMoveSpeedTarget0 > PAN_MAX_JOG_STEPS_PER_SEC) signedMotorMoveSpeedTarget0 = PAN_MAX_JOG_STEPS_PER_SEC;
  if (signedMotorMoveSpeedTarget1 > TILT_MAX_JOG_STEPS_PER_SEC) signedMotorMoveSpeedTarget1 = TILT_MAX_JOG_STEPS_PER_SEC;
  if (signedMotorMoveSpeedTarget2 > AUX_MAX_JOG_STEPS_PER_SEC) signedMotorMoveSpeedTarget2 = AUX_MAX_JOG_STEPS_PER_SEC;
  
  //pan accel
  if (signedMotorMoveSpeedTarget0 != signedlastMotorMoveSpeed0)
  {
    if ((signedMotorMoveSpeedTarget0 > signedlastMotorMoveSpeed0)
    && ((signedMotorMoveSpeedTarget0 - signedlastMotorMoveSpeed0) > accelmax0)) //accel
    {
      signedMotorMoveSpeedTarget0 = signedlastMotorMoveSpeed0 + accelmax0;
    }
    else if ((signedlastMotorMoveSpeed0 > signedMotorMoveSpeedTarget0)
         && ((signedlastMotorMoveSpeed0 - signedMotorMoveSpeedTarget0) > accelmax0) ) //decel
    {
      signedMotorMoveSpeedTarget0 = signedlastMotorMoveSpeed0 - accelmax0;
    }
  }
  //tilt accel
  if (signedMotorMoveSpeedTarget1 != signedlastMotorMoveSpeed1)
  {
    if ((signedMotorMoveSpeedTarget1 > signedlastMotorMoveSpeed1)
    && ((signedMotorMoveSpeedTarget1 - signedlastMotorMoveSpeed1) > accelmax1))//accel
    {
      signedMotorMoveSpeedTarget1 = signedlastMotorMoveSpeed1 + accelmax1;
    }

    else if ((signedlastMotorMoveSpeed1 > signedMotorMoveSpeedTarget1)
         && ((signedlastMotorMoveSpeed1 - signedMotorMoveSpeedTarget1) > accelmax1) ) //decel
    {
      signedMotorMoveSpeedTarget1 = signedlastMotorMoveSpeed1 - accelmax1;
    }
  }
  //aux accel
  if (signedMotorMoveSpeedTarget2 != signedlastMotorMoveSpeed2)
  {
    if ((signedMotorMoveSpeedTarget2 > signedlastMotorMoveSpeed2)
    && ((signedMotorMoveSpeedTarget2 - signedlastMotorMoveSpeed2) > accelmax2))//accel
    {
      signedMotorMoveSpeedTarget2 = signedlastMotorMoveSpeed2 + accelmax2;
    }

    else if ((signedlastMotorMoveSpeed2 > signedMotorMoveSpeedTarget2)
         && ((signedlastMotorMoveSpeed2 - signedMotorMoveSpeedTarget2) > accelmax2) ) //decel
    {
      signedMotorMoveSpeedTarget2 = signedlastMotorMoveSpeed2 - accelmax2;
    }
  }

  motors[0].nextMotorMoveSpeed = abs(signedMotorMoveSpeedTarget0); //top is 65535
  motors[1].nextMotorMoveSpeed = abs(signedMotorMoveSpeedTarget1); //top is 65535
  motors[2].nextMotorMoveSpeed = abs(signedMotorMoveSpeedTarget2); //top is 65535

  for (uint8_t mot = 0; mot < 3; mot++)
  {
    if (motors[mot].nextMotorMoveSpeed) bitSet(motorMoving, mot);
    else                                bitClear(motorMoving, mot);
    //Serial.print("motorMoving:");Serial.println(motorMoving);
  }

  motors[0].dir = (signedMotorMoveSpeedTarget0 > 0) ? 1 : 0;
  motors[1].dir = (signedMotorMoveSpeedTarget1 > 0) ? 1 : 0;
  motors[2].dir = (signedMotorMoveSpeedTarget2 > 0) ? 1 : 0;

  *motorAccumulator[0] = 65535;
  *motorAccumulator[1] = 65535;
  *motorAccumulator[2] = 65535;

  //This is just to get us into the loop in the interrupt for each motor to check the test.
  motorMoveSteps0 = 32000;
  motorMoveSteps1 = 32000;
  motorMoveSteps2 = 32000;

  nextMoveLoaded = true;
}
