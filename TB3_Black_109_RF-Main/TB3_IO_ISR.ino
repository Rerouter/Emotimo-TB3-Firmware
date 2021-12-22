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

void init_external_triggering()
{
	pinMode(IO_3, INPUT);
	digitalWrite(IO_3, HIGH);
	attachInterrupt(1, cam_change, CHANGE);  
}
  
  
void cam_change()
{
	changehappened = true;
	iostate = digitalRead(IO_3);
	#if DEBUG
	Serial.print("i");
	#endif
}


void setupstartISR1 ()
{
	TCCR1A = 0;
	TCCR1B = _BV(WGM13);
  
	ICR1 = (F_CPU / 4000000) * TIME_CHUNK; // goes twice as often as time chunk, but every other event turns off pins
	TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
	TIMSK1 = _BV(TOIE1);
	TCCR1B |= _BV(CS10);
}


void startISR1 ()
{
	TIMSK1 = _BV(TOIE1);
}

void stopISR1 ()
{
	TIMSK1 &= ~_BV(TOIE1);
}


void Jogloop()
{
	while (true) //short fast loop pull this out later
	{
		if (!nextMoveLoaded)		updateMotorVelocities2();
	}
}//end of loop
