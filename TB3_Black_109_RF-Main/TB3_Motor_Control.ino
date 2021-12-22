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


void move_motors()
{
	int32_t x = 0;
	int32_t y = 0;
	int32_t z = 0;

	//need this routine for both 2 and 3 point moves since 3 points can use this logic to determine aux motor progress	  

	if (camera_fired<keyframe[0][1])	{ //Leadin
		Move_State_2PT = LeadIn2PT;
		if (DEBUG_MOTOR) Serial.print("LeadIn ");
	}
	else if (camera_fired<keyframe[0][2]) {  //Rampup
		Move_State_2PT = RampUp2PT;
		if (DEBUG_MOTOR) Serial.print("Rampup ");
	}
	else if (camera_fired<keyframe[0][3]) {  //Linear
		Move_State_2PT = Linear2PT;
		if (DEBUG_MOTOR) Serial.print("Linear ");
	}
	else if (camera_fired<keyframe[0][4]) {  //RampDown
		Move_State_2PT = RampDown2PT;
		if (DEBUG_MOTOR) Serial.print("RampDn ");
	}
	else if (camera_fired<keyframe[0][5]) {  //Leadout
		Move_State_2PT = LeadOut2PT;
		if (DEBUG_MOTOR) Serial.print("LeadOut ");
	}
	else
	{
		Move_State_2PT = Finished2PT;   //Finished
	} 

	if (progtype==REG2POINTMOVE || progtype==REV2POINTMOVE || progtype==AUXDISTANCE)
	{   //2 point moves	

		//figure out our move size 2 point SMS and VIDEO   
		if (Trigger_Type==Video_Trigger)
		{ //video moves
			x = current_steps.x + motor_get_steps_2pt_video(0);
			y = current_steps.y + motor_get_steps_2pt_video(1);
			z = current_steps.z + motor_get_steps_2pt_video(2);
		}
		else
		{
			x = current_steps.x + motor_get_steps_2pt(0);
			y = current_steps.y + motor_get_steps_2pt(1);
			z = current_steps.z + motor_get_steps_2pt(2);
		}
	}  //end progtype 0
  
  
	if (progtype==REG3POINTMOVE|| progtype==REV3POINTMOVE)
	{  //3 point moves VIDEO

		if (camera_fired<keyframe[1][1]) { //Lead In
			Move_State_3PT = LeadIn3PT;
			percent = 0;
			if (DEBUG_MOTOR) Serial.print("LeadIn: " + String(percent));
		}
		else if (camera_fired<keyframe[1][2]) { //First Leg
			Move_State_3PT = FirstLeg3PT;
			percent = (camera_fired - keyframe[1][1]) / (keyframe[1][2] - keyframe[1][1]);
			if (DEBUG_MOTOR) Serial.print("Leg 1: " + String(percent));
		}
		else if (camera_fired<keyframe[1][3]) {  //Second Leg
			Move_State_3PT = SecondLeg3PT;
			percent = (camera_fired - keyframe[1][2]) / (keyframe[1][3] - keyframe[1][2]);
			if (DEBUG_MOTOR) Serial.print("Leg 2: " + String(percent));
		}
		//else if (camera_fired<keyframe[3]) {  //Third Leg
		// Move_State_3PT = ThirdLeg3PT;
		// percent = (camera_fired - keyframe[2]) / float(keyframe[3] - keyframe[2]);
		//  if (DEBUG_MOTOR) Serial.print("Leg 3: " + String(percent));
		//}
		else if (camera_fired<keyframe[1][4]) {  //Lead Out
			Move_State_3PT = LeadOut3PT;
			percent = 0.0;
			if (DEBUG_MOTOR) Serial.print("LeadOT: " + String(percent));
		}
		else
		{
			Move_State_3PT = Finished3PT;   //Finished
			if (DEBUG_MOTOR) Serial.print("Finished " + String(percent));
			return;
		} 

		if (DEBUG_MOTOR) Serial.print(";");
		// DONT FORGET TO UPDATE REAL TIME TESTS ON RT Page

		x = motor_get_steps_3pt(0);
		y = motor_get_steps_3pt(1);
		z = current_steps.z + motor_get_steps_2pt(2);  //use linear for this

	}//end progtype 1

	if (DEBUG_MOTOR) Serial.print("Shot " + String(camera_fired) + ";");
	if (DEBUG_MOTOR) Serial.print("B;" + String(current_steps.x) + ";");
	if (DEBUG_MOTOR) Serial.print(String(current_steps.y) + ";"); 
	if (DEBUG_MOTOR) Serial.print(String(current_steps.z) + ";");
	
	set_target(x, y, z); //we are in incremental mode to start abs is false

	if (DEBUG_MOTOR) Serial.print("D;");
	if (DEBUG_MOTOR) Serial.print(String(delta_steps.x) + ";");
	if (DEBUG_MOTOR) Serial.print(String(delta_steps.y) + ";");
	if (DEBUG_MOTOR) Serial.print(String(delta_steps.z) + ";");

	//calculate feedrate - update this to be dynamic based on settle window

	//VIDEO Loop 2 Point

	if ((progtype==REG2POINTMOVE || progtype==REV2POINTMOVE ||progtype==AUXDISTANCE) && (Trigger_Type==Video_Trigger))
	{ // must lock this down to be only 2point, not three
		feedrate_micros = calculate_feedrate_delay_video();
		if (Move_State_2PT == Linear2PT)
		{
			camera_fired += (keyframe[0][3]-keyframe[0][2]); //skip all the calcs mid motor move
		}
		if (DEBUG_MOTOR) Serial.print("Feedrate:" + String(feedrate_micros) + ";");
		dda_move(feedrate_micros); 
	}

	//SMS Loop and all 3 point moves
	else {
		feedrate_micros = calculate_feedrate_delay_1(); //calculates micro delay based on available move time
		if (Trigger_Type!= Video_Trigger) feedrate_micros = min(abs(feedrate_micros), 2000); //get a slow move, but not too slow, give the motors a chance to rest for non video moves.
		if (DEBUG_MOTOR) Serial.print("Feedrate:" + String(feedrate_micros) + ";");
		dda_move(feedrate_micros);
		Move_Engaged=false; //clear move engaged flag
	}
	if (DEBUG_MOTOR) Serial.print("A;");
	if (DEBUG_MOTOR) Serial.print(String(current_steps.x) + ";");
	if (DEBUG_MOTOR) Serial.print(String(current_steps.y) + ";");
	if (DEBUG_MOTOR) Serial.println(String(current_steps.z) + ";");
	return;
}//end move motors


int32_t motor_get_steps_2pt(uint8_t motor)
{
	int32_t steps=0; //updated this and tested against 12 foot move
	int32_t cur_steps[3];
	cur_steps[0] = current_steps.x;
	cur_steps[1] = current_steps.y;
	cur_steps[2] = current_steps.z;

	//if (DEBUG) Serial.print(motor + "motor ");

	switch (Move_State_2PT)
	{
		case LeadIn2PT:  //Lead In - 0 Steps
			steps = 0;
			if (DEBUG_MOTOR) Serial.print(steps);
			break;

		case RampUp2PT: //RampUp 
			steps = camera_fired - lead_in * linear_steps_per_shot[motor] / rampval;
			if (DEBUG_MOTOR) Serial.print(steps);
			break;

		case Linear2PT:  // Linear portion
			steps = (motor_steps_pt[2][motor] - ramp_params_steps[motor] - cur_steps[motor]) / (keyframe[0][3] - camera_fired); //  Point 2 in the end point
			if (DEBUG_MOTOR) Serial.print(steps);
			break;

		case RampDown2PT:  // RampDown
			steps = ((motor_steps_pt[2][motor] - cur_steps[motor]) * 2) / (keyframe[0][4] - camera_fired); // Point 2 in the end point for 2 point move
			if (DEBUG_MOTOR) Serial.print(steps);
			break;

		case LeadOut2PT:  //Lead Out
			steps=0;
			if (DEBUG_MOTOR) Serial.print(steps);
			break;

		case Finished2PT:  //5 - finished	
            break;
	}
	if (DEBUG_MOTOR) Serial.print(";");
	return(steps);  
}


int32_t motor_get_steps_3pt(uint8_t motor)
{
	int32_t steps=0; //updated this and tested against 12 foot move with and inch or so of travel with a 50:1 gear ration - needed to get above 32000 steps before overflow
	int32_t cur_steps[3];
	cur_steps[0] = current_steps.x;
	cur_steps[1] = current_steps.y;
	cur_steps[2] = current_steps.z;

	//if (DEBUG) Serial.print(motor + "motor ");
	switch (Move_State_3PT)
	{
		case LeadIn3PT:  //3Point Move - Lead In
			steps =0;
			if (DEBUG_MOTOR) Serial.print(steps);
			break;

		case FirstLeg3PT:  //3Point Move - First Leg - 
			steps = catmullrom(percent, motor_steps_pt[1][motor], 0.0, motor_steps_pt[1][motor], motor_steps_pt[2][motor]);
			if (DEBUG_MOTOR) Serial.print(steps);
			break;

		case SecondLeg3PT: //3Point Move - Second Leg 
			steps = catmullrom(percent, 0.0, motor_steps_pt[1][motor], motor_steps_pt[2][motor], motor_steps_pt[1][motor]);
			if (DEBUG_MOTOR) Serial.print(steps);
			break;

		//case ThirdLeg3PT: //3Point Move - Third Leg 
		//	steps = catmullrom(percent, motor_steps_pt[1][motor], motor_steps_pt[2][motor], motor_steps_pt[3][motor], motor_steps_pt[2][motor]);
		//	if (DEBUG_MOTOR) Serial.print(steps);
		//	break;

		case LeadOut3PT: //3Point Move - Lead Out
			steps = cur_steps[motor]; //this is not delta but relative to start.
			if (DEBUG_MOTOR) Serial.print(steps);
			break;

		case Finished3PT:  //109 - finished
			steps =0.0;
			break;
	}	  
	if (DEBUG_MOTOR) Serial.print(";");
	return(steps);
}


float catmullrom(float t, float p0, float p1, float p2, float p3)
{
	return 0.5f * (
				  (2 * p1) +
				  (-p0 + p2) * t +
				  (2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t +
				  (-p0 + 3 * p1 - 3 * p2 + p3) * t * t * t
				  );
}  


int32_t motor_get_steps_2pt_video(uint8_t motor)
{
	int32_t steps = 0; //updated this and tested against 12 foot move with and inch or so of travel with a 50:1 gear ration - needed to get above 32000 steps before overflow
	int32_t cur_steps[3];
	cur_steps[0] = current_steps.x;
	cur_steps[1] = current_steps.y;
	cur_steps[2] = current_steps.z;

	//if (DEBUG) Serial.print(motor + "motor ");

	switch (Move_State_2PT)
	{
		case LeadIn2PT:  //Lead In - 0 Steps
			steps = 0;
			#if DEBUG_MOTOR
			Serial.print(steps);
			#endif
			break;

		case RampUp2PT: //RampUp 
			steps = (camera_fired - lead_in) * linear_steps_per_shot[motor] / rampval;
			#if DEBUG_MOTOR
			Serial.print(steps);
      #endif
			break;

		case Linear2PT:  // Linear portion
			//steps = (motor_steps_pt[2][motor] - ramp_params_steps[motor] - cur_steps[motor]) / (keyframe[0][3] - camera_fired); //  Point 2 in the end point  //THIS IS THE OLD ROUTINE
			steps = (motor_steps_pt[2][motor] - ramp_params_steps[motor] - cur_steps[motor]); //  This is total steps of the linear portion.  End point  - ramp down-where we are
			#if DEBUG_MOTOR
			Serial.print(steps);
      #endif
			break;

		case RampDown2PT:  // RampDown
			steps = ((motor_steps_pt[2][motor] - cur_steps[motor]) * 2) / (keyframe[0][4] - camera_fired); // Point 2 in the end point for 2 point move
			if (DEBUG_MOTOR) Serial.print(steps);
			break;

		case LeadOut2PT:  //Lead Out
			steps = 0;
			if (DEBUG_MOTOR) Serial.print(steps);
			break;

		case Finished2PT:  //5 - finished	
			break;
	}
	#if DEBUG_MOTOR
	Serial.print(";");
  #endif
	return(steps);  
}  


void go_to_origin_max_speed() // interrupt routine
{
	#if DEBUG_MOTOR
	Serial.println("motors[0].dest:" + motors[0].destination);
	Serial.println("motors[1].dest:" + motors[1].destination);
	Serial.println("motors[2].dest:"+  motors[2].destination);
  #endif

	synched3PtMove_max(0, 0, 0);

	#if DEBUG_MOTOR
	Serial.println("motors[0].position:" + motors[0].position);
	Serial.println("motors[1].position:" + motors[1].position);
	Serial.println("motors[2].position:" + motors[2].position);

	Serial.println("motors[0].dest:" + motors[0].destination);
	Serial.println("motors[1].dest:" + motors[1].destination);
	Serial.println("motors[2].dest:" + motors[2].destination);
  #endif

	//bitSet(motorMoving, 0);
	//bitSet(motorMoving, 1);
	//bitSet(motorMoving, 2);

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

	//this is making sure position is keeping up and accurate with granular steps
	#if DEBUG_MOTOR
	Serial.println("Mot 0 current steps after move:" + String(current_steps.x));
	Serial.println("Mot 1 current steps after move:" + String(current_steps.y));
	Serial.println("Mot 2 current steps after move:" + String(current_steps.z));

	Serial.println("motors[0].position:" + String(motors[0].position));
	Serial.println("motors[1].position:" + String(motors[1].position));
	Serial.println("motors[2].position:" + String(motors[2].position));
  #endif
}


void go_to_origin_slow() // interrupt routine
{

	#if DEBUG_MOTOR
	Serial.println("motors[0].dest:" + String(motors[0].destination));
	Serial.println("motors[1].dest:" + String(motors[1].destination));
	Serial.println("motors[2].dest:" + String(motors[2].destination));
  #endif

	//synched3PtMove_timed(0.0, 0.0, 0.0,15.0,0.25);
	synched3PtMove_max(0, 0, 0); 

	#if DEBUG_MOTOR
	Serial.println("motors[0].position:" + String(motors[0].position));
	Serial.println("motors[1].position:" + String(motors[1].position));
	Serial.println("motors[2].position:" + String(motors[2].position));

	Serial.println("motors[0].dest:" + String(motors[0].destination));
	Serial.println("motors[1].dest:" + String(motors[1].destination));
	Serial.println("motors[2].dest:" + String(motors[2].destination));
  #endif

	//bitSet(motorMoving, 0);
	//bitSet(motorMoving, 1);
	//bitSet(motorMoving, 2);

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

	//this is making sure position is keeping up and accurate with granular steps
	#if DEBUG_MOTOR
	Serial.println("Mot 0 current steps after move:" + String(current_steps.x));
	Serial.println("Mot 1 current steps after move:" + String(current_steps.y));
	Serial.println("Mot 2 current steps after move:" + String(current_steps.z));

	Serial.println("motors[0].position:" + String(motors[0].position));
	Serial.println("motors[1].position:" + String(motors[1].position));
	Serial.println("motors[2].position:" + String(motors[2].position));
  #endif
}  


void DisplayMove(uint8_t motorIndex) //ca
{
	Motor *motor = &motors[motorIndex];

	for (uint8_t i = 0; i < 5; i++)
	{
		Serial.print("M" + String(motorIndex) + "Seg:" + i);
		Serial.print("T:" + String(motor->moveTime[i]) + ",");
		Serial.print("P:" + String(motor->movePosition[i]) + ",");
		Serial.print("V:" + String(motor->moveVelocity[i]) + ",");
		Serial.print("A:" + String(motor->moveAcceleration[i]) + ",");
		Serial.println("Dest:" + String(motor->destination) +" ");
	}
	//Serial.print("Tmax:");Serial.println(tmax);
	//Serial.print("Dmax:");Serial.println(dmax);
	//Serial.print("Dist:");Serial.println(dist);
	//Serial.print("Dir:");Serial.println(dir);
}


void go_to_start_old()
{
	// This is the first time we look at movement.  Here is where we need to update our positions to accomodate reverse programming.
	// Goal here is to just reverse the stored targets.  These are floating point so we should be able to just multiply by -1 - verify with debug.  
	// Additionally we have to set zeros for the current position - the move to start should do nothing.

	if (REVERSE_PROG_ORDER&&!MOVE_REVERSED_FOR_RUN) 
	{
		MOVE_REVERSED_FOR_RUN = true;
		//first flip the stored variables to help with direction of the move
		#if DEBUG_MOTOR
		Serial.print("entering rev loop of interest");
		Serial.println(REVERSE_PROG_ORDER);
    #endif

		//if we aren't at the end position, move there now
		//
		//	x = motor_steps_pt[2][0];
		//	y = motor_steps_pt[2][1];
		//	z = motor_steps_pt[2][2];
		// set_target(x, y, z);
		// lcd.at(1,1,"Going to End Pt.");
		// dda_move(10);
		//end of move to end position

		//calc the move
		synched3PtMove_max(motor_steps_pt[2][0],motor_steps_pt[2][1],motor_steps_pt[2][2]);

		//Start us moving  
		startISR1 ();

		stopISR1 ();
		//Clean up positions so we don't drift
		motors[0].position = current_steps.x;
		motors[1].position = current_steps.y;
		motors[2].position = current_steps.z;

		#if DEBUG_MOTOR
		Serial.print("midpoint_preflip");
		Serial.print(String(motor_steps_pt[1][0]) + ",");
		Serial.print(String(motor_steps_pt[1][1]) + ",");
		Serial.println(String(motor_steps_pt[1][2]));
    #endif
		
		motor_steps_pt[1][0] = (motor_steps_pt[2][0] - motor_steps_pt[1][0]) * -1;
		motor_steps_pt[1][1] = (motor_steps_pt[2][1] - motor_steps_pt[1][1]) * -1;
		motor_steps_pt[1][2] = (motor_steps_pt[2][2] - motor_steps_pt[1][2]) * -1;

		#if DEBUG_MOTOR
		Serial.print("endpoint_preflip");
		Serial.print(String(motor_steps_pt[1][0]) + ",");
		Serial.print(String(motor_steps_pt[1][1]) + ",");
		Serial.println(String(motor_steps_pt[1][2]));
    #endif
		
		motor_steps_pt[2][0] *= -1;
		motor_steps_pt[2][1] *= -1;
		motor_steps_pt[2][2] *= -1;

		#if DEBUG_MOTOR
		Serial.print("endpoint_postflip");
		Serial.print(String(motor_steps_pt[2][0]) + ",");
		Serial.print(String(motor_steps_pt[2][1]) + ",");
		Serial.println(String(motor_steps_pt[2][2]));
    #endif

		//we need to figure out where we are before we jump home, can't assume the correct end point.
		set_position(0, 0, 0); //setting home
	}
	if (progtype==REG2POINTMOVE || progtype==REV2POINTMOVE || progtype==AUXDISTANCE)
	{ //2point move
		for (uint8_t i=0; i < MOTORS; i++)
		{
			linear_steps_per_shot[i] = motor_steps_pt[2][i] / (camera_moving_shots-rampval); //This assumes ramp is equal on either side   SIGNED!!!
			#if DEBUG_MOTOR
			Serial.println("LinSteps/Shot_" + String(i) + "_" + String(linear_steps_per_shot[i]));
      #endif
		}

		//This is to calc the steps at the end of rampup for each motor.  Each array value is for a motor
		for (uint8_t i=0; i < MOTORS; i++)
		{
			ramp_params_steps[i] = rampval * linear_steps_per_shot[i] / 2; //steps at end of ramping target
			#if DEBUG_MOTOR
			Serial.println("Ramp_Steps_" + String(i) + "_" + String(ramp_params_steps[i]));
      #endif
		}
	}
	if (progtype==REG3POINTMOVE || progtype==REV3POINTMOVE)
	{ //3point move
		keyframe[1][0] = 0; //start frame - must find and replace this 
		keyframe[1][1] = lead_in; //end of static, start of leg 1.
		keyframe[1][2] = (camera_moving_shots / (MAX_MOVE_POINTS-1))+lead_in; //end of leg 1, start of leg 2
		keyframe[1][3] = camera_moving_shots + lead_in; //end of leg 2, start of lead_out
		keyframe[1][4] = camera_moving_shots + lead_in + lead_out; //end of lead_out/shot
		/*
		keyframe[1][0] = 0; //start frame
		keyframe[1][1] = camera_moving_shots /     (MAX_MOVE_POINTS-1); //mid frame 
		keyframe[1][2] = camera_moving_shots * 2 / (MAX_MOVE_POINTS-1); //end frame
		*/

		//keyframe[3] = camera_moving_shots; //end point

		motor_steps_pt[1][2] = motor_steps_pt[2][2];  //overwrite the middle points for the aux - not needed here and used for linear calcs on motor control 

		for (uint8_t i=2; i < MOTORS; i++)
		{ //overwrite for just the aux motor using second point
			linear_steps_per_shot[i] = motor_steps_pt[2][i] / (camera_moving_shots - rampval); //This assumes ramp is equal on either side   SIGNED!!!
			#if DEBUG_MOTOR
			Serial.println("LinSteps/Shot_" + String(i) + "_" + String(linear_steps_per_shot[i]));
      #endif
		}

		//This is to calc the steps at the end of rampup for each motor.  Each array value is for a motor
		for (uint8_t i=2; i < MOTORS; i++)
		{ //overwritw for just the aux motor using second point
			ramp_params_steps[i] = rampval * linear_steps_per_shot[i] / 2; //steps at end of ramping target
			#if DEBUG_MOTOR
			Serial.println("Ramp_Steps_" + String(i) + "_" + String(ramp_params_steps[i]));
      #endif
		}
	} //end of three point calcs

	delay (prompt_time);
	draw(40,1,1);//lcd.at(1,1," Going to Start"); //Moving back to start point
	digitalWrite(MS1, HIGH); //ensure microstepping before jog back home
	digitalWrite(MS2, HIGH);
	digitalWrite(MS3, HIGH);


	//Sync DF positions variables to floating points
	motors[0].position = current_steps.x;
	motors[1].position = current_steps.y;
	motors[2].position = current_steps.z;

	enable_PanTilt();
	enable_AUX();
	go_to_origin_max_speed();
}


void go_to_start_new() // interrupt routine
{
	enable_PanTilt();
	enable_AUX();
	//Add Section to allow for reverse Stuff.

	// This is the first time we look at movement.  Here is where we need to update our positions to accomodate reverse programming.  
	// Goal here is to just reverse the stored targets.  These are floating point so we should be able to just multiply by -1 - verify with debug.  
	// Additionally we have to set zeros for the current position - the move to start should do nothing.

	if (REVERSE_PROG_ORDER && !MOVE_REVERSED_FOR_RUN)
	{
		MOVE_REVERSED_FOR_RUN = true;
		//first flip the stored variables to help with direction of the move
		#if DEBUG_MOTOR
		Serial.println("entering rev loop" + REVERSE_PROG_ORDER);
    #endif

		//if we aren't at the end position, move there now
		// int32_t x = motor_steps_pt[2][0];
		// int32_t y = motor_steps_pt[2][1];
		// int32_t z = motor_steps_pt[2][2];
		// set_target(x, y, z);
		// lcd.at(1,1,"Going to End Pt."); 
		// dda_move(10);
		//end of move to end position

		//calc the move
		synched3PtMove_max(motor_steps_pt[2][0],motor_steps_pt[2][1],motor_steps_pt[2][2]);

		//Start us moving  
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

		//Clean up positions so we don't drift
		motors[0].position = current_steps.x;
		motors[1].position = current_steps.y;
		motors[2].position = current_steps.z;

		#if DEBUG_MOTOR
		Serial.print("midpoint_preflip:");
		Serial.print(String(motor_steps_pt[1][0]) + ",");
		Serial.print(String(motor_steps_pt[1][1]) + ",");
		Serial.println(String(motor_steps_pt[1][2]));
    #endif

		motor_steps_pt[1][0] = (motor_steps_pt[2][0] - motor_steps_pt[1][0]) * -1;
		motor_steps_pt[1][1] = (motor_steps_pt[2][1] - motor_steps_pt[1][1]) * -1;
		motor_steps_pt[1][2] = (motor_steps_pt[2][2] - motor_steps_pt[1][2]) * -1;

		#if DEBUG_MOTOR
		Serial.print("endpoint_preflip:");
		Serial.print(String(motor_steps_pt[1][0]) + ",");
		Serial.print(String(motor_steps_pt[1][1]) + ",");
		Serial.println(String(motor_steps_pt[1][2]));
    #endif

		motor_steps_pt[2][0] *= -1;
		motor_steps_pt[2][1] *= -1;
		motor_steps_pt[2][2] *= -1;

		#if DEBUG_MOTOR
		Serial.print("endpoint_postflip:");
		Serial.print(String(motor_steps_pt[1][0]) + ",");
		Serial.print(String(motor_steps_pt[1][1]) + ",");
		Serial.println(String(motor_steps_pt[1][2]));
    #endif

		//we need to figure out where we are before we jump home, can't assume the correct end point.
		set_position(0, 0, 0); //setting home
	}

	#if DEBUG_MOTOR
  Serial.println("motors[0].dest:" + String(motors[0].destination));
	Serial.println("motors[1].dest:" + String(motors[1].destination));
	Serial.println("motors[2].dest:" + String(motors[2].destination));
  #endif

	//synched3PtMove_timed(0.0, 0.0, 0.0,15.0,0.25);
	//need to turn on the motors
	synched3PtMove_max(0.0, 0.0, 0.0); 

	#if DEBUG_MOTOR
	Serial.println("motors[0].position:" + String(motors[0].position));
	Serial.println("motors[1].position:" + String(motors[1].position));
	Serial.println("motors[2].position:" + String(motors[2].position));
	
	Serial.println("motors[0].dest:" + String(motors[0].destination));
	Serial.println("motors[1].dest:" + String(motors[1].destination));
	Serial.println("motors[2].dest:" + String(motors[2].destination));
  #endif

	//bitSet(motorMoving, 0);
	//bitSet(motorMoving, 1);
	//bitSet(motorMoving, 2);
	draw(40,1,1);//lcd.at(1,1," Going to Start"); //Moving back to start point

	startISR1 ();
	do 
	{
		if (!nextMoveLoaded) updateMotorVelocities();
	}
	while (motorMoving);
	stopISR1 ();

	//this is making sure position is keeping up and accurate with granular steps
	#if DEBUG_MOTOR
	Serial.println("Mot 0 current steps after move:" + String(current_steps.x));
	Serial.println("Mot 1 current steps after move:" + String(current_steps.y));
	Serial.println("Mot 2 current steps after move:" + String(current_steps.z));

	Serial.println("motors[0].position:" + String(motors[0].position));
	Serial.println("motors[1].position:" + String(motors[1].position));
	Serial.println("motors[2].position:" + String(motors[2].position));
  #endif

	//still missing the reverse, but this calcs for ramp are now in.

	if (progtype==REG2POINTMOVE || progtype==REV2POINTMOVE || progtype==AUXDISTANCE)
	{ //2point move
		for (uint8_t i=0; i < MOTORS; i++)
		{
			linear_steps_per_shot[i] = motor_steps_pt[2][i] / (camera_moving_shots-rampval); //This assumes ramp is equal on either side   SIGNED!!!
			#if DEBUG_MOTOR
			Serial.println("LinSteps/Shot_" + String(i)+ "_" + String(linear_steps_per_shot[i]));
      #endif
		}

		//This is to calc the steps at the end of rampup for each motor.  Each array value is for a motor
		for (uint8_t i=0; i < MOTORS; i++)
		{
			ramp_params_steps[i] = rampval * linear_steps_per_shot[i] / 2; //steps at end of ramping target
			#if DEBUG_MOTOR
			Serial.println("Ramp_Steps_" + String(i) + "_" + String(ramp_params_steps[i]));
      #endif
		}
	}

	if (progtype==REG3POINTMOVE || progtype==REV3POINTMOVE)
	{ //3point move
		keyframe[1][0] = 0; //start frame - must find and replace this 
		keyframe[1][1] = lead_in; //end of static, start of leg 1.
		keyframe[1][2] = (camera_moving_shots / (MAX_MOVE_POINTS-1))+lead_in; //end of leg 1, start of leg 2
		keyframe[1][3] = camera_moving_shots + lead_in; //end of leg 2, start of lead_out
		keyframe[1][4] = camera_moving_shots + lead_in + lead_out; //end of lead_out/shot

		/*
		keyframe[1][0] = 0; //start frame
		keyframe[1][1] = camera_moving_shots /     (MAX_MOVE_POINTS-1); //mid frame 
		keyframe[1][2] = camera_moving_shots * 2 / (MAX_MOVE_POINTS-1); //end frame
		*/

		//keyframe[3] = camera_moving_shots; //end point

		motor_steps_pt[1][2] = motor_steps_pt[2][2];  //overwrite the middle points for the aux - not needed here and used for linear calcs on motor control 

		for (uint8_t i=2; i < MOTORS; i++)
		{ //overwrite for just the aux motor using second point
			linear_steps_per_shot[i] = motor_steps_pt[2][i] / (camera_moving_shots-rampval); //This assumes ramp is equal on either side   SIGNED!!!
			#if DEBUG_MOTOR
			Serial.println("LinSteps/Shot_" + String(i) + "_" + String(linear_steps_per_shot[i]));
      #endif
		}

		//This is to calc the steps at the end of rampup for each motor.  Each array value is for a motor
		for (uint8_t i=2; i < MOTORS; i++)
		{ //overwritw for just the aux motor using second point
			ramp_params_steps[i] = rampval * linear_steps_per_shot[i] / 2; //steps at end of ramping target
			#if DEBUG_MOTOR
			Serial.println("Ramp_Steps_" + String(i) + "_" + String(ramp_params_steps[i]));
      #endif
		}
	} //end of three point calcs

	//We don't know how long we will be waiting - go to powersave.
	if (POWERSAVE_PT > PWR_PROGRAM_ON  && !sequence_repeat_type) disable_PT(); //don't powersave for continuous
	if (POWERSAVE_AUX > PWR_PROGRAM_ON && !sequence_repeat_type) disable_AUX(); //don't powersave for continuous
}
