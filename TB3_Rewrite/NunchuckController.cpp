//NunchuckController.cpp

#include "NunchuckController.h"

NunchuckController::NunchuckController() :
  _nunchuck(),
  _readErrorCount(0),
  _deadband(60),
  _threshold(120),
  _centerOffsetX(127),
  _centerOffsetY(127),
  _gravityX(0),
  _gravityY(0),
  _gravityZ(0),
  _isGravityVectorSet(false)
{
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    _buttons[i] = {0, false, false, false, ButtonEvent::NONE};
  }
}

uint32_t NunchuckController::handle() {
  _nunchuck.requestData();
  _readErrorCount = _nunchuck.getErrorCount();

  if (_readErrorCount >= ERROR_LIMIT) {
    resetController();
  } else {
    updateButtonStates();
    UpdateJoystickDirection();
  }

  return micros() + UPDATE_RATE_MS;  // Return the next time it should be called
}


void NunchuckController::updateButtonStates() {
  bool newButtonState[2] = {_nunchuck.getButtonC(), _nunchuck.getButtonZ()};
  uint32_t now = micros();

  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    // Button state has changed
    if (newButtonState[i] != _buttons[i].currentState) {
      _buttons[i].lastPressTime = now;
      _buttons[i].currentState = newButtonState[i];
    }

    // Debounce the button state
    if (now - _buttons[i].lastPressTime > DEBOUNCE_DELAY * 1000) {
      _buttons[i].debouncedState = _buttons[i].currentState;

      // Button was just pressed
      if (_buttons[i].debouncedState && _buttons[i].event == ButtonEvent::NONE) {
        _buttons[i].event = ButtonEvent::PRESSED; // Store the pressed event
      }

      // Button is held
      if (_buttons[i].debouncedState && now - _buttons[i].lastPressTime > HELD_DELAY * 1000 && !_buttons[i].wasHeld) {
        _buttons[i].wasHeld = true;
        _buttons[i].event = ButtonEvent::HELD;  // Overwrite with held if it's held for longer
      }

      // Button was released
      if (!_buttons[i].debouncedState && _buttons[i].wasHeld) {
        _buttons[i].wasHeld = false;  // Reset the wasHeld flag
      }
    }
  }
}

bool NunchuckController::getButtonState(uint8_t button) {
  return _buttons[button].debouncedState;
}

ButtonEvent NunchuckController::getButtonEvent(uint8_t button) {
  ButtonEvent currentEvent = _buttons[button].event;
  if (currentEvent != ButtonEvent::NONE) {
    _buttons[button].event = ButtonEvent::NONE;  // Consume the event
  }
  return currentEvent;
}


void NunchuckController::resetController() {
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    _buttons[i] = {0, false, false, false, ButtonEvent::NONE};
  }
  _joystickDirection = JoystickDirection::RELEASED;
}


void NunchuckController::UpdateJoystickDirection() {
  int8_t x = getX();
  int8_t y = getY();

  if (abs(y) > abs(x)) {
    if (y > _threshold) {
      _joystickDirection = JoystickDirection::UP;
    } else if (y < -_threshold) {
      _joystickDirection = JoystickDirection::DOWN;
    }
  } else {  // Note: If y and x are both within the threshold, x's direction is considered
    if (x > _threshold) {
      _joystickDirection = JoystickDirection::RIGHT;
    } else if (x < -_threshold) {
      _joystickDirection = JoystickDirection::LEFT;
    }
  }

  _joystickDirection = JoystickDirection::RELEASED;
}


int8_t NunchuckController::getX() {
  return getAxisValue(_nunchuck.getJoyX(), _centerOffsetX);
}

int8_t NunchuckController::getY() {
  return getAxisValue(_nunchuck.getJoyY(), _centerOffsetY);
}

int8_t NunchuckController::getAxisValue(uint8_t rawAxisValue, uint8_t centerOffset) {
    if (_readErrorCount >= ERROR_LIMIT) {
        return 0;
    }
    
    int8_t rawValue = rawAxisValue - centerOffset;
    if (abs(rawValue) <= _deadband) {
        return 0;
    }
    
    int16_t adjustedValue = abs(rawValue) - _deadband;
    adjustedValue = constrain(adjustedValue, 0, 255 - _deadband);
    return (rawValue > 0) ? adjustedValue : -adjustedValue;
}


int16_t NunchuckController::getExponentialX() {
  int8_t linearX = this->getX(); // Get the linear joystick value
  if (linearX == 0) { // If the value is within the deadband
    return 0; // Return 0
  } else {
    int32_t absX = abs(linearX); // absolute value
    int32_t normalizedX = (absX * absX * absX) >> 16; // normalize using bit shifting for fixed-point math
    return linearX > 0 ? normalizedX : -normalizedX; // retain the original sign
  }
}

int16_t NunchuckController::getExponentialY() {
  int8_t linearY = this->getY(); // Get the linear joystick value
  if (linearY == 0) { // If the value is within the deadband
    return 0; // Return 0
  } else {
    int32_t absY = abs(linearY); // absolute value
    int32_t normalizedY = (absY * absY * absY) >> 16; // normalize using bit shifting for fixed-point math
    return linearY > 0 ? normalizedY : -normalizedY; // retain the original sign
  }
}


void NunchuckController::calibrate() {
  uint16_t sumX = 0;
  uint16_t sumY = 0;

  // We can take average of 10 readings
  for (uint8_t i = 0; i < 10; i++) {
    _nunchuck.requestData();
    if (_nunchuck.getJoyX() < _deadband && _nunchuck.getJoyY() < _deadband) {
      sumX += _nunchuck.getJoyX();
      sumY += _nunchuck.getJoyY();
      delay(10);
    }
  }

  _centerOffsetX = sumX / 10;
  _centerOffsetY = sumY / 10;
}

void NunchuckController::setDeadband(uint8_t deadband) {
  _deadband = deadband;
}

void NunchuckController::setThreshold(uint8_t threshold) {
  _threshold = threshold;
}

int16_t NunchuckController::getRoll() {
  if (_readErrorCount >= ERROR_LIMIT || !_isGravityVectorSet) {
    return 0; // or some error value, depends on your program
  }
  int16_t y = _nunchuck.getAccelY() - _gravityY;
  int16_t z = _nunchuck.getAccelZ() - _gravityZ;

  if (z == 0) { // Prevent division by zero
    z = 1; // Assign to 1 as a safe approximation
  }

  return (y * 5730) / z;
}

inline uint16_t sqrt_approximation(uint32_t input) {
  uint8_t counter = 0;
  uint16_t approx = 1;

  while (counter < 10) {
    uint32_t squared = approx * approx;
    if ( squared == input) {
      break;
    }

    if (squared > input || squared + 2 * approx < input) {
      approx = (approx + input / approx) / 2;
      counter += 1;
    }

    else {
      break;
    }
  }

  return approx;
}


int16_t NunchuckController::getPitch() {
  if (_readErrorCount >= ERROR_LIMIT || !_isGravityVectorSet) {
    return 0; // or some error value, depends on your program
  }
  int16_t x = _nunchuck.getAccelX() - _gravityX;
  int16_t y = _nunchuck.getAccelY() - _gravityY;
  int16_t z = _nunchuck.getAccelZ() - _gravityZ;
  int16_t denominator = sqrt_approximation(int32_t(y) * y + int32_t(z) * z);

  if (denominator == 0) { // Prevent division by zero
    denominator = 1; // Assign to 1 as a safe approximation
  }

  return (x * 5730) / denominator;
}


void NunchuckController::setGravityVector() {
  if (_readErrorCount < ERROR_LIMIT) {
    _gravityX = _nunchuck.getAccelX();
    _gravityY = _nunchuck.getAccelY();
    _gravityZ = _nunchuck.getAccelZ();
    _isGravityVectorSet = true;
  }
}

bool NunchuckController::isGravityVectorSet() {
  return _isGravityVectorSet;
}
