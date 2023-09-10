#include "AccelStepper.h"

void AccelStepper::moveTo(int32_t absolute) {
  if (_targetPos != absolute) {
    _targetPos = absolute;
    computeNewSpeed();
  }
}

void AccelStepper::move(int32_t relative) {
  moveTo(_currentPos + relative);
}

int32_t AccelStepper::distanceToGo() {
  return _targetPos - _currentPos;
}

int32_t AccelStepper::targetPosition() {
  return _targetPos;
}

int32_t AccelStepper::currentPosition() {
  return _currentPos;
}

void AccelStepper::setCurrentPosition(int32_t position) {
  _currentPos = position;
  computeNewSpeed();
}

AccelStepper::AccelStepper(uint8_t pin1, uint8_t pin2, uint8_t enablePin, bool enable) {
  _currentPos = 0;
  _targetPos = 0;
  _speed = 0;
  _maxSpeed = 0;
  _acceleration = 0;
  _stepInterval = 0;
  _enablePin = enablePin;
  _lastStepTime = 0;
  _pin[0] = pin1;
  _pin[1] = pin2;
  _enableInverted = false;

  _n = 0;
  _c0 = 0;
  _cmin = 1;
  _direction = DIRECTION_CCW;
  for (uint8_t i = 0; i < sizeof(_pin); i++) _pinInverted[i] = 0;
  if (enable) enableOutputs();
  setAcceleration(1);
  setMaxSpeed(1);
}

inline uint32_t AccelStepper::roundedDivision(uint32_t numerator, uint32_t denominator) {
  return (numerator + denominator / 2) / denominator;
}

uint32_t AccelStepper::computeNewSpeed() {
  int32_t distance = distanceToGo();

  // If we don't need to move or are already stopped
  if (distance == 0 && _speed == 0)
  {
    _stepInterval = 0;
    return _stepInterval;
  }

  // Determine desired direction based on distanceTo sign
  uint8_t desired_direction = (distance > 0) ? DIRECTION_CW : DIRECTION_CCW;

  // Compute step interval and speed
  if (_n == 0) {
    _direction = desired_direction;
  } else {
    // stepsToStop calculation using simplified rounded division
    // Adjust n based on current movement and desired direction
    if (_n > 0) {  // Currently accelerating
      if (abs(_n) >= abs(distance) || _direction != desired_direction) { _n = -_n; }
    } else {  // Currently decelerating
      if (abs(_n) < abs(distance) && _direction == desired_direction)  { _n = -_n; }
    }

    _stepInterval = max(roundedDivision(_c0, sqrtApprox(_n)), _cmin);
  }

  _n ++;
  _speed = roundedDivision(1000000, _stepInterval);
  if (_direction == DIRECTION_CCW)
    _speed = -_speed;

  return _stepInterval;
}

uint32_t AccelStepper::run() {
  uint32_t nextTime = runSpeed();
  if (nextTime) computeNewSpeed();
  return nextTime;
}

uint32_t AccelStepper::runSpeed() {
  if (!_stepInterval) return 0;
  _currentPos += (_direction == DIRECTION_CW) ? 1 : -1;
  step();
  _lastStepTime += _stepInterval;
  return _lastStepTime;
}


void AccelStepper::step() {
  digitalWrite(_pin[0], _direction ^ _pinInverted[0]);
  digitalWrite(_pin[1], HIGH ^ _pinInverted[1]);
  digitalWrite(_pin[1], LOW ^ _pinInverted[1]);
}

void AccelStepper::disableOutputs() {
  digitalWrite(_pin[0], LOW ^ _pinInverted[0]);
  digitalWrite(_pin[1], LOW ^ _pinInverted[1]);
  if (_enablePin != INVALID_PIN) {
    pinMode(_enablePin, OUTPUT);
    digitalWrite(_enablePin, LOW ^ _enableInverted);
  }
}

void AccelStepper::enableOutputs() {
  pinMode(_pin[0], OUTPUT);
  pinMode(_pin[1], OUTPUT);
  if (_enablePin != INVALID_PIN) {
    pinMode(_enablePin, OUTPUT);
    digitalWrite(_enablePin, HIGH ^ _enableInverted);
  }
}

void AccelStepper::setMaxSpeed(uint16_t maxSpeed) {
  if (_maxSpeed != maxSpeed) {
    _maxSpeed = maxSpeed;
    _cmin = roundedDivision(1000000, maxSpeed);
  }
}

uint16_t AccelStepper::maxSpeed() {
  return _maxSpeed;
}

void AccelStepper::setSpeed(uint16_t speed) {
  if (speed > _maxSpeed) {
    speed = _maxSpeed;
  }
  if (speed == 0) {
    _stepInterval = 0;
  } else {
    _stepInterval = roundedDivision(1000000, speed);
  }
  _speed = speed;
}

uint16_t AccelStepper::speed() {
  return _speed;
}

inline uint16_t AccelStepper::sqrtApprox(int32_t number) {
  uint32_t input = abs(number);
  if (input < 3) {
    return 1; // Slightly hacky way to keep a division safe
  }
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

void AccelStepper::setAcceleration(uint16_t acceleration) {
  if (acceleration == 0) return;
  if (_acceleration != acceleration) {
    _n = roundedDivision(_n * _acceleration, acceleration);
    _c0 = roundedDivision(1000000, sqrtApprox(acceleration));
    _acceleration = acceleration;
  }
}

uint16_t AccelStepper::acceleration() {
  return _acceleration;
}

void AccelStepper::setEnablePin(uint8_t enablePin) {
  _enablePin = enablePin;
  if (_enablePin != INVALID_PIN) {
    pinMode(_enablePin, OUTPUT);
    digitalWrite(_enablePin, HIGH ^ _enableInverted);
  }
}

void AccelStepper::setPinsInverted(bool directionInvert, bool stepInvert, bool enableInvert)
{
  _pinInverted[0] = stepInvert;
  _pinInverted[1] = directionInvert;
  _enableInverted = enableInvert;
}

inline AccelStepper::Direction AccelStepper::direction(int32_t distanceTo) {
  return (distanceTo > 0) ? DIRECTION_CW : DIRECTION_CCW;
}

void AccelStepper::stop() {
  if (_speed != 0) {
    move(_n);
  }
}

bool AccelStepper::isRunning() {
  return !distanceToGo();
}
