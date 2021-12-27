//Program Status Flags
boolean Flag_Prefire_Focus_Enabled  = false;
boolean Flag_Shot_Timer_Active      = false;
boolean Shot_Sequence_Started       = false;

//Start of variables for Pano Mode
boolean   move_with_acceleration = true; // false = no accel, true = accel, so far read only


void ShootMoveShoot()
{
  static uint8_t smsState = 1; // Start out with the first shot
  static uint8_t smsFlag  = 0; // Start out with an invalid flag
//  Serial.print("smsState: ");
//  Serial.print(smsState);
//  Serial.print("  smsFlag: ");
//  Serial.print(smsFlag);
//  Serial.print("  TriggerType: ");
//  Serial.print(Trigger_Type);
//  Serial.print("  progtype: ");
//  Serial.println(progtype);
  
  switch(smsState) {
    case 0: // Idle State, waiting for flag conditions to advance
      break;
      
    case 1: // Setup State, prefocuses the camera and enables the motors
      smsFlag = 1;
      interval_tm = millis(); // Start the clock for out image
      CameraFocus(true); // Wake up the camera and begin focusing in the prefire time
      if (POWERSAVE_PT <= PWR_PROGRAM_ON)   enable_AUX(); // Turn on power to the motors if not move only
      if (AUX_ON && POWERSAVE_AUX <= PWR_PROGRAM_ON)   enable_AUX(); // Turn on power to the motors if not move only
      break;

    case 2:
      if (POWERSAVE_PT == PWR_SHOOTMOVE_ON)   enable_PT(); // Turn on power to the motors if shoot only
      if (AUX_ON && POWERSAVE_AUX <= PWR_SHOOTMOVE_ON)   enable_AUX(); // Turn on power to the motors if not move only
      if ( Trigger_Type == External_Trigger || Trigger_Type == Button_Trigger) {
        smsFlag = 5; // Wait for external trigger
      }
      else {
        smsFlag = 2;
        CameraShoot((uint32_t)static_tm * 100); // Set the camera to take an image x milliseconds long
        camera_fired++;
        display_status();
      }
      break;

    case 3:
      smsFlag = 3; // Set motors moving to the next position
      if (POWERSAVE_PT == PWR_MOVEONLY_ON)   enable_PT(); // Turn on power to the motors if move only
      if (AUX_ON && POWERSAVE_AUX <= PWR_MOVEONLY_ON)   enable_AUX(); // Turn on power to the motors if not move only
      move_motors();
      break;

    case 4:
      //if (!nextMoveLoaded)  updateMotorVelocities();  //finished up the interrupt routine
      if (!motorMoving) {  // motors completed the move
        if (POWERSAVE_PT >= PWR_SHOOTMOVE_ON)   disable_PT();
        if (AUX_ON && POWERSAVE_AUX <= PWR_SHOOTMOVE_ON)  disable_AUX();
        smsFlag = 4;
      }
      break;
      
    case 5:
      if (changehappened)
      {
        changehappened = false;
        if (!iostate) {  //The trigger is active, fire the camera
         smsFlag = 6;
         CameraShutter(true); // Fire the camera immediatly
        }
      }
      break;
    case 6: // External Trigger Handling
      if (changehappened)
      {
        changehappened = false;
        if (iostate) {  //The trigger is released, stop the camera
         smsFlag = 2;
         CameraShutter(false); // Fire the camera immediatly
         CameraFocus(false);   // Release the camera focus
         camera_fired++;
         interval_tm = millis();
         display_status();
        }
      }
      break;
  }

  switch(smsFlag)
  {
    case 0: // Idle State, waiting for external trigger to advance
      break;
    case 1: // Wait for prefire time to elapse
      if((millis() - interval_tm) > (prefire_time * 100)) smsState = 2; // Fire Camera
      else smsState = 0;
      break;
    case 2:  // Wait for exposure time to elapse
      if(!CameraShutter() && (millis() - interval_tm) > (prefire_time * 100 + static_tm * 100)) smsState = 3; // Prepare motors
      else if (!CameraShutter() && (Trigger_Type == External_Trigger || Trigger_Type == Button_Trigger)) smsState = 3;
      else smsState = 0;
      break;
    case 3: // Wait for camera shutter and focus to release
      if(!CameraShutter() && (millis() - interval_tm) > (postfire_time * 100 + prefire_time * 100 + static_tm * 100)) smsState = 4; // Move Motors
      else if (Trigger_Type == External_Trigger || Trigger_Type == Button_Trigger) {
        if(!CameraShutter() && (millis() - interval_tm) > (postfire_time * 100)) smsState = 4;
      }
      else smsState = 0;
      break;
    case 4: // Jump Back to Start
      smsState = 1;
      break;
    case 5: // Wait for external trigger
      smsState = 5; 
      smsFlag = 0;
      break;
    case 6: // Handle external trigger
      smsState = 6;
      smsFlag = 0;
      break;
  }

  // ------ End of state machine ------ //

  if (!CameraShutter() && camera_fired >= camera_total_shots ) {  // end of program
    smsState = 1;
    smsFlag = 0;
    lcd.empty();
    draw(58, 1, 1); // lcd.at(1,1,"Program Complete");
    Program_Engaged = false;
    if (POWERSAVE_PT > PWR_ALWAYS_ON)   disable_PT();
    if (AUX_ON && POWERSAVE_AUX > PWR_ALWAYS_ON)   disable_AUX();
    delay(prompt_time);
    progstep = 90;
    redraw = true;
  }

  if ((millis() - NClastread) > NCdelay) { // Nunchuck update for button events
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    if (smsFlag == 5 && HandleButtons() == C_Held && Trigger_Type == Button_Trigger ) smsState = 1; // Trigger advance with the button
    if (PAUSE_ENABLED && HandleButtons() == CZ_Held && !Shot_Sequence_Started ) SMS_In_Shoot_Paused_Menu();         // Pause Panorama and be able to change settings
  }
}
/* 
  // //Step 1 if internal interval.Kick off the shot sequence. This happens once per camera shot.
  if (!(Shot_Sequence_Started) && Trigger_Type >= External_Trigger && Program_Engaged && ((millis() - interval_tm) > interval) )
  {
    interval_tm = millis(); //start the clock on our shot sequence
    Flag_Wait_For_Trigger = false; //clear this flag to avoid rentering this routine
    Shot_Sequence_Started = true; //
    Flag_Prefire_Focus_Enabled = true; //
    CameraFocus(true); //for longer shot interval, wake up the camera

    if (POWERSAVE_PT < PWR_MOVEONLY_ON)              enable_PT(); //don't power on for shot for high power saving
    if (AUX_ON && POWERSAVE_AUX < PWR_MOVEONLY_ON)   enable_AUX(); //don't power on for shot for high power saving
  }



  //Step 1 if external triggering. This happens once per camera shot.
  if ( Program_Engaged && !(Shot_Sequence_Started) && (Trigger_Type == External_Trigger) && Flag_Wait_For_Trigger )
  {
    interval_tm = millis(); //start the clock on our shot sequence

    Flag_Wait_For_Trigger = false; //clear this flag to avoid rentering this routine

    Shot_Sequence_Started = true; //
    Flag_Prefire_Focus_Enabled = true; //
    CameraFocus(true); //for longer shot interval, wake up the camera

    if (POWERSAVE_PT < PWR_MOVEONLY_ON)              enable_PT(); //don't power on for shot for high power saving
    if (AUX_ON && POWERSAVE_AUX < PWR_MOVEONLY_ON)   enable_AUX(); //don't power on for shot for high power saving
  }

  //End our prefire - check that we are in program active,shot cycle engaged, and prefire engaged and check against our prefire time
  //If so set prefire flag off, static flag on, fire camera for static time value, update the display

  if ((Shot_Sequence_Started) && (Flag_Prefire_Focus_Enabled)  && ((millis() - interval_tm) > prefire_time * 100))
  {
    Flag_Prefire_Focus_Enabled = false;
    Flag_Shot_Timer_Active = true;
    //Fire Camera
    if (Trigger_Type != External_Trigger ) CameraShoot((uint32_t)static_tm * 100); //start shutter sequence
    camera_fired++;
  }
  //End out static time - check that we are in an program active and static time,  Shutter not engaged, check shot cycle time agains prefire+statictime
  //If so remove flags from Static Time Engaged and IO engaged, Turn off I/O port, set flags for motors moving, move motors
  //move motors - figure out delays.   Long delays mean really slow - choose the minimum of the calculated or a good feedrate that is slow

  //if (Program_Engaged && Shot_Sequence_Started && Flag_Shot_Timer_Active && !Shutter_Signal_Engaged && ((millis() - interval_tm) > (prefire_time * 100 + static_tm * 100)) ) {
  if (Shot_Sequence_Started && Flag_Shot_Timer_Active && !CameraShutter() && ((millis() - interval_tm) > (prefire_time * 100 + static_tm * 100)) )
  { //removed requirement for Program Engaged for external interrupt
    Flag_Shot_Timer_Active = false; //Static Time Engaged is OFF

    //Move the motors - each motor move is calculated by where we are in the sequence - we still call this for lead in and lead out - motors just don't move
    
    Move_Engaged = true; //move motors
    move_motors();

    //Turn off the motors if we have selected powersave 3 and 4 are the only ones we want here
    if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
    if (POWERSAVE_AUX > PWR_PROGRAM_ON)  disable_AUX(); //

    //Update display
    if (Trigger_Type != External_Trigger ) display_status(); //update after shot complete to avoid issues with pausing

    Shot_Sequence_Started = false; //Shot sequence engaged flag is is off - we are ready for our next
    Flag_Wait_For_Trigger = false;
    //InterruptAction_Reset(); //enable the external interrupts to start a new shot
  }

  // ------ End of State Machine ------ //

  if ( camera_moving_shots > 0  && camera_fired >= camera_total_shots && !CameraShutter())
  { //end of program
    lcd.empty();
    draw(58, 1, 1); //lcd.at(1,1,"Program Complete");
    Program_Engaged = false;
    if (POWERSAVE_PT > PWR_ALWAYS_ON)   disable_PT(); //  low, standard, high, we power down at the end of program
    if (POWERSAVE_AUX > PWR_ALWAYS_ON)  disable_AUX(); // low, standard, high, we power down at the end of program
    delay(prompt_time);
    progstep = 90;
    redraw = true;
  }

  //This portion always runs in empty space of loop.

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    //if (HandleButtons() == CZ_Held && Trigger_Type==External_Trigger ) Flag_Wait_For_Trigger=true; // manual trigger
    //if (PAUSE_ENABLED && HandleButtons() == CZ_Held && Trigger_Type>External_Trigger  && !Shot_Sequence_Started ) Pause_Prog(); //pause an SMS program
    if (PAUSE_ENABLED && HandleButtons() == CZ_Held && Trigger_Type >= External_Trigger  && !Shot_Sequence_Started) SMS_In_Shoot_Paused_Menu(); //jump into shooting menu
  }
}
*/

void VideoLoop ()
{
  //main video loop interrupt based.  This runs for 2 point moves only.

  if (progtype == REG2POINTMOVE || progtype == REV2POINTMOVE)
  {
    synched3AxisMove_timed(motor_steps_pt[2][0], motor_steps_pt[2][1], motor_steps_pt[2][2], float(overaldur), float(rampval / 100.0));
    if (maxVelLimit) { //indicates the move is limited to enforce velocity limit on motors)
      lcd.at(2, 1, "Speed Limit");
    }
    //Start us moving
    // interval_tm_last = interval_tm;
    interval_tm = millis();
    startISR1 ();
    do
    {
      if (!nextMoveLoaded)
      {
        updateMotorVelocities();
      }
    }
    while (motorMoving);
    stopISR1 ();
#if DEBUG
    Serial.print("Video Runtime"); Serial.println(millis() - interval_tm);
#endif

    if (!motorMoving && sequence_repeat_type)
    { //new end condition for RUN CONTINOUS
      boolean break_continuous = false;
      lcd.empty();
      draw(58, 1, 1); //lcd.at(1,1,"Program Complete");
      Program_Engaged = false;
      for (uint8_t i = 0; i < 30; i++) {
        NunChuckRequestData();
        NunChuckProcessData();
        if (PAUSE_ENABLED && HandleButtons() == CZ_Held ) {
          break_continuous = true;
          lcd.empty();
          lcd.at(1, 1, "Stopping Run");
          lcd.at(2, 1, "Release Buttons");
          do {
            NunChuckRequestData();
            NunChuckProcessData();
            delay(NCdelay);
          }
          while (HandleButtons() == CZ_Held);
          progstep = 9;
        }
      }

      //add section to delay here if the delay is set.
      while (start_delay_tm > millis() / 1000L)
      {
        //enter delay routine
        calc_time_remain_start_delay ();
        if ((millis() - display_last_tm) > 1000) display_time(2, 1);
        //NunChuckRequestData();
        //NunChuckProcessData();
        //if (HandleButtons() == CZ_Held && !Program_Engaged) {
        //  start_delay_tm=((millis()/1000L)+5); //start right away by lowering this to 5 seconds.
        //}
      }
      //end start delay

      if (!break_continuous) Auto_Repeat_Video(); //only run this if there isn't a break command
      redraw = true;
    }
    else if (!motorMoving && !sequence_repeat_type) { //new end condition for RUN ONCE
      lcd.empty();
      draw(58, 1, 1); //lcd.at(1,1,"Program Complete");
      Program_Engaged = false;
      if (POWERSAVE_PT > PWR_ALWAYS_ON)   disable_PT(); //  low, standard, high, we power down at the end of program
      if (POWERSAVE_AUX > PWR_ALWAYS_ON)  disable_AUX(); // low, standard, high, we power down at the end of program
      progstep = 90;
      redraw = true;
    }
  } // end interrupt routine driven for 2 points

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

  //Start video loop for 3 Point moves - this loop does not use the new motor move profile as it needs to calculate each and every segment
  else if (progtype == REG3POINTMOVE || progtype == REV3POINTMOVE)
  { //this is regular 3 point move program that can be modified later
#if DEBUG
    interval_tm_last = interval_tm;
    interval_tm = millis();
    //Start us moving
    Serial.print("trueinterval ");
    Serial.print(interval_tm - interval_tm_last);
    Serial.print(";");
#endif

#if DEBUG_MOTOR
    Serial.print("MoveStart ");
    Serial.print(millis());
    Serial.print(";");
#endif

    camera_fired ++; //still need this for framing
    move_motors();

#if DEBUG_MOTOR
    Serial.print("Moveend ");
    Serial.println(millis());
#endif

    if ( camera_total_shots  && camera_fired >= camera_total_shots) {
      lcd.empty();
      //draw(58,1,1);//lcd.at(1,1,"Program Complete");
      Program_Engaged = false;
      if (POWERSAVE_PT > PWR_ALWAYS_ON)   disable_PT(); //  low, standard, high, we power down at the end of program
      if (POWERSAVE_AUX > PWR_ALWAYS_ON)  disable_AUX(); // low, standard, high, we power down at the end of program
      delay(prompt_time);
      progstep = 90;
      redraw = true;
    }
  } // End video loop for 3 Point moves
}

void ExternalTriggerLoop ()
{

  //New interrupt Flag Checks
  if (changehappened)
  {
    changehappened = false;
    if (!iostate) //The trigger is active, start recording the time
    {
      ext_shutter_open = true;
    }
    else //shutter closed - sense pin goes back high - stop the clock and report
    {
      ext_shutter_open = false;
      ext_shutter_count++;
    }
  }
  //end interrupt check and flagging

  //  Start of states for external shooting loop
  if ( Program_Engaged && !(Shot_Sequence_Started) && !CameraShutter() && (ext_shutter_open) ) { //start a shot sequence flag
    Shot_Sequence_Started = true; //
  }
  if ( Program_Engaged && (Shot_Sequence_Started) && !CameraShutter() && (ext_shutter_open) ) { //fire the camera can happen more than once in a shot sequence with HDR

    //Fire the Camera without a timer, just turn on the focus and shutter pins - and stop when the shot is done.
    CameraFocus(true); //Fire without a timer
    CameraShutter(true);
  }
  if (Shot_Sequence_Started && CameraShutter() && !(ext_shutter_open) ) { //shutter just closed, stop the camera port and move
    CameraStop();

    if (ext_shutter_count >= ext_hdr_shots) { //this is future functionality  - leave at 1 for now
      camera_fired++;
      ext_shutter_count = 0;
      //Move the motors - each motor move is calculated by where we are in the sequence - we still call this for lead in and lead out - motors just don't move

      Move_Engaged = true; //move motors
      move_motors();

      //Turn off the motors if we have selected powersave 3 and 4 are the only ones we want here
      if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
      if (POWERSAVE_AUX > PWR_PROGRAM_ON)  disable_AUX(); //

      //Update display
      display_status();  //update after shot complete to avoid issues with pausing

      Shot_Sequence_Started = false; //Shot sequence engaged flag is is off - we are ready for our next
      //InterruptAction_Reset(); //enable the external interrupts to start a new shot
#if DEBUG
      Serial.println("EOL");
#endif
    }
  }
  if ( camera_moving_shots && camera_fired >= camera_total_shots) {  //end of program
    lcd.empty();
    draw(58, 1, 1); //lcd.at(1,1,"Program Complete");
    Program_Engaged = false;
    if (POWERSAVE_PT > PWR_ALWAYS_ON)   disable_PT(); //  low, standard, high, we power down at the end of program
    if (POWERSAVE_AUX > PWR_ALWAYS_ON)  disable_AUX(); // low, standard, high, we power down at the end of program
    delay(prompt_time * 2);
    progstep = 90;
    redraw = true;
  }
  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    //if (HandleButtons() == CZ_Held && Trigger_Type == External_Trigger ) Flag_Wait_For_Trigger = true; // manual trigger
    if (PAUSE_ENABLED && HandleButtons() == CZ_Held && !Shot_Sequence_Started ) Pause_Prog(); //pause an SMS program
  }
}

void EndOfProgramLoop ()
{
  if (redraw) {
    lcd.empty();
    lcd.at(1, 4, "Repeat - C");
    lcd.at(2, 4, "Reverse - Z");
    redraw = true;
    delay(prompt_time);
  }

  //This portion always runs in empty space of loop.

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    button_actions_end_of_program();
  }
}

void PanoLoop ()
{
  static uint8_t panoState = 1; // Start out with the first shot
  static uint8_t panoFlag  = 0; // Start out with an invalid flag

  switch(panoState) {
    case 0: // Idle State, waiting for flag conditions to advance
      break;
      
    case 1: // Setup State, prefocuses the camera and enables the motors
      panoFlag = 1;
      interval_tm = millis(); // Start the clock for out image
      CameraFocus(true); // Wake up the camera and begin focusing in the prefire time
      if (POWERSAVE_PT <= PWR_PROGRAM_ON)   enable_PT(); // Turn on power to the motors if not move only
      break;

    case 2:
      if (POWERSAVE_PT == PWR_SHOOTMOVE_ON)   enable_PT(); // Turn on power to the motors if shoot only
      if ( Trigger_Type == External_Trigger || Trigger_Type == Button_Trigger) {
        panoFlag = 5; // Wait for external trigger
      }
      else {
        panoFlag = 2;
        CameraShoot((uint32_t)static_tm * 100); // Set the camera to take an image x milliseconds long
        camera_fired++;
      }
      break;

    case 3:
      panoFlag = 3; // Set motors moving to the next position
      if (POWERSAVE_PT == PWR_MOVEONLY_ON)   enable_PT(); // Turn on power to the motors if move only
      if (progtype == PANOGIGA) {
        if (move_with_acceleration) move_motors_pano_accel();                    
      }
      else if (progtype == PORTRAITPANO) {
        move_motors_accel_array();
      }
      display_status(); // Image is complete, update display
      break;

    case 4:
      if (!nextMoveLoaded)  updateMotorVelocities();  //finished up the interrupt routine
      if (!motorMoving) {  // motors completed the move
        if (POWERSAVE_PT >= PWR_SHOOTMOVE_ON)   disable_PT();
        panoFlag = 4;
      }
      break;
      
    case 5:
      if (changehappened)
      {
        changehappened = false;
        if (!iostate) {  //The trigger is active, fire the camera
         panoFlag = 6;
         CameraShutter(true); // Fire the camera immediatly
        }
      }
      break;
    case 6: // External Trigger Handling
      if (changehappened)
      {
        changehappened = false;
        if (iostate) {  //The trigger is released, stop the camera
         panoFlag = 2;
         CameraShutter(false); // Fire the camera immediatly
         CameraFocus(false);   // Release the camera focus
         camera_fired++;
         interval_tm = millis();
        }
        break;
      }
  }

  switch(panoFlag)
  {
    case 0: // Idle State, waiting for external trigger to advance
      break;
    case 1: // Wait for prefire time to elapse
      if((millis() - interval_tm) > (prefire_time * 100)) panoState = 2; // Fire Camera
      else panoState = 0;
      break;
    case 2:  // Wait for exposure time to elapse
      if(!CameraShutter() && (millis() - interval_tm) > (prefire_time * 100 + static_tm * 100)) panoState = 3; // Prepare motors
      else if (!CameraShutter() && (Trigger_Type == External_Trigger || Trigger_Type == Button_Trigger)) panoState = 3;
      else panoState = 0;
      break;
    case 3: // Wait for camera shutter and focus to release
      if(!CameraShutter() && (millis() - interval_tm) > (postfire_time * 100 + prefire_time * 100 + static_tm * 100)) panoState = 4; // Move Motors
      else if (Trigger_Type == External_Trigger || Trigger_Type == Button_Trigger) {
        if(!CameraShutter() && (millis() - interval_tm) > (postfire_time * 100)) panoState = 4;
      }
      else panoState = 0;
      break;
    case 4: // Jump Back to Start
      panoState = 1;
      break;
    case 5: // Wait for external trigger
      panoState = 5; 
      panoFlag = 0;
      break;
    case 6: // Handle external trigger
      panoState = 6;
      panoFlag = 0;
      break;
  }

  // ------ End of state machine ------ //

  if (panoState == 4 && camera_fired >= camera_total_shots ) {  // end of program
    delay(prompt_time); // Just so you can see the last image completed
    panoState = 1;
    panoFlag = 0;
    Program_Engaged = false;
    if (POWERSAVE_PT > PWR_ALWAYS_ON)   disable_PT();
    progstep = 290;
    redraw = true;
  }

  if ((millis() - NClastread) > NCdelay) { // Nunchuck update for button events
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    if (panoFlag == 5 && HandleButtons() == C_Held && Trigger_Type == Button_Trigger ) panoState = 1; // Trigger advance with the button
    if (PAUSE_ENABLED && HandleButtons() == CZ_Held && !Shot_Sequence_Started ) Pano_Pause();         // Pause Panorama and be able to change settings
  }
}


void PanoEnd ()
{
  if (redraw)
  {
    lcd.empty();
    draw(58, 1, 1); //lcd.at(1,1,"Program Complete");
    draw(59, 2, 1); //lcd.at(2,1," Repeat Press C");
    redraw = false;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    button_actions290();  //read buttons, look for c button press to start run
  }
}

//void EndOfProgramLoop ()
//{
//  if (redraw) {
//    lcd.empty();
//    lcd.at(1, 4, "Repeat - C");
//    lcd.at(2, 4, "Reverse - Z");
//    redraw = true;
//    delay(prompt_time);
//  }
//
//  //This portion always runs in empty space of loop.
//
//  if ((millis() - NClastread) > NCdelay) {
//    NClastread = millis();
//    NunChuckRequestData();
//    NunChuckProcessData();
//    button_actions_end_of_program();
//  }
//}
