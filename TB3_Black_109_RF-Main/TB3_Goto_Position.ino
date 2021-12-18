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


void goto_position(uint32_t gotoshot_temp)
{
	//We take stock of where we are and hold temps vars for position and camera shots
	//reset the camera fired
	//run the move, but don't actually move
	//Run the mock move starting from zero to the shot - do this so you can actually go backwards.
	//Once we get to the move, set this as our target, set current steps as what we have in temp.   Set target for coordinated move, go
	//set the camera fired to that amount, pause.

	//capture current conditions
	LONG BeforeMove;
	BeforeMove.x = EEPROM_STORED.current_steps.x;
	BeforeMove.y = EEPROM_STORED.current_steps.y;
	BeforeMove.z = EEPROM_STORED.current_steps.z; 
	//end capture current concitions.

	//reset the move to start fresh
	EEPROM_STORED.current_steps.x = 0;
	EEPROM_STORED.current_steps.y = 0;
	EEPROM_STORED.current_steps.z = 0;
	EEPROM_STORED.camera_fired = 0;
	//end reset the move

	//enable the motors
	enable_PanTilt();
	enable_AUX();

	//start the for loop here;
	for (uint32_t i=0; i < gotoshot_temp; i++)
	{
		EEPROM_STORED.camera_fired++;

		int32_t x = 0;
		int32_t y = 0;
		int32_t z = 0;

		//need this routine for both 2 and three point moves since 3 points can use this logic to determine aux motor progress	  

		if (EEPROM_STORED.camera_fired<EEPROM_STORED.keyframe[0][1]) { //Leadin
			Move_State_2PT = LeadIn2PT;
			if (DEBUG_GOTO) Serial.print("LeadIn ");
		}
		else if (EEPROM_STORED.camera_fired<EEPROM_STORED.keyframe[0][2]) {  //Rampup
			Move_State_2PT = RampUp2PT;
			if (DEBUG_GOTO) Serial.print("Rampup ");
		}
		else if (EEPROM_STORED.camera_fired<EEPROM_STORED.keyframe[0][3]) {  //Linear
			Move_State_2PT = Linear2PT;
				Serial.println("Linear ");
		}
		else if (EEPROM_STORED.camera_fired<EEPROM_STORED.keyframe[0][4]) {  //RampDown
			Move_State_2PT = RampDown2PT;
			if (DEBUG_GOTO) Serial.print("RampDn ");
		}
		else if (EEPROM_STORED.camera_fired<EEPROM_STORED.keyframe[0][5]) {  //Leadout
			Move_State_2PT = LeadOut2PT;
			if (DEBUG_GOTO) Serial.print("LeadOut ");
		}
		else {
			Move_State_2PT = Finished2PT;   //Finished
		} 

		if (EEPROM_STORED.progtype==REG2POINTMOVE || EEPROM_STORED.progtype==REV2POINTMOVE || EEPROM_STORED.progtype==AUXDISTANCE) {   //2 point moves	
			  
			//figure out our move size 2 point SMS and VIDEO   
			if (EEPROM_STORED.intval==VIDEO_INTVAL)
			{ //video moves
				x = EEPROM_STORED.current_steps.x + motor_get_steps_2pt_video(0);
				y = EEPROM_STORED.current_steps.y + motor_get_steps_2pt_video(1);
				z = EEPROM_STORED.current_steps.z + motor_get_steps_2pt_video(2);
			}
			else
			{
				x = EEPROM_STORED.current_steps.x + motor_get_steps_2pt(0);
				y = EEPROM_STORED.current_steps.y + motor_get_steps_2pt(1);
				z = EEPROM_STORED.current_steps.z + motor_get_steps_2pt(2);
			}
		}  //end progtype 0

		if (EEPROM_STORED.progtype==REG3POINTMOVE || EEPROM_STORED.progtype==REV3POINTMOVE)
		{  //3 point moves
			if (EEPROM_STORED.camera_fired<EEPROM_STORED.keyframe[1][1]) { //Lead In
				Move_State_3PT = LeadIn3PT;
				percent = 0.0;
				if (DEBUG_GOTO) Serial.print("LeadIn: " + String(percent));
			}	   
			else if (EEPROM_STORED.camera_fired<EEPROM_STORED.keyframe[1][2]) { //First Leg
				Move_State_3PT = FirstLeg3PT;
				percent = (EEPROM_STORED.camera_fired-EEPROM_STORED.keyframe[1][1]) / (EEPROM_STORED.keyframe[1][2] - EEPROM_STORED.keyframe[1][1]);
				if (DEBUG_GOTO) Serial.print("Leg 1: " + String(percent));
			}
			else if (EEPROM_STORED.camera_fired<EEPROM_STORED.keyframe[1][3]) {  //Second Leg
				Move_State_3PT = SecondLeg3PT;
				percent = (EEPROM_STORED.camera_fired - EEPROM_STORED.keyframe[1][2]) / (EEPROM_STORED.keyframe[1][3] - EEPROM_STORED.keyframe[1][2]);
				if (DEBUG_GOTO) Serial.print("Leg 2: " + String(percent));
			}
			//else if (EEPROM_STORED.camera_fired<EEPROM_STORED.keyframe[3]) {  //Third Leg
			// Move_State_3PT = ThirdLeg3PT;
			// percent = (EEPROM_STORED.camera_fired - EEPROM_STORED.keyframe[2]) / EEPROM_STORED.keyframe[3] - EEPROM_STORED.keyframe[2];
			//  if (DEBUG_GOTO) Serial.print("Leg 3: " + String(percent));
			//}
			else if (EEPROM_STORED.camera_fired<EEPROM_STORED.keyframe[1][4]) {  //Lead Out
				Move_State_3PT = LeadOut3PT;
				percent = 0;
				if (DEBUG_GOTO) Serial.print("LeadOT: " + String(percent));
			}
			else
			{
				Move_State_3PT = Finished3PT;   //Finished
				if (DEBUG_GOTO) Serial.print("Finished " + String(percent));  
				return;
			}

			if (DEBUG_GOTO) Serial.print(";");
			// DONT FORGET TO UPDATE REAL TIME TESTS ON RT Page

			x = motor_get_steps_3pt(0);
			y = motor_get_steps_3pt(1);
			z = EEPROM_STORED.current_steps.z + motor_get_steps_2pt(2);  //use linear for this

		}//end progtype 1

		#if DEBUG_GOTO
		Serial.print("Shot " + String(EEPROM_STORED.camera_fired) + ";");
		Serial.print("B;" + String(EEPROM_STORED.current_steps.x) + ";");
		Serial.print(String(EEPROM_STORED.current_steps.y) + ";");
		Serial.println(String(EEPROM_STORED.current_steps.z) + ";");
		#endif

		set_target(x,y,z); //we are in incremental mode to start abs is false

		#if DEBUG_GOTO
		Serial.print("D;" + String(delta_steps.x) + ";");
		Serial.print(String(delta_steps.y) + ";");
		Serial.println(String(delta_steps.z) + ";");
		#endif

		//calculate feedrate - update this to be dynamic based on settle window

		//VIDEO Loop
		if ((EEPROM_STORED.progtype==REG2POINTMOVE || EEPROM_STORED.progtype==REV2POINTMOVE || EEPROM_STORED.progtype==AUXDISTANCE) && (EEPROM_STORED.intval==VIDEO_INTVAL)) { // must lock this down to be only 2point, not three
			feedrate_micros = calculate_feedrate_delay_video();
			if (Move_State_2PT == Linear2PT) {
				EEPROM_STORED.camera_fired += (EEPROM_STORED.keyframe[0][3] - EEPROM_STORED.keyframe[0][2]); //skip all the calcs mid motor move
			}
			if (DEBUG_GOTO) Serial.print("Feedrate:" + String(feedrate_micros) + ";");
			// pull this from the actual move, reset with just adding deltas to the dda_move(feedrate_micros);

			EEPROM_STORED.current_steps.x = x;
			EEPROM_STORED.current_steps.y = y;
			EEPROM_STORED.current_steps.z = z;
		}
		//SMS Loop and all three point moves
		else {
			feedrate_micros = calculate_feedrate_delay_1(); //calculates micro delay based on available move time
			if (EEPROM_STORED.intval!= VIDEO_INTVAL) feedrate_micros = min(abs(feedrate_micros), 2000); //get a slow move, but not too slow, give the motors a chance to rest for non video moves.
			if (DEBUG_GOTO) Serial.print("Feedrate:" + String(feedrate_micros) + ";");
			// pull this from the actual move, reset with just adding deltas to the dda_move(feedrate_micros); 
			EEPROM_STORED.current_steps.x = x;
			EEPROM_STORED.current_steps.y = y;
			EEPROM_STORED.current_steps.z = z;

			FLAGS.Move_Engauged=false; //clear move engaged flag
		}
		#if DEBUG_GOTO
		Serial.print("A;");
		Serial.print(String(EEPROM_STORED.current_steps.x) + ";");
		Serial.print(String(EEPROM_STORED.current_steps.y) + ";");
		Serial.println(String(EEPROM_STORED.current_steps.z) + ";");
		#endif

	}  //end of the for loop

	//now that we know the position at the gotoframe, set it as our temp target
	LONG Target_Position;
	Target_Position.x = EEPROM_STORED.current_steps.x; //because current_steps, holds this information
	Target_Position.y = EEPROM_STORED.current_steps.y;
	Target_Position.z = EEPROM_STORED.current_steps.z;

	//Reset the current steps vars from the temp.

	EEPROM_STORED.current_steps.x = BeforeMove.x;
	EEPROM_STORED.current_steps.y = BeforeMove.y;
	EEPROM_STORED.current_steps.z = BeforeMove.z; 

	//calc the move
	synched3PtMove_max(Target_Position.x,Target_Position.y,Target_Position.z);

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
	//delay(10000);
	stopISR1 ();

	inprogtype=0; //set this to resume

	return;
}//end goto routine
