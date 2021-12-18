/*
  This code started with OpenMoco Time Lapse Engine openmoco.org and was modified by Brian Burling
  to work with eMotimo product.

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

  ========================================
  Camera control functions
  ========================================
  Canons have a 90ms Pre-fire delay and 3ms per shutter transition
  Implied that the 300d needs more than 100ms, but less than 200ms
  EOS 30D 128ms without pre-focus, 70ms with pre-focus
  G10 have about 70ms
  Canon 5D 133ms without pre-focus, 78 with
  Canon 6D 75 without, 59 with
  Canon 550D 180ms without pre-focus
  600d 160ms without pre-focus
  Always fastest with live pre-veiw turned off
*/

boolean ShutterActive = false;
boolean FocusActive =   false;
uint8_t SHUTTER_PIN =    12; // drives tip of 2.5 mm connector
uint8_t FOCUS_PIN =     13; // drives  middle of 2.5mm connector


void CameraInit()
{
  pinMode(SHUTTER_PIN, OUTPUT);
  pinMode(FOCUS_PIN, OUTPUT);
  CameraFocus(false);
  CameraShutter(false);
}


void CameraFocus(boolean focusstate)
{
  // Pre-focus camera
  digitalWrite(FOCUS_PIN, focusstate);
  FocusActive = focusstate; //Flag for if the focus pin is held
}


void CameraFocus(boolean focusstate, uint8_t focusdelay)
{
  // Pre-focus camera and wait
  digitalWrite(FOCUS_PIN, focusstate);
  delay(focusdelay);
  FocusActive = focusstate; //Flag for if the focus pin is held
}


bool CameraFocus()
{
  // Read back Camera Focus State
  return FocusActive;
}


void CameraShutter(boolean shutterstate)
{
  // Pre-focus camera
  digitalWrite(SHUTTER_PIN, shutterstate);
  ShutterActive = shutterstate; //Flag for if the focus pin is held
}


bool CameraShutter()
{
  // Read back Camera Shutter State
  return ShutterActive;
}


void CameraShoot(uint32_t exp_tm)
{
  // determine if focus pin should be brought high
  // w. the shutter pin (for some nikons, etc.)


  if (exp_tm)  // if time is 0, don't start the camera, treat as infinite
  {
    CameraFocus(true);
    CameraShutter(true);
    // start timer to stop camera exposure
    MsTimer2_set(exp_tm);
    MsTimer2_start();
  }

#if DEBUG
  Serial.print("ShutON "); Serial.print(millis() - interval_tm); Serial.print(";");
#endif
}


void CameraStop()
{
  MsTimer3_stop();                  // Disable timer
  CameraFocus(false);
  CameraShutter(false);

#if DEBUG
  Serial.print("ShutOff "); Serial.print(millis() - interval_tm); Serial.print(";");
#endif
}

// Hardware timer magic
//***************************************************************************************//

//Timer2flags
unsigned long MsTimer2_msecs;
//void (*MsTimer2_func)();
volatile uint32_t   MsTimer2_count;
volatile uint8_t  MsTimer2_tcnt2;
volatile boolean  MsTimer2_overflowing;


void (*MsTimer2_func)();


void MsTimer2_set(uint32_t ms)
// Timer counts up to
{
  if (ms)  MsTimer2_msecs = ms;
  else		 MsTimer2_msecs = 1;

  //Set overflow interupt mask
  TIMSK2 &= ~(1 << TOIE2);

  // Set normal counter mode
  TCCR2A &= ~((1 << WGM21) | (1 << WGM20)); //Normal Mode
  TCCR2B &= ~(1 << WGM22);

  // Timer/Counter2 is clocked from the I/O clock
  ASSR &= ~(1 << AS2);

  // OCIE2A: Timer/Counter2 Output Compare Match A Interrupt Enable
  TIMSK2 &= ~(1 << OCIE2A);

  // Set prescaller 64
  TCCR2B |= (1 << CS22);
  TCCR2B &= ~((1 << CS21) | (1 << CS20));

  // Compute how often we want our timer to interrupt (4ms)
  const uint16_t prescaler = 64;
  MsTimer2_tcnt2 = 256 - F_CPU / (prescaler * 1000);
}


void MsTimer2_start()
{
  MsTimer2_count = 0;
  MsTimer2_overflowing = 0;
  TCNT2 = MsTimer2_tcnt2;   // Setup Timer to interrupt every 4ms
  TIMSK2 |= (1 << TOIE2);	 // Turn on the Interrupt to trigger
}


void MsTimer3_stop()
{
  TIMSK2 &= ~(1 << TOIE2);
}


void MsTimer2_overflow()
{
  MsTimer2_count += 1;

  if (MsTimer2_count >= MsTimer2_msecs && !MsTimer2_overflowing)
  {
    MsTimer2_overflowing = 1;
    MsTimer2_count = MsTimer2_count - MsTimer2_msecs; // subtract ms to catch missed overflows
    // set to 0 if you don't want this.
    CameraStop();
    MsTimer2_overflowing = 0;
  }
}


ISR(TIMER2_OVF_vect)
// Timer overflow interrupt
{
  TCNT2 = MsTimer2_tcnt2;   // Reset the timer to count another 4ms
  MsTimer2_overflow();
}
