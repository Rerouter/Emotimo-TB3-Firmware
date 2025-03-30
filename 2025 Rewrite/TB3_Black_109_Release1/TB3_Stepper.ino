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

//our direction vars
int32_t delta_steps[MOTORS] = {0, 0, 0};

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)  // Mega

//Start TB3 Black Port Mapping
volatile uint8_t* MOTOR_STEP_PORT[MOTORS] = { &PORTE, &PORTH, &PORTH };
volatile uint8_t MOTOR_STEP_PIN[MOTORS] = { B00001000, B00001000, B00010000 };

volatile uint8_t* MOTOR_DIR_PORT[MOTORS] = { &PORTH, &PORTH, &PORTB };
volatile uint8_t MOTOR_DIR_PIN[MOTORS] = { B00100000, B01000000, B00010000 };
//End TB3 Black Port Mapping

#else  // Default case

volatile uint8_t MOTOR_STEP_PORT[MOTORS] = { PORTD, PORTD, PORTD };
volatile uint8_t MOTOR_STEP_PIN[MOTORS] = { B00100000, B01000000, B10000000 };

volatile uint8_t* MOTOR_DIR_PORT[MOTORS] = { &PORTB, &PORTB, &PORTB };
volatile uint8_t MOTOR_DIR_PIN[MOTORS] = { B00000001, B00000010, B00000100 };

#endif


void PIN_ON(volatile uint8_t* port, uint8_t pin) {
    *port |= pin;
}

void PIN_OFF(volatile uint8_t* port, uint8_t pin) {
    *port &= static_cast<uint8_t>(~pin);
}



void init_steppers() {
  //turn them off to start.
  disable_PT();   //  low, standard, high, we power down at the end of program
  disable_AUX();  // low, standard, high, we power down at the end of program
  calculate_deltas();
}

void dda_move(uint32_t step_delay_us) {
  //enable our steppers
  UpdatePowersave(Powersave::Shoot_Minimum); // Need to Move

  //figure out our deltas
  int32_t max_delta = max(max(delta_steps[0], delta_steps[1]), delta_steps[2]);
  uint16_t milli_delay = 0;
  uint16_t micro_remainder = 0;
  int32_t counter[MOTORS] = { -max_delta / 2, -max_delta / 2, -max_delta / 2 };
  bool dda_can_step[MOTORS] = { false, false, false };

  // Split micro_delay into milliseconds and microseconds
  if (step_delay_us > 1000) {
      milli_delay = static_cast<uint16_t>(step_delay_us / 1000); // Convert to milliseconds
      micro_remainder = static_cast<uint16_t>(step_delay_us % 1000); // Remainder in microseconds
  }
  else {
      micro_remainder = static_cast<uint16_t>(step_delay_us);
  }

  //do our DDA line!
  do {
    for (uint8_t i = 0; i < MOTORS; i++) {
      dda_can_step[i] = motors[i].position != motors[i].destination;

      if (dda_can_step[i]) {
        counter[i] += delta_steps[i];

        if (counter[i] > 0) {
          PIN_ON(MOTOR_STEP_PORT[i], MOTOR_STEP_PIN[i]);
          counter[i] -= max_delta;
          if (motors[i].dir) { motors[i].position++; }
          else               { motors[i].position--; }
        }
      }

      PIN_OFF(MOTOR_STEP_PORT[i], MOTOR_STEP_PIN[i]);
    }

    // Wait for the next step using the combined delay
    if (milli_delay > 0) {
        delay(milli_delay); // Millisecond delay
    }
    if (micro_remainder > 0) {
        delayMicroseconds(micro_remainder); // Remainder microsecond delay
    }
  } while (dda_can_step[0] || dda_can_step[1] || dda_can_step[2]);

  for (uint8_t i = 0; i < MOTORS; i++) { //set our points to be the same
    motors[i].position = motors[i].destination;
  }
  calculate_deltas();
}


void set_target(const int32_t steps[3]) {
  for (uint8_t i = 0; i < MOTORS; ++i) {
      motors[i].destination = steps[i];
  }
  calculate_deltas();
}


void set_position(const int32_t steps[3]) {
    for (uint8_t i = 0; i < MOTORS; ++i) {
        motors[i].position = steps[i];
    }
    calculate_deltas();
}


void set_home() {
    for (uint8_t i = 0; i < MOTORS; ++i) {
        motors[i].position = 0;
        motors[i].destination = 0;
    }
    calculate_deltas();
}


void calculate_deltas() {
  for (uint8_t i = 0; i < MOTORS; i++) {
    delta_steps[i] = uabs(motors[i].destination - motors[i].position);
    motors[i].dir = (motors[i].destination >= motors[i].position);    //what is our direction
    if (motors[i].dir) {
      PIN_ON(MOTOR_DIR_PORT[i], MOTOR_DIR_PIN[i]);
    }
    else {
      PIN_OFF(MOTOR_DIR_PORT[i], MOTOR_DIR_PIN[i]);  //set our direction pins as well
    }                    
  }
}


__attribute__ ((warn_unused_result))
uint32_t calculate_feedrate_delay_1() {
  int32_t max_delta = max( max(delta_steps[0], delta_steps[1]), delta_steps[2]);

  if (config.shot_interval_time == VIDEO_EXPVAL) {
    return ((IntervalTime() * 1000L) / max_delta);
  } 
  else if (config.shot_interval_time == EXTTRIG_EXPVAL) {
    return 1000000L / (max_delta * 2);  //  Use half available time to move for stills
  }
  else {
    return ((IntervalTime() - (ShutterTime() + PrefireTime() + PostfireTime())) * 1000L) / (max_delta * 2);  //  Use half available time to move for stills
  }
}


__attribute__ ((warn_unused_result))
int32_t calculate_feedrate_delay_video() {
  int32_t current_feedrate = 0;

  int32_t max_delta = max( max(delta_steps[0], delta_steps[1]), delta_steps[2]);
  if (program_progress_2PT == MoveProgress2PT::Linear) {
    current_feedrate = ((IntervalTime() * (VIDEO_FEEDRATE_NUMERATOR) * static_cast<int32_t>(config.keyframe[0][3] - config.keyframe[0][2])) / max_delta);  //  total move for all linear
  } 
  else {                                                                                                                                        //20 hz program
    current_feedrate = ((IntervalTime() * (VIDEO_FEEDRATE_NUMERATOR)) / max_delta);                                                        //  Use the full time for video - hardcoded to 1000 *50 mc or 50000us or 0.050 seconds or 20hz
  }
  return (current_feedrate);
}


__attribute__ ((warn_unused_result))
int32_t calculate_feedrate_delay_2()  //used for real time moves
{
  int32_t max_delta = max( max(delta_steps[0], delta_steps[1]), delta_steps[2]);
  return 10000L / max_delta;  // read about every 42 ms (24 times a second)
}


void disable_PT() {
  digitalWrite(MOTOR_EN, HIGH);
}


void disable_AUX() {
  digitalWrite(MOTOR_EN2, HIGH);
}

void enable_PT() {
  digitalWrite(MOTOR_EN, LOW);
}

void enable_AUX() {
  digitalWrite(MOTOR_EN2, LOW);
}
