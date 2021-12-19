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
uint8_t  joy_x_axis_Offset,    joy_y_axis_Offset;
uint8_t  joy_x_axis_Threshold, joy_y_axis_Threshold;
uint16_t acc_x_axis_Offset,    acc_y_axis_Offset;
uint16_t acc_x_axis_Threshold, acc_y_axis_Threshold;

int prev_joy_x_reading = 0;
int prev_joy_y_reading = 0;
int prev_acc_x_reading = 0;
int prev_acc_y_reading = 0;


void calibrate_joystick(uint8_t tempx, uint8_t tempy)
{
  joy_x_axis_Offset = tempx;  joy_x_axis_Threshold = 100; //int joy_x_axis_map=180;
  joy_y_axis_Offset = tempy;  joy_y_axis_Threshold = 100; //int joy_y_axis_map=180;
  acc_x_axis_Offset = 512;    acc_x_axis_Threshold = 200; //hardcode this, don't calibrate
}


void NunChuckRequestData() //error correction and reinit on disconnect  - takes about 1050 microsecond
{
  uint8_t NCReadStatus = 0; //control variable for NC error handling
  do
  {
    Nunchuck.getData();
    
    if (!Nunchuck.joyx() && !Nunchuck.joyy() )
    { //error condition //throw this out and read again
      delay(1);
      NCReadStatus ++;
    }
    else if (Nunchuck.joyx() == 255 && Nunchuck.joyy() == 255 && Nunchuck.accelx() == 1023)
    { //nunchuck disconnected, then reconnected  - needs initializing
      Nunchuck.init(0);
      NCReadStatus ++;
    }
    else if (!Nunchuck.accelx() && !Nunchuck.accely() && !Nunchuck.accelz())
    { //nunchuck just reintialized - needs a few more reads before good
      delay(1);
      NCReadStatus ++;
    }
    else break;
  }
  while (NCReadStatus);
}


int16_t NunchuckDeadband(int16_t input, int16_t deadband)
{
  if      (input > deadband)  input -= deadband;
  else if (input < -deadband) input += deadband;
  else                        input = 0;
  return input;
}


void NunChuckProcessData()
{
  GLOBAL.joy_x_axis = constrain(int16_t(Nunchuck.joyx() - joy_x_axis_Offset), -100, 100); //gets us to +- 100
  GLOBAL.joy_y_axis = constrain(int16_t(Nunchuck.joyy() - joy_y_axis_Offset), -100, 100); //gets us to +- 100
  GLOBAL.acc_x_axis = constrain(int16_t(Nunchuck.accelx() - acc_x_axis_Offset), -100, 100); //gets us to +- 100
  if (SETTINGS.AUX_REV) GLOBAL.acc_x_axis *= -1;
  if (GLOBAL.joy_y_axis) GLOBAL.joy_y_axis *= -1;

  //create a deadband
  uint8_t deadband = 7; // results in 100-7 or +-93 - this is for the joystick
  uint8_t deadband2 = 100; //  this is for the accelerometer

  GLOBAL.joy_x_axis = NunchuckDeadband(GLOBAL.joy_x_axis, deadband);
  GLOBAL.joy_y_axis = NunchuckDeadband(GLOBAL.joy_y_axis, deadband);
  GLOBAL.acc_x_axis = NunchuckDeadband(GLOBAL.acc_x_axis, deadband2);

  //check for joystick y lock for more than one second
  if (abs(GLOBAL.joy_y_axis) > 83)	  GLOBAL.joy_y_lock_count++;
  else						                    GLOBAL.joy_y_lock_count = 0;
  if (GLOBAL.joy_y_lock_count > 250)  GLOBAL.joy_y_lock_count = 250; //prevent overflow

  //check for joystick x lock for more than one second
  if (abs(GLOBAL.joy_x_axis) > 83)	  GLOBAL.joy_x_lock_count++;
  else						                    GLOBAL.joy_x_lock_count = 0;
  if (GLOBAL.joy_x_lock_count > 250)  GLOBAL.joy_x_lock_count = 250; //prevent overflow

  ButtonState = (Nunchuck.zbutton() << 1) | Nunchuck.cbutton();
}


void NunChuckClearData()
{
  GLOBAL.joy_x_axis   = 0;
  GLOBAL.joy_y_axis   = 0;
  GLOBAL.acc_x_axis   = 0;
  GLOBAL.acc_y_axis   = 0;
  ButtonState = Read_Again;
}


void applyjoymovebuffer_exponential()  //exponential stuff
{
  //scale based on read frequency  base is 500 reads per second  - now 20 reads per second = 25x
  int32_t int_joy_x_axis   = (int32_t(GLOBAL.joy_x_axis) *   int32_t(GLOBAL.joy_x_axis) *   int32_t(GLOBAL.joy_x_axis)) >> 4;
  int32_t int_joy_y_axis   = (int32_t(GLOBAL.joy_y_axis) *   int32_t(GLOBAL.joy_y_axis) *   int32_t(GLOBAL.joy_y_axis)) >> 4;
  int32_t int_acc_x_axis   = (int32_t(GLOBAL.acc_x_axis) *   int32_t(GLOBAL.acc_x_axis) *   int32_t(GLOBAL.acc_x_axis)) >> 4;

  //slow down changes to avoid sudden stops and starts
  //uint8_t ss_buffer=100;
  int32_t buffer_x;
  int32_t buffer_y;
  int32_t buffer_z;

  //if ((joy_x_axis-prev_joy_x_reading)>ss_buffer) joy_x_axis=(prev_joy_x_reading+ss_buffer);
  //else if ((joy_x_axis-prev_joy_x_reading)<-ss_buffer) joy_x_axis=(prev_joy_x_reading-ss_buffer);

  buffer_x = (int_joy_x_axis - prev_joy_x_reading) / 5;
  int_joy_x_axis = prev_joy_x_reading + buffer_x;
  if (abs(int_joy_x_axis) < 5) int_joy_x_axis = 0;

  //if ((joy_y_axis-prev_joy_y_reading)>ss_buffer) joy_y_axis=(prev_joy_y_reading+ss_buffer);
  //else if ((joy_y_axis-prev_joy_y_reading)<-ss_buffer) joy_y_axis=(prev_joy_y_reading-ss_buffer);

  buffer_y = (int_joy_y_axis - prev_joy_y_reading) / 5;
  int_joy_y_axis = prev_joy_y_reading + buffer_y;
  if (abs(int_joy_y_axis) < 5) int_joy_y_axis = 0;

  //if ((acc_x_axis-prev_acc_x_reading)>ss_buffer) acc_x_axis=(prev_acc_x_reading+ss_buffer);
  //else if ((acc_x_axis-prev_acc_x_reading)<-ss_buffer) acc_x_axis=(prev_acc_x_reading-ss_buffer);

  buffer_z = (int_acc_x_axis - prev_acc_x_reading) / 2;
  int_acc_x_axis = prev_acc_x_reading + buffer_z;
  if (abs(int_acc_x_axis) < 5) int_acc_x_axis = 0;

  //Serial.print(joy_x_axis);Serial.print(" ___ ");Serial.println(joy_y_axis);

  prev_joy_x_reading = int_joy_x_axis;
  prev_joy_y_reading = int_joy_y_axis;
  prev_acc_x_reading = int_acc_x_axis;

  int32_t x = int_joy_x_axis   + EEPROM_STORED.current_steps.x;
  int32_t y = int_joy_y_axis   + EEPROM_STORED.current_steps.y;
  int32_t z = int_acc_x_axis + EEPROM_STORED.current_steps.z;
  if (SETTINGS.AUX_ON) set_target(x, y, z);
  else		set_target(x, y, 0);
  GLOBAL.feedrate_micros = calculate_feedrate_delay_2();
}


void applyjoymovebuffer_linear()
{
  //control max speeds of the axis
  GLOBAL.joy_y_axis = map(GLOBAL.joy_y_axis, -90, 90, -35, 35); //

  //slow down changes to avoid sudden stops and starts

  uint8_t ss_buffer = 1;

  if ((GLOBAL.joy_x_axis - prev_joy_x_reading) > ss_buffer) GLOBAL.joy_x_axis = (prev_joy_x_reading + ss_buffer);
  else if ((GLOBAL.joy_x_axis - prev_joy_x_reading) < -ss_buffer) GLOBAL.joy_x_axis = (prev_joy_x_reading - ss_buffer);

  if ((GLOBAL.joy_y_axis - prev_joy_y_reading) > ss_buffer) GLOBAL.joy_y_axis = (prev_joy_y_reading + ss_buffer);
  else if ((GLOBAL.joy_y_axis - prev_joy_y_reading) < -ss_buffer) GLOBAL.joy_y_axis = (prev_joy_y_reading - ss_buffer);

  if ((GLOBAL.acc_x_axis - prev_acc_x_reading) > ss_buffer) GLOBAL.acc_x_axis = (prev_acc_x_reading + ss_buffer);
  else if ((GLOBAL.acc_x_axis - prev_acc_x_reading) < -ss_buffer) GLOBAL.acc_x_axis = (prev_acc_x_reading - ss_buffer);

  //Serial.print(GLOBAL.joy_x_axis);Serial.print(" ___ ");Serial.println(GLOBAL.joy_y_axis);

  prev_joy_x_reading   = GLOBAL.joy_x_axis;
  prev_joy_y_reading   = GLOBAL.joy_y_axis;
  prev_acc_x_reading   = GLOBAL.acc_x_axis;

  int32_t x = GLOBAL.joy_x_axis   + EEPROM_STORED.current_steps.x;
  int32_t y = GLOBAL.joy_y_axis   + EEPROM_STORED.current_steps.y;
  int32_t z = GLOBAL.acc_x_axis + EEPROM_STORED.current_steps.z;

  set_target(x, y, z);
  GLOBAL.feedrate_micros = calculate_feedrate_delay_2();
}


void nc_sleep()
{
  if (abs(GLOBAL.joy_x_axis) > 15 || abs(GLOBAL.joy_y_axis) > 15)  digitalWrite(MOTOR_EN, LOW);
  else																				                     digitalWrite(MOTOR_EN, HIGH);
}


int8_t joy_capture2() //captures joystick input
{
  GLOBAL.prompt_delay = (500 - 5 * abs(GLOBAL.joy_y_axis)); // The joystick is constrained to +-100 so this should never equal below 0
  return -1 * constrain(map(GLOBAL.joy_y_axis, -20, 20, -1, 2), -1, 1);
}


int8_t joy_capture3() //captures joystick input and conditions it for UI
{
  if (GLOBAL.joy_y_lock_count > 245) { //really really fast
    GLOBAL.prompt_delay = 0;
    return -1 * map(GLOBAL.joy_y_axis, -55, 55, -40, 40);
  }
  else if (GLOBAL.joy_y_lock_count > 50) { //really fast
    GLOBAL.prompt_delay = 10;
    return -1 * map(GLOBAL.joy_y_axis, -100, 100, -10, 10);
  }
  else if (GLOBAL.joy_y_lock_count > 20) { //pretty fast
    GLOBAL.prompt_delay = 70;
    return -1 * map(GLOBAL.joy_y_axis, -80, 80, -10, 10);
  }
  else if (abs(GLOBAL.joy_y_axis) > 10) { //go variable add delay which we run later
    GLOBAL.prompt_delay = (500 - 5 * abs(GLOBAL.joy_y_axis)); // The joystick is constrained to +-100 so this should never equal below 0
    return -1 * constrain(map(GLOBAL.joy_y_axis, -20, 20, -1, 2), -1, 1);
  }
  else { //null loop
    GLOBAL.prompt_delay = 0;
    return (0);
  }
}


int8_t joy_capture_4() //captures joystick input and conditions it for UI
{
  if (GLOBAL.joy_y_lock_count > 100) { //really really fast
    GLOBAL.prompt_delay = 0;
    return -1 * map(GLOBAL.joy_y_axis, -55, 55, -40, 40);
  }
  else if (GLOBAL.joy_y_lock_count > 50) { //really fast
    GLOBAL.prompt_delay = 10;
    return -1 * map(GLOBAL.joy_y_axis, -100, 100, -10, 10);
  }
  else if (GLOBAL.joy_y_lock_count > 20) { //pretty fast
    GLOBAL.prompt_delay = 70;
    return -1 * map(GLOBAL.joy_y_axis, -80, 80, -10, 10);
  }
  else if (abs(GLOBAL.joy_y_axis) > 10) { //go variable add delay which we run later
    GLOBAL.prompt_delay = (500 - 5 * abs(GLOBAL.joy_y_axis)); // The joystick is constrained to +-100 so this should never equal below 0
    return -1 * constrain(map(GLOBAL.joy_y_axis, -20, 20, -1, 2), -1, 1);
  }
  else { //null loop
    GLOBAL.prompt_delay = 0;
    return (0);
  }
}


int joy_capture_x_map() //captures joystick input for left and right
{
  return map(GLOBAL.joy_x_axis, -85, 85, -1, 1);
}


int8_t joy_capture_y_map() //captures joystick input for up down
{
  return map(GLOBAL.joy_y_axis, -85, 85, -1, 1);
}


int joy_capture_x3() //captures joystick input and conditions it for UI
{
  if (GLOBAL.joy_x_lock_count > 245) { //really really fast
    GLOBAL.prompt_delay = 0;
    return -1 * map(-GLOBAL.joy_x_axis, -55, 55, -40, 40);
  }
  else if (GLOBAL.joy_x_lock_count > 50) { //really fast
    GLOBAL.prompt_delay = 10;
    return -1 * map(-GLOBAL.joy_x_axis, -100, 100, -10, -10);
  }
  else if (GLOBAL.joy_x_lock_count > 20) { //pretty fast
    GLOBAL.prompt_delay = 70;
    return -1 * map(-GLOBAL.joy_x_axis, -80, 80, -10, 10);
  }
  else if (abs(GLOBAL.joy_x_axis) > 10) { //go variable add delay which we run later
    GLOBAL.prompt_delay = (500 - 5 * abs(GLOBAL.joy_x_axis)); // The joystick is constrained to +-100 so this should never equal below 0
    return -1 * constrain(map(-GLOBAL.joy_x_axis, -20, 20, -1, 2), -1, 1);
  }
  else { //null loop
    GLOBAL.prompt_delay = 0;
    return (0);
  }
}


void updateMotorVelocities2()   //Happens  20 times a second
{
  //limit speeds
  //uint32_t motormax0 = SETTINGS.PAN_MAX_JOG_STEPS_PER_SEC;
  //uint32_t motormax1 = SETTINGS.TILT_MAX_JOG_STEPS_PER_SEC;
  //uint32_t motormax2 = SETTINGS.AUX_MAX_JOG_STEPS_PER_SEC;
  //if (motormax2 > 0.75) motormax2 = .75; //limits max speed during jog to reduce vibration

  //accelerations - accumulator limit is is 65553. Loop is 20Hz.   If we want zero to max to be 1 sec, we choose
  //example 1 If we want zero to max to be 1 sec, we choose (65535/2)/10 =3276 this is the max per cycle.
  //example 2 If we want zero to max to be 2 sec, we choose (65535/2)/20 =1638 this is the max per cycle.

  uint16_t accelmax0 = (65535 / 2) / 5;
  uint16_t accelmax1 = (65535 / 2) / 5;
  uint16_t accelmax2 = (65535 / 2) / 5;
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
  int32_t signedMotorMoveSpeedTarget0 = (int32_t(GLOBAL.joy_x_axis) * int32_t(GLOBAL.joy_x_axis) * int32_t(GLOBAL.joy_x_axis)) >> 4;
  int32_t signedMotorMoveSpeedTarget1 = (int32_t(GLOBAL.joy_y_axis) * int32_t(GLOBAL.joy_y_axis) * int32_t(GLOBAL.joy_y_axis)) >> 4;
  int32_t signedMotorMoveSpeedTarget2 = (int32_t(GLOBAL.acc_x_axis) * int32_t(GLOBAL.acc_x_axis) * int32_t(GLOBAL.acc_x_axis)) >> 4;

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
    if (motors[mot].nextMotorMoveSpeed) bitSet(FLAGS.motorMoving, mot);
    else								                bitClear(FLAGS.motorMoving, mot);
    //Serial.print("FLAGS.motorMoving:");Serial.println(FLAGS.motorMoving);
  }

  motors[0].dir = (signedMotorMoveSpeedTarget0 > 0) ? 1 : 0;
  motors[1].dir = (signedMotorMoveSpeedTarget1 > 0) ? 1 : 0;
  motors[2].dir = (signedMotorMoveSpeedTarget2 > 0) ? 1 : 0;

  //don't write digital pins here - allow interrupt loop to do it
  //digitalWrite(motors[0].dirPin, motors[0].dir);
  //digitalWrite(motors[1].dirPin, motors[1].dir);
  //digitalWrite(motors[2].dirPin, motors[2].dir);

  *motorAccumulator[0] = 65535;
  *motorAccumulator[1] = 65535;
  *motorAccumulator[2] = 65535;

  //This is just to get us into the loop in the interrupt for each motor to check the test.
  motorMoveSteps0 = 32000;
  motorMoveSteps1 = 32000;
  motorMoveSteps2 = 32000;

  nextMoveLoaded = true;
}
