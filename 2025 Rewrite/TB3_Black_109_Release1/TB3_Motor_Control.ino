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

void UpdatePowersave(const enum Powersave threshold){
  if (config.POWERSAVE_PT >= threshold)  { disable_PT(); } 
  else                                   { enable_PT();  }

  if (config.POWERSAVE_AUX >= threshold) { disable_AUX(); } 
  else if (config.AUX_ON)                { enable_AUX();  }
}

void WaitForMotorStop() {
  ClearNunChuck(); // zero our joystick axis's
  do  //run this loop until the motors stop
  {
    if (!nextMoveLoaded) {
      updateMotorVelocities2();
    }
  } while (motorMoving);
  stopISR1();
}

void update2PTState() {
    //need this routine for both 2 and 3 point moves since 3 points can use this logic to determine aux motor progress
  if (config.camera_fired < config.keyframe[0][1])      { program_progress_2PT = MoveProgress2PT::Lead_In;   }
  else if (config.camera_fired < config.keyframe[0][2]) { program_progress_2PT = MoveProgress2PT::Ramp_Up;   }
  else if (config.camera_fired < config.keyframe[0][3]) { program_progress_2PT = MoveProgress2PT::Linear;    }
  else if (config.camera_fired < config.keyframe[0][4]) { program_progress_2PT = MoveProgress2PT::Ramp_Down; }
  else if (config.camera_fired < config.keyframe[0][5]) { program_progress_2PT = MoveProgress2PT::Lead_Out;  }
  else                                                  { program_progress_2PT = MoveProgress2PT::Finished;  }
}

void goto_position() {
  UpdatePowersave(Powersave::Shoot_Minimum); // Need to Move
  
  switch(progtype)
  {
    case Menu_Options::REG2POINTMOVE:
    case Menu_Options::REV2POINTMOVE:
    case Menu_Options::AUXDISTANCE:
      update2PTState();
      if (config.shot_interval_time == VIDEO_EXPVAL) { move_motors_s_curve_video(); }
      else                                           { move_motors_s_curve();       }
      break;
    case Menu_Options::REG3POINTMOVE:
    case Menu_Options::REV3POINTMOVE:
      move_motors_catmull_spline();
      break;
    case Menu_Options::PORTRAITPANO:
      move_motors_accel_array();
      break;
    case Menu_Options::PANOGIGA:
      move_motors_pano();
      break;
    case Menu_Options::DFSLAVE:
    case Menu_Options::SETUPMENU:
    case Menu_Options::Count: {
      break;
    }
  }

  // //Start us moving
  // startISR1();
  // do {
  //   if (!nextMoveLoaded) {
  //     updateMotorVelocities();
  //   }
  // } while (motorMoving);
  // stopISR1();

  //inprogtype = Inprog_Options::INPROG_RESUME;
  //UpdatePowersave(Powersave::Programs);
  return;
}  //end goto routine


__attribute__ ((warn_unused_result))
int32_t motor_get_steps_2pt(const uint8_t motor) {
  //updated this and tested against 12 foot move
  int32_t steps = 0;

  switch (program_progress_2PT) {
    case MoveProgress2PT::Lead_In:  //Lead In - 0 Steps
      break;

    case MoveProgress2PT::Ramp_Up:  //RampUp
      steps = static_cast<int32_t>(static_cast<float>((config.camera_fired - config.lead_in) * config.linear_steps_per_shot[motor]) / static_cast<float>(config.rampval));
      break;

    case MoveProgress2PT::Linear:                                                                                                                                                   // Linear portion
      steps = static_cast<int32_t>(static_cast<float>(config.motor_steps_pt[2][motor] - config.ramp_params_steps[motor] - motors[motor].position) / static_cast<float>(config.keyframe[0][3] - config.camera_fired));  //  Point 2 in the end point
      break;

    case MoveProgress2PT::Ramp_Down:                                                                                                                       // RampDown
      steps = static_cast<int32_t>(static_cast<float>((config.motor_steps_pt[2][motor] - motors[motor].position) * 2) / static_cast<float>(config.keyframe[0][4] - config.camera_fired));  // Point 2 in the end point for 2 point move
      break;

    case MoveProgress2PT::Lead_Out:  //Lead Out
      break;

    case MoveProgress2PT::Finished:  //5 - finished
      break;
  }
  return steps;
}


void move_motors_s_curve_video() {
  static uint32_t last_update_time = millis(); // Track the last update time
  uint16_t image_total = config.camera_total_shots - config.lead_in - config.lead_out;
  uint16_t image_num = min(max(int16_t(config.camera_fired - config.lead_in), 0), image_total);
  uint16_t image_ramp = config.rampval;

  // Cap the image range to between lead in and lead out frames to simplify computation
  if (REVERSE_PROG_ORDER) {
    image_num = image_total - image_num;
  }

  int32_t Target[MOTORS] = {0, 0, 0};
  uint32_t current_time = micros(); // Get current time
  float elapsed_time = static_cast<float>(current_time - last_update_time) / static_cast<float>(config.move_time);

  // Scale the position dynamically based on elapsed time
  float scaled_image_num = static_cast<float>(image_num) + (elapsed_time * 0.5f);

  for (uint8_t i = 0; i < MOTORS; i++) {
    if (scaled_image_num < image_ramp) {
      float scaled_image_num_sq = 0.5f * scaled_image_num * scaled_image_num;
      Target[i] = static_cast<int32_t>(config.s_curve_accel[i] * scaled_image_num_sq);
    } else if (scaled_image_num < image_total - image_ramp) {
      Target[i] = static_cast<int32_t>(config.s_curve_d_ramp[i] +
                                   config.s_curve_max_vel[i] * (scaled_image_num - image_ramp));
    } else {
      float remain_sq = 0.5f * (image_total - scaled_image_num) * (image_total - scaled_image_num);
      Target[i] = static_cast<int32_t>(config.motor_steps_pt[1][i] - config.s_curve_accel[i] * remain_sq);
    }
  }

  // Update the motor position
  // for (uint8_t i = 0; i < MOTORS; i++) {
  //   Target[i] = config.motor_steps_pt[1][i] - Target[i];
  // }
  setPulsesPerSecond(config.MAX_JOG_STEPS_PER_SEC);
  setupMotorMove(Target);
  updateMotorVelocities();

  // Update the last update time
  last_update_time = current_time;

  return;
}



void compute_s_curve() {
  update2PTState();
  uint16_t N_total = config.camera_total_shots - config.lead_in - config.lead_out;
  uint16_t N_ramp = config.rampval;

  for (uint8_t i = 0; i < MOTORS; i++) {
    int32_t d_total = config.motor_steps_pt[1][i] - config.motor_steps_pt[0][i];

    // Calculate max velocity
    if (2 * N_ramp >= N_total) {
      N_ramp = N_total / 2;
      config.s_curve_max_vel[i] = 0.0f;
      config.s_curve_accel[i] = 4.0f * static_cast<float>(d_total) / static_cast<float>(N_total * N_total);
    }
    else {
      // Derived parameters
      uint32_t t_linear = N_total - 2 * N_ramp;  // Time for linear phase (number of linear images)

      // Calculate ramp and linear distances
      config.s_curve_d_ramp[i] = static_cast<float>(d_total) / 2.0f * (static_cast<float>(N_ramp) / static_cast<float>(N_ramp + t_linear));  // Equal ramp up and ramp down
      float d_linear = d_total - 2 * config.s_curve_d_ramp[i];

      config.s_curve_max_vel[i] = d_linear / static_cast<float>(t_linear);  // Max velocity during linear phase
      config.s_curve_accel[i] = config.s_curve_max_vel[i] / static_cast<float>(N_ramp);  // Acceleration per image during ramp phase
    }
  }
}


void move_motors_s_curve() {
  uint16_t image_total = config.camera_total_shots - config.lead_in - config.lead_out;
  uint16_t image_num = min(max(int16_t(config.camera_fired - config.lead_in), 0), image_total);
  uint16_t image_ramp = config.rampval;

  // cap the image range to between lead in and lead out frames to simplify computation
  if (REVERSE_PROG_ORDER) { image_num = image_total - image_num; }

  int32_t Target[MOTORS] = { 0, 0, 0 };
  if (image_num < image_ramp) {
    float image_num_sq = 0.5f * static_cast<float>(image_num) * static_cast<float>(image_num);
    for (uint8_t i = 0; i < MOTORS; i++) {
      Target[i] = static_cast<int32_t>(config.s_curve_accel[i] * image_num_sq);
    }
  }
  else if (image_num < image_total - image_ramp ) {
    for (uint8_t i = 0; i < MOTORS; i++) {
      Target[i] = static_cast<int32_t>(config.s_curve_d_ramp[i] +  config.s_curve_max_vel[i] * static_cast<float>(image_num - image_ramp));
    }
  }
  else {
    float image_remain_sq = 0.5f * static_cast<float>(image_total - image_num) * static_cast<float>(image_total - image_num);
    for (uint8_t i = 0; i < MOTORS; i++) {
      Target[i] = static_cast<int32_t>(config.motor_steps_pt[1][i] - config.s_curve_accel[i] * image_remain_sq);
    }
  }

  if (P2PAccelMove) {
    for (uint8_t i = 0; i < MOTORS; i++) {
      Target[i] = config.motor_steps_pt[1][i] - Target[i];
    }
    setPulsesPerSecond(config.MAX_JOG_STEPS_PER_SEC);
    setupMotorMove(Target);
    updateMotorVelocities();
  }
  else if (config.shot_interval_time == EXTTRIG_EXPVAL) {
    stopISR1();
    set_target(Target);
    uint32_t max_move = 0;
    for (uint8_t i = 0; i < MOTORS; i++) {
      max_move = max(max_move, uabs(motors[i].destination - motors[i].position));
    }
    dda_move(100000 / max_move);
  }
  else {
    stopISR1();
    set_target(Target);
    dda_move(calculate_feedrate_delay_video());
  }
  return;
}  //end move motors accel


void move_motors_catmull_spline() {
  uint16_t image_total = config.camera_total_shots - config.lead_in - config.lead_out;
  uint16_t image_num = min(max(int16_t(config.camera_fired - config.lead_in), 0), image_total);

  // cap the image range to between lead in and lead out frames to simplify computation
  if (REVERSE_PROG_ORDER) { image_num = image_total - image_num; }

  // spline stuff
  int32_t Target[3] = { 0, 0, 0 };
  for (uint8_t i = 0; i < MOTORS; i++) {
    Target[i] = config.motor_steps_pt[MAX_MOVE_POINTS - 1][i] - motor_get_steps_3pt(i);
  }
  setPulsesPerSecond(config.MAX_JOG_STEPS_PER_SEC);
  setupMotorMove(Target);

  updateMotorVelocities();
  return;
}  //end move motors accel


__attribute__ ((warn_unused_result))
int32_t motor_get_steps_3pt(const uint8_t motor) {
  int32_t steps = 0;
  if (config.camera_fired < config.keyframe[1][0]) {} // Lead In
  else if (config.camera_fired > config.keyframe[1][MAX_MOVE_POINTS - 1]) {} // Lead Out
  else {
    // Determine the current segment based on camera_fired and keyframes
    uint8_t segment = 0;
    for (uint8_t i = 0; i < MAX_MOVE_POINTS - 1; i++) {
        if (config.camera_fired < config.keyframe[1][i + 1]) {
            segment = i;
            break;
        }
    }

    // Calculate percentage within the segment
    float percent = static_cast<float>(config.camera_fired - config.keyframe[1][segment]) /
                    static_cast<float>(config.keyframe[1][segment + 1] - config.keyframe[1][segment]);

    // Dynamically determine control points with ternary operators
    int32_t p0 = (segment == 0) ? config.motor_steps_pt[segment + 1][motor] : config.motor_steps_pt[segment - 1][motor];
    int32_t p1 = config.motor_steps_pt[segment][motor];
    int32_t p2 = config.motor_steps_pt[segment + 1][motor];
    int32_t p3 = (segment == MAX_MOVE_POINTS - 2) ? config.motor_steps_pt[segment][motor] : config.motor_steps_pt[segment + 2][motor];

    // Compute steps using Catmull-Rom interpolation
    steps = catmullrom(percent, p0, p1, p2, p3);
  }
  return steps;
}


__attribute__ ((const))
__attribute__ ((warn_unused_result))
int32_t catmullrom(
  const float interpolationFactor, 
  const int32_t previousPoint, 
  const int32_t startPoint, 
  const int32_t endPoint, 
  const int32_t nextPoint
  ) {
  
  // Precompute integer terms
  const int32_t a = 2 * startPoint;
  const int32_t b = -previousPoint + endPoint;
  const int32_t c = 2 * previousPoint - 5 * startPoint + 4 * endPoint - nextPoint;
  const int32_t d = -previousPoint + 3 * startPoint - 3 * endPoint + nextPoint;

  // Compute the result with minimal casting
  return static_cast<int32_t>( 0.5f * (
      static_cast<float>(a) +
      static_cast<float>(b) * interpolationFactor +
      static_cast<float>(c) * interpolationFactor * interpolationFactor +
      static_cast<float>(d) * interpolationFactor * interpolationFactor * interpolationFactor
  ));
}


__attribute__ ((warn_unused_result))
int32_t motor_get_steps_2pt_video(const uint8_t motor) {
  //updated this and tested against 12 foot move with and inch or so of travel with a 50:1 gear ration - needed to get above 32000 steps before overflow
  int32_t steps = 0;

  switch (program_progress_2PT) {
    case MoveProgress2PT::Lead_In: // Lead In
      break;

    case MoveProgress2PT::Ramp_Up: // RampUp
      steps = static_cast<int32_t>(static_cast<float>((config.camera_fired - config.lead_in) * config.linear_steps_per_shot[motor]) / static_cast<float>(config.rampval));
      break;

    case MoveProgress2PT::Linear:  // Linear portion
      steps = (config.motor_steps_pt[2][motor] - config.ramp_params_steps[motor] - motors[motor].position);  //  This is total steps of the linear portion.  End point  - ramp down-where we are
      break;

    case MoveProgress2PT::Ramp_Down: // RampDown
      steps = static_cast<int32_t>(static_cast<float>((config.motor_steps_pt[2][motor] - motors[motor].position) * 2) / static_cast<float>(config.keyframe[0][4] - config.camera_fired));  // Point 2 in the end point for 2 point move
      break;

    case MoveProgress2PT::Lead_Out:  // Lead Out
      break;

    case MoveProgress2PT::Finished:  // Finished
      break;
  }
  return steps;
}

void go_to_origin_max_speed()  // interrupt routine
{
  synched3PtMove_origin();
  startISR1();
  do {
    if (!nextMoveLoaded) {
      updateMotorVelocities();
    }
  } while (motorMoving);
  stopISR1();
}



void go_to_origin_slow()  // interrupt routine
{
  synched3PtMove_origin();
  startISR1();
  do {
    if (!nextMoveLoaded) {
      updateMotorVelocities();
    }
  } while (motorMoving);
  stopISR1();
}

void DisplayMove(const uint8_t motorIndex)  //ca
{
  Motor *motor = &motors[motorIndex];
  for (uint8_t i = 0; i < 5; i++) {
    Serial.print('M');
    Serial.print(motorIndex);
    Serial.print("Seg:");
    Serial.print(i);
    Serial.print("T:");
    Serial.print(motor->moveTime[i]);
    Serial.print(',');
    Serial.print("P:");
    Serial.print(motor->movePosition[i]);
    Serial.print(',');
    Serial.print("V:");
    Serial.print(motor->moveVelocity[i]);
    Serial.print(',');
    Serial.print("A:");
    Serial.print(motor->moveAcceleration[i]);
    Serial.print(',');
    Serial.print("Dest:");
    Serial.print(motor->destination);
    Serial.println(' ');
  }
}
