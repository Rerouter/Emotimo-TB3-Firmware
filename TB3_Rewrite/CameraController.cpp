#include "CameraController.h"

CameraController::CameraController(uint8_t focusPin, uint8_t shutterPin, uint8_t triggerPin)
  : _focusPin(focusPin), _shutterPin(shutterPin), _triggerPin(triggerPin),
    _focusTime(200), _shutterTime(100), _triggerDebounceTime(50), _shotRate(1000),
    _numShots(1), _shotCount(0), _focusState(false), _shutterState(false), _triggerState(false), _triggered(false),
    _shouldHoldFocus(false), _shouldPrefocus(true), _useTrigger(false)
{
  pinMode(focusPin, OUTPUT);
  pinMode(shutterPin, OUTPUT);
  pinMode(triggerPin, INPUT);
  digitalWrite(_focusPin, HIGH); // active low
  digitalWrite(_shutterPin, HIGH); // active low
}

void CameraController::focus(bool status) {
  _focusState = status;
  digitalWrite(_focusPin, _focusState ? LOW : HIGH); // active low
}

bool CameraController::focus() {
  return _focusState;
}

void CameraController::shutter(bool status) {
  _shutterState = status;
  digitalWrite(_shutterPin, _shutterState ? LOW : HIGH); // active low
}

bool CameraController::shutter() {
  return _shutterState;
}

bool CameraController::triggered() {
  bool trigger = digitalRead(_triggerPin) == LOW; // active low
  if (trigger && !_triggered && (micros() - _lastTriggerTime) > _triggerDebounceTime) {
    _triggerState = true;
    _triggered = true;
    _lastTriggerTime = micros();
  }
  if (!trigger && _triggered) {
    _triggered = false; // reset the trigger when the signal is released
  }
  return _triggerState;
}

void CameraController::stop() {
  focus(false);
  shutter(false);
}

void CameraController::focusTime(uint32_t duration) {
  _focusTime = duration;
}

uint32_t CameraController::focusTime() {
  return _focusTime;
}

void CameraController::shutterTime(uint32_t duration) {
  _shutterTime = duration;
}

uint32_t CameraController::shutterTime() {
  return _shutterTime;
}

void CameraController::triggerDebounce(uint32_t duration) {
  _triggerDebounceTime = duration;
}

uint32_t CameraController::triggerDebounce() {
  return _triggerDebounceTime;
}

void CameraController::shotRate(uint32_t duration) {
  _shotRate = duration;
}

uint32_t CameraController::shotRate() {
  return _shotRate;
}

void CameraController::shoot(uint32_t duration, uint8_t numShots) {
  _numShots = numShots;
  _shotDuration = duration;
  _shotCount = 0;
}

void CameraController::shoot(uint32_t duration) {
  _numShots = 1;
  _shotDuration = duration;
  _shotCount = 0;
}

void CameraController::shoot() {
  _numShots = 1;
  _shotDuration = _shutterTime;
  _shotCount = 0;
}


void CameraController::holdfocus(bool shouldHoldFocus) {
  _shouldHoldFocus = shouldHoldFocus;
}

void CameraController::prefocus(bool shouldPrefocus) {
  _shouldPrefocus = shouldPrefocus;
}

void CameraController::triggered(bool useTrigger) {
  _useTrigger = useTrigger;
}

bool CameraController::isBusy() {
  return focus() || shutter() || (_shotCount < _numShots);
}

uint32_t CameraController::handle() {
  uint32_t currentTime = micros();
  uint32_t nextCallTime = 0; // Default to no task scheduled

  if (_shouldPrefocus && !focus()) {
    focus(true);
    _lastFocusTime = currentTime;
  }

  if (focus() && currentTime - _lastFocusTime > _focusTime * 1000) {
    if (!_shouldHoldFocus) {
      focus(false);
    } else {
      nextCallTime = _lastFocusTime + _focusTime * 1000 - currentTime;
    }
  }

  if (!shutter() && ((_useTrigger && triggered()) || (!_useTrigger && currentTime - _lastShotTime > _shotRate * 1000)) && _shotCount < _numShots) {
    shutter(true);
    _lastShotTime = currentTime;
    _shotCount++;
    _triggerState = false; // Consume the trigger
  } else if (!shutter() && _shotCount < _numShots) {
    uint32_t potentialNextCall = _lastShotTime + _shotRate * 1000 - currentTime;
    nextCallTime = (nextCallTime == 0) ? potentialNextCall : min(nextCallTime, potentialNextCall);
  }

  if (shutter() && currentTime - _lastShotTime > _shutterTime * 1000) {
    shutter(false);
  } else if (shutter()) {
    uint32_t potentialNextCall = _lastShotTime + _shutterTime * 1000 - currentTime;
    nextCallTime = (nextCallTime == 0) ? potentialNextCall : min(nextCallTime, potentialNextCall);
  }

  return nextCallTime;
}
