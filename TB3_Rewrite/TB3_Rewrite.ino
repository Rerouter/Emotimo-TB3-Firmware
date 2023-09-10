#include "MyMultiStepper.h"
#include "AccelStepper.h"
#include "NunchuckController.h"
#include "LCDController.h"
#include "CameraController.h"
#include "PanoramaController.h"
#include "TaskScheduler.h"
#include "GUI.h"

AccelStepper stepper1(2, 3, 1);
AccelStepper stepper2(4, 5, 6);
MyMultiStepper Motion;
NunchuckController Joystick;
LCDController LCD(2);
CameraController Camera(2, 3, 4);
PanoramaController Pano(&Camera, &Motion);
TaskScheduler scheduler;


uint32_t handleLCD() {
    return LCD.handle();
}

uint32_t handleCAM() {
    return Camera.handle();
}

uint32_t handleJOY() {
    return Joystick.handle();
}

uint32_t handleMOT() {
    return Motion.handle();
}

uint32_t handlePANO() {
    return Pano.handle();
}


void setup() {
  stepper1.setMaxSpeed(1000.0);
  stepper2.setMaxSpeed(1000.0);
  Motion.addStepper(stepper1);
  Motion.addStepper(stepper2);
  LCD.on();
  LCD.Brightness(5);
  LCD.moveCursor(0, 0); // Move to beginning of first line
  LCD.print("Hello, World!");
  LCD.moveCursor(0, 1); // Move to beginning of second line
  LCD.print("LCD Initialized");
  scheduler.addTask(handleLCD);

  scheduler.addTask(handleJOY);

  Camera.focusTime(120);
  Camera.shutterTime(1);
  Camera.triggerDebounce(50);
  Camera.shotRate(200);

  // Define behaviours
  Camera.holdfocus(true);  // hold focus after shooting
  Camera.prefocus(true);  // prefocus before shooting
  Camera.triggered(true);  // use trigger signal
  scheduler.addTask(handleCAM);
  scheduler.addTask(handleMOT);
}

void loop() {
  scheduler.runTasks();
  if (!Motion.atPosition()) {
    int32_t targets[2] = {1000, 2000};
    Motion.moveTo(targets);
  }
  if (Camera.triggered()) {
    Camera.shoot();  // shoot 5 shots with a duration of 1000ms
  }
}
