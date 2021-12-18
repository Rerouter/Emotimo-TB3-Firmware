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

//BEGIN STEPPER SECTION

#define PIN_ON(port, pin)  { port |= pin; }
#define PIN_OFF(port, pin) { port &= ~pin; }

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //Mega

#define MOTOR0_STEP_PORT PORTE
#define MOTOR0_STEP_PIN  B00001000 //Pin 5  PE3

#define MOTOR1_STEP_PORT PORTH
#define MOTOR1_STEP_PIN  B00001000 //Pin 6  PH3

#define MOTOR2_STEP_PORT PORTH
#define MOTOR2_STEP_PIN  B00010000 //Pin 7  PH4

#else
#define MOTOR0_STEP_PORT PORTD
#define MOTOR0_STEP_PIN  B00100000 //Pin 5

#define MOTOR1_STEP_PORT PORTD
#define MOTOR1_STEP_PIN  B01000000 //Pin 6

#define MOTOR2_STEP_PORT PORTD
#define MOTOR2_STEP_PIN  B10000000 //Pin 7
#endif

//Stepper Setup

//our direction vars
bool x_direction = 1;
bool y_direction = 1;
bool z_direction = 1;

//End setup of Steppers

//init our variables
long max_delta;
long x_counter;
long y_counter;
long z_counter;
bool x_can_step;
bool y_can_step;
bool z_can_step;
uint16_t milli_delay;


void init_steppers()
{
  //turn them off to start.
  disable_PT();  //  low, standard, high, we power down at the end of program
  disable_AUX();  // low, standard, high, we power down at the end of program
  calculate_deltas();
}

void dda_move(long micro_delay)
{
  //enable our steppers
  if (SETTINGS.AUX_ON) enable_AUX();
  enable_PanTilt();

  //figure out our deltas
  max_delta = max(GLOBAL.delta_steps.x, GLOBAL.delta_steps.y);
  max_delta = max(GLOBAL.delta_steps.z, max_delta);

  //init stuff.
  long x_counter = -max_delta / 2;
  long y_counter = -max_delta / 2;
  long z_counter = -max_delta / 2;

  //our step flags
  bool x_can_step = 0;
  bool y_can_step = 0;
  bool z_can_step = 0;

  if (micro_delay >= 16383)	milli_delay = micro_delay / 1000;
  else						milli_delay = 0;

  //do our DDA line!
  do
  {
    x_can_step = can_step(EEPROM_STORED.current_steps.x, GLOBAL.target_steps.x);
    y_can_step = can_step(EEPROM_STORED.current_steps.y, GLOBAL.target_steps.y);
    z_can_step = can_step(EEPROM_STORED.current_steps.z, GLOBAL.target_steps.z);

    if (x_can_step)
    {
      x_counter += GLOBAL.delta_steps.x;

      if (x_counter > 0)
      {
        //do_step(MOTOR0_STEP);
        PIN_ON(MOTOR0_STEP_PORT, MOTOR0_STEP_PIN);
        x_counter -= max_delta;

        if (x_direction)	EEPROM_STORED.current_steps.x++;
        else				EEPROM_STORED.current_steps.x--;
      }
    }

    if (y_can_step)
    {
      y_counter += GLOBAL.delta_steps.y;
      if (y_counter > 0)
      {
        //do_step(MOTOR1_STEP);
        PIN_ON(MOTOR1_STEP_PORT, MOTOR1_STEP_PIN);
        y_counter -= max_delta;

        if (y_direction)	EEPROM_STORED.current_steps.y++;
        else				EEPROM_STORED.current_steps.y--;
      }
    }

    if (z_can_step)
    {
      z_counter += GLOBAL.delta_steps.z;
      if (z_counter > 0)
      {
        //do_step(MOTOR2_STEP);
        PIN_ON(MOTOR2_STEP_PORT, MOTOR2_STEP_PIN);
        z_counter -= max_delta;

        if (z_direction)	EEPROM_STORED.current_steps.z++;
        else				EEPROM_STORED.current_steps.z--;
      }
    }

    PIN_OFF(MOTOR0_STEP_PORT, MOTOR0_STEP_PIN);
    PIN_OFF(MOTOR1_STEP_PORT, MOTOR1_STEP_PIN);
    PIN_OFF(MOTOR2_STEP_PORT, MOTOR2_STEP_PIN);

    //wait for next step.
    if (milli_delay > 0)	delay(milli_delay);
    else					delayMicroseconds(micro_delay);
  }

  while (x_can_step || y_can_step || z_can_step);

  //set our points to be the same
  EEPROM_STORED.current_steps.x = GLOBAL.target_steps.x;
  EEPROM_STORED.current_steps.y = GLOBAL.target_steps.y;
  EEPROM_STORED.current_steps.z = GLOBAL.target_steps.z;
  calculate_deltas();
}


bool can_step(long current, long target)
{
  //stop us if we're on target
  if (target == current) return false;

  //default to being able to step
  return true;
}


void set_target(int32_t x, int32_t y, int32_t z)
{
  GLOBAL.target_steps.x = x;
  GLOBAL.target_steps.y = y;
  GLOBAL.target_steps.z = z;

  motors[0].destination = x;
  motors[1].destination = y;
  motors[2].destination = z;

  calculate_deltas();
}


void set_position(int32_t x, int32_t y, int32_t z)
{
  EEPROM_STORED.current_steps.x = x;
  EEPROM_STORED.current_steps.y = y;
  EEPROM_STORED.current_steps.z = z;

  motors[0].position = x;
  motors[1].position = y;
  motors[2].position = z;

  calculate_deltas();
}


void calculate_deltas()
{
  GLOBAL.delta_steps.x = abs(GLOBAL.target_steps.x - EEPROM_STORED.current_steps.x);
  GLOBAL.delta_steps.y = abs(GLOBAL.target_steps.y - EEPROM_STORED.current_steps.y);
  GLOBAL.delta_steps.z = abs(GLOBAL.target_steps.z - EEPROM_STORED.current_steps.z);

  //what is our direction
  x_direction = (GLOBAL.target_steps.x >= EEPROM_STORED.current_steps.x);
  y_direction = (GLOBAL.target_steps.y >= EEPROM_STORED.current_steps.y);
  z_direction = (GLOBAL.target_steps.z >= EEPROM_STORED.current_steps.z);

  //set our direction pins as well
  digitalWrite(MOTOR0_DIR, x_direction);
  digitalWrite(MOTOR1_DIR, y_direction);
  digitalWrite(MOTOR2_DIR, z_direction);
}


long calculate_feedrate_delay_1()
{
  long master_steps = 0;

  //find the dominant axis.
  if (GLOBAL.delta_steps.x > GLOBAL.delta_steps.y)
  {
    if (GLOBAL.delta_steps.z > GLOBAL.delta_steps.x)	master_steps = GLOBAL.delta_steps.z;
    else								master_steps = GLOBAL.delta_steps.x;
  }
  else
  {
    if (GLOBAL.delta_steps.z > GLOBAL.delta_steps.y)	master_steps = GLOBAL.delta_steps.z;
    else								master_steps = GLOBAL.delta_steps.y;
  }

#if DEBUG_MOTOR
  Serial.print("master_steps= ");
  Serial.print(master_steps);
  Serial.print(";");
#endif
  if (EEPROM_STORED.intval == VIDEO_INTVAL)
  {
    //return ((EEPROM_STORED.interval*(1000L))/master_steps); //   Use the full time for video - hardcoded to 1000 *50 mc or 50000us or 0.050 seconds
#if DEBUG_MOTOR
    Serial.print("feedratedelay_1_vid= ");
    Serial.print((EEPROM_STORED.interval * (1000L)) / master_steps);
    Serial.print(";");
#endif
    return ((EEPROM_STORED.interval * (1000L)) / master_steps); //  This is the issue - intervla
  }
  else if (EEPROM_STORED.intval == EXTTRIG_INTVAL)
  {
#if DEBUG_MOTOR
    Serial.print("feedratedelay_1_StopMo= ");
    Serial.print((((EEPROM_STORED.intval - EEPROM_STORED.static_tm) * 100000) / master_steps) * 0.5);
    Serial.print(";");
#endif
    return ((((10L) * 100000L) / master_steps) * 0.5); //  Use half available time to move for stills
  }
  else
  {
#if DEBUG_MOTOR
    Serial.print("feedratedelay_1_SMS=");
    Serial.print((((EEPROM_STORED.intval - EEPROM_STORED.static_tm - EEPROM_STORED.prefire_time) * 100000L) / master_steps) * 0.5);
    Serial.print(";");
#endif
    return (abs((((EEPROM_STORED.intval - EEPROM_STORED.static_tm - EEPROM_STORED.prefire_time) * 100000L) / master_steps) * 0.5)); //  Use half available time to move for stills
  }
}


long calculate_feedrate_delay_video()
{
  long master_steps = 0;
  long current_feedrate = 0;

  //find the dominant axis.
  if (GLOBAL.delta_steps.z > GLOBAL.delta_steps.x)
  {
    if (GLOBAL.delta_steps.y > GLOBAL.delta_steps.z)	master_steps = GLOBAL.delta_steps.y;
    else								master_steps = GLOBAL.delta_steps.z;
  }
  else
  {
    if (GLOBAL.delta_steps.y > GLOBAL.delta_steps.x)	master_steps = GLOBAL.delta_steps.y;
    else								master_steps = GLOBAL.delta_steps.x;
  }
#if DEBUG_MOTOR
  Serial.print("master_steps= ");
  Serial.print(master_steps);
  Serial.print(";");
#endif

  //return ((EEPROM_STORED.interval*(1000L))/master_steps); //

  if (Move_State_2PT == Linear2PT)  current_feedrate = ((EEPROM_STORED.interval * (VIDEO_FEEDRATE_NUMERATOR) * long(EEPROM_STORED.keyframe[0][3] - EEPROM_STORED.keyframe[0][2])) / master_steps); //  total move for all linear
  else							current_feedrate = ((EEPROM_STORED.interval * (VIDEO_FEEDRATE_NUMERATOR)) / master_steps); //  Use the full time for video - hardcoded to 1000 *50 mc or 50000us or 0.050 seconds or 20hz

#if DEBUG_MOTOR
  Serial.print("feedratedelay_1_vid= ");
  Serial.print(current_feedrate);
  Serial.print(";");
#endif
  return (current_feedrate);
}


long calculate_feedrate_delay_2() //used for real time moves
{
  long master_steps = 0;

  //find the dominant axis.
  if (GLOBAL.delta_steps.x > GLOBAL.delta_steps.y)
  {
    if (GLOBAL.delta_steps.z > GLOBAL.delta_steps.x) master_steps = GLOBAL.delta_steps.z;
    else							   master_steps = GLOBAL.delta_steps.x;
  }
  else
  {
    if (GLOBAL.delta_steps.z > GLOBAL.delta_steps.y) master_steps = GLOBAL.delta_steps.z;
    else							   master_steps = GLOBAL.delta_steps.y;
  }
  //Serial.print("master_steps="); Serial.println(master_steps);
  long fr = 10000L / master_steps;
  //Serial.print("fr="); Serial.println(fr);
  return (fr); // read about every 42 ms (24 times a second)
}


void disable_PT()
{
  digitalWrite(MOTOR_EN, HIGH);
}


void disable_AUX()
{
  digitalWrite(MOTOR_EN2, HIGH);
}


void enable_PanTilt()
{
  digitalWrite(MOTOR_EN, LOW);
}


void enable_AUX()
{
  digitalWrite(MOTOR_EN2, LOW);
}
