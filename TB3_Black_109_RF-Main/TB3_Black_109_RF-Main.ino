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

#include <Wire.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "WiiNunchuck3.h"
#include "NHDLCD9.h"

NHDLCD9 lcd(4, 2, 16); // desired pin, rows, cols   //BB for LCD

//Global Parameters
#define DEBUG 1
#define DEBUG_MOTOR 0
#define DEBUG_NC 0
#define DEBUG_PANO 1
#define DEBUG_GOTO 0

#define POWERDOWN_LV false //set this to cause the TB3 to power down below 10 volts
#define MAX_MOVE_POINTS 3
#define VIDEO_FEEDRATE_NUMERATOR 375L // Set this for 42000L, or 375L for faster calc moves
#define PAN_MAX_JOG_STEPS_PER_SEC 10000
#define TILT_MAX_JOG_STEPS_PER_SEC 10000
//#define AUX_MAX_JOG_STEPS_PER_SEC 15000.0 //this is defined in the setup menu now.


//Interval Options
#define VIDEO_INTVAL  2
#define EXTTRIG_INTVAL 3
#define MIN_INTERVAL_STATIC_GAP 3  //min gap between interval and static time
//#define STOPMOT //not used

//TB3 section - Black or Orange Port Mapping for Step pins on Stepper Page
#define MOTORS		  3
#define MOTOR0_STEP	5
#define MOTOR1_STEP	6
#define MOTOR2_STEP	7
#define MOTOR0_DIR	8
#define MOTOR1_DIR	9
#define MOTOR2_DIR	10
#define MOTOR_EN	  A3
#define MOTOR_EN2	  11
#define MS1		    	A1
#define MS2			    A2
#define MS3			    A2
#define IO_2		    2 // drives middle of 2.5 mm connector on I/O port
#define IO_3		    3 // drives tip of 2.5 mm connector on I/O port
#define STEPS_PER_DEG  444.444 //160000 MS per 360 degees = 444.4444444

/*
  STEPS_PER_INCH_AUX for various motors with 17 tooth final gear on 5mm pitch belt
  Phidgets 99:1	95153
  Phidgets 27:1	25676
  Phidgets 5:1	4955
  20:1 Ratio	19125
  10:1 Ratio	9562
*/

#define STEPS_PER_INCH_AUX 19125 //
#define MAX_AUX_MOVE_DISTANCE 311 //(31.1 inches)
//end TB3 section

uint32_t      build_version = 10953; //this value is compared against what is stored in EEPROM and resets EEPROM and setup values if it doesn't match
uint32_t      intval = 2; //seconds x10  - used for the interval prompt and display
uint32_t      interval = 2000; //calculated and is in ms
uint32_t      camera_fired     = 0; //number of shots fired
uint32_t      camera_moving_shots = 200; //frames for new duration/frames prompt
uint32_t      camera_total_shots = 0; //used at the end target for camera fired to compare against
unsigned int  overaldur = 20; //seconds now for video only
unsigned int  prefire_time = 1; //currently hardcoded here to .1 second - this powers up motor early for the shot
unsigned int  rampval = 50;
uint16_t      static_tm = 1; //new variable
uint16_t      lead_in = 1;
uint16_t      lead_out = 1;
uint16_t      start_delay_sec = 0;
float         total_pano_move_time = 0;

//External Interrupt Variables
volatile bool iostate = 0; //new variable for interrupt
volatile bool changehappened = false; //new variable for interrupt
unsigned long shuttertimer_open = 0;
unsigned long shuttertimer_close = 0;
boolean   ext_shutter_open = false;
uint8_t   ext_shutter_count = 0;
uint8_t   ext_hdr_shots = 1; //this is how many shots are needed before moving - leave at one for normal shooting - future functionality with external

//3 Point motor routine values
int32_t   motor_steps_pt[MAX_MOVE_POINTS][MOTORS];  // 3 total points.   Start point is always 0
uint32_t  percent; //% through a leg
uint16_t   keyframe[2][6] = {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}; //this is basically the keyframes {start, end of rampup, start or rampdown, end}   - doesn't vary by motor at this point
int32_t   linear_steps_per_shot [MOTORS] = {0, 0, 0}; //{This is for the calculated or estimated steps per shot in a segment for each motor
int32_t   ramp_params_steps [MOTORS] = {0, 0, 0}; //This is to calc the steps at the end of rampup for each motor.  Each array value is for a motor

//Program Status Flags
boolean Program_Engaged =        false;
boolean Move_Engaged =           false;
boolean Interrupt_Fire_Engaged = false;

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

//CVariables that are set during the Setup Menu store these in EEPROM
uint8_t  POWERSAVE_PT;  //1=None - always on  2 - low   3=standard    4=High
uint8_t  POWERSAVE_AUX;  //1=None - always on  2 - low   3=standard    4=High
boolean  AUX_ON;  //1=Aux Enabled, 0=Aux disabled
boolean  PAUSE_ENABLED;  //1=Pause Enabled, 0=Pause disabled
boolean  REVERSE_PROG_ORDER; //Program ordering 0=normal, start point first. 1=reversed, set end point first to avoid long return to start
boolean  MOVE_REVERSED_FOR_RUN = 0;
uint8_t  LCD_BRIGHTNESS_DURING_RUN;  //0 is off 8 is max
uint16_t AUX_MAX_JOG_STEPS_PER_SEC; //value x 1000  20 is the top or 20000 steps per second.
boolean  AUX_REV;  //1=Aux Enabled, 0=Aux disabled


//control variable, no need to store in EEPROM - default and setup during shot
uint16_t  progstep = 0; //used to define case for main loop
boolean   progstep_forward_dir = true; //boolean to define direction of menu travel to allow for easy skipping of menus

uint16_t  progtype = 0; //updownmenu selection
//Main Menu Ordering

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
  ASCOMSLAVE    = 98,
  AUXDISTANCE   = 99
};

uint8_t   inprogtype = 0; //updownmenu selection during shoot
//In Program Menu Ordering

#define INPROG_OPTIONS  5    //up this when code for gotoframe
enum inprogtype : uint8_t {
  INPROG_RESUME     = 0,
  INPROG_RTS        = 1,
  INPROG_GOTO_END   = 2,
  INPROG_GOTO_FRAME = 3,
  INPROG_INTERVAL   = 4,
  INPROG_STOPMOTION = 99,
};

boolean   reset_prog  = 1; //used to handle program reset or used stored
boolean   first_time  = 1; //variable to help with LCD dispay variable that need to show one time
boolean   first_time2 = 1;

uint32_t     max_shutter; // Maximum shutter time in 0.1 second increments
//unsigned int max_prefire;

uint8_t      Move_State_2PT = 1;
enum Move_State_2PT : uint8_t {
  LeadIn2PT   = 1,
  RampUp2PT   = 2,
  Linear2PT   = 3,
  RampDown2PT = 4,
  LeadOut2PT  = 5,
  Finished2PT = 9
};

uint8_t      Move_State_3PT = 1;
enum Move_State_3PT : uint8_t {
  LeadIn3PT    = 101,
  FirstLeg3PT  = 102,
  SecondLeg3PT = 103,
  ThirdLeg3PT  = 104,
  LeadOut3PT   = 105,
  Finished3PT  = 109
};

uint32_t     interval_tm      = 0;  //mc time to help with interval comparison
uint32_t     interval_tm_last = 0; //mc time to help with interval comparison

uint8_t lcd_dim_tm     = 10;
unsigned long diplay_last_tm = 0;
uint8_t  lcd_backlight_cur = 100;
unsigned int  prompt_time = 500; // in ms for delays of instructions
//unsigned int  prompt_time=350; // for faster debugging
int  prompt_delay = 0; //to help with joystick reads and delays for inputs - this value is set during joystick read and executed later in the loop
//int prompt_val;

uint8_t reviewprog = 1;
enum reviewprog : uint8_t {
  RowsColumns  = 0,
  TotalPhotos  = 1,
  Ready        = 2,
  StartDelay   = 3
};

uint32_t start_delay_tm = 0;  //ms timestamp to help with delay comparison
uint32_t goto_shot = 0;

int8_t   sequence_repeat_type = 1; //1 Defaults - Run Once, 0 Continuous Loop,  -1 Continuous Forward
uint16_t sequence_repeat_count = 0; //counter to hold variable for how many time we have repeated

//remote and interface variables

int8_t joy_x_axis, joy_y_axis, acc_x_axis, accel_y_axis;

int PanStepCount;
int TiltStepCount;

unsigned int joy_y_lock_count = 0;
unsigned int joy_x_lock_count = 0;

uint8_t ButtonState = 4;
enum ButtonState : uint8_t {
  Released   = 0,
  C_Pressed  = 1,
  Z_Pressed  = 2,
  CZ_Pressed = 3,
  Read_Again = 4
};



int CZ_Button_Read_Count = 0;
int C_Button_Read_Count = 0;
int Z_Button_Read_Count = 0;


unsigned int NCReadMillis = 42; //frequency at which we read the nunchuck for moves  1000/24 = 42  1000/30 = 33
long NClastread = 1000; //control variable for NC reads cycles

//Stepper Setup
unsigned long  feedrate_micros = 0;

struct LONG {
  int32_t x;
  int32_t y;
  int32_t z;
};

LONG current_steps;
LONG target_steps;
LONG delta_steps;

//End setup of Steppers

//Start of DF Vars
#define DFMOCO_VERSION 1
#define DFMOCO_VERSION_STRING "1.2.6"


// supported boards
#define ARDUINO			1
#define ARDUINOMEGA		2

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


//Start TB3 Black Port Mapping

#define MOTOR0_STEP_PORT PORTE
#define MOTOR0_STEP_PIN  B00001000 //Pin 5 PE3

#define MOTOR1_STEP_PORT PORTH
#define MOTOR1_STEP_PIN  B00001000 //Pin  6 PH3

#define MOTOR2_STEP_PORT PORTH
#define MOTOR2_STEP_PIN  B00010000 //Pin 7 PH4

#define MOTOR3_STEP_PORT PORTC //  Map this to pin 30 PC7 on the Mega board for debug
#define MOTOR3_STEP_PIN  B10000000 //
//End TB3 Black Port Mapping


volatile boolean nextMoveLoaded;  // Program flag for next move ready
boolean maxVelLimit = false;      // Program Flag for motor speed limited
uint8_t motorMoving = 0;          // Program Flag for motor moving


//End of DFVars


/*
	=========================================
	Setup functions
	=========================================
*/


void setup()
{
  // setup motor pins
  pinMode(MOTOR0_STEP,	OUTPUT);
  pinMode(MOTOR0_DIR,		OUTPUT);
  pinMode(MOTOR1_STEP,	OUTPUT);
  pinMode(MOTOR1_DIR,		OUTPUT);
  pinMode(MOTOR2_STEP,	OUTPUT);
  pinMode(MOTOR2_DIR,		OUTPUT);

  pinMode(MS1,			OUTPUT);
  pinMode(MS2,			OUTPUT);
  pinMode(MS3,			OUTPUT);

  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, HIGH);

  pinMode(MOTOR_EN, OUTPUT);
  pinMode(MOTOR_EN2, OUTPUT);

  digitalWrite(MOTOR_EN,  HIGH); //LOW Enables output, High Disables
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
  delay(100);
  draw(2, 1, 1); // Connect Joystick

  //Setup Serial Connection

  Serial.begin(57600);
  Serial.println("Opening Serial Port");

  // Handle EEPROM Interaction and upgrades
  //Check to see if our hardcoded build version set in progam is different than what was last put in EEPROM - detect upgrade.
  if (build_version != check_version())
  { //4 byte string that now holds the build version.
#if DEBUG
    Serial.println(check_version());
    Serial.println("Upgrading Memory");
#endif
    write_defaults_to_eeprom_memory();  //these are for setting for last shot
    set_defaults_in_setup(); //this is for our setup values that should only be defaulted once.
    //review_RAM_Contents();
  }
  else { //load last setting into memory - no upgrade
#if DEBUG
    Serial.println("Restoring EEPROM Values");
#endif
    restore_from_eeprom_memory();
    //review_RAM_Contents();
  }
  //End Setup of EEPROM

  //begin  Setup for Nunchuck
  Nunchuck.init(0);
  delay(50);
  for (uint8_t reads = 1; reads < 17; reads++)
  {
    Nunchuck.getData();
    //Nunchuck.printData();
    lcd.at(2, reads, "+");
    if (abs(Nunchuck.joyx() - 127) > 60 || abs(Nunchuck.joyy() - 127) > 60 )
    {
      lcd.empty();
      draw(57, 1, 1); //lcd.at(1,1,"Center Joystick");
      reads = 1;
    }
    delay(10);
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


void loop2()
{
  if (!(progstep % 100)) {
    Choose_Program();
  }
  else
  {
    switch (progstep / 100)
    {
      case 0: // 2 Point Move Forward
        Move2PointMenu();
        break;
      case 1: // 2 Point Move Reverse
        Move2PointMenu();
        break;
      case 2: // 3 Point Move Forward
        Move3PointMenu();
        break;
      case 3: // 3 Point Move Reverse
        Move2PointMenu();
        break;
      case 4: // Giga Panorama
        PanoGigaMenu();
        break;
      case 5: // Simple Panorama
        PanoSimpleMenu();
        break;
      case 6: // DF Slave
        break;
      case 7: // Setup Menu
        SetupMenu();
        break;
    }
  }
}

void Move2PointMenu() // 0
{
  switch (progstep)
  {
    case 1:   // Move to Start Point
      Move_to_Startpoint();
      break;

    case 2:   // Move to End Point
      Move_to_Endpoint();
      break;

    case 3: //  Set Camera Interval
      Set_Cam_Interval();
      break;

    case 4: //
      Set_Duration();
      break;

    case 5: //        Static Time
      if (intval == VIDEO_INTVAL) { //don't show this for video
        if (progstep_forward_dir) progstep_forward(); //skip the menu, go forward
        else progstep_backward();  //skip the menu, go backward
      }
      else  Set_Static_Time(); //not needed for video
      break;

    case 6: //
      Set_Ramp();
      break;

    case 7: //  Lead in and lead out
      if (intval == VIDEO_INTVAL) { //don't show this for video
        if (progstep_forward_dir) {
          Calculate_Shot(); //
          progstep_forward();
        } //skip the menu, go forward
        else progstep_backward();  //skip the menu, go backward
      }
      else  Set_LeadIn_LeadOut(); //  not needed for video
      break;

    case 8: //  Set Shot Type
      if (intval != VIDEO_INTVAL) { //skip for non video
        if (progstep_forward_dir) {
          progstep_forward();  //skip the menu, go forward
        }
        else progstep_backward();  //skip the menu, go backward
      }
      else Set_Shot_Repeat();
      break;

    case 9: //  review and confirm
      Review_Confirm(); //also has the delay start options here
      break;
      //end of the two point move
  }
}

void Move3PointMenu() // 100
{
  switch (progstep)
  {
    case 101:   // Move Point 0
      Move_to_Point_X(0);
      break;

    case 102:   // Move Point 1
      Move_to_Point_X(1);
      break;

    case 103:   // Move Point 2
      Move_to_Point_X(2);
      break;

    case 104: //  Set Camera Interval
      Set_Cam_Interval();
      break;

    case 105: //
      Set_Duration();
      break;

    case 106: //
      Set_Static_Time();
      break;

    case 107: //
      Set_Ramp();
      break;

    case 108: //
      Set_LeadIn_LeadOut();
      break;

    case 109: //  review and confirm
      Review_Confirm();
      break;
      //end of the three point move
  }
}

void PanoGigaMenu() // 200
{
  switch (progstep)
  {
    //start of pano Mode

    //define field of view
    //We want to know how wide and tall our field of view is in steps so we can get our overlap right.  Anytime you zoom or change lenses, this need to be redefined
    //This should be a 10 seconds process to define by specifying corners
    //Step 1 - Put a point in the upper right corner - set zeros, pan up and right to hit same point with lower left corner of viewfinder
    //Display values  - write to ram - use these values

    case 201:  //
      Move_to_Point_X(0); // move to angle of veiw first corner
      break;

    case 202:  //
      Set_angle_of_view();
      break;

    case 203:   //
      Define_Overlap_Percentage();
      break;

    case 204:   //
      Move_to_Point_X(0);
      break;

    case 205: //
      Move_to_Point_X(1);
      break;

    case 206: //
      Set_Static_Time();
      break;

    case 207: //
      Pano_Review_Confirm();
      break;
      //end of Pano Mode
  }
}

void PanoSimpleMenu() // 300
{
  switch (progstep)
  {
    //start of Portrait Pano Method

    //  define field of view
    //We want to know how wide and tall our field of view is in steps so we can get our overlap right.  Anytime you zoom or change lenses, this need to be redefined
    //This should be a 10 seconds process to define by specifying corners
    //Step 1 - Put a point in the upper right corner - set zeros, pan up and right to hit same point with lower left corner of viewfinder
    //Display values  - write to ram - use these values

    case 301:  //
      Move_to_Point_X(0); //move to sharp point
      break;

    case 302:  //
      Set_angle_of_view();
      break;

    case 303:   //
      Define_Overlap_Percentage();
      break;

    case 304:   //
      Move_to_Point_X(0);  //set subject point
      break;

    case 305: //
      Set_PanoArrayType();   //this sets variable that define how we move camera - load the appropriate array.
      break;

    case 306: //
      Set_Static_Time();
      break;

    case 307: //
      Pano_Review_Confirm();
      break;
      //end of Pano Mode
  }
}

void AuxDistanceMenu()  // 400
{
  switch (progstep)
  {
    //start of entered distance on aux mode
    case 401:   // Move to Start Point
      Move_to_Startpoint();
      break;

    case 402:   // Move to End Point
      Enter_Aux_Endpoint();
      break;

    case 403: //  Set Camera Interval
      Set_Cam_Interval();
      break;

    case 404: //
      Set_Duration();
      break;

    case 405: //
      Set_Static_Time();
      break;

    case 406: //
      Set_Ramp();
      break;

    case 407: //
      Set_LeadIn_LeadOut();
      break;

    case 408: //  review and confirm
      Review_Confirm();
      break;

      //end entered distance mode
  }
}

void SetupMenu() // 900
{
  switch (progstep)
  {
    //start of setup

    case 901:   // AUX_ON
      Setup_AUX_ON();
      break;

    case 902:   // PAUSE_ENABLED
      Setup_PAUSE_ENABLED();
      break;

    case 903:   // POWERSAVE_PT
      Setup_POWERSAVE_PT();
      break;

    case 904: //  POWERSAVE_AUX
      Setup_POWERSAVE_AUX();
      break;

    case 905: //  LCD Bright
      Setup_LCD_BRIGHTNESS_DURING_RUN();
      break;

    case 906: //  Aux Motor Max Speed
      Setup_Max_AUX_Motor_Speed();
      break;

    case 907: //  LCD Bright
      Setup_AUX_Motor_DIR();
      break;

    case 908: //  Exit
      delay(100);
      break;

      //end of setup
  }
}

void MenuMess()
{
  switch (progstep)
  {
    //start of in program menu options

    case 1001:   // AUX_ON
      InProg_Select_Option();
      break;

    //end of in program menu

    //----------------------------------------------------------------------------------------------------

    case 50:  // loop for SMS
      ShootMoveShoot();
      break; //break 50

    case 51:
      VideoLoop();
      break; //break 51 - VIDEO

    case 52:  // loop for external interrupt - external triggering
      ExternalTriggerLoop();
      break; //break 52 - end external triggering loop

    case 90: // end of program - offer repeat and reverse options - check the nuncuck
      EndOfProgramLoop();
      break;  // break 90

    case 250:  // loop for Pano
      PanoLoop();
      break; //break 250


    case 290: //  finished up pano
      PanoEnd();
      break;  // break 90
  }
}

void loop()
{ //Main Loop
  while (1)
  { //use debugging WHEN HIT here for monitoring - {sequence_repeat_type},{progstep},{progtype},{camera_fired}
    switch (progstep)
    {
      //start of 2 point SMS/Video routine
      case REG2POINTMOVE:   //
        Choose_Program();
        break;

      case 1:   // Move to Start Point
        Move_to_Startpoint(); //don't jump in this loop by accident
        break;

      case 2:   // Move to End Point
        Move_to_Endpoint(); //don't jump in this loop by accident
        break;

      case 3: //  Set Camera Interval
        Set_Cam_Interval();
        break;

      case 4: //
        Set_Duration();
        break;

      case 5: //  	    Static Time
        if (intval == VIDEO_INTVAL) { //don't show this for video
          if (progstep_forward_dir) progstep_forward(); //skip the menu, go forward
          else progstep_backward();  //skip the menu, go backward
        }
        else  Set_Static_Time(); //not needed for video
        break;

      case 6: //
        Set_Ramp();
        break;

      case 7: //  Lead in and lead out
        if (intval == VIDEO_INTVAL) { //don't show this for video
          if (progstep_forward_dir) {
            Calculate_Shot(); //
            progstep_forward();
          } //skip the menu, go forward
          else progstep_backward();  //skip the menu, go backward
        }
        else  Set_LeadIn_LeadOut(); //  not needed for video
        break;

      case 8: //  Set Shot Type
        if (intval != VIDEO_INTVAL) { //skip for non video
          if (progstep_forward_dir) {
            progstep_forward();  //skip the menu, go forward
          }
          else progstep_backward();  //skip the menu, go backward
        }
        else Set_Shot_Repeat();
        break;

      case 9: //  review and confirm
        Review_Confirm(); //also has the delay start options here
        break;
      //end of the two point move

      //-------------------------------------------------------------------------------------

      //start of the three point move
      case 100:
        Choose_Program();
        break;

      case 101:   // Move Point 0
        Move_to_Point_X(0);
        break;

      case 102:   // Move Point 1
        Move_to_Point_X(1);
        break;

      case 103:   // Move Point 2
        Move_to_Point_X(2);
        break;

      case 104: //  Set Camera Interval
        Set_Cam_Interval();
        break;

      case 105: //
        Set_Duration();
        break;

      case 106: //
        Set_Static_Time();
        break;

      case 107: //
        Set_Ramp();
        break;

      case 108: //
        Set_LeadIn_LeadOut();
        break;

      case 109: //  review and confirm
        Review_Confirm();
        break;
      //end of the three point move

      //----------------------------------------------------------------------------------------------------------------------

      //start of pano Mode

      //  define field of view
      //We want to know how wide and tall our field of view is in steps so we can get our overlap right.  Anytime you zoom or change lenses, this need to be redefined
      //This should be a 10 seconds process to define by specifying corners
      //Step 1 - Put a point in the upper right corner - set zeros, pan up and right to hit same point with lower left corner of viewfinder
      //Display values  - write to ram - use these values

      case 200:  //
        Choose_Program();
        break;

      case 201:  //
        Move_to_Point_X(0); //move to sharp point
        break;

      case 202:  //
        Set_angle_of_view();
        break;

      case 203:   //
        Define_Overlap_Percentage();
        break;

      case 204:   //
        Move_to_Point_X(0);
        break;

      case 205: //
        Move_to_Point_X(1);
        break;

      case 206: //
        Set_Static_Time();
        break;

      case 207: //
        Pano_Review_Confirm();
        break;
      //end of Pano Mode

      //----------------------------------------------------------------------------------------------------------------------------

      //start of Portrait Pano Method

      //  define field of view
      //We want to know how wide and tall our field of view is in steps so we can get our overlap right.  Anytime you zoom or change lenses, this need to be redefined
      //This should be a 10 seconds process to define by specifying corners
      //Step 1 - Put a point in the upper right corner - set zeros, pan up and right to hit same point with lower left corner of viewfinder
      //Display values  - write to ram - use these values

      case 300:  //
        Choose_Program();
        break;

      case 301:  //
        Move_to_Point_X(0); //move to sharp point
        break;

      case 302:  //
        Set_angle_of_view();
        break;

      case 303:   //
        Define_Overlap_Percentage();
        break;

      case 304:   //
        Move_to_Point_X(0);  //set subject point
        break;

      case 305: //
        Set_PanoArrayType();   //this sets variable that define how we move camera - load the appropriate array.
        break;

      case 306: //
        Set_Static_Time();
        break;

      case 307: //
        Pano_Review_Confirm();
        break;
      //end of Pano Mode

      //------------------------------------------------------------------------------------------------------------------

      //start of entered distance on aux mode
      case 400:   //
        Choose_Program();
        break;

      case 401:   // Move to Start Point
        Move_to_Startpoint();
        break;

      case 402:   // Move to End Point
        Enter_Aux_Endpoint();
        break;

      case 403: //  Set Camera Interval
        Set_Cam_Interval();
        break;

      case 404: //
        Set_Duration();
        break;

      case 405: //
        Set_Static_Time();
        break;

      case 406: //
        Set_Ramp();
        break;

      case 407: //
        Set_LeadIn_LeadOut();
        break;

      case 408: //  review and confirm
        Review_Confirm();
        break;

      //end entered distance mode

      //--------------------------------------------------------------------------------------------------------------

      //start of setup

      case 901:   // AUX_ON
        Setup_AUX_ON();
        break;

      case 902:   // PAUSE_ENABLED
        Setup_PAUSE_ENABLED();
        break;

      case 903:   // POWERSAVE_PT
        Setup_POWERSAVE_PT();
        break;

      case 904: //  POWERSAVE_AUX
        Setup_POWERSAVE_AUX();
        break;

      case 905: //  LCD Bright
        Setup_LCD_BRIGHTNESS_DURING_RUN();
        break;

      case 906: //  Aux Motor Max Speed
        Setup_Max_AUX_Motor_Speed();
        break;

      case 907: //  LCD Bright
        Setup_AUX_Motor_DIR();
        break;

      case 908: //  Exit
        delay(100);
        break;

      //end of setup

      //-----------------------------------------------------------------------------------------------------

      //start of in program menu options

      case 1001:   // AUX_ON
        InProg_Select_Option();
        break;

      //end of in program menu

      //----------------------------------------------------------------------------------------------------

      case 50:  // loop for SMS
        ShootMoveShoot();
        break; //break 50

      case 51:
        VideoLoop();
        break; //break 51 - VIDEO

      case 52:  // loop for external interrupt - external triggering
        ExternalTriggerLoop();
        break; //break 52 - end external triggering loop

      case 90: // end of program - offer repeat and reverse options - check the nuncuck
        EndOfProgramLoop();
        break;  // break 90

      case 250:  // loop for Pano
        PanoLoop();
        break; //break 250


      case 290: //  finished up pano
        PanoEnd();
        break;  // break 90

      default:
        lcd.at(1, 2, "Menu Error");
    } //switch
  } // while
} //loop[