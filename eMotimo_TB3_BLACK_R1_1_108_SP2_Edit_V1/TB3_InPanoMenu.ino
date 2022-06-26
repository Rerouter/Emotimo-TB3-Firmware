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

uint16_t panoprogtype = 0;
uint16_t restore_progstep = 0;

//In Program Menu Ordering

#define PANO_OPTIONS  5
enum panoprogtype : uint8_t {
  PANO_RESUME,
  PANO_GOTO_START,
  PANO_GOTO_END,
  PANO_GOTO_IMAGE,
  PANO_SHUTTER_TIME,
  //PANO_FOCUS_INT    = 5,
  //PANO_EXT_TRIGGER  = 6,
  //PANO_DETOUR       = 7,
  //PANO_AOV          = 8,
  //PANO_OVERLAP      = 9,
  //PANO_START_POINT  = 10,
  //PANO_END_POINT    = 11
};

static uint32_t intcamerafired;  // Internal variable to hold the current image in a panorama so we can edit it, and only apply it if we press throug
static uint16_t intstatic_tm;    // Internal variable to hold the current image firing time.

void Pano_Pause() //this runs once and is quick - not persistent
{
  lcd.empty();
  lcd.at(1, 1, "Pausing");
  delay(prompt_time);
  Program_Engaged = false; //toggle off the loop
  if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
  if (POWERSAVE_AUX > PWR_PROGRAM_ON)   disable_AUX();
  panoprogtype = 0;   //default this to the first option
  restore_progstep = progstep; // Grab the progstep so we can mess with enums without needing to worry
  progstep_goto(299); //go to the Pano_Paused_Menu item and loop
}


void Pano_Resume() //this runs once and is quick - not persistent
{
  Program_Engaged = true; //toggle off the loop
  lcd.empty();
  lcd.at(1, 1, "Resuming");
  delay(prompt_time);
  progstep_goto(restore_progstep); //send us back to the main Panorama Loop
}


void Pano_Paused_Menu()
{
  if (redraw)
  {
    lcd.empty();
    switch(panoprogtype)
    {
      case PANO_RESUME:
        draw(86, 1, 5); //lcd.at(1,6,"Resume");
        break;

      case PANO_GOTO_START:
        draw(87, 1, 5); //lcd.at(1,6,"Restart");
        break;

      case PANO_GOTO_END:
        draw(89, 1, 4); //lcd.at(1,5,"Go to End");
        break;

      case PANO_GOTO_IMAGE:
        draw(88, 1, 1); //lcd.at(1,1,"GoTo Frame:");
        intcamerafired = camera_fired;
        DisplayGoToPanoShot(intcamerafired);
        break;

      case PANO_SHUTTER_TIME:
        draw(23, 1, 1); //lcd.at(1,1,"Stat_T:   .  sec");
        intstatic_tm = static_tm;
        DisplayStatic_tm(intstatic_tm);
        break;

//      case PANO_FOCUS_INT:
//        lcd.at(1,1,"Change FocusTime");
//        break;
//
//      case PANO_EXT_TRIGGER:
//        lcd.at(1,1,"Change Ext_Trig");
//        break;
//
//      case PANO_DETOUR:
//        lcd.at(1,1,"Detour Mid Shot");
//        break;
//
//      case PANO_AOV:
//        lcd.at(1,1,"Change AOV");
//        break;
//
//      case PANO_OVERLAP:
//        lcd.at(1,1,"Change Overlap");
//        break;
//
//      case PANO_START_POINT:
//        lcd.at(1,1,"Change Start Pos");
//        break;
//
//      case PANO_END_POINT:
//        lcd.at(1,1,"Change End Pos");
//        break;

      default:
        panoprogtype = 0;
    }

    lcd.at(2, 1, "UpDown  C-Select");
    if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
    if (POWERSAVE_AUX > PWR_PROGRAM_ON)   disable_AUX();
    delay(prompt_time);
    redraw = false;
  } //end first time
    
  switch(panoprogtype)
  {
    case PANO_GOTO_IMAGE:
    {
      //read leftright values for the goto frames
      uint32_t intcamerafired_last = intcamerafired;
  
      intcamerafired -= joy_capture3(0);
      uint16_t maxval = camera_total_shots;
      uint16_t overflowval = max(maxval + 100, 65400);
      if (intcamerafired > overflowval)  { intcamerafired = maxval; }
      else if (intcamerafired > maxval)  { intcamerafired = 0;  }

      if (intcamerafired_last != intcamerafired) {
        DisplayGoToPanoShot(intcamerafired);
        delay(prompt_delay);
      }
      break;
    }

    case PANO_SHUTTER_TIME:
      //read leftright values for the goto frames
      uint32_t intstatic_tm_last = intstatic_tm;
  
      intstatic_tm -= joy_capture3(0);
      uint16_t maxval = max_shutter;
      uint16_t overflowval = max(maxval + 100, 65400);
      if (!intstatic_tm || intstatic_tm > overflowval)  { intstatic_tm = maxval; }
      else if (intstatic_tm > maxval)                   { intstatic_tm = 1; }
    
      if (intstatic_tm_last != intstatic_tm) {
        DisplayStatic_tm(intstatic_tm);
        delay(prompt_delay);
      }
      break;
  }

  if ((millis() - NClastread) > 50)
  {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    
    switch(joy_capture_y_map())
    {
      case -1: // Up
        panoprogtype++;
        if (panoprogtype > (PANO_OPTIONS - 1))  panoprogtype = 0;
        else
        {
          redraw = true;
        }
        break;
  
      case 1: // Down
        panoprogtype--;
        if (panoprogtype > (PANO_OPTIONS - 1))  panoprogtype = (PANO_OPTIONS - 1);
        else
        {
          redraw = true;
        }
        break;
    }

    button_actions_Pano_Paused_Menu();
  }
}


void button_actions_Pano_Paused_Menu()
{
  switch (HandleButtons())
  {
    case C_Pressed:
      lcd.empty();
      switch(panoprogtype)
      {
        case PANO_RESUME:
          Pano_Resume();
          break;

        case PANO_GOTO_START: //Return to restart the shot  - send to review screen of relative move
          camera_fired = camera_total_shots;
          lcd.at(1, 2, "Going to Start");
          delay(prompt_time);
          Pano_Resume();
          break;

        case PANO_GOTO_END:
          camera_fired = camera_total_shots;
          lcd.at(1, 3, "Going to End");
          delay(prompt_time);
          Pano_Resume();
          break;

        case PANO_GOTO_IMAGE:
          redraw = true;
          lcd.at(1, 4, "Going to");
          lcd.at(2, 4, "Frame:");
          lcd.at(2, 11, intcamerafired);
          camera_fired = intcamerafired;
          move_motors_pano_accel();
          do {
            if (!nextMoveLoaded)  updateMotorVelocities();  //finished up the interrupt routine
          }
          while (motorMoving);
          panoprogtype = PANO_RESUME;
          break;

        case PANO_SHUTTER_TIME:
          redraw = true;
          lcd.at(1, 4, "Updated to");
          lcd.at(2, 4, "Seconds:");
          lcd.at(2, 9, intcamerafired/10);
          static_tm = intstatic_tm;
          panoprogtype = PANO_RESUME;
          break;

//        case PANO_DETOUR:
//          lcd.at(1,1,"Detour Mid Shot");
//          break;
      }
      break;
    case Z_Pressed:
      break;
  }
}


void DisplayGoToPanoShot(uint32_t image_count)
{
  lcd.at(1, 13, image_count);
  if      (image_count < 10)    lcd.at(1, 14, "   "); //clear extra if goes from 3 to 2 or 2 to 1
  else if (image_count < 100)   lcd.at(1, 15, "  ");  //clear extra if goes from 3 to 2 or 2 to 1
  else if (image_count < 1000)  lcd.at(1, 16, " ");   //clear extra if goes from 3 to 2 or 2 to 1
  else if (image_count < 10000) lcd.at(1, 16, "");    //clear extra if goes from 3 to 2 or 2 to 1
}
