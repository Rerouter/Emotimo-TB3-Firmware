//Program Status Flags
boolean Flag_Camera_Triggers_In_Use = false;
boolean Flag_Prefire_Focus_Enabled  = false;
boolean Flag_Shot_Timer_Active      = false;
boolean Shot_Sequence_Started       = false;

//Local Variables
uint8_t   PanoPostMoveDelay = 200;  // delay in microseconds after completing a move

//Start of variables for Pano Mode
boolean   move_with_acceleration = true; // false = no accel, true = accel, so far read only


void ShootMoveShoot()
{
  // //Step 1 if internal interval.Kick off the shot sequence. This happens once per camera shot.
  if ( (Trigger_Type > Video_Trigger) && Program_Engaged && !(Shot_Sequence_Started) && ((millis() - interval_tm) > interval) )
  {
#if DEBUG
    interval_tm_last = interval_tm; //just used for shot timing comparison
#endif
    interval_tm = millis(); //start the clock on our shot sequence

#if DEBUG
    Serial.print("trueinterval: ");
    Serial.print(interval_tm - interval_tm_last);
    Serial.print(";");
#endif
    Interrupt_Fire_Engaged = false; //clear this flag to avoid rentering this routine
    Shot_Sequence_Started = true; //
    Flag_Prefire_Focus_Enabled = true; //
    Flag_Camera_Triggers_In_Use = true; //
    CameraFocus(true); //for longer shot interval, wake up the camera

    if (POWERSAVE_PT < PWR_MOVEONLY_ON)              enable_PanTilt(); //don't power on for shot for high power saving
    if (AUX_ON && POWERSAVE_AUX < PWR_MOVEONLY_ON)   enable_AUX(); //don't power on for shot for high power saving
  }

  //Step 1 if external triggering. This happens once per camera shot.
  if ( Program_Engaged && !(Shot_Sequence_Started) && (Trigger_Type == External_Trigger) && Interrupt_Fire_Engaged )
  {
    interval_tm_last = interval_tm; //just used for shot timing comparison
    interval_tm = millis(); //start the clock on our shot sequence

#if DEBUG
    Serial.print("trueinterval: ");
    Serial.print(interval_tm - interval_tm_last);
    Serial.print(";");
#endif
    Interrupt_Fire_Engaged = false; //clear this flag to avoid rentering this routine

    Shot_Sequence_Started = true; //
    Flag_Prefire_Focus_Enabled = true; //
    Flag_Camera_Triggers_In_Use = true; //
    CameraFocus(true); //for longer shot interval, wake up the camera

    if (POWERSAVE_PT < PWR_MOVEONLY_ON)              enable_PanTilt(); //don't power on for shot for high power saving
    if (AUX_ON && POWERSAVE_AUX < PWR_MOVEONLY_ON)   enable_AUX(); //don't power on for shot for high power saving
  }

  //End our prefire - check that we are in program active,shot cycle engaged, and prefire engaged and check against our prefire time
  //If so set prefire flag off, static flag on, fire camera for static time value, update the display

  if ((Shot_Sequence_Started) && (Flag_Prefire_Focus_Enabled)  && ((millis() - interval_tm) > prefire_time * 100))
  {
    Flag_Prefire_Focus_Enabled = false;
#if DEBUG
    Serial.print("PreDoneAt ");
    Serial.print(millis() - interval_tm);
    Serial.print(";");
#endif
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
    Flag_Camera_Triggers_In_Use = false; //IO Engaged is off

    //Move the motors - each motor move is calculated by where we are in the sequence - we still call this for lead in and lead out - motors just don't move

#if DEBUG_MOTOR
    Serial.print("MoveStart ");
    Serial.print(millis() - interval_tm);
    Serial.print(";");
#endif
    Move_Engaged = true; //move motors
    move_motors();
#if DEBUG_MOTOR
    Serial.print("Moveend ");
    Serial.print(millis() - interval_tm);
    Serial.print(";");
#endif
    //Turn off the motors if we have selected powersave 3 and 4 are the only ones we want here
    if (POWERSAVE_PT > PWR_PROGRAM_ON)   disable_PT();
    if (POWERSAVE_AUX > PWR_PROGRAM_ON)  disable_AUX(); //

    //Update display
    if (Trigger_Type != External_Trigger ) display_status(); //update after shot complete to avoid issues with pausing

    Shot_Sequence_Started = false; //Shot sequence engaged flag is is off - we are ready for our next
    Interrupt_Fire_Engaged = false;
    //InterruptAction_Reset(); //enable the external interrupts to start a new shot
#if DEBUG
    Serial.println("EOL");
#endif
  }

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
    //if (HandleButtons() == CZ_Held && Trigger_Type==External_Trigger ) Interrupt_Fire_Engaged=true; // manual trigger
    //if (PAUSE_ENABLED && HandleButtons() == CZ_Held && Trigger_Type>External_Trigger  && !Shot_Sequence_Started ) Pause_Prog(); //pause an SMS program
    if (PAUSE_ENABLED && HandleButtons() == CZ_Held && Trigger_Type >= External_Trigger  && !Shot_Sequence_Started) SMS_In_Shoot_Paused_Menu(); //jump into shooting menu
  }
}

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
#if DEBUG
  uint32_t shuttertimer_open;
  uint32_t shuttertimer_close;
#endif

  //New interrupt Flag Checks
  if (changehappened)
  {
    changehappened = false;
    if (!iostate) //The trigger is active, start recording the time
    {
      ext_shutter_open = true;
#if DEBUG
      shuttertimer_open = micros(); //turn on the led / shutter
      Serial.print("shuttertimer_a="); Serial.print(shuttertimer_open);
#endif
    }
    else //shutter closed - sense pin goes back high - stop the clock and report
    {
      ext_shutter_open = false;
      ext_shutter_count++;
#if DEBUG
      shuttertimer_close = micros(); //turn on the led / shutter
      Serial.print(" ext_shutter_count="); Serial.print(ext_shutter_count);
      Serial.print(" shuttertimer_b="); Serial.print(shuttertimer_close); Serial.print("diff="); Serial.println(shuttertimer_close - shuttertimer_open);
#endif
    }
  }
  //end interrupt check and flagging

  //  Start of states for external shooting loop
  if ( Program_Engaged && !(Shot_Sequence_Started) && !CameraShutter() && (ext_shutter_open) ) { //start a shot sequence flag
    Shot_Sequence_Started = true; //
  }
  if ( Program_Engaged && (Shot_Sequence_Started) && !CameraShutter() && (ext_shutter_open) ) { //fire the camera can happen more than once in a shot sequence with HDR
#if DEBUG
    Serial.print("Startshot_at:");
    Serial.print(millis());
    Serial.println(";");
#endif
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

#if DEBUG_MOTOR
      Serial.print("MoveStart ");
      Serial.print(millis() - interval_tm);
      Serial.print(";");
#endif
      Move_Engaged = true; //move motors
      move_motors();
#if DEBUG_MOTOR
      Serial.print("Moveend ");
      Serial.print(millis() - interval_tm);
      Serial.print(";");
#endif

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
    //if (HandleButtons() == CZ_Held && Trigger_Type == External_Trigger ) Interrupt_Fire_Engaged = true; // manual trigger
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
  //Kick off the shot sequence!!!  This happens once per camera shot.
  if (Trigger_Type > Video_Trigger && Program_Engaged && !Shot_Sequence_Started && ((millis() - interval_tm) > interval) )
  {
#if DEBUG
    interval_tm_last = interval_tm; //just used for shot timing comparison
#endif
    interval_tm = millis(); //start the clock on our shot sequence

#if DEBUG
    Serial.print("trueinterval: ");
    Serial.print(interval_tm - interval_tm_last);
    Serial.print(";");
#endif

    Interrupt_Fire_Engaged      = false; // clear this flag to avoid re-entering this routine
    Shot_Sequence_Started       = true;  //
    Flag_Prefire_Focus_Enabled  = true;  //
    Flag_Camera_Triggers_In_Use = true;  //
    
    CameraFocus(true); //for longer shot interval, wake up the camera

    // if (POWERSAVE_PT < PWR_MOVEONLY_ON)            enable_PanTilt(); // don't power on for shot for high power saving
    // if (AUX_ON && POWERSAVE_AUX < PWR_MOVEONLY_ON) enable_AUX();     // don't power on for shot for high power saving
    enable_PanTilt();
  }
  
  // End our prefire - check that we are in program active,shot cycle engaged, and prefire engaged and check against our prefire time
  // If so set prefire flag off, static flag on, fire camera for static time value, update the display

  if (Shot_Sequence_Started && Flag_Prefire_Focus_Enabled  && ((millis() - interval_tm) > prefire_time * 100)) {
    Flag_Prefire_Focus_Enabled = false;
    
#if DEBUG
    Serial.print("PreDoneAt ");
    Serial.print(millis() - interval_tm);
    Serial.print(";");
#endif
    Flag_Shot_Timer_Active = true;
    //Fire Camera
    CameraShoot((uint32_t)static_tm * 100); //start shutter sequence
    camera_fired++;
  }

  //End out static time - check that we are in an program active and static time,  Shutter not engaged, check shot cycle time against prefire+statictime
  //If so remove flags from Static Time Engaged and IO engaged, Turn off I/O port, set flags for motors moving, move motors
  //move motors - figure out delays.   Long delays mean really slow - choose the minimum of the calculated or a good feedrate that is slow

  //if (Program_Engaged && Shot_Sequence_Started && Flag_Shot_Timer_Active && !Shutter_Signal_Engaged && ((millis() - interval_tm) > (prefire_time * 100 + static_tm * 100)) ) {
  if (Shot_Sequence_Started && Flag_Shot_Timer_Active && !CameraShutter() && ((millis() - interval_tm) > (prefire_time * 100 + static_tm * 100)) ) { //removed requirement for Program Engaged for external interrupt
    Flag_Shot_Timer_Active = false; //Static Time Engaged is OFF
    Flag_Camera_Triggers_In_Use = false; //IO Engaged is off

    //Move the motors - each motor move is calculated by where we are in the sequence - we still call this for lead in and lead out - motors just don't move
    Move_Engaged = true; //move motors
#if DEBUG_MOTOR
    Serial.print("MoveStart ");
    Serial.print(millis() - interval_tm);
    Serial.print(";");
#endif
#if DEBUG_PANO
    Serial.print("progtype "); Serial.println(progtype);
#endif
    if (progtype == PANOGIGA) //regular pano
    {
      if (move_with_acceleration) move_motors_pano_accel();
      else                        move_motors_pano_basic();                       
    }
    else if (progtype == PORTRAITPANO) //PORTRAITPANO method array load
    {
#if DEBUG_PANO
      Serial.print("entered PORTRAITPANO loop");
#endif
      move_motors_accel_array();
      delay (PanoPostMoveDelay);
    }
#if DEBUG_MOTOR
    Serial.print("Moveend ");
    Serial.print(millis() - interval_tm);
    Serial.print(";");
#endif

    //Turn off the motors if we have selected powersave 3 and 4 are the only ones we want here
    // if (POWERSAVE_PT >  PWR_PROGRAM_ON)   disable_PT();
    // if (POWERSAVE_AUX > PWR_PROGRAM_ON)   disable_AUX();

    if (!move_with_acceleration)
    {
#if DEBUG
      Serial.println("finished basic move");
#endif
      if (Trigger_Type != External_Trigger ) display_status(); //update after shot complete to avoid issues with pausing
      Move_Engaged = false;
      Shot_Sequence_Started = false; //Shot sequence engaged flag is is off - we are ready for our next
      Interrupt_Fire_Engaged = false;
      //InterruptAction_Reset(); //enable the external interrupts to start a new shot
#if DEBUG
      Serial.println("EOL");
#endif
    }
  } //end test

  //just have this repeat like we are in loop
  if (move_with_acceleration) //acceleration profiles
  {
    if (!nextMoveLoaded)
    {
      updateMotorVelocities();  //finished up the interrupt routine
      //Print_Motor_Params(2);
    }
    //test for completed move
    if (Shot_Sequence_Started && Move_Engaged && !motorMoving) //motors completed the move
    {
#if DEBUG
      Serial.println("finished accel move");
#endif
      if (Trigger_Type != External_Trigger ) display_status(); //update after shot complete to avoid issues with pausing
      Move_Engaged = false;
      Shot_Sequence_Started = false; //Shot sequence engaged flag is is off - we are ready for our next
      Interrupt_Fire_Engaged = false;
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
    delay(prompt_time);
    progstep = 290;
    redraw = true;
  }
  //updateMotorVelocities();  //uncomment this for DF Loop

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();

    // if (HandleButtons() == C_Held && Trigger_Type==External_Trigger ) Interrupt_Fire_Engaged = true; // manual trigger
    if (PAUSE_ENABLED && HandleButtons() == CZ_Held && !Shot_Sequence_Started ) Pano_Pause();        // Pause Panorama and be able to change settings
  }
}


void PanoEnd ()
{
  if (redraw)
  {
    lcd.empty();
    stopISR1();
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
