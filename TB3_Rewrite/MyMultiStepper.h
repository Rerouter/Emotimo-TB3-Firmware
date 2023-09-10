#ifndef MyMultiStepper_h
#define MyMultiStepper_h

#include <stdlib.h>
#include <Arduino.h>
#include "AccelStepper.h"

#define MULTISTEPPER_MAX_STEPPERS 8

class MyMultiStepper
{
public:
    MyMultiStepper();
    bool addStepper(AccelStepper& stepper);
    void moveByLongestTime(int32_t positions[], bool relative);
    void moveTo(int32_t absolute[]);
    void moveRelative(int32_t relative[]);
    uint32_t handle();
    bool atPosition();
    void enable();
    void disable();
    void stop();
    template<size_t N>
    void disableMotor(uint8_t (&motorIndices)[N]);
    template<size_t N>
    void enableMotor(uint8_t (&motorIndices)[N]);
    void setPowerSave(uint8_t index, bool mode);
    
private:
    AccelStepper* _steppers[MULTISTEPPER_MAX_STEPPERS];
    bool _powersave[MULTISTEPPER_MAX_STEPPERS];
    uint8_t _num_steppers;
};

#endif
