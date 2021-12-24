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

//Debug Parameters
#define DEBUG 0
#define DEBUG_MOTOR 0
#define DEBUG_NC 0
#define DEBUG_PANO 0
#define DEBUG_GOTO 0

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
  // ASCOMSLAVE    = 98,
  AUXDISTANCE   = 99
};
uint8_t      progtype = REG2POINTMOVE; //updownmenu selection

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
uint8_t       inprogtype = INPROG_RESUME; //updownmenu selection during shoot


enum Move_State_2PT : uint8_t {
  LeadIn2PT   = 1,
  RampUp2PT   = 2,
  Linear2PT   = 3,
  RampDown2PT = 4,
  LeadOut2PT  = 5,
  Finished2PT = 9
};
uint8_t      Move_State_2PT = LeadIn2PT;


enum Move_State_3PT : uint8_t {
  LeadIn3PT    = 1,
  FirstLeg3PT  = 2,
  SecondLeg3PT = 3,
  ThirdLeg3PT  = 4,
  LeadOut3PT   = 5,
  Finished3PT  = 9
};
uint8_t      Move_State_3PT = LeadIn3PT;


enum Trigger_Type : uint16_t {
  Feedback_Trigger    = 1,  // Wait for camera hot shoe singal feedback to advance
  Video_Trigger       = 2,  // Basically just keep the camera pins held for the entire program
  External_Trigger    = 3,  // Only advance a program when an external trigger signal is recieved
  Button_Trigger      = 4,  // Only advance a program when the C button is held
  Static_Time_Trigger = 5   // Advance at a fixed time for a program
};
uint16_t      Trigger_Type = 3;


//Interval Options
#define MIN_INTERVAL_STATIC_GAP 3  //min gap between interval and static time
//#define STOPMOT //not used

// Defines
#define POWERDOWN_LV false //set this to cause the TB3 to power down below 10 volts
#define MAX_MOVE_POINTS 3
#define VIDEO_FEEDRATE_NUMERATOR 375L // Set this for 42000L, or 375L for faster calc moves


//TB3 section - Black or Orange Port Mapping for Step pins on Stepper Page
#define MOTORS      3

#define MOTOR0_STEP	5
#define MOTOR1_STEP	6
#define MOTOR2_STEP	7

#define MOTOR0_DIR	8
#define MOTOR1_DIR	9
#define MOTOR2_DIR	10

#define MOTOR_EN    A3
#define MOTOR_EN2   11

#define MS1         A1
#define MS2         A2
#define MS3         A2

#define IO_2        2 // drives middle of 2.5 mm connector on I/O port
#define IO_3        3 // drives tip of 2.5 mm connector on I/O port

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

uint32_t      build_version         = 10952; // this value is compared against what is stored in EEPROM and resets EEPROM and setup values if it doesn't match
//uint32_t    intval                = 2;     //0.1x Seconds  - used for the interval prompt and display
uint32_t      interval              = 2000;  // calculated and is in ms
uint32_t      camera_fired          = 0;     // number of shots fired
uint32_t      camera_moving_shots   = 200;   // frames for new duration/frames prompt
uint32_t      camera_total_shots    = 0;     // used at the end target for camera fired to compare against
uint16_t      overaldur             = 20;    // seconds now for video only
uint16_t      static_tm             = 1;     // 0.1x Seconds exposure time of an image
uint16_t      prefire_time          = 1;     // 0.1x Seconds how long to power on the motors and focus the camera before taking a photo
uint16_t      postfire_time         = 1;     // 0.1x Seconds how long to wait after an image to begin moving again
uint16_t      rampval               = 50;
uint16_t      lead_in               = 1;
uint16_t      lead_out              = 1;
uint16_t      start_delay_sec       = 0;
float         total_pano_move_time  = 0;
uint8_t       HeldThreshold         = 1;     // Number of read events to recognise a button as held

//External Interrupt Variables
volatile bool iostate               = false; //new variable for interrupt
volatile bool changehappened        = false; //new variable for interrupt 
boolean       ext_shutter_open      = false;
uint8_t       ext_shutter_count     = 0; // How many times the shutter has been triggered to count up the HDR images remaining
uint8_t       ext_hdr_shots         = 1; //this is how many shots are needed before moving - leave at one for normal shooting - future functionality with external

//3 Point motor routine values
int32_t       motor_steps_pt[MAX_MOVE_POINTS][MOTORS];  // 3 total points.   Start point is always 0
uint32_t      percent; //% through a leg
uint16_t      keyframe[2][6]; //this is basically the keyframes {start, end of rampup, start or rampdown, end}   - doesn't vary by motor at this point
int32_t       linear_steps_per_shot [MOTORS]; //{This is for the calculated or estimated steps per shot in a segment for each motor
int32_t       ramp_params_steps [MOTORS]; //This is to calc the steps at the end of rampup for each motor.  Each array value is for a motor

//Program Status Flags
boolean       Program_Engaged =        false;
boolean       Move_Engaged =           false;
boolean       Flag_Wait_For_Trigger = false;

//New Powersave flags
/*Power Save explanation
  We can power up and power down the Pan Tilt motors together.  We can power up and power down the Aux motor port as well.  We see three levels of power saving:
  1)  None - Motors are always on - for VFX work where power isn't a factor and precision is most important.  Motors will get warm here on hot days.
  2)  Low - only at the end of program
  3)  Standard - Power up the motors for the shooting time (all the time we hold the trigger down), and move, power down between shots.
  4)  High - Only power on for motor moves, turn off the motors when we reach the shooting position.
*/

enum powersave : uint8_t {
  PWR_ALWAYS_ON    = 0, // Motors are always on,
  PWR_PROGRAM_ON   = 1, // Motors at full power when ever a program is active, in low power in main menu
  PWR_SHOOTMOVE_ON = 2, // Motors at full power for moves and image capturing, in low power otherwise
  PWR_MOVEONLY_ON  = 3  // Motors only powered for movements, it can loose up to 8 steps position per stop
};

//CVariables that are set during the Setup Menu store these in EEPROM
uint8_t       POWERSAVE_PT;  //1=None - always on  2 - low   3=standard    4=High
uint8_t       POWERSAVE_AUX;  //1=None - always on  2 - low   3=standard    4=High
boolean       AUX_ON;  //1=Aux Enabled, 0=Aux disabled
boolean       PAUSE_ENABLED;  //1=Pause Enabled, 0=Pause disabled
boolean       REVERSE_PROG_ORDER; //Program ordering 0=normal, start point first. 1=reversed, set end point first to avoid long return to start
boolean       MOVE_REVERSED_FOR_RUN;
uint8_t       LCD_BRIGHTNESS_RUNNING; //0 is off 8 is max
uint8_t       LCD_BRIGHTNESS_MENU;    //0 is off 8 is max
uint16_t      AUX_MAX_JOG_STEPS_PER_SEC; //value x 1000  20 is the top or 20000 steps per second.
uint16_t      PAN_MAX_JOG_STEPS_PER_SEC = 65535;
uint16_t      TILT_MAX_JOG_STEPS_PER_SEC = 65535;
boolean       AUX_REV;  //1=Aux Enabled, 0=Aux disabled
boolean       SERPENTINE = 1; // 0=All rows start from same side, 1 = Rows alternate to minimise time


//control variable, no need to store in EEPROM - default and setup during shot
uint16_t      progstep = 0; //used to define case for main loop
boolean       progstep_forward_dir = true; //boolean to define direction of menu travel to allow for easy skipping of menus

boolean       reset_prog  = true; //used to handle program reset or used stored
boolean       redraw  = true; //variable to help with LCD dispay variable that need to show one time
boolean       redraw2 = true;

uint16_t      max_shutter; // Maximum shutter time in 0.1 second increments
//unsigned int max_prefire; // Maximum focus / motor wakeup time in 0.1 second increments
uint32_t      interval_tm      = 0;  //mc time to help with interval comparison
uint32_t      interval_tm_last = 0; //mc time to help with interval comparison

uint32_t      display_last_tm = 0;
uint16_t      prompt_time = 500; // in ms for delays of instructions
int16_t       prompt_delay; //to help with joystick reads and delays for inputs - this value is set during joystick read and executed later in the loop

uint8_t       reviewprog = 1;

uint32_t      start_delay_tm = 0;  //ms timestamp to help with delay comparison
uint32_t      goto_shot = 0;

boolean       sequence_repeat_type = false; //Defaults - Run Once = 0, Continuous Loop = 1
uint8_t       sequence_repeat_count = 0; //counter to hold variable for how many time we have repeated

//remote and interface variables

int8_t        joy_x_axis, joy_y_axis;
int8_t        acc_x_axis, acc_y_axis;

int16_t       PanStepCount;
int16_t       TiltStepCount;

//remote and interface variables

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
uint8_t ButtonState = Read_Again;

uint32_t NClastread = 1000; //control variable for NC reads cycles
uint32_t NCdelay    = 10;      //milliseconds to wait before reading the joystick again

//Stepper Setup
uint32_t  feedrate_micros = 0;

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
uint8_t motorMoving = 0;          // Program Flag bit array for motor moving

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
  pinMode(MOTOR0_DIR,	OUTPUT);
  
  pinMode(MOTOR1_STEP,	OUTPUT);
  pinMode(MOTOR1_DIR,	OUTPUT);
  
  pinMode(MOTOR2_STEP,	OUTPUT);
  pinMode(MOTOR2_DIR,	OUTPUT);

  pinMode(MS1,	        OUTPUT);
  pinMode(MS2,	        OUTPUT);
  pinMode(MS3,	        OUTPUT);

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
  // To make sure we handle cases where its been set to 0,
  // 1 = backlight off
  // 2 = minimum backlight on
  if (LCD_BRIGHTNESS_MENU) {lcd.bright(LCD_BRIGHTNESS_MENU);}
  else lcd.bright(2); {LCD_BRIGHTNESS_MENU = 2;} 

  delay(prompt_time * 2);
  lcd.empty();
  
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

  //Setup Nunchucks and Calibrate
  draw(2, 1, 1); // Connect Joystick
  Nunchuck.init(0);
  for (uint8_t reads = 1; reads < 17; reads++)
  {
    NunChuckRequestData();
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
  lcd.empty();
  //end  Setup for Nunchuk


  //Setup Motors
  init_steppers();

  //init_external_triggering();
  pinMode(IO_3, INPUT);
  digitalWrite(IO_3, HIGH);
  attachInterrupt(1, cam_change, CHANGE);

} //end of setup


void loop()
{ //Main Loop
  while (1)
  { //use debugging WHEN HIT here for monitoring - {!sequence_repeat_type},{progstep},{progtype},{camera_fired}
    switch (progstep)
    {
      //start of 2 point SMS/Video routine
      
      case 0:  Choose_Program();      break; 
      case 1:  Move_to_Startpoint();  break;   // Move to Start Point
      case 2:  Move_to_Endpoint();    break;  // Move to End Point
      case 3:  Set_Cam_Interval();    break;  //  Set Camera Interval
      case 4:  Set_Duration();        break;

      case 5: //  	    Static Time
        if (Trigger_Type == Video_Trigger) { //don't show this for video
          if (progstep_forward_dir) progstep_forward(); //skip the menu, go forward
          else progstep_backward();  //skip the menu, go backward
        }
        else  Set_Static_Time(); //not needed for video
        break;

      case 6:  Set_Ramp();            break;

      case 7: //  Lead in and lead out
        if (Trigger_Type == Video_Trigger) { //don't show this for video
          if (progstep_forward_dir) {
            Calculate_Shot(); //
            progstep_forward();
          } //skip the menu, go forward
          else progstep_backward();  //skip the menu, go backward
        }
        else  Set_LeadIn_LeadOut(); //  not needed for video
        break;

      case 8: //  Set Shot Type
        if (Trigger_Type != Video_Trigger) { //skip for non video
          if (progstep_forward_dir) {
            progstep_forward();  //skip the menu, go forward
          }
          else progstep_backward();  //skip the menu, go backward
        }
        else Set_Shot_Repeat();
        break;

      case 9:  Review_Confirm();     break;  // review and confirm
      
      //end of the two point move

      //-------------------------------------------------------------------------------------

      //start of the three point move
      
      case 100:  Choose_Program();      break;
      case 101:  Move_to_Point_X(0);    break;  // Move Point 0
      case 102:  Move_to_Point_X(1);    break;  // Move Point 1
      case 103:  Move_to_Point_X(2);    break;  // Move Point 2
      case 104:  Set_Cam_Interval();    break;  // Set Camera Interval
      case 105:  Set_Duration();        break;
      case 106:  Set_Static_Time();     break;
      case 107:  Set_Ramp();            break;
      case 108:  Set_LeadIn_LeadOut();  break;
      case 109:  Review_Confirm();      break;  // review and confirm
      
      //end of the three point move

      //----------------------------------------------------------------------------------------------------------------------

      //start of pano Mode

      //  define field of view
      //We want to know how wide and tall our field of view is in steps so we can get our overlap right.  Anytime you zoom or change lenses, this need to be redefined
      //This should be a 10 seconds process to define by specifying corners
      //Step 1 - Put a point in the upper right corner - set zeros, pan up and right to hit same point with lower left corner of viewfinder
      //Display values  - write to ram - use these values

      case 200:  Choose_Program();             break;
      case 201:  Set_angle_of_view();          break;
      case 202:  Define_Overlap_Percentage();  break;
      case 203:  Move_to_Point_X(0);           break;
      case 204:  Move_to_Point_X(1);           break;
      case 205:  Set_Static_Time();            break;
      case 206:  Pano_Review_Confirm();        break;
      
      //end of Pano Mode

      case 250:  PanoLoop();                   break;  // loop for Pano
      case 290:  PanoEnd();                    break;  // finished up pano
      case 299:  Pano_Paused_Menu();           break;

      //----------------------------------------------------------------------------------------------------------------------------

      //start of Portrait Pano Method

      //  define field of view
      //We want to know how wide and tall our field of view is in steps so we can get our overlap right.  Anytime you zoom or change lenses, this need to be redefined
      //This should be a 10 seconds process to define by specifying corners
      //Step 1 - Put a point in the upper right corner - set zeros, pan up and right to hit same point with lower left corner of viewfinder
      //Display values  - write to ram - use these values

      case 300:  Choose_Program();             break;
      case 301:  Set_angle_of_view();          break;
      case 302:  Define_Overlap_Percentage();  break;
      case 303:  Move_to_Point_X(0);           break;  // set subject point
      case 304:  Set_PanoArrayType();          break;  // this sets variable that define how we move camera - load the appropriate array.
      case 305:  Set_Static_Time();            break;
      case 306:  Pano_Review_Confirm();        break;
      
      //end of Pano Mode

      //------------------------------------------------------------------------------------------------------------------

      //start of entered distance on aux mode
      
      case 400:  Choose_Program();        break;
      case 401:  Move_to_Startpoint();    break;  // Move to Start Point
      case 402:  Enter_Aux_Endpoint();    break;  // Move to End Point
      case 403:  Set_Cam_Interval();      break;  // Set Camera Interval
      case 404:  Set_Duration();          break;
      case 405:  Set_Static_Time();       break;
      case 406:  Set_Ramp();              break;
      case 407:  Set_LeadIn_LeadOut();    break;
      case 408:  Review_Confirm();        break;  // review and confirm

      //end entered distance mode

      //--------------------------------------------------------------------------------------------------------------

      //start of setup
      
      case 901:  Setup_AUX_ON();                     break;  // AUX_ON
      case 902:  Setup_PAUSE_ENABLED();              break;  // PAUSE_ENABLED
      case 903:  Setup_POWERSAVE_PT();               break;  // POWERSAVE_PT
      case 904:  Setup_POWERSAVE_AUX();              break;  // POWERSAVE_AUX
      case 905:  Setup_LCD_BRIGHTNESS_DURING_RUN();  break;  // LCD Bright
      case 906:  Setup_Max_AUX_Motor_Speed();        break;  // Aux Motor Max Speed
      case 907:  Setup_AUX_Motor_DIR();              break;  // LCD Bright
      case 908:  ReturnToMenu();                     break;  // Exit
      
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

      default:
        lcd.at(1, 2, "Menu Error");
        delay(prompt_time);
        ReturnToMenu();
    } //switch
  } // while
} //loop
