// CameraController.h

#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "Arduino.h"

class CameraController {
  public:
    CameraController(uint8_t focusPin, uint8_t shutterPin, uint8_t triggerPin);

    void focus(bool status);
    bool focus();

    void shutter(bool status);
    bool shutter();

    bool triggered();

    void stop();

    void focusTime(uint32_t duration);
    uint32_t focusTime();

    void shutterTime(uint32_t duration);
    uint32_t shutterTime();

    void triggerDebounce(uint32_t duration);
    uint32_t triggerDebounce();

    void shotRate(uint32_t duration);
    uint32_t shotRate();

    void shoot(uint32_t duration, uint8_t numShots);
    void shoot(uint32_t duration);
    void shoot();

    void holdfocus(bool shouldHoldFocus);
    void prefocus(bool shouldPrefocus);
    void triggered(bool useTrigger);

    bool isBusy();

    uint32_t handle();

  private:
    uint8_t _focusPin, _shutterPin, _triggerPin;
    uint32_t _focusTime, _shutterTime, _triggerDebounceTime, _shotRate, _shotDuration;
    uint32_t _lastFocusTime, _lastShotTime, _lastTriggerTime;
    uint8_t _numShots, _shotCount;
    bool _focusState, _shutterState, _triggerState, _triggered;
    bool _shouldHoldFocus, _shouldPrefocus, _useTrigger;
};

#endif
