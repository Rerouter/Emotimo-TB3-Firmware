#include "MyMultiStepper.h"
#include "AccelStepper.h"

MyMultiStepper::MyMultiStepper()
  : _num_steppers(0)
{
}

bool MyMultiStepper::addStepper(AccelStepper& stepper)
{
  if (_num_steppers >= MULTISTEPPER_MAX_STEPPERS)
    return false;

  _steppers[_num_steppers] = &stepper;
  _powersave[_num_steppers++] = false;
  return true;
}

void MyMultiStepper::moveByLongestTime(int32_t positions[], bool relative)
{
  int32_t longestTicks = 0;

  // Calculate the longest time in terms of "ticks" (scaled integers)
  for (uint8_t i = 0; i < _num_steppers; i++)
  {
    int32_t thisDistance = relative ? positions[i] : positions[i] - _steppers[i]->currentPosition();
    int32_t thisTicks = (abs(thisDistance) * 1000) / _steppers[i]->maxSpeed(); // Factor of 1000 for precision

    if (thisTicks > longestTicks)
      longestTicks = thisTicks;
  }

  if (longestTicks > 0)
  {
    for (uint8_t i = 0; i < _num_steppers; i++)
    {
      int32_t thisDistance = relative ? positions[i] : positions[i] - _steppers[i]->currentPosition();
      int32_t thisSpeed = (thisDistance * 1000) / longestTicks; // Convert back from "ticks" to speed

      if (_powersave[i] && thisDistance != 0) _steppers[i]->enableOutputs();

      if (relative)
        _steppers[i]->move(thisDistance);
      else
        _steppers[i]->moveTo(positions[i]);

      _steppers[i]->setSpeed(thisSpeed / 1000); // Convert back from "ticks" to regular units
    }
  }
}

void MyMultiStepper::moveTo(int32_t absolute[])
{
  moveByLongestTime(absolute, false);
}

void MyMultiStepper::moveRelative(int32_t relative[])
{
  moveByLongestTime(relative, true);
}


uint32_t MyMultiStepper::handle()
{
  uint32_t nextCallTimestamp = UINT32_MAX; // Max value as a starting point
  bool allIdle = true;

  for (uint8_t i = 0; i < _num_steppers; i++)
  {
    if (_steppers[i]->distanceToGo() != 0)
    {
      uint32_t thisTimestamp = _steppers[i]->runSpeed();
      if (thisTimestamp != 0 && thisTimestamp < nextCallTimestamp)
        nextCallTimestamp = thisTimestamp;

      allIdle = false;
    }
    else
    {
      if (_powersave[i]) _steppers[i]->disableOutputs();
    }
  }

  return allIdle ? 0 : nextCallTimestamp;
}

bool MyMultiStepper::atPosition()
{
  for (uint8_t i = 0; i < _num_steppers; i++)
  {
    if (_steppers[i]->distanceToGo() != 0) return false;
  }
  return true;
}

void MyMultiStepper::enable()
{
  for (uint8_t i = 0; i < _num_steppers; i++)
  {
    _steppers[i]->enableOutputs();
  }
}

void MyMultiStepper::disable()
{
  for (uint8_t i = 0; i < _num_steppers; i++)
  {
    _steppers[i]->disableOutputs();
  }
}

template<size_t N>
void MyMultiStepper::disableMotor(uint8_t (&motorIndices)[N]) {
    for (uint8_t i = 0; i < N; i++) {
        uint8_t index = motorIndices[i];
        if (index < _num_steppers) _steppers[index]->disableOutputs();
    }
}

template<size_t N>
void MyMultiStepper::enableMotor(uint8_t (&motorIndices)[N]) {
    for (uint8_t i = 0; i < N; i++) {
        uint8_t index = motorIndices[i];
        if (index < _num_steppers) _steppers[index]->enableOutputs();
    }
}

void MyMultiStepper::setPowerSave(uint8_t index, bool mode) {
  if (index < _num_steppers) _powersave[index] = mode;
}


void MyMultiStepper::stop() {
  for (uint8_t i = 0; i < _num_steppers; i++) {
    _steppers[i]->stop();
  }
}
