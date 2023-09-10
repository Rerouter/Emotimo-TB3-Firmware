#ifndef PanoramaController_h
#define PanoramaController_h

#include <Arduino.h>
#include "MyMultiStepper.h"
#include "CameraController.h"

enum class Pattern {
  Zigzag,
  Serpentine
};

enum class Mode {
  CenterDefined,
  CornerDefined
};

class PanoramaController {
  private:
    CameraController* cam;
    MyMultiStepper* motion;

    uint32_t totalImagesShot;
    uint16_t numVerticalPanels;
    uint16_t numHorizontalPanels;

    int32_t imageDimensions[2];
    int32_t coordinates[2][2];

    int32_t currentTarget[2];
    int32_t keyframes[16][2];
    uint8_t numKeyframes;
    uint32_t duration; // Duration of the operation in milliseconds
    uint32_t startTime; // Time when operation starts
    uint32_t nextActionTime; // Time when next panorama should start
    uint16_t currentRepeat; // Counter for repeats
    uint16_t repeats;
    bool startLeft;
    bool startTop;
    bool rowFirst;
    Pattern pattern;

    enum State { Idle, Moving, Shooting } currentState;

    bool planNextMove();
    int32_t* calculateKeyframeOffset();
    void reset();
    void calculatePanelDimensions();
    void captureImage();

  public:
    PanoramaController(CameraController* camera, MyMultiStepper* motionController, bool startLeft = true, bool startTop = true, bool rowFirst = true, Pattern pattern = Pattern::Zigzag);

    void initialize(Mode mode, int32_t fov[2][2], int32_t values[2][2], uint8_t overlap, int32_t keyframes[][2], uint8_t numKeyframes, uint16_t repeats);
    bool handle();
    bool isBusy();
};

#endif
