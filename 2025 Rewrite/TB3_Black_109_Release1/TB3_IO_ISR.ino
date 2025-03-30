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

volatile bool state = true;           //new variable for interrupt
volatile bool changeHappened = false;  //new variable for interrupt

// Start waiting for the button press (attach interrupt here)
void StartExternalTrigger() {
    pinMode(IO_3, INPUT);
    digitalWrite(IO_3, HIGH);
    changeHappened = false;  // Reset state
    attachInterrupt(digitalPinToInterrupt(IO_3), ExternalInterruptHandler, CHANGE);
}

__attribute__ ((warn_unused_result))
bool ExternalTriggerChanged() {
  return changeHappened;
}

__attribute__ ((warn_unused_result))
bool ExternalTriggerPressed() {
  return (state == 0);
}

// Stop waiting for the button press (detach interrupt)
void StopTriggerWaiting() {
    detachInterrupt(digitalPinToInterrupt(IO_3));
    digitalWrite(IO_3, LOW);
}

// Reset the external trigger state
void ResetTriggerState() {
    changeHappened = false;
}

// External interrupt handler
void ExternalInterruptHandler() {
    changeHappened = true;                // Indicate a state change occurred
    state = digitalRead(IO_3);            // Update button state (pressed if LOW)
}


// No longer needed as using a pin change interrupt
// void setupstartISR1() {
//   TCCR1A = 0;
//   TCCR1B = _BV(WGM13);

//   ICR1 = (F_CPU / 4000000) * TIME_CHUNK;  // goes twice as often as time chunk, but every other event turns off pins
//   TCCR1B &= static_cast<uint8_t>(~(_BV(CS10) | _BV(CS11) | _BV(CS12)));
//   TIMSK1 = _BV(TOIE1);
//   TCCR1B |= _BV(CS10);
// }

void startISR1() {
  TIMSK1 = _BV(TOIE1);
}

void stopISR1() {
  TIMSK1 &= static_cast<uint8_t>(~_BV(TOIE1));
}
