//NunchuckController.h

#ifndef NUNCHUCK_CONTROLLER_H
#define NUNCHUCK_CONTROLLER_H

#include "WiiNunchuck.h"

const uint32_t UPDATE_RATE_MS = 10; // Update every 10ms for a maximum of 100Hz
const uint32_t BUTTON_COUNT = 2;
const uint16_t DEBOUNCE_DELAY = 50;
const uint16_t HELD_DELAY = 1000;
const uint8_t  ERROR_LIMIT = 5;

enum class ButtonEvent {
    NONE,
    PRESSED,
    HELD,
};

struct ButtonInfo {
    uint32_t lastPressTime;
    bool currentState;       // Current state of the button (pressed/released)
    bool debouncedState;     // State after debouncing
    bool wasHeld;            // Flag to check if button was previously held
    ButtonEvent event;       // Public event for the button
};

enum class JoystickDirection {
    RELEASED,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class NunchuckController {
  public:
    NunchuckController();
    uint32_t handle();
    ButtonEvent getButtonEvent(uint8_t button);
    bool getButtonState(uint8_t button);
    void setDeadband(uint8_t deadband);
    void setThreshold(uint8_t threshold);
    
    void calibrate();
    int8_t getX();
    int8_t getY();
    int16_t getExponentialX();
    int16_t getExponentialY();
    int16_t getRoll();
    int16_t getPitch();
    void setGravityVector();
    bool isGravityVectorSet();

  private:
    void resetController();
    void resetButtonStates(uint8_t button);
    void updateButtonStates();
    
    void UpdateJoystickDirection();
    int8_t getAxisValue(uint8_t rawAxisValue, uint8_t centerOffset);

    WiiNunchuck _nunchuck;

    int16_t _readErrorCount;

    ButtonInfo _buttons[BUTTON_COUNT];

    uint8_t  _deadband;
    uint8_t  _threshold;
    int16_t _centerOffsetX;
    int16_t _centerOffsetY;
    
    JoystickDirection _joystickDirection;
    
    int16_t _gravityX;
    int16_t _gravityY;
    int16_t _gravityZ;
    bool _isGravityVectorSet;
};

#endif //NUNCHUCK_CONTROLLER_H
