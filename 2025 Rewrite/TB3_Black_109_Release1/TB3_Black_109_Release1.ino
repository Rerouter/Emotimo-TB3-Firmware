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
	
111 Target - Aux Distance with ability for continuous loop - shorten lean.
Add Stopmotion to Inshot menu
Add Brightness to in shot menu

109 Release Notes

-Fixed Aux reverse to work in all cases
-Added ability to reverse AUX_REV to EEPROM direction.
-Added interval change functionality to in shot menu - don't recommend this is used ever unless your shot is already ruined.  Changing anything mid shot will show.
-Added functionality to use left right to set frame to go to.  Can go forward or back.
-Relaxed tight requirements for joystick absolute centering - check
-Fixed motor feedrate issue when Static Time was maxed out.  If you are maxing out your static time by default, you are not using this setting correctly and hurting your shot!  
-Made Static time a max of Interval minus 0.3 seconds to allow at least a .15 second move - check
-Added abs on feedrate min calc to accommodate spurious overrun negatives on SMS shoots resulting in single long frame delays
-Added check on minimum for video to ensure we don't catch 3PT video moves on min calc.
-Added test against power policy for loop 52 (ext triggering)  
-Added ramping and new motor move to starts and ends (decoupled inputs)
-Added coordinated return to start and three axis moves.
-Updated the motorMoving to accurately assign this
-Broke up move profiles.  Added slow down routine to the move to start/move to end.
-Throttled the calc of the move to respect max jog speeds by axis.  If we hit this we indicates "Speed Limit" on video run screen.  If you hit this, lengthen move and/or decrease ramp
-Added to the Setup Menu the Motor Speed parameter - from 2000 to 20000 max to allow folks to tune.the speeds for AUX.  Pan and Tilt are hardcoded.
-Start delay cleaned up and fixed - now down to the second - also a bailout of CZ to get to 5 seconds so you aren't stuck with accidental long delays
-Add Going to End LCD prompt if heading there.
-Target, Go To End. This now works
-Focus on return to start method.  Pause parameters improved to prevent toggling - added CZ released to check for long holds and released.
-Added new Tab for TB3_InShootMenu - just pauses now and only from progstep 50 (regular SMS)
-Added return to start - just called the same routine from the repeat move at the end of the shot - finds start fine (0's) not sure
*/

/* 
 =========================================
Main Program
 =========================================
*/

#include "WiiNunchuck3.h"
#include "NHDLCD9.h"

NHDLCD9 lcd(4);  // desired pin, rows, cols   //BB for LCD

constexpr size_t MAX_STRING_LENGTH = 17;
char lcdbuffer1[MAX_STRING_LENGTH];  //16 Characters and a null byte

constexpr size_t constexpr_strlen(const char* str, size_t len = 0) {
  return str[len] == '\0' ? len : constexpr_strlen(str, len + 1);
}

// Macro to define strings in PROGMEM with compile-time safety
#define F_STR(value) ([]() -> const char* { \
  static const char _str[] PROGMEM = value; \
  return _str; \
}())

//Debug Flags
constexpr bool DEBUG = false;
constexpr bool DEBUG_MOTOR = false;
constexpr bool DEBUG_NC = false;
constexpr bool DEBUG_PANO = false;
constexpr bool DEBUG_GOTO = false;


constexpr bool POWERDOWN_LV = false;  //set this to cause the TB3 to power down below 10 volts
constexpr uint32_t VIDEO_FEEDRATE_NUMERATOR = 375;  // Set this for 42000L, or 375L for faster calc moves


//Main Menu Ordering
enum class Menu_Options : uint8_t {
    REG2POINTMOVE,
    REV2POINTMOVE,
    REG3POINTMOVE,
    REV3POINTMOVE,
    PANOGIGA,
    PORTRAITPANO,
    DFSLAVE,
    SETUPMENU,
    AUXDISTANCE, // Not currently Used
    Count, // Sentinel value to indicate the total number of options
};


//Portrait Pano
enum class Pano_Array_Type : uint8_t {
    PANO_9ShotCenter,
    PANO_25ShotCenter,
    PANO_7X3,
    PANO_9X5,
    PANO_5x5TopThird,
    PANO_7X5TopThird,
    Count, // Sentinel value to indicate the total number of options
};

void DisplayPanoArrayType(const enum Pano_Array_Type typeval);


//In Program Menu Ordering
enum class Inprog_Options : uint8_t {
    INPROG_RESUME,
    INPROG_RESTART,         //return to start
    INPROG_GOTO_END,    //Go to end
    INPROG_GOTO_FRAME,  //go to frame
    INPROG_EXPOSURE,    //Set Exposire Time
    INPROG_STOPMOTION,   //Manual Forward and Back
    Count, // Sentinel value to indicate the total number of options
};


//In Program Menu Ordering
enum class Powersave : uint8_t {
    Always,          // Motors Always On
    Programs,        // Motors on out of menu
    Shoot_Accuracy,  // Motors on and stay on while Shooting
    Shoot_Minimum,   // Motors only on to move the motors
    Count,
};

void DisplayPowerSave(const enum Powersave value);
void UpdatePowersave(const enum Powersave threshold);

enum class Repeat : uint8_t { 
    Run_Once,              // Run the sequence once
    Continuous_Loop,       // Repeat the sequence as soon as its finished
    Continuous_Alternate,  // Repeat with an alternating starting point
    Count,
};

void DisplaySequenceRepeatType(const enum Repeat value);

enum class MoveProgress2PT : uint8_t {
    Lead_In,
    Ramp_Up,
    Linear,
    Ramp_Down,
    Lead_Out,
    Finished,
};

enum class MoveProgress3PT : uint8_t {
    Lead_In,
    Leg,
    Lead_Out,
    Finished,
};


enum class ShootMoveState : uint8_t {
  Idle,
  Waiting_Trigger,  // If its set to external trigger, wait for the event
  Waiting_Release,  // Wait for the release of the external trigger,
  Waiting_Camera,   // Wait for the camera to complete
  Waiting_Moving,   // Wait while moving to next position
};

ShootMoveState PanoState = ShootMoveState::Idle;
ShootMoveState PT2State = ShootMoveState::Idle;

//Interval Options
constexpr uint8_t VIDEO_EXPVAL = 0;
constexpr uint8_t EXTTRIG_EXPVAL = 1;
constexpr uint16_t MIN_INTERVAL_STATIC_GAP = 3;  //min gap between interval and static time


//TB3 section - Black or Orange Port Mapping for Step pins on Stepper Page
constexpr uint8_t MOTORS = 3;

constexpr uint8_t motorStepPins[MOTORS] = { 5, 6, 7 };  // Step pins for motors
constexpr uint8_t motorDirPins[MOTORS] = { 8, 9, 10 };  // Direction pins for motors

constexpr uint8_t MOTOR_EN = A3;
constexpr uint8_t MOTOR_EN2 = 11;
constexpr uint8_t MS1 = A1;
constexpr uint8_t MS2 = A2;
constexpr uint8_t MS3 = A2;
constexpr uint8_t IO_2 = 2;                 // drives middle of 2.5 mm connector on I/O port
constexpr uint8_t IO_3 = 3;                 // drives tip of 2.5 mm connector on I/O port
constexpr float STEPS_PER_DEG = 444.444;  //160000 MS per 360 degees = 444.4444444

/*
STEPS_PER_INCH_AUX for various motors with 17 tooth final gear on 5mm pitch belt
Phidgets 99:1	95153
Phidgets 27:1	25676
Phidgets 5:1	4955
20:1 Ratio	19125
10:1 Ratio	9562
*/

constexpr uint16_t STEPS_PER_INCH_AUX = 19125;   //
constexpr int16_t MAX_AUX_MOVE_DISTANCE = 311;  //(31.1 inches)
//end TB3 section

constexpr uint32_t build_version = 10953;  //this value is compared against what is stored in EEPROM and resets EEPROM and setup values if it doesn't match
uint16_t start_delay_sec = 0;
int16_t aux_dist;

//External Interrupt Variables
int32_t shuttertimer_open = 0;
int32_t shuttertimer_close = 0;
bool ext_shutter_open = false;
int16_t ext_shutter_count = 0;
constexpr uint8_t hdr_shots = 1;  //this is how many shots are needed before moving - leave at one for normal shooting - future functionality

//Start of variables for Pano Mode
constexpr bool P2PAccelMove = true;  // 0 = no accel, 1= accel

//Program Status Flags
bool Shot_Sequence_Engaged = false;
bool Static_Time_Engaged = false;
bool IO_Engaged = false;
bool Interrupt_Fire_Engaged = false;

//New Powersave flags
/*Power Save explanation
We can power up and power down the Pan Tilt motors together.  We can power up and power down the Aux motor port as well.  We see three levels of power saving:
1)  None - Motors are always on - for VFX work where power isn't a factor and precision is most important.  Motors will get warm here on hot days.
2)  Low - only at the end of program 
3)  Standard - Power up the motors for the shooting time (all the time we hold the trigger down), and move, power down between shots.
4)  High - Only power on for motor moves, turn off the motors when we reach the shooting position.  
    We are powered down for the shot and only power on for moves. This saves a ton of battery for long astro shots.   
    We do lose microstep resolution for this, but it usually is not visible.   We could be off by as much as 8/16 mircosetps for a shot or 0.018 degrees - Really small stuff!  Try this mode out!
*/


constexpr uint8_t MAX_MOVE_POINTS = 3;
Menu_Options progtype = Menu_Options::REG2POINTMOVE;                // updownmenu selection

struct ConfigData {
  // EEPROM variables
  uint16_t shot_interval_time;   // x0.1s, wait time between shots
  uint32_t move_time;            // x1ms, how long we are moving for
  uint16_t overaldur;            // seconds now for video only

  uint16_t video_segments;

  uint16_t camera_fired;         // number of shots fired
  uint16_t camera_total_shots;   // used at the end target for camera fired to compare against

  uint16_t total_shots_x;     //calulated value for to divide up scene evenly
  uint16_t total_shots_y;     //calulated value for to divide up scene evenly
  int32_t linear_steps_per_shot[MOTORS];    // This is for the calculated or estimated steps per shot in a segment for each motor
  
  bool panoRowFirst;
  bool panoSerpentine;

  uint16_t rampval;              // Frames
  uint16_t lead_in;              // Frames
  uint16_t lead_out;             // Frames

  float s_curve_accel[MOTORS];
  float s_curve_max_vel[MOTORS];
  float s_curve_d_ramp[MOTORS];

  int32_t motor_steps_pt[MAX_MOVE_POINTS][MOTORS];  // 3 total points.   Start point is always 0
  uint16_t keyframe[2][6];                        // this is basically the keyframes {start, end of rampup, start or rampdown, end}   - doesn't vary by motor at this point
  // 2 Point moves [start (implicit), lead in, ramp up, linear, ramp down, lead out, end (implicit)]
  // 3 Point moves [start (implicit), lead in, leg 1, leg 2, lead out, end (imlicit)]
  int32_t ramp_params_steps[MOTORS];                // This is to calc the steps at the end of rampup for each motor.  Each array value is for a motor
  uint16_t MAX_JOG_STEPS_PER_SEC[MOTORS];  // value x 1000  20 is the top or 20000 steps per second.

  Powersave POWERSAVE_PT;               // 1=None - always on  2 - low   3=standard    4=High
  Powersave POWERSAVE_AUX;              // 1=None - always on  2 - low   3=standard    4=High
  bool AUX_ON;                        // 1=Aux Enabled, 0=Aux disabled
  bool AUX_REV;                       // 1=Aux Reversed, 0=Aux Normal
  bool PAUSE_ENABLED;                 // 1=Pause Enabled, 0=Pause disabled
  
  uint8_t LCD_BRIGHTNESS_DURING_RUN;  // 0 is off 8 is max
  uint8_t LCD_BRIGHTNESS_DURING_MENU; // 0 is off 8 is max

  // Centralized default initializer
  void setDefaults() {
    // Default variables
    shot_interval_time = 20;
    move_time = 0;
    overaldur = 20;

    video_segments = 150;   //arbitrary

    camera_fired = 0;
    camera_total_shots = 0;

    total_shots_x = 0;
    total_shots_y = 0;

    panoRowFirst = true;
    panoSerpentine = true;

    rampval = 50;
    lead_in = 1;
    lead_out = 1;

    for (uint8_t i = 0; i < MOTORS; i++) {
      for (uint8_t j = 0; j < MAX_MOVE_POINTS; j++) {
        motor_steps_pt[i][j] = 0;
      }
      linear_steps_per_shot[i] = 0;
      ramp_params_steps[i] = 0;
      MAX_JOG_STEPS_PER_SEC[i] = 10000;
      s_curve_accel[i] = 0;
      s_curve_max_vel[i] = 0;
      s_curve_d_ramp[i] = 0;
    }

    // Initialize keyframe
    for (uint8_t i = 0; i < 2; i++) {
      for (uint8_t j = 0; j < 6; j++) {
        keyframe[i][j] = 0;
      }
    }

    POWERSAVE_PT = Powersave::Programs;
    POWERSAVE_AUX = Powersave::Programs;
    AUX_ON = 1;
    AUX_REV = 0;
    PAUSE_ENABLED = 0;
    LCD_BRIGHTNESS_DURING_RUN = 3;
    LCD_BRIGHTNESS_DURING_MENU = 6;
  }
};

ConfigData config;

//CVariables that are set during the Setup Menu store these in EEPROM
bool REVERSE_PROG_ORDER = false;  //Program ordering 0=normal, start point first. 1=reversed, set end point first to avoid long return to start
bool MOVE_REVERSED_FOR_RUN = 0;



//control variable, no need to store in EEPROM - default and setup during shot
bool first_ui = true;             //variable to help with LCD dispay variable that need to show one time
bool second_ui = true;
uint8_t reviewprog = 1;

bool progstep_forward_dir = true;  //bool to define direction of menu travel to allow for easy skipping of menus
Inprog_Options inprogtype = Inprog_Options::INPROG_RESUME;    //updownmenu selection during shoot
MoveProgress2PT program_progress_2PT = MoveProgress2PT::Lead_In;  //Lead in, ramp, linear, etc for motor routine case statement
MoveProgress3PT program_progress_3PT = MoveProgress3PT::Lead_In;  //phase 1, phase 2

uint32_t interval_tm = 0;          //mc time to help with interval comparison
uint32_t interval_tm_last = 0;     //mc time to help with interval comparison
int16_t cursorpos = 1;                      //use 1 for left, 2 for right  - used for lead in, lead out

uint32_t move_last_time = 0;
constexpr uint32_t move_delay = 1000 / 20;

uint32_t display_last_tm = 0;
constexpr uint16_t screen_delay = 1000;      // in ms for delays of instructions
constexpr uint16_t update_delay = 350;      // in ms for delay after value change
int16_t scroll_delay = 0;          //to help with joystick reads and delays for inputs - this value is set during joystick read and executed later in the loop

uint32_t start_delay_tm = 0;  //ms timestamp to help with delay comparison
uint16_t goto_shot = 0;

Repeat sequence_repeat_type = Repeat::Run_Once;
uint8_t sequence_repeat_count = 0;  //counter to hold variable for how many time we have repeated

bool z_button = 0;
bool c_button = 0;
uint8_t joy_y_lock_count = 0;
uint8_t joy_x_lock_count = 0;
uint8_t CZ_Button_Read_Count = 0;
uint8_t C_Button_Read_Count = 0;
uint8_t Z_Button_Read_Count = 0;
uint8_t CZ_Count_Threshold = 20;

//Stepper Setup
uint32_t feedrate_micros = 0;

//End setup of Steppers

//Start of DF Vars
constexpr uint8_t DFMOCO_VERSION = 1;
constexpr char DFMOCO_VERSION_STRING[] = "1.2.6";


//eMotimo TB3 - Set this PINOUT_VERSION 3 for TB3 Orange (Uno)
//eMotimo TB3 - Set this PINOUT_VERSION 4 for TB3 Black (MEGA)
constexpr uint8_t PINOUT_VERSION = 4;

/*
  This is PINOUT_VERSION 1
  
  channel 5
        PIN  22   step
        PIN  23   direction
  channel 6
        PIN  24   step
        PIN  25   direction
  channel 7
        PIN  26   step
        PIN  27   direction
  channel 8
        PIN  28   step
        PIN  29   direction
*/

// update velocities 20 x second
constexpr uint16_t TIME_CHUNK = 50;
constexpr uint16_t VELOCITY_UPDATE_RATE = (50000 / TIME_CHUNK);

/*
 Motor data.
 */

volatile bool nextMoveLoaded;

volatile uint16_t velocityUpdateCounter;
volatile uint8_t sendPositionCounter;

uint8_t sendPosition = 0;  // bit array
uint8_t motorMoving = 0;   // bit array

constexpr uint8_t P2P_MOVE_COUNT = 7;

struct Motor {
  // pre-computed move
  float moveTime[P2P_MOVE_COUNT];
  int32_t movePosition[P2P_MOVE_COUNT];
  float moveVelocity[P2P_MOVE_COUNT];
  float moveAcceleration[P2P_MOVE_COUNT];

  float gomoMoveTime[P2P_MOVE_COUNT];
  int32_t gomoMovePosition[P2P_MOVE_COUNT];
  float gomoMoveVelocity[P2P_MOVE_COUNT];
  float gomoMoveAcceleration[P2P_MOVE_COUNT];

  uint8_t currentMove;
  float currentMoveTime;

  volatile bool dir;

  volatile int32_t position;
  int32_t destination;

  uint16_t moveMaxVelocity;      //Pass this into calculator for synchronized moves
  uint16_t moveMaxAcceleration;  //Pass this into calculator for synchronized moves

  uint16_t jogMaxVelocity;      //replaced the original maxVelocity
  uint16_t jogMaxAcceleration;  //replaced the original maxAcceleration

  uint32_t nextMotorMoveSteps;
  uint16_t nextMotorMoveSpeed;

  volatile uint16_t motorAccumulator;
  volatile uint32_t motorMoveSteps;
  volatile uint16_t motorMoveSpeed;
};

bool maxVelLimit = false;

bool goMoReady;
int goMoDelayTime;

Motor motors[MOTORS];

//End of DFVars


/* 
 =========================================
 Setup functions
 =========================================
*/



void setup() {
  lcd.setup();

    //Setup Motors
  init_steppers();
  DFSetup();
  stopISR1();

  //Setup External Trigger
  StartExternalTrigger();

  // Setup Serial
  Serial.begin(57600);

  // setup motor pins
  pinMode(motorStepPins[0], OUTPUT);
  pinMode(motorDirPins[0], OUTPUT);
  pinMode(motorStepPins[1], OUTPUT);
  pinMode(motorDirPins[1], OUTPUT);
  pinMode(motorStepPins[2], OUTPUT);
  pinMode(motorDirPins[2], OUTPUT);

  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);

  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, HIGH);

  pinMode(MOTOR_EN, OUTPUT);
  pinMode(MOTOR_EN2, OUTPUT);
  digitalWrite(MOTOR_EN, HIGH);   //LOW Enables output, High Disables
  digitalWrite(MOTOR_EN2, HIGH);  //LOW Enables output, High Disables

  SetupCameraPins();

  //Setup of I/0 Pings Start with output of I/Oport
  pinMode(IO_2, OUTPUT);
  pinMode(IO_3, OUTPUT);

  digitalWrite(IO_2, LOW);
  digitalWrite(IO_3, LOW);

  pinMode(A0, INPUT);  //this is for the voltage reading

  // Setup LCD
  lcd.bright(config.LCD_BRIGHTNESS_DURING_MENU);
  lcd.contrast(50);
  lcd.cursorOff();
  lcd.empty();

  draw(F_STR("Setup Complete"), 1, 2);  // Setup Complete
  draw(F_STR("TB3_109"), 2, 6);  // Version Number
  delay(screen_delay);
 
  config.setDefaults();
  saveConfigToEEPROM(config);

  // Setup EEPROM
  //Check to see if our hardcoded build version set in progam is different than what was last put in EEPROM - detect upgrade.
  if (build_version != check_version()) {
    config.setDefaults();
    saveConfigToEEPROM(config);
  } else {  //load last setting into memory - no upgrade
    loadConfigFromEEPROM(config);
  }

  lcd.empty();
  draw(F_STR("Connect Joystick"), 1, 1);

  // Setup Nunchuck
  Nunchuck.init(0);
  for (uint8_t reads = 1; reads <= 16; reads++) {
    Nunchuck.getData();
    lcd.at(2, reads, '+');
    if (abs(Nunchuck.joyx() - 127) > 35 || abs(Nunchuck.joyy() - 127) > 35) {
      draw(F_STR("Center Joystick "), 1, 1);
      reads = 1;
    }
  }
  calibrate_joystick(Nunchuck.joyx(), Nunchuck.joyy());
  lcd.empty();
}  //end of setup


enum class Menu : uint16_t { // progstep
    PT2_MainMenu      = 0,
    PT2_StartPoint    = 1,
    PT2_EndPoint      = 2,
    PT2_CamInterval   = 3,
    PT2_Duration      = 4,
    PT2_StaticTime    = 5,
    PT2_Ramp          = 6,
    PT2_LeadInLeadOut = 7,
    PT2_ShotRepeat    = 8,
    PT2_Reveiw        = 9,
    PT2_Gaurd         = 10,

    PT2_ShootLoop     = 50,
    PT2_VideoLoop     = 51,
    PT2_TriggerLoop   = 52,
    PT2_Repeat        = 90,

    PT3_MainMenu      = 100,
    PT3_Point0        = 101,
    PT3_Point1        = 102,
    PT3_Point2        = 103,
    PT3_CamInterval   = 104,
    PT3_Duration      = 105,
    PT3_StaticTime    = 106,
    PT3_Ramp          = 107,
    PT3_LeadInLeadOut = 108, //  No Repeat Type?
    PT3_Reveiw        = 109,

    GIGA_MainMenu     = 200,
    GIGA_AOV          = 201,
    GIGA_Overlap      = 202,
    GIGA_Point0       = 203,
    GIGA_Point1       = 204,
    GIGA_CamInterval  = 205,
    GIGA_Reveiw       = 206,

    PANO_MainMenu     = 210,
    PANO_AOV          = 211,
    PANO_Overlap      = 212,
    PANO_Point0       = 213,
    PANO_Pattern      = 214,
    PANO_CamInterval  = 215,
    PANO_Reveiw       = 216,
    PANO_Gaurd        = 217,

    SMS_Loop         = 250,
    PANO_Repeat       = 290,

    AUX_MainMenu      = 300,
    AUX_StartPoint    = 301,
    AUX_EndPoint      = 302,
    AUX_CamInterval   = 303,
    AUX_Duration      = 304,
    AUX_StaticTime    = 305,
    AUX_Ramp          = 306,
    AUX_LeadInLeadOut = 307,
    AUX_Reveiw        = 308,

    SET_MainMenu      = 900,
    SET_AuxOn         = 901,
    SET_PauseEnabled  = 902,
    SET_PowerSavePT   = 903,
    SET_PowerSaveAUX  = 904,
    SET_LCDBright     = 905,
    SET_AuxSpeed      = 906,
    SET_AuxDir        = 907,
    SET_Gaurd         = 908,

    PAUSE_Menu        = 1001,
};

void progstep_goto(enum Menu prgstp);
Menu progstep = Menu::PT2_StartPoint;   // used for location in the menu trees



void loop() { //Main Loop
  while (1) { // use debugging WHEN HIT here for monitoring - {sequence_repeat_type},{progstep},{progtype},{camera_fired}
    switch (progstep) {
      //start of 2 point SMS/Video routine
      case Menu::PT2_MainMenu:    { Choose_Program(); break; }
      case Menu::PT2_StartPoint:  { Move_to_Point_X(0); break; }
      case Menu::PT2_EndPoint:    { Move_to_Point_X(1); break; }
      case Menu::PT2_CamInterval: { Set_Shot_Interval(); break; }
      case Menu::PT2_Duration:    { Set_Duration(); break; }
      case Menu::PT2_StaticTime:
        if (config.shot_interval_time == VIDEO_EXPVAL) {      // don't show this for video
          if (progstep_forward_dir) { progstep_forward();  }  // skip the menu, go forward
          else                      { progstep_backward(); }  // skip the menu, go backward
        }
        else Set_Static_Time();                        //not needed for video
        break;
      case Menu::PT2_Ramp:          { Set_Ramp(); break;}
      case Menu::PT2_LeadInLeadOut:
        if (config.shot_interval_time == VIDEO_EXPVAL) {      // don't show this for video
          if (progstep_forward_dir) { progstep_forward();  }  // skip the menu, go forward
          else                      { progstep_backward(); }  // skip the menu, go backward
          config.lead_in = 0;
          config.lead_out = 0;
        }
        else Set_LeadIn_LeadOut();                        //not needed for video
        break;
      case Menu::PT2_ShotRepeat:
        if (config.shot_interval_time >= EXTTRIG_EXPVAL) {   //skip for non video
          if (progstep_forward_dir) { progstep_forward();  } //skip the menu, go forward
          else                      { progstep_backward(); } //skip the menu, go backward
        }
        else Set_Shot_Repeat();
        break;

      case Menu::PT2_Reveiw:        { Review_Confirm();              break; }
      case Menu::PT2_Gaurd:         { progstep = Menu::PT2_MainMenu; break; }

      //start of the three point move
      case Menu::PT3_MainMenu:      { progstep = Menu::PT2_MainMenu; break; }
      case Menu::PT3_Point0:        { Move_to_Point_X(0);            break; }
      case Menu::PT3_Point1:        { Move_to_Point_X(1);            break; }
      case Menu::PT3_Point2:        { Move_to_Point_X(2);            break; }
      case Menu::PT3_CamInterval:   { Set_Shot_Interval();           break; }
      case Menu::PT3_Duration:      { Set_Duration();                break; }
      case Menu::PT3_StaticTime:    { Set_Static_Time();             break; }
      case Menu::PT3_Ramp:          { Set_Ramp();                    break; }
      case Menu::PT3_LeadInLeadOut: { Set_LeadIn_LeadOut();          break; }
      case Menu::PT3_Reveiw:        { Review_Confirm();              break; }

      //start of pano Mode
      //  define field of view
      //We want to know how wide and tall our field of view is in steps so we can get our overlap right.  Anytime you zoom or change lenses, this need to be redefined
      //This should be a 10 seconds process to define by specifying corners
      //Step 1 - Put a point in the upper right corner - set zeros, pan up and right to hit same point with lower left corner of viewfinder
      //Display values  - write to ram - use these values

      case Menu::GIGA_MainMenu:     { progstep = Menu::PT2_MainMenu; break; }
      case Menu::GIGA_AOV:          { Set_angle_of_view();           break; }
      case Menu::GIGA_Overlap:      { Define_Overlap_Percentage();   break; }
      case Menu::GIGA_Point0:       { Move_to_Point_X(0);            break; }
      case Menu::GIGA_Point1:       { Move_to_Point_X(1);            break; }
      case Menu::GIGA_CamInterval:  { Set_Shot_Interval();           break; }
      case Menu::GIGA_Reveiw:       { Pano_Review_Confirm();         break; }

      //start of Portrait Pano Method
      //  define field of view
      //We want to know how wide and tall our field of view is in steps so we can get our overlap right.  Anytime you zoom or change lenses, this need to be redefined
      //This should be a 10 seconds process to define by specifying corners
      //Step 1 - Put a point in the upper right corner - set zeros, pan up and right to hit same point with lower left corner of viewfinder
      //Display values  - write to ram - use these values

      case Menu::PANO_MainMenu:     { progstep = Menu::PT2_MainMenu; break; }
      case Menu::PANO_AOV:          { Set_angle_of_view();           break; }
      case Menu::PANO_Overlap:      { Define_Overlap_Percentage();   break; }
      case Menu::PANO_Point0:       { Move_to_Point_X(0);            break; } // set subject point
      case Menu::PANO_Pattern:      { Set_PanoArrayType();           break; } // this sets variable that define how we move camera - load the appropriate array.
      case Menu::PANO_CamInterval:  { Set_Shot_Interval();           break; }
      case Menu::PANO_Reveiw:       { Pano_Review_Confirm();         break; }
      case Menu::PANO_Gaurd:        { progstep = Menu::PT2_MainMenu; break; }

      //start of entered distance on aux mode

      case Menu::AUX_MainMenu:      { progstep = Menu::PT2_MainMenu; break; }
      case Menu::AUX_StartPoint:    { Move_to_Startpoint();          break; }
      case Menu::AUX_EndPoint:      { Enter_Aux_Endpoint();          break; }
      case Menu::AUX_CamInterval:   { Set_Shot_Interval();           break; }
      case Menu::AUX_Duration:      { Set_Duration();                break; }
      case Menu::AUX_StaticTime:    { Set_Static_Time();             break; }
      case Menu::AUX_Ramp:          { Set_Ramp();                    break; }
      case Menu::AUX_LeadInLeadOut: { Set_LeadIn_LeadOut();          break; }
      case Menu::AUX_Reveiw:        { Review_Confirm();              break; }

      //start of setup

      case Menu::SET_MainMenu:      { progstep = Menu::PT2_MainMenu; break; }
      case Menu::SET_AuxOn:         { Setup_AUX_ON();                break; }
      case Menu::SET_PauseEnabled:  { Setup_PAUSE_ENABLED();         break; }
      case Menu::SET_PowerSavePT:   { Setup_POWERSAVE_PT();          break; }
      case Menu::SET_PowerSaveAUX:  { Setup_POWERSAVE_AUX();         break; }
      case Menu::SET_LCDBright:     { Setup_LCD_BRIGHTNESS();        break; }
      case Menu::SET_AuxSpeed:      { Setup_Max_AUX_Motor_Speed();   break; }
      case Menu::SET_AuxDir:        { Setup_AUX_Motor_DIR();         break; }
      case Menu::SET_Gaurd:         { progstep = Menu::PT2_MainMenu; break; }

      //start of in program menu options
      case Menu::PAUSE_Menu:        { InProg_Select_Option();        break; }
      //end of in program menu

      case Menu::PT2_ShootLoop:  // loop for SMS
        if (IsCameraIdle() && motorMoving == 0 ) { // Both Camera and Motor are Idle
          switch(PT2State) {
            case ShootMoveState::Idle:
              display_status();
              if (config.shot_interval_time == VIDEO_EXPVAL) { // Video is constantly recording, keep moving
                PT2State = ShootMoveState::Waiting_Camera;
              }
              else if (config.shot_interval_time == EXTTRIG_EXPVAL) { // We are looking for an external trigger, so jump to that state
                PT2State = ShootMoveState::Waiting_Trigger;
                CameraFocus(true);
              }
              else if (config.shot_interval_time > EXTTRIG_EXPVAL) { // Intervalometer mode, skip looking for a trigger
                PT2State = ShootMoveState::Waiting_Camera;
                FireCamera();  //start shutter sequence with pre and postfire
                config.camera_fired++;
              }
              break;

            case ShootMoveState::Waiting_Trigger: // Waiting for an external trigger
              if (ExternalTriggerChanged()) {
                ResetTriggerState();
                if (ExternalTriggerPressed())
                {
                  CameraShutter(true);
                  config.camera_fired++;
                  PT2State = ShootMoveState::Waiting_Release;
                }
              }
              break;
            
            case ShootMoveState::Waiting_Release: // Waiting for an external trigger release
              if (ExternalTriggerChanged()) {
                ResetTriggerState();
                if (!ExternalTriggerPressed())
                {
                  PT2State = ShootMoveState::Waiting_Camera;
                  CameraShutter(false);
                  CameraFocus(false);
                }
              }             
              break;
        
            case ShootMoveState::Waiting_Camera:  // Camera is done, now to start our move
              if (config.camera_fired >= config.camera_total_shots) {  // Have we hit the end of our program?
                PT2State = ShootMoveState::Idle;
                lcd.empty();
                draw(F_STR("Program Complete"), 1, 1);
                UpdatePowersave(Powersave::Always); // we power down at the end of program
                delay(screen_delay);
                progstep_goto(Menu::PT2_Repeat);
              }
              else
              {
                UpdatePowersave(Powersave::Shoot_Minimum); // Need to Move
                PT2State = ShootMoveState::Waiting_Moving;
                move_motors_s_curve();
              }
              break;
            
            case ShootMoveState::Waiting_Moving: // Motor has finished moving, Start Setting up the next shot
              UpdatePowersave(Powersave::Shoot_Accuracy);
              PT2State = ShootMoveState::Idle;
              break;
          }
          if (!nextMoveLoaded) {
            updateMotorVelocities();  //finished up the interrupt routine
          }
        }
        break;  //break 50

      case Menu::PT2_VideoLoop:
        //main video loop interrupt based.  This runs for 2 point moves only.

        if (progtype == Menu_Options::REG2POINTMOVE || progtype == Menu_Options::REV2POINTMOVE) {
          synched3AxisMove_timed(config.motor_steps_pt[1], static_cast<float>(config.overaldur), static_cast<float>(config.rampval) / 100.0f);
          if (maxVelLimit) {  //indicates the move is limited to enforce velocity limit on motors)
            draw(F_STR("Speed Limit"), 2, 1);
          }
          //Start us moving
          // interval_tm_last=interval_tm;
          interval_tm = millis();

          startISR1();
          do {
            if (!nextMoveLoaded) {
              updateMotorVelocities();
            }
          } while (motorMoving);
          stopISR1();

          if (DEBUG) {
            Serial.print("Video Runtime");
            Serial.println(millis() - interval_tm);
          }

          if (!motorMoving && (sequence_repeat_type == Repeat::Continuous_Alternate)) {  //new end condition for RUN CONTINOUS
            bool break_continuous = false;
            lcd.empty();
            draw(F_STR("Program Complete"), 1, 1);
            for (uint8_t i = 0; i < 30; i++) {
              NunChuckjoybuttons();
              if (config.PAUSE_ENABLED && CZ_Button_Read_Count > 25) {
                break_continuous = true;
                lcd.empty();
                draw(F_STR("Stopping Run"), 1, 1);
                draw(F_STR("Release Buttons"), 2, 1);
                do {
                  NunChuckjoybuttons();
                } while (c_button || z_button);
                progstep_goto(Menu::PT2_Reveiw);
              }
            }

            //add section to delay here if the delay is set.
            while (start_delay_tm > millis() / 1000L) {
              //enter delay routine
              if ((millis() - display_last_tm) > 1000)
              {
                display_time(calc_time_remain_start_delay(), 2, 1);
              }
              NunChuckjoybuttons();

              if (CZ_Button_Read_Count > CZ_Count_Threshold) {
                start_delay_tm = ((millis() / 1000L) + 5); //start right away by lowering this to 5 seconds.
                CZ_Button_Read_Count=0; //reset this to zero to start
              }
            }

            //end start delay

            if (!break_continuous) Auto_Repeat_Video();  //only run this if there isn't a break command
            first_ui = true;
          } else if (!motorMoving && (sequence_repeat_type == Repeat::Run_Once)) {  //new end condition for RUN ONCE
            lcd.empty();
            draw(F_STR("Program Complete"), 1, 1);
            UpdatePowersave(Powersave::Always); // we power down at the end of program
            progstep_goto(Menu::PT2_Repeat);
            first_ui = true;
            delay(100);
          }

        }  // end interrupt routine driven for 2 points

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //Start video loop for 3 Point moves - this loop does not use the new motor move profile as it needs to calculate each and every segment
        if (progtype == Menu_Options::REG3POINTMOVE || progtype == Menu_Options::REV3POINTMOVE) {  //this is regular 3 point move program that can be modified later
          if (DEBUG) {
            interval_tm_last = interval_tm;
            interval_tm = millis();
          }
          //Start us moving

          if (DEBUG) {
            Serial.print("trueinterval ");
            Serial.print(interval_tm - interval_tm_last);
            Serial.print(';');
          }

          if (DEBUG_MOTOR) {
            Serial.print("MoveStart ");
            Serial.print(millis());
            Serial.print(';');
          }

          config.camera_fired++;  //still need this for framing
          goto_position();

          if (DEBUG_MOTOR) {
            Serial.print("Moveend ");
            Serial.println(millis());
          }

          if (config.camera_total_shots > 0 && config.camera_fired >= config.camera_total_shots) {
            lcd.empty();
            draw(F_STR("Program Complete"),1,1);
            UpdatePowersave(Powersave::Always); // we power down at the end of program
            delay(screen_delay);
            progstep_goto(Menu::PT2_Repeat);
            first_ui = true;
          }

        }  // End video loop for 3 Point moves

        break;  //break 51 - VIDEO

      case Menu::PT2_TriggerLoop:  // loop for external interrupt - external triggering

        //New interrupt Flag Checks

        if (ExternalTriggerChanged()) {
          ResetTriggerState();
          if (ExternalTriggerPressed()) //start the clock as the cam shutter witch closed and sense pin, was brought low
          {
            ext_shutter_open = true;
            shuttertimer_open = micros();
          }
          else    //shutter closed - sense pin goes back high - stop the clock and report
          {
            ext_shutter_open = false;
            shuttertimer_close = micros();  //turn on the led / shutter
            ext_shutter_count++;
          }
        }
        //end interrupt check and flagging

        //  Start of states for external shooting loop

        if (!(Shot_Sequence_Engaged) && !(CameraShutter()) && (ext_shutter_open)) {  //start a shot sequence flag

          Shot_Sequence_Engaged = true;  //
        }

        if ((Shot_Sequence_Engaged) && !(CameraShutter()) && (ext_shutter_open)) {  //fire the camera can happen more than once in a shot sequence with HDR

          if (DEBUG) {
            Serial.print("Startshot_at:");
            Serial.print(millis());
            Serial.println(';');
          }

          //Fire Camera
          //don't fire the camera with the timer, just turn on our focus and shutter pins - we will turn them off when we sense the shot is done.
          CameraFocus(HIGH);
          CameraShutter(HIGH);
          //config.camera_fired++;
        }

        if (Shot_Sequence_Engaged && (CameraShutter()) && !(ext_shutter_open)) {  //shutter just closed, stop the camera port and move
          CameraFocus(LOW);
          CameraShutter(LOW);

          if (ext_shutter_count >= hdr_shots) {  //this is future functionality  - leave at 1 for now

            config.camera_fired++;
            ext_shutter_count = 0;
            //Move the motors - each motor move is calculated by where we are in the sequence - we still call this for lead in and lead out - motors just don't move

            if (DEBUG_MOTOR) {
              Serial.print("MoveStart ");
              Serial.print(millis() - interval_tm);
              Serial.print(';');
            }
            goto_position();
            if (DEBUG_MOTOR) {
              Serial.print("Moveend ");
              Serial.print(millis() - interval_tm);
              Serial.print(';');
            }

            //Turn off the motors if we have selected powersave 3 and 4 are the only ones we want here
            UpdatePowersave(Powersave::Programs);

            //Update display
            display_status();  //update after shot complete to avoid issues with pausing

            Shot_Sequence_Engaged = false;  //Shot sequence engaged flag is is off - we are ready for our next
            CZ_Button_Read_Count = 0;
            //InterruptAction_Reset(); //enable the external interrupts to start a new shot
            if (DEBUG) {
              Serial.println("EOL");
            }
          }
        }

        if (config.camera_fired >= config.camera_total_shots) {  //end of program
          lcd.empty();
          draw(F_STR("Program Complete"), 1, 1);
          UpdatePowersave(Powersave::Always); // we power down at the end of program
          delay(screen_delay);
          progstep_goto(Menu::PT2_Repeat);
          first_ui = true;
        }
        NunChuckjoybuttons();
        if (CZ_Button_Read_Count > 10 && config.shot_interval_time == EXTTRIG_EXPVAL) Interrupt_Fire_Engaged = true;  // manual trigger
        //if (config.PAUSE_ENABLED && CZ_Button_Read_Count>CZ_Count_Threshold && config.shot_interval_time>EXTTRIG_EXPVAL && !Shot_Sequence_Engaged ) Pause_Prog(); //pause an SMS program
        break;  //break 52 - end external triggering loop

      case Menu::PT2_Repeat:  // end of program - offer repeat and reverse options - check the nuncuck
        if (first_ui) {
          first_ui = false;
          lcd.empty();
          draw(F_STR("Repeat - C"), 1, 4);
          draw(F_STR("Reverse - Z"), 2, 4);
        }

        //This portion always runs in empty space of loop.
        NunChuckjoybuttons();
        button_actions_end_of_program();
        break;  // break 90

      case Menu::SMS_Loop:  // loop for Pano This happens once per camera shot.
        if (IsCameraIdle() && (motorMoving == 0)) { // Both Camera and Motor are Idle
          switch(PanoState) {
            case ShootMoveState::Idle:
              display_last_tm = millis();
              //display_status();
              if (config.shot_interval_time == VIDEO_EXPVAL) { // We are looking for an external trigger, so jump to that state
                PanoState = ShootMoveState::Waiting_Camera;
                config.camera_fired++;
              }
              if (config.shot_interval_time == EXTTRIG_EXPVAL) { // We are looking for an external trigger, so jump to that state
                PanoState = ShootMoveState::Waiting_Trigger;
                CameraFocus(true);
              }
              else if (config.shot_interval_time > EXTTRIG_EXPVAL) { // Intervalometer mode, skip looking for a trigger
                PanoState = ShootMoveState::Waiting_Camera;
                FireCamera();  //start shutter sequence with pre and postfire
                config.camera_fired++;
              }
              break;
            
            case ShootMoveState::Waiting_Trigger: // Waiting for an external trigger
              if (ExternalTriggerChanged()) {
                ResetTriggerState();
                if (ExternalTriggerPressed())
                {
                  CameraShutter(true);
                  config.camera_fired++;
                  PanoState = ShootMoveState::Waiting_Release;
                }
              }
              else if (C_Button_Read_Count > CZ_Count_Threshold) { // Trigger with the C Button
                CameraShutter(true);
                config.camera_fired++;
                PanoState = ShootMoveState::Waiting_Release;
              }
              break;
            
            case ShootMoveState::Waiting_Release: // Waiting for an external trigger release
              if (ExternalTriggerChanged()) {
                ResetTriggerState();
                if (!ExternalTriggerPressed())
                {
                  PanoState = ShootMoveState::Waiting_Camera;
                  CameraShutter(false);
                  CameraFocus(false);
                }
              }
              else if ((!c_button) && (C_Button_Read_Count > CZ_Count_Threshold)) { // Just after releasing the C Button from a manual trigger
                PanoState = ShootMoveState::Waiting_Camera;
                CameraShutter(false);
                CameraFocus(false);
                C_Button_Read_Count = 0;
              }
              break;
        
            case ShootMoveState::Waiting_Camera:  // Camera is done, now to start our move
              if (config.camera_fired >= config.camera_total_shots) {  // Have we hit the end of our program?
                PanoState = ShootMoveState::Idle;
                lcd.empty();
                draw(F_STR("Program Complete"), 1, 1);
                UpdatePowersave(Powersave::Always); // we power down at the end of program
                delay(screen_delay);
                progstep_goto(Menu::PANO_Repeat);
              }
              else
              {
                PanoState = ShootMoveState::Waiting_Moving;
                goto_position();
                //UpdatePowersave(Powersave::Shoot_Minimum); // Need to Move
              }
              break;
            
            case ShootMoveState::Waiting_Moving: // Motor has finished moving, Start Setting up the next shot
              //UpdatePowersave(Powersave::Shoot_Accuracy);
              PanoState = ShootMoveState::Idle;
              break;
          }
        }
        if (millis() - display_last_tm > 1000 && motorMoving == 0) {
          display_last_tm = millis();
          //display_status();
          DisplayDebug();
        }


        if (P2PAccelMove) {  //acceleration profiles
          if (!nextMoveLoaded) {
            updateMotorVelocities();  //finished up the interrupt routine
          }
        }

        NunChuckjoybuttons();

        if (CZ_Button_Read_Count > CZ_Count_Threshold) {
          SMS_In_Shoot_Paused_Menu();
        }
        if (Z_Button_Read_Count > CZ_Count_Threshold && config.camera_fired && !z_button) {
          PanoState = ShootMoveState::Waiting_Camera;
          config.camera_fired--;
        }

        break;  //break 250

      case Menu::PANO_Repeat:  //  finished up pano
        if (first_ui) {
          first_ui = false;
          lcd.empty();
          lcd.bright(config.LCD_BRIGHTNESS_DURING_MENU);
          draw(F_STR("Program Complete"), 1, 1);
          draw(F_STR("C-Repeat  Z-Back"), 2, 1);
        }
        NunChuckjoybuttons();
        button_actions290();  //read buttons, look for c button press to start run
        break;  // break 90
    }  //switch
  }  // while
}  //loop
