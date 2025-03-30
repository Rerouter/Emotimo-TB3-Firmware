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

// ========================================
// Camera control functions
// ========================================


// Pin assignments

constexpr uint8_t CAMERA_PIN = 12;  // drives tip of 2.5 mm connector
constexpr uint8_t FOCUS_PIN = 13;   // drives  middle of 2.5mm connector  

enum class CameraState : uint8_t {
    Idle,      // Camera is idle, ready for a new sequence
    Interval,  // If the shot interval is longer, wait for it. 
    Prefire,   // Focus pin held for prefire preparation
    Exposing,  // Shutter signal held for exposure
    Postfire,  // Cleanup period after releasing signals
};

// Camera state variables
CameraState currentCameraState = CameraState::Idle;
uint32_t prefire_ms = 100;     // Prefire time in ms 
uint32_t shutter_ms = 100;  // Exposure time in ms, this powers up motor early for the shot
uint32_t postfire_ms = 100;  // Postfire time in ms, this holds power for the motors after the shot
uint32_t shot_interval_ms = 0;

// Signal state variables
volatile bool Shutter_Signal_Engaged = false;
volatile bool Focus_Signal_Engaged = false;

// Timer structure and state
struct Timer2State {
    volatile uint32_t msecs;       // Timer duration in milliseconds
    volatile uint32_t count;       // Counter for overflow events
    volatile uint8_t tcnt2;             // Precomputed timer reload value
    volatile bool overflowing;          // Overflow state flag
};

static Timer2State timer2 = {0, 0, 0, 0}; // Timer state instance

__attribute__ ((warn_unused_result))
bool CameraFocus() { return Focus_Signal_Engaged; }
void CameraFocus(const bool focus_state) {
    digitalWrite(FOCUS_PIN, focus_state ? HIGH : LOW);
    Focus_Signal_Engaged = focus_state;
}

__attribute__ ((warn_unused_result))
bool CameraShutter() { return Shutter_Signal_Engaged; }
void CameraShutter(const bool shutter_state) {
    digitalWrite(CAMERA_PIN, shutter_state ? HIGH : LOW);
    Shutter_Signal_Engaged = shutter_state;
}

__attribute__ ((warn_unused_result))
uint32_t ShutterTime() { return shutter_ms; }
void ShutterTime(uint32_t time) {shutter_ms = time; }

__attribute__ ((warn_unused_result))
uint32_t PrefireTime() { return prefire_ms; }
void PrefireTime(uint32_t time) {prefire_ms = time; }

__attribute__ ((warn_unused_result))
uint32_t PostfireTime() { return postfire_ms; }
void PostfireTime(uint32_t time) {postfire_ms = time; }

__attribute__ ((warn_unused_result))
uint32_t IntervalTime() { return shot_interval_ms; }
void IntervalTime(uint32_t time) {shot_interval_ms = time; }

bool IsCameraIdle() { return currentCameraState == CameraState::Idle; }


void FireCamera() {
  if (shot_interval_ms > (prefire_ms + shutter_ms + postfire_ms)) {
    currentCameraState = CameraState::Interval;
    MsTimer2_set(shot_interval_ms - (prefire_ms + shutter_ms + postfire_ms));
  }
  else if (prefire_ms > 0) {
    CameraFocus(true);
    currentCameraState = CameraState::Prefire;
    MsTimer2_set(prefire_ms);
  }
  else {
    CameraFocus(true);
    currentCameraState = CameraState::Exposing;
    CameraShutter(true);
    MsTimer2_set(shutter_ms);
  }
  MsTimer2_start();
}

// Fires the camera with the specified exposure time
void FireCamera(const uint32_t exp_ms) {
    CameraFocus(true);          // Engage focus
    CameraShutter(true);        // Engage shutter

    MsTimer2_set(exp_ms);    // Set exposure timer
    MsTimer2_start();
}

// Stops the camera
void StopCamera() {
    MsTimer2_stop();       // Stop exposure timer
    CameraShutter(false);  // Disengage shutter
    CameraFocus(false);    // Disengage focus
}

void SetupCameraPins() {
  pinMode(CAMERA_PIN, OUTPUT);
  pinMode(FOCUS_PIN, OUTPUT);

  CameraShutter(false);
  CameraFocus(false);
}

//-----------------------------------

// Timer2 setup
void MsTimer2_set(const uint32_t ms) {
    if (ms == 0) {
        timer2.msecs = 1;
    } else {
        timer2.msecs = ms;
    }

    TIMSK2 &= static_cast<uint8_t>(~(1 << TOIE2));
    TCCR2A &= static_cast<uint8_t>(~((1 << WGM21) | (1 << WGM20)));
    TCCR2B &= static_cast<uint8_t>(~(1 << WGM22));
    ASSR &= static_cast<uint8_t>(~(1 << AS2));
    TIMSK2 &= static_cast<uint8_t>(~(1 << OCIE2A));

    TCCR2B |= (1 << CS22);
    TCCR2B &= static_cast<uint8_t>(~((1 << CS21) | (1 << CS20)));
    constexpr uint32_t prescaler = 64;
    constexpr uint8_t tcnt2 = static_cast<uint8_t>(256 - (F_CPU / (prescaler * 1000)));

    timer2.tcnt2 = tcnt2;
}

void MsTimer2_start() {
    timer2.count = 0;
    timer2.overflowing = false;
    TCNT2 = timer2.tcnt2;
    TIMSK2 |= (1 << TOIE2);
}

void MsTimer2_stop() {
  TIMSK2 &= static_cast<uint8_t>(~(1 << TOIE2));
  timer2.count = 0;
  switch (currentCameraState) {
    case CameraState::Idle: // Do nothing, it means we manually fired the camera
      break;
    case CameraState::Interval: // Interval is over,
      CameraFocus(true);
      if (prefire_ms > 0) {
        currentCameraState = CameraState::Prefire;
        MsTimer2_set(prefire_ms);
      }
      else {
        currentCameraState = CameraState::Exposing;
        CameraShutter(true);
        MsTimer2_set(shutter_ms);
      }
      MsTimer2_start();
      break;

    case CameraState::Prefire: // Prefire is over, start shooting
      currentCameraState = CameraState::Exposing;
      CameraShutter(true);
      MsTimer2_set(shutter_ms);
      MsTimer2_start();
      break;

    case CameraState::Exposing:  // Exposure is over, start postfire
      if (postfire_ms > 0) {
        currentCameraState = CameraState::Postfire;
        CameraShutter(false);
        MsTimer2_set(postfire_ms);
        MsTimer2_start();
      }
      else {
        currentCameraState = CameraState::Idle;
        CameraFocus(false);
      }
      break;

    case CameraState::Postfire: // Postfire is over, return to idle
      currentCameraState = CameraState::Idle;
      CameraFocus(false);
      break;
    }
}

void MsTimer2_overflow() {
    timer2.count++;
    if (timer2.count >= timer2.msecs && !timer2.overflowing) {
        timer2.overflowing = true;
        timer2.count -= timer2.msecs; // Catch missed overflows
        StopCamera();               // Stop the camera
        timer2.overflowing = false;
    }
}

ISR(TIMER2_OVF_vect) {
    TCNT2 = timer2.tcnt2;
    MsTimer2_overflow();
}