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

  Release Notes TB3_Black R1.1.108_SP2 Released 8/28/2014
  -Relaxed tight requiriements for joystick absolute centering.
  -Fixed motor feedrate issue when Static Time was maxed out.  If you are maxing out your static time by default, you are not using this setting correctly and hurting your shot!
  Read the instructions! Additionally made Static time a max of Interval minus 0.3 seconds to allow at least a .15 second move.

  Release Notes: TB3_Black R1.1.108_SP1
  -Added abs on feedrate min calc to accomodate spurious overrnn negatives on SMS shoots resulting in single long frame delays
  -Added check on minimum for video to ensure we don't catch 3PT video moves on min calc.
  -Added test agaist power policy for loop 52 (ext triggering)


  Release Notes: TB3_Black R1.1.108
  -Added reverse 2-point moves
  -Added reverse 3-point moves
  -Added pano/giga mode
  -Added external triggering by listening to camera's PC ports.
  -Added backlight during run setting in setup menu - you can turn it off completely if you like!
  -Added smoother 2-point video moves for better dolly shots
  -Added  Reverse and Repeat functions at the end of each shot
  -Built in Dragonframe mode supports Dragonframe 3.5
  -Lots and lots of little things to make our code better.

  Instructions for this version can be viewed here - eMotimo TB3 Instruction Manual

  Release Notes: TB3_Black R1.1.107
  This is a minor update to address a bug that Mac Dragonframe users were seeing when trying to connect.  This disabled verbose debugging by default that preventing connection to Dragonframe.

  Release Notes: TB3_Black R1.1.106
  This is a minor update to address a bug some users reported when programming a 2 point moves. The Move to start screen cycling without user input. This fix resolves that issue.


  Release Notes: TB3_Black R1.1.105
    Addition of Setup Menu
      Included AUX on and off - Default is off
      Powersave options for the Pan and Tilt as well as the Aux Motor
      Pause Enabled Option - default is false
    Powersaver options - PowerSaver 0 - on all the time, 1 = power down after complete, 2 power down between shots PT only, 3 power down between shots PTA on for static time, 4 Power up only while moving.
    Updated the return to start to be much faster.
    Added setup menu to turn off aux motor, enable pause feature, pt and aux powersave modes x 4
    BugFix - Too fast on the interval select - slow it down
    Removed load up repeat on startup - only have this for live action
    Bugfix - video duration not right until we move the stick
    Resolved flicker issue.
    Added three point moves
    Update 2 point move params
    Manual Trigger Added for Stop Motion Animation




  Release Notes: TB3_Black R1.1.102
   Addition of Dragonframe on board
   Update of Move Parameters

*/

/*

  =========================================
  Main Program
  =========================================

*/

#include <Wire.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <avr/interrupt.h>
#include "WiiNunchuck3.h"
#include "NHDLCD9.h"  //BB for LCD

NHDLCD9 lcd(4, 2, 16); // desired pin, rows, cols   //BB for LCD

//Global Parameters
#define DEBUG 1//
#define DEBUG_MOTOR 1//
#define DEBUG_NC 0 //turn on for verbose logging of nunchuck
#define DEBUG_PANO 0
#define POWERDOWN_LV false //set this to cause the TB3 to power down below 10 volts
#define MAX_MOVE_POINTS 3

#define MENU_ITEMS  8
enum progtype : uint8_t {
  REG2POINTMOVE = 0,
  REV2POINTMOVE = 1,
  REG3POINTMOVE = 2,
  REV3POINTMOVE = 3,
  PANOGIGA      = 4,
  PORTRAITPANO  = 5,
  DFSLAVE       = 6,
  SETUPMENU     = 7,
  // ASCOMSLAVE    = 98,
  AUXDISTANCE   = 99
};
uint8_t      progtype = REG2POINTMOVE; //updownmenu selection


#define MIN_INTERVAL_STATIC_GAP 3  //min gap between interval and static time

//Interval Options
#define VIDEO_INTVAL  2
#define EXTTRIG_INTVAL 3
//#define STOPMOT //not used

//TB3 section - Black or Orange Port Mapping for Step pins on Stepper Page
#define MOTORS 3
#define MOTOR0_STEP  5
#define MOTOR1_STEP  6
#define MOTOR2_STEP  7
#define MOTOR0_DIR   8
#define MOTOR1_DIR   9
#define MOTOR2_DIR   10
#define MOTOR_EN  A3
#define MOTOR_EN2  11
#define MS1 A1
#define MS2 A2
#define MS3 A2
#define IO_2  2 // drives middle of 2.5 mm connector on I/O port
#define IO_3  3 // drives tip of 2.5 mm connector on I/O port
#define STEPS_PER_DEG  444.444 //160000 MS per 360 degees = 444.4444444
//end TB3 section

unsigned long build_version = 10831; //this value is compared against what is stored in EEPROM and resets eeprom and setup values if it doesn't match
unsigned int  intval = 20; //seconds x10  - used for the interval prompt and display
unsigned long interval = 2000; //calculated and is in ms
unsigned long camera_exp_tm    = 100; //length of shutter signal sent to camera
unsigned int  camera_fired     = 0; //number of shots fired
unsigned int  camera_moving_shots = 200; //frames for new duration/frames prompt
unsigned int  camera_total_shots = 0; //used at the end target for camera fired to compare against
unsigned int  overaldur = 20; //seconds now for video only
unsigned int  prefire_time = 1; //currently hardcoded here to .1 second - prefire function not enabled.  this powerers up motor early for the shot
uint16_t      postfire_time         = 1;     // 0.1x Seconds how long to wait after an image to begin moving again
unsigned int  rampval = 50;
unsigned int  static_tm = 1; //new variable
unsigned int  lead_in = 1;
unsigned int  lead_out = 1;
uint16_t      start_delay_sec = 0;;
float         total_pano_move_time  = 0;

//External Interrupt Variables
volatile int state = 0; //new variable for interrupt
volatile boolean changehappened = false; //new variable for interrupt
long shuttertimer_open = 0;
long shuttertimer_close = 0;
boolean ext_shutter_open = false;
int ext_shutter_count = 0;
int ext_hdr_shots = 1; //this is how many shots are needed before moving - leave at one for normal shooting - future functionality with external


//Start of variables for Pano Mode
unsigned int FOV_X = 5000; //Field of View X for pano in steps  444.44 steps per degree
unsigned int FOV_y = 3000; //Field of View Y for pano in steps  444.44 steps per degree
unsigned int Pano_Mode = 1; //1 is pano, 2 is giga
unsigned int shots_per_row = 1; //calced value for a given shot
unsigned int shots_per_col = 1; //calced value for a given shot
unsigned int P2PType = 1; // 0 = no accel, 1= accel


//3 Point motor routine values
float motor_steps_pt[MAX_MOVE_POINTS][MOTORS];  // 3 total points.   Start point is always 0.0
float percent; //% through a leg
unsigned int keyframe[2][6] = {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}; //this is basically the keyframes {start, end of rampup, start or rampdown, end}   - doesn't vary by motor at this point
float linear_steps_per_shot [MOTORS] = {0.0, 0.0, 0.0}; //{This is for the calculated or estimated steps per shot in a segment for each motor
float ramp_params_steps [MOTORS] = {0.0, 0.0, 0.0}; //This is to calc the steps at the end of rampup for each motor.  Each array value is for a motor

//Program Status Flags
boolean Program_Engaged = false;
boolean Shot_Sequence_Engaged = false;
boolean Prefire_Engaged = false;
boolean Shutter_Signal_Engaged = false;
boolean Static_Time_Engaged = false;
boolean IO_Engaged = false;
boolean Move_Engaged = false;
boolean Interrupt_Fire_Engaged = false;
int Button_Read_Count = 0;


//New Powersave flags - 1=always on   2=standard     3=High
/*Power Save explanation
  We can power up and power down the Pan Tilt motors together.  We can power up and power down the Aux motor port as well.  We see three levels of power saving:
  1)  None - Motors are always on - for VFX work where power isn't a factor and precision is most important.  Motors will get warm here on hot days.
  2)  Low - only at the end of program
  3)  Standard - Power up the motors for the shooting time (all the time we hold the trigger down), and move, power down between shots.
  4)  High - Only power on for motor moves, turn off the motors when we reach the shooting position.
    We are powered down for the shot and only power on for moves. This saves a ton of battery for long astro shots.
    We do lose microstep resolution for this, but it usually is not visible.   We could be off by as much as 8/16 mircosetps for a shot or 0.018 degrees - Really small stuff!  Try this mode out!
*/



/*New Setup Menu
  We are adding a setup menu to control power options through the front end
  1)  Aux Port - On,Off  Prevents accidental long moves on the AUX - keeps everything powered down
  2)  PT PowerSave- Off, Standard, High
  3)  AUX PowerSave - Off, Standard, High
  4)  Pause - Enabled, Disabled

*/

enum powersave : uint8_t {
  PWR_ALWAYS_ON    = 0, // Motors are always on,
  PWR_PROGRAM_ON   = 1, // Motors at full power when ever a program is active, in low power in main menu
  PWR_SHOOTMOVE_ON = 2, // Motors at full power for moves and image capturing, in low power otherwise
  PWR_MOVEONLY_ON  = 3  // Motors only powered for movements, it can loose up to 8 steps position per stop
};

//CVariables that are set during the Setup Menu
unsigned int  POWERSAVE_PT;  //1=None - always on  2 - low   3=standard    4=High
unsigned int  POWERSAVE_AUX;  //1=None - always on  2 - low   3=standard    4=High
boolean AUX_ON;  //1=Aux Enabled, 2=Aux disabled
boolean PAUSE_ENABLED;  //1=Pause Enabled, 0=Pause disabled
boolean REVERSE_PROG_ORDER; //Program ordering 0=normal, start point first. 1=reversed, set end point first to avoid long return to start
boolean MOVE_REVERSED_FOR_RUN = 0;
unsigned int  LCD_BRIGHTNESS_DURING_RUN;  //0 is off 8 is max
boolean       AUX_REV;  //1=Aux Enabled, 0=Aux disabled
uint16_t      AUX_MAX_JOG_STEPS_PER_SEC = 32000;
uint16_t      PAN_MAX_JOG_STEPS_PER_SEC = 32000;
uint16_t      TILT_MAX_JOG_STEPS_PER_SEC = 32000;

//control varable, no need to store in EEPROM
unsigned int progstep = 0; //used to define case for main loop
boolean reset_prog = 1; //used to handle program reset or used stored
boolean redraw = 1; //variable to help with LCD dispay variable that need to show one time
boolean redraw2 = true;
int batt_low_cnt = 0;
unsigned int max_shutter;
unsigned int max_prefire;
unsigned int program_progress_2PT = 1; //Lead in, ramp, linear, etc for motor routine case statement
unsigned int program_progress_3PT = 1; //phase 1, phase 2
unsigned long interval_tm        = 0;  //uc time to help with interval comparison
unsigned long interval_tm_last = 0; //uc time to help with interval comparison
int cursorpos = 1; //use 1 for left, 2 for right  - used for lead in, lead out
unsigned int lcd_dim_tm     = 10;
unsigned long input_last_tm = 0;
unsigned long display_last_tm = 0;
unsigned int  lcd_backlight_cur = 100;
unsigned int  prompt_time = 500; // in ms for delays of instructions
int  prompt_delay = 0; //to help with joystick reads and delays for inputs - this value is set during joystick read and executed later in the loop
int prompt_val;
unsigned int  video_sample_ms = 100;
unsigned int video_segments = 250;
int reviewprog = 1;
//variables for display of remaining time
int aux_dist;
uint32_t      start_delay_tm = 0;  //ms timestamp to help with delay comparison


enum ButtonState : uint8_t {
  Released   = 0,
  C_Pressed  = 1,
  Z_Pressed  = 2,
  CZ_Pressed = 3,
  C_Held     = 4,
  Z_Held     = 5,
  CZ_Held    = 6,
  Read_Again = 7
};

int32_t prev_joy_x_reading = 0; //prevents buffer from moving axis from previous input
int32_t prev_joy_y_reading = 0;

uint8_t ButtonState = Read_Again;
uint8_t       HeldThreshold         = 1;     // Number of read events to recognise a button as held
int8_t        joy_x_axis, joy_y_axis;
int8_t        acc_x_axis, acc_y_axis;

uint32_t NClastread = 1000; //control variable for NC reads cycles
uint32_t NCdelay    = 10;      //milliseconds to wait before reading the joystick again

//Stepper Setup
unsigned long  feedrate_micros = 0;

struct FloatPoint {
  float x;
  float y;
  float z;
};
FloatPoint fp;

FloatPoint current_steps;
FloatPoint target_steps;
FloatPoint delta_steps;

//our direction vars
byte x_direction = 1;
byte y_direction = 1;
byte z_direction = 1;

//End setup of Steppers

//Start of DF Vars
#define DFMOCO_VERSION 1
#define DFMOCO_VERSION_STRING "1.2.6"



//#include <SoftwareSerial.h>
//#include <NHDLCD8.h>  //BB for LCD
//NHDLCD8 lcd(4,2,16); // desired pin, rows, cols   //BB for LCD


// supported boards
#define ARDUINO      1
#define ARDUINOMEGA  2

//eMotimo TB3 - Set this PINOUT_VERSION 3 for TB3 Orange (Uno)
//eMotimo TB3 - Set this PINOUT_VERSION 4 for TB3 Black (MEGA)
#define PINOUT_VERSION 4

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

// detect board type
#define BOARD ARDUINOMEGA

#define SERIAL_DEVICE Serial

#define PIN_ON(port, pin)  { port |= pin; }
#define PIN_OFF(port, pin) { port &= ~pin; }

#define MOTOR_COUNT 4

#define TIME_CHUNK 50
#define SEND_POSITION_COUNT 20000

// update velocities 20 x second
#define VELOCITY_UPDATE_RATE (50000 / TIME_CHUNK)
#define VELOCITY_INC(maxrate) (max(1.0f, maxrate / 70.0f))

#if ( PINOUT_VERSION == 4 )

//Start TB3 Black Port Mapping

#define MOTOR0_STEP_PORT PORTE
#define MOTOR0_STEP_PIN  B00001000 //Pin 5 PE3

#define MOTOR1_STEP_PORT PORTH
#define MOTOR1_STEP_PIN  B00001000//Pin  6 PH3

#define MOTOR2_STEP_PORT PORTH
#define MOTOR2_STEP_PIN  B00010000 //Pin 7 PH4

#define MOTOR3_STEP_PORT PORTC //  Map this to pin 30 PC7 on the Mega board for debug
#define MOTOR3_STEP_PIN  B10000000 //
//End TB3 Black Port Mapping


#elif ( PINOUT_VERSION == 3 )

//Start eMotimo TB3 Orange Port Mapping
#define MOTOR0_STEP_PORT PORTD
#define MOTOR0_STEP_PIN  B00100000 //Pin 5

#define MOTOR1_STEP_PORT PORTD
#define MOTOR1_STEP_PIN  B01000000 //Pin 6

#define MOTOR2_STEP_PORT PORTD
#define MOTOR2_STEP_PIN  B10000000 //Pin 7

#define MOTOR3_STEP_PORT PORTD //use the same port as MOTOR2 for now - commented out section where this is called
#define MOTOR3_STEP_PIN  B10000000 //use the same port as MOTOR2 for now - commented out section where this is called
// End  eMotimo TB3 Orange Port Mapping

#endif


/**
   Serial output specialization
*/
#if defined(UBRRH)
#define TX_UCSRA UCSRA
#define TX_UDRE  UDRE
#define TX_UDR   UDR
#else
#define TX_UCSRA UCSR0A
#define TX_UDRE  UDRE0
#define TX_UDR   UDR0
#endif

char txBuf[32];
char *txBufPtr;

#define TX_MSG_BUF_SIZE 16

#define MSG_STATE_START 0
#define MSG_STATE_CMD   1
#define MSG_STATE_DATA  2
#define MSG_STATE_ERR   3

#define MSG_STATE_DONE  100

/*
   Command codes from user
*/
#define USER_CMD_ARGS 40

#define CMD_NONE       0
#define CMD_HI         10
#define CMD_MS         30
#define CMD_NP         31
#define CMD_MM         40 // move motor
#define CMD_PR         41 // pulse rate
#define CMD_SM         42 // stop motor
#define CMD_MP         43 // motor position
#define CMD_ZM         44 // zero motor
#define CMD_SA         50 // stop all (hard)
#define CMD_BF         60 // blur frame
#define CMD_GO         61 // go!

#define CMD_JM         70 // jog motor
#define CMD_IM         71 // inch motor


#define MSG_HI 01
#define MSG_MM 02
#define MSG_MP 03
#define MSG_MS 04
#define MSG_PR 05
#define MSG_SM 06
#define MSG_SA 07
#define MSG_BF 10
#define MSG_GO 11
#define MSG_JM 12
#define MSG_IM 13


struct UserCmd
{
  byte command;
  byte argCount;
  int32_t args[USER_CMD_ARGS];
} ;

/*
   Message state machine variables.
*/
byte lastUserData;
int  msgState;
int  msgNumberSign;
UserCmd userCmd;


struct txMsg
{
  byte msg;
  byte motor;
};

struct TxMsgBuffer
{
  txMsg buffer[TX_MSG_BUF_SIZE];
  byte head;
  byte tail;
};

TxMsgBuffer txMsgBuffer;


/*
  Motor data.
*/

uint16_t           motorAccumulator0;
uint16_t           motorAccumulator1;
uint16_t           motorAccumulator2;
uint16_t           motorAccumulator3;
#if MOTOR_COUNT > 4
uint16_t           motorAccumulator4;
uint16_t           motorAccumulator5;
uint16_t           motorAccumulator6;
uint16_t           motorAccumulator7;
#endif
uint16_t*          motorAccumulator[MOTOR_COUNT] =
{
  &motorAccumulator0, &motorAccumulator1, &motorAccumulator2, &motorAccumulator3,
#if MOTOR_COUNT > 4
  &motorAccumulator4, &motorAccumulator5, &motorAccumulator6, &motorAccumulator7
#endif
};

uint16_t           motorMoveSteps0;
uint16_t           motorMoveSteps1;
uint16_t           motorMoveSteps2;
uint16_t           motorMoveSteps3;
#if MOTOR_COUNT > 4
uint16_t           motorMoveSteps4;
uint16_t           motorMoveSteps5;
uint16_t           motorMoveSteps6;
uint16_t           motorMoveSteps7;
#endif
uint16_t*          motorMoveSteps[MOTOR_COUNT] =
{
  &motorMoveSteps0, &motorMoveSteps1, &motorMoveSteps2, &motorMoveSteps3,
#if MOTOR_COUNT > 4
  &motorMoveSteps4, &motorMoveSteps5, &motorMoveSteps6, &motorMoveSteps7
#endif
};


uint16_t           motorMoveSpeed0;
uint16_t           motorMoveSpeed1;
uint16_t           motorMoveSpeed2;
uint16_t           motorMoveSpeed3;
#if MOTOR_COUNT > 4
uint16_t           motorMoveSpeed4;
uint16_t           motorMoveSpeed5;
uint16_t           motorMoveSpeed6;
uint16_t           motorMoveSpeed7;
#endif
uint16_t         * motorMoveSpeed[MOTOR_COUNT] =
{
  &motorMoveSpeed0, &motorMoveSpeed1, &motorMoveSpeed2, &motorMoveSpeed3,
#if MOTOR_COUNT > 4
  &motorMoveSpeed4, &motorMoveSpeed5, &motorMoveSpeed6, &motorMoveSpeed7
#endif
};

volatile boolean nextMoveLoaded;


unsigned int   velocityUpdateCounter;
byte           sendPositionCounter;
boolean        hardStopRequested;

byte sendPosition = 0;
byte motorMoving = 0;
byte toggleStep = 0;


#define P2P_MOVE_COUNT 7

struct Motor
{
  byte   stepPin;
  byte   dirPin;

  // pre-computed move
  float   moveTime[P2P_MOVE_COUNT];
  int32_t movePosition[P2P_MOVE_COUNT];
  float   moveVelocity[P2P_MOVE_COUNT];
  float   moveAcceleration[P2P_MOVE_COUNT];

  float   gomoMoveTime[P2P_MOVE_COUNT];
  int32_t gomoMovePosition[P2P_MOVE_COUNT];
  float   gomoMoveVelocity[P2P_MOVE_COUNT];
  float   gomoMoveAcceleration[P2P_MOVE_COUNT];

  int       currentMove;
  float     currentMoveTime;

  volatile  boolean   dir;

  int32_t   position;
  int32_t   destination;
  float     maxVelocity;
  float     maxAcceleration;

  uint16_t  nextMotorMoveSteps;
  float     nextMotorMoveSpeed;

};

boolean goMoReady;
int     goMoDelayTime;

Motor motors[MOTOR_COUNT];

//End of DF Vars





/*
  =========================================
  Setup functions
  =========================================
*/




void setup() {

  // setup motor pins
  pinMode(MOTOR0_STEP, OUTPUT);
  pinMode(MOTOR0_DIR, OUTPUT);
  pinMode(MOTOR1_STEP, OUTPUT);
  pinMode(MOTOR1_DIR, OUTPUT);
  pinMode(MOTOR2_STEP, OUTPUT);
  pinMode(MOTOR2_DIR, OUTPUT);

  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);

  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, HIGH);

  pinMode(MOTOR_EN, OUTPUT);
  pinMode(MOTOR_EN2, OUTPUT);

  digitalWrite(MOTOR_EN, HIGH); //LOW Enables output, High Disables
  digitalWrite(MOTOR_EN2, HIGH); //LOW Enables output, High Disables

  // setup camera pins
  CameraInit();

  //Setup of I/0 Pings Start with output of I/Oport
  pinMode(IO_2, OUTPUT);
  pinMode(IO_3, OUTPUT);

  digitalWrite(IO_2, LOW);
  digitalWrite(IO_3, LOW);

  pinMode(A0, INPUT); //this is for the voltage reading

  //Setup LCD
  lcd.setup();
  delay(100);
  draw(0, 1, 2); // Setup Complete
  draw(1, 2, 1); // Version Number
  lcd.contrast(50);
  lcd.cursorOff();
  lcd.bright(4);

  delay(prompt_time * 2);
  lcd.empty();
  //delay(100);

  draw(2, 1, 1); // Connect Joystick

  //Setup Serial Connection

  Serial.begin(57600);
  Serial.println("Starting Debug");


  // Handle EEPROM Interaction and upgrades

  //Check to see if our hardcoded build version set in progam is different than what was last put in EEPROM - detect upgrade.
  if (build_version != check_version()) { //4 byte string that now holds the build version.
    if (DEBUG) Serial.println(check_version());
    if (DEBUG) Serial.println("Upgrading Memory");
    write_defaults_to_eeprom_memory();  //these are for setting for last shot
    set_defaults_in_setup(); //this is for our setup values that should only be defaulted once.
    //review_RAM_Contents();
  }
  else { //load last setting into memory - no upgrade
    if (DEBUG) Serial.println("Restoring EEPROM Values");
    restore_from_eeprom_memory();
    //review_RAM_Contents();
  }
  //End Setup of EEPROM

  //begin  Setup for Nunchuck
  Nunchuck.init(0);
  delay(50);
  for (int reads = 1; reads < 17; reads++) {
    Nunchuck.getData();
    //Nunchuck.printData();
    lcd.at(2, reads, "+");
    if (abs(Nunchuck.joyx() - 127) > 60 || abs(Nunchuck.joyy() - 127) > 60 ) {
      lcd.empty();
      draw(57, 1, 1); //lcd.at(1,1,"Center Joystick");
      reads = 1;
    }
    delay(25);

  }

  calibrate_joystick(Nunchuck.joyx(), Nunchuck.joyy());

  //end  Setup for Nunchuk
  lcd.empty();


  //Setup Motors
  init_steppers();

  //init_external_triggering();
  pinMode(IO_3, INPUT);
  digitalWrite(IO_3, HIGH);
  attachInterrupt(1, cam_change, CHANGE);



} //end of setup

void loop() {  //Main Loop
  while (1) {
    switch (progstep)
    {

      //start of 2 point SMS/Video routine
      case 0: Choose_Program();        break;
      case 1: Move_to_Startpoint();    break;  // Move to Start Point
      case 2: Move_to_Endpoint();      break;  // Move to End Point
      case 3: Set_Cam_Interval();      break;  // Set Camera Interval
      case 4: Set_Duration();          break;
      case 5: Set_Static_Time();       break;
      case 6: Set_Ramp();              break;
      case 7: Set_LeadIn_LeadOut();    break;
      case 8: Review_Confirm();        break;  // review and confirm

      /*
      case 40: Choose_Program();        break;
      case 41: Move_to_Startpoint();    break;  // Move to Start Point
      case 42: Move_to_Endpoint();      break;  // Move to End Point
      case 43: Set_Cam_Interval();      break;  // Set Camera Interval
      case 44: Set_Duration();          break;
      case 45: Set_Static_Time();       break;
      case 46: Set_Ramp();              break;
      case 47: Set_LeadIn_LeadOut();    break;
      case 48: Review_Confirm();        break;  // review and confirm
      //end of 2 point SMS/Video

      */

      //start of the three point move
      case 100: Choose_Program();       break;
      case 101: Move_to_Point_X(0);     break;  // Move Point 0
      case 102: Move_to_Point_X(1);     break;  // Move Point 1
      case 103: Move_to_Point_X(2);     break;  // Move Point 2
      case 104: Set_Cam_Interval();     break;  // Set Camera Interval
      case 105: Set_Duration();         break;
      case 106: Set_Static_Time();      break;
      case 107: Set_Ramp();             break;
      case 108: Set_LeadIn_LeadOut();   break;
      case 109: Review_Confirm();       break;  // review and confirm
      //end of the three point move

      //start of pano Mode

      //  define field of view
      //We want to know how wide and tall our field of view is in steps so we can get our overlap right.  Anytime you zoom or change lenses, this need to be redefined
      //This should be a 10 seconds process to define by specifying corners
      //Step 1 - Put a point in the upper right corner - set zeros, pan up and right to hit same point with lower left corner of viewfinder
      //Display values  - write to ram - use these values

      case 200: Choose_Program();              break;
      case 201: Set_angle_of_view();           break;
      case 202: Define_Overlap_Percentage();   break;
      case 203: Move_to_Point_X(0);            break;  // specify overlap
      case 204: Move_to_Point_X(1);            break;
      case 205: Set_Static_Time();             break;
      case 206: Pano_Review_Confirm();         break;

      case 207: lcd.at(1, 1, "207");           break;
      case 208: lcd.at(1, 1, "208");           break;
      case 209: lcd.at(1, 1, "209");           break;

      case 250: PanoLoop();                    break;  // loop for Pano
      case 290: PanoEnd();                     break;  // finished up pano
      //end of Pano Mode


      //start of setup
      case 901: Setup_AUX_ON();                     break;  // AUX_ON
      case 902: Setup_PAUSE_ENABLED();              break;  // PAUSE_ENABLED
      case 903: Setup_POWERSAVE_PT();               break;  // POWERSAVE_PT
      case 904: Setup_POWERSAVE_AUX();              break;  // POWERSAVE_AUX
      case 905: Setup_LCD_BRIGHTNESS_DURING_RUN();  break;  // LCD Bright
      case 906:                                     break;  // Exit
      //end of setup


      case 50:  // loop for SMS
        ShootMoveShoot();
        break; //break 50

      case 51:  //main video loop - small and tight strip everything that is not needed
        VideoLoop();
        break; //break 51

      case 52:  // loop for external interrupt
        ExternalTriggerLoop();
        break; //break 52 - end external triggering loop

      case 90: //  review and confirm
        EndOfProgramLoop();
        break;  // break 90

    } //switch
  } // while
} //loop
