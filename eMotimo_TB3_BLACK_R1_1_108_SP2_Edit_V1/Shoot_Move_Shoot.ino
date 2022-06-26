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
      if ( intval == EXTTRIG_INTVAL) {
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
        if (!state) {  //The trigger is active, fire the camera
         smsFlag = 6;
         CameraShutter(true); // Fire the camera immediatly
        }
      }
      break;
    case 6: // External Trigger Handling
      if (changehappened)
      {
        changehappened = false;
        if (state) {  //The trigger is released, stop the camera
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
      else if (!CameraShutter() && (intval == EXTTRIG_INTVAL)) smsState = 3;
      else smsState = 0;
      break;
    case 3: // Wait for camera shutter and focus to release
      if(!CameraShutter() && (millis() - interval_tm) > (postfire_time * 100 + prefire_time * 100 + static_tm * 100)) smsState = 4; // Move Motors
      else if (intval == EXTTRIG_INTVAL) {
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

    //if (smsFlag == 5 && HandleButtons() == C_Held && Trigger_Type == Button_Trigger ) smsState = 1; // Trigger advance with the button
    if (PAUSE_ENABLED && HandleButtons() == CZ_Held && !motorMoving ) Pause_Prog();         // Pause Panorama and be able to change settings
  }
}
//void ShootMoveShoot()
//{
//  //Kick off the shot sequence!!!  This happens once per camera shot.
//  if ( (intval > 3) && (Program_Engaged) && !(Shot_Sequence_Engaged) && ((millis() - interval_tm) > interval) ) {
//    interval_tm_last = interval_tm; //just used for shot timing comparison
//
//    interval_tm = millis(); //start the clock on our shot sequence
//    if (DEBUG) {
//      Serial.print("trueinterval: ");
//      Serial.print(interval_tm - interval_tm_last);
//      Serial.print(";");
//    }
//    Interrupt_Fire_Engaged = false; //clear this flag to avoid rentering this routine
//
//    Shot_Sequence_Engaged = true; //
//    Prefire_Engaged = true; //
//    IO_Engaged = true; //
//    CameraFocus(true); //for longer shot interval, wake up the camera
//
//    if (POWERSAVE_PT < 4)   enable_PT(); //don't power on for shot for high power saving
//    if (AUX_ON && POWERSAVE_AUX < 4)   enable_AUX(); //don't power on for shot for high power saving
//
//  }
//
//
//  if ( (Program_Engaged) && !(Shot_Sequence_Engaged) && (intval == EXTTRIG_INTVAL) && Interrupt_Fire_Engaged ) {
//    interval_tm_last = interval_tm; //just used for shot timing comparison
//
//    interval_tm = millis(); //start the clock on our shot sequence
//    if (DEBUG) {
//      Serial.print("trueinterval: ");
//      Serial.print(interval_tm - interval_tm_last);
//      Serial.print(";");
//    }
//    Interrupt_Fire_Engaged = false; //clear this flag to avoid rentering this routine
//
//    Shot_Sequence_Engaged = true; //
//    Prefire_Engaged = true; //
//    IO_Engaged = true; //
//    CameraFocus(true); //for longer shot interval, wake up the camera
//
//    if (POWERSAVE_PT < 4)   enable_PT(); //don't power on for shot for high power saving
//    if (AUX_ON && POWERSAVE_AUX < 4)   enable_AUX(); //don't power on for shot for high power saving
//  }
//
//
//
//
//  //End our prefire - check that we are in program active,shot cycle engaged, and prefire engaged and check against our prefire time
//  //If so set prefire flag off, static flag on, fire camera for static time value, update the display
//
//  if ((Shot_Sequence_Engaged) && (Prefire_Engaged)  && ((millis() - interval_tm) > prefire_time * 100)) {
//
//    Prefire_Engaged = false;
//    if (DEBUG) {
//      Serial.print("PreDoneAt ");
//      Serial.print(millis() - interval_tm);
//      Serial.print(";");
//    }
//
//    Static_Time_Engaged = true;
//    //Fire Camera
//    if (intval != 3) CameraShoot((long)static_tm * 100); //start shutter sequence
//    camera_fired++;
//  }
//
//  //End out static time - check that we are in an program active and static time,  Shutter not engaged, check shot cycle time agains prefire+statictime
//  //If so remove flags from Static Time Engaged and IO engaged, Turn off I/O port, set flags for motors moving, move motors
//  //move motors - figure out delays.   Long delays mean really slow - choose the minimum of the calculated or a good feedrate that is slow
//
//  //if (Program_Engaged && Shot_Sequence_Engaged && Static_Time_Engaged && !Shutter_Signal_Engaged && ((millis() - interval_tm) > (prefire_time*100+static_tm*100)) ) {
//  if (Shot_Sequence_Engaged && Static_Time_Engaged && !Shutter_Signal_Engaged && ((millis() - interval_tm) > (prefire_time * 100 + static_tm * 100)) ) { //removed requirement for Program Engaged for external interrupt
//
//
//    Static_Time_Engaged = false; //Static Time Engaged is OFF
//    IO_Engaged = false; //IO Engaged is off
//    //digitalWrite(IO_2, LOW); //Use this as the interrupt
//    //digitalWrite(IO_3, LOW);  //Turn off Pin 3
//    //Serial.print("IO3_off"); //Serial.println(millis()-interval_tm);
//
//
//    //Move the motors - each motor move is calculated by where we are in the sequence - we still call this for lead in and lead out - motors just don't move
//
//    Move_Engaged = true; //move motors
//    if (DEBUG_MOTOR) {
//      Serial.print("MoveStart ");
//      Serial.print(millis() - interval_tm);
//      Serial.print(";");
//    }
//    move_motors();
//    if (DEBUG_MOTOR) {
//      Serial.print("Moveend ");
//      Serial.print(millis() - interval_tm);
//      Serial.print(";");
//    }
//
//    //Turn off the motors if we have selected powersave 3 and 4 are the only ones we want here
//    if (POWERSAVE_PT > 2)   disable_PT();
//    if (POWERSAVE_AUX > 2)   disable_AUX(); //
//
//    //Update display
//    if (intval != 3) display_status(); //update after shot complete to avoid issues with pausing
//
//
//    Shot_Sequence_Engaged = false; //Shot sequence engaged flag is is off - we are ready for our next
//    Interrupt_Fire_Engaged = false;
//    Button_Read_Count = 0;
//    //InterruptAction_Reset(); //enable the external interrupts to start a new shot
//    if (DEBUG) {
//      Serial.println("EOL");
//    }
//  }
//
//  if ( camera_moving_shots > 0  && camera_fired >= camera_total_shots) {  //end of program
//    lcd.empty();
//    draw(58, 1, 1); //lcd.at(1,1,"Program Complete");
//    Program_Engaged = false;
//    if (POWERSAVE_PT > 1)   disable_PT(); //  low, standard, high, we power down at the end of program
//    if (POWERSAVE_AUX > 1)  disable_AUX(); // low, standard, high, we power down at the end of program
//    delay(prompt_time * 2);
//    progstep = 90;
//    redraw = 1;
//  }
//  NunChuckRequestData();
//  NunChuckProcessData();
//  //if (Button_Read_Count>10 && intval==EXTTRIG_INTVAL ) Interrupt_Fire_Engaged=true; // manual trigger
//  if (PAUSE_ENABLED && Button_Read_Count > 10 && intval > 3 && !Shot_Sequence_Engaged ) Pause_Prog(); //pause an SMS program
//}


void VideoLoop ()
{
  if (DEBUG) {
    interval_tm_last = interval_tm;
    interval_tm = millis();
  }
  //Start us moving

  if (DEBUG) {
    Serial.print("trueinterval ");
    Serial.print(interval_tm - interval_tm_last);
    Serial.print(";");
  }

  if (DEBUG_MOTOR) {
    Serial.print("MoveStart ");
    Serial.print(millis());
    Serial.print(";");
  }

  camera_fired ++; //still need this for framing
  move_motors();

  if (DEBUG_MOTOR) {
    Serial.print("Moveend ");
    Serial.println(millis());
  }


  if ( camera_total_shots > 0  && camera_fired >= camera_total_shots) {
    lcd.empty();
    draw(58, 1, 1); //lcd.at(1,1,"Program Complete");
    Program_Engaged = false;
    if (POWERSAVE_PT > 1)   disable_PT(); //  low, standard, high, we power down at the end of program
    if (POWERSAVE_AUX > 1)  disable_AUX(); // low, standard, high, we power down at the end of program
    delay(prompt_time * 2);
    progstep = 90;
    redraw = 1;
    //delay(100);
    //NunChuckQuerywithEC();
  }
}


void ExternalTriggerLoop ()
{
  //New interrupt Flag Checks

  if (changehappened)
  {
    changehappened = false;
    if (!state) //start the clock as the cam shutter witch closed and sense pin, was brought low
    {
      ext_shutter_open = true;
      shuttertimer_open = micros();
      if (DEBUG) Serial.print("shuttertimer_a="); Serial.print(shuttertimer_open);

    }
    else if (state) //shutter closed - sense pin goes back high - stop the clock and report
    {
      ext_shutter_open = false;
      shuttertimer_close = micros(); //turn on the led / shutter
      ext_shutter_count++;
      if (DEBUG) Serial.print(" ext_shutter_count="); Serial.print(ext_shutter_count);
      if (DEBUG) Serial.print(" shuttertimer_b="); Serial.print(shuttertimer_close); Serial.print("diff="); Serial.println(shuttertimer_close - shuttertimer_open);
    }

  }
  //end interrupt check and flagging


  //  Start of states for external shooting loop

  if ( (Program_Engaged) && !(Shot_Sequence_Engaged) && !(Shutter_Signal_Engaged) && (ext_shutter_open) ) { //start a shot sequence flag

    Shot_Sequence_Engaged = true; //

  }


  if ( (Program_Engaged) && (Shot_Sequence_Engaged) && !(Shutter_Signal_Engaged) && (ext_shutter_open) ) { //fire the camera can happen more than once in a shot sequence with HDR

    if (DEBUG) {
      Serial.print("Startshot_at:");
      Serial.print(millis());
      Serial.println(";");
    }

    //Fire Camera
    //don't fire the camera with the timer, just turn on our focus and shutter pins - we will turn them off when we sense the shot is done.
    CameraFocus(true);
    CameraShutter(true);
    Shutter_Signal_Engaged = true;
    //camera_fired++;
  }


  if (Shot_Sequence_Engaged && (Shutter_Signal_Engaged) && !(ext_shutter_open) ) { //shutter just closed, stop the camera port and move

    CameraFocus(false);
    CameraShutter(true);
    Shutter_Signal_Engaged = false;


    if (ext_shutter_count >= ext_hdr_shots) { //this is future functionality  - leave at 1 for now

      camera_fired++;
      ext_shutter_count = 0;
      //Move the motors - each motor move is calculated by where we are in the sequence - we still call this for lead in and lead out - motors just don't move

      Move_Engaged = true; //move motors
      if (DEBUG_MOTOR) {
        Serial.print("MoveStart ");
        Serial.print(millis() - interval_tm);
        Serial.print(";");
      }
      move_motors();
      if (DEBUG_MOTOR) {
        Serial.print("Moveend ");
        Serial.print(millis() - interval_tm);
        Serial.print(";");
      }

      //Turn off the motors and go into a low power state
      disable_PT();
      disable_AUX();


      //Update display
      display_status();  //update after shot complete to avoid issues with pausing

      Shot_Sequence_Engaged = false; //Shot sequence engaged flag is is off - we are ready for our next
      Button_Read_Count = 0;
      //InterruptAction_Reset(); //enable the external interrupts to start a new shot
      if (DEBUG) {
        Serial.println("EOL");
      }
    }

  }

  if ( camera_moving_shots > 0  && camera_fired >= camera_total_shots) {  //end of program
    lcd.empty();
    draw(58, 1, 1); //lcd.at(1,1,"Program Complete");
    Program_Engaged = false;
    if (POWERSAVE_PT > 1)   disable_PT(); //  low, standard, high, we power down at the end of program
    if (POWERSAVE_AUX > 1)  disable_AUX(); // low, standard, high, we power down at the end of program
    delay(prompt_time * 2);
    progstep = 90;
    redraw = 1;
  }
  NunChuckRequestData();
  NunChuckProcessData();
  if (Button_Read_Count > 10 && intval == EXTTRIG_INTVAL ) Interrupt_Fire_Engaged = true; // manual trigger
  //if (PAUSE_ENABLED && Button_Read_Count>20 && intval>3 && !Shot_Sequence_Engaged ) Pause_Prog(); //pause an SMS program
}



void EndOfProgramLoop ()
{
  if (redraw == 1) {
    lcd.empty();
    lcd.at(1, 4, "Repeat - C");
    lcd.at(2, 4, "Reverse - Z");
    redraw = 0;
    delay(prompt_time);
  }

  if ((millis() - NClastread) > NCdelay) {
    NClastread = millis();
    NunChuckRequestData();
    NunChuckProcessData();
    button_actions90();  //read buttons, look for c button press to start run
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
      if ( intval == EXTTRIG_INTVAL) {
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
        if (P2PType) move_motors_pano_accel();                    
      }
//            else if (progtype == PORTRAITPANO) {
//              move_motors_accel_array();
//            }
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
        if (!state) {  //The trigger is active, fire the camera
         panoFlag = 6;
         CameraShutter(true); // Fire the camera immediatly
        }
      }
      break;
    case 6: // External Trigger Handling
      if (changehappened)
      {
        changehappened = false;
        if (state) {  //The trigger is released, stop the camera
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
      else if (!CameraShutter() && (intval == EXTTRIG_INTVAL)) panoState = 3;
      else panoState = 0;
      break;
    case 3: // Wait for camera shutter and focus to release
      if(!CameraShutter() && (millis() - interval_tm) > (postfire_time * 100 + prefire_time * 100 + static_tm * 100)) panoState = 4; // Move Motors
      else if (intval == EXTTRIG_INTVAL) {
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

    //if (panoFlag == 5 && HandleButtons() == C_Held && Trigger_Type == Button_Trigger ) panoState = 1; // Trigger advance with the button
    if (PAUSE_ENABLED && HandleButtons() == CZ_Held && !motorMoving ) Pano_Pause();         // Pause Panorama and be able to change settings
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
