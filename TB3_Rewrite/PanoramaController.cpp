#include "PanoramaController.h"

PanoramaController::PanoramaController(CameraController* camera, MyMultiStepper* motionController, bool startLeft, bool startTop, bool rowFirst, Pattern pattern) {
  this->cam = camera;
  this->motion = motionController;
  this->startLeft = startLeft;
  this->startTop = startTop;
  this->rowFirst = rowFirst;
  this->pattern = pattern;
  this->currentState = Idle;
}


void PanoramaController::initialize(Mode mode, int32_t fov[2][2], int32_t values[2][2], uint8_t overlap, int32_t keyframes[][2], uint8_t numKeyframes, uint16_t repeats) {
  this->numKeyframes = numKeyframes;
  this->repeats = repeats;
  this->duration = duration;
  this->startTime = micros();
  this->nextActionTime = this->startTime;
  this->currentRepeat = 0;

  overlap = constrain(overlap, 1, 99);

  for (uint8_t i = 0; i < numKeyframes; i++) {
    this->keyframes[i][0] = keyframes[i][0];
    this->keyframes[i][1] = keyframes[i][1];
  }

  // Calculating the image dimensions based on FOV points
  imageDimensions[0] = abs(fov[0][0] - fov[1][0]); // Width
  imageDimensions[1] = abs(fov[0][1] - fov[1][1]); // Height

  uint32_t adjWidth = imageDimensions[0] * (100 - overlap) / 100;
  uint32_t adjHeight = imageDimensions[1] * (100 - overlap) / 100;

  if (mode == Mode::CenterDefined) {
    // Calculate the top left corner based on center and number of rows/columns
    int32_t centerX = values[0][0];
    int32_t centerY = values[0][1];
    numHorizontalPanels = abs(values[1][0]);
    numVerticalPanels = abs(values[1][1]);

    // Calculating the corners of the panorama
    coordinates[0][0] = centerX - (numHorizontalPanels * adjWidth / 2);  // Top left corner X
    coordinates[0][1] = centerY + (numVerticalPanels   * adjHeight / 2); // Top left corner Y
    coordinates[1][0] = centerX + (numHorizontalPanels * adjWidth / 2);  // Bottom right corner X
    coordinates[1][1] = centerY - (numVerticalPanels   * adjHeight / 2); // Bottom right corner Y
  } else { // CornerDefined
    // The corners of the panorama are directly provided
    coordinates[0][0] = values[0][0]; // Top left corner X
    coordinates[0][1] = values[0][1]; // Top left corner Y
    coordinates[1][0] = values[1][0]; // Bottom right corner X
    coordinates[1][1] = values[1][1]; // Bottom right corner Y

    // Calculate the number of panels
    numHorizontalPanels = (abs(coordinates[0][0] - coordinates[1][0]) + adjWidth - 1) / adjWidth;
    numVerticalPanels = (abs(coordinates[0][1] - coordinates[1][1]) + adjHeight - 1) / adjHeight;
  }

  // Adjust the overlap if necessary
  adjWidth = (abs(coordinates[0][0] - coordinates[1][0]) + numHorizontalPanels - 1) / numHorizontalPanels;
  adjHeight = (abs(coordinates[0][1] - coordinates[1][1]) + numVerticalPanels - 1) / numVerticalPanels;
  overlap = 100 - max(((100 * adjWidth) / imageDimensions[0]), ((100 * adjHeight) / imageDimensions[1]));
}

bool PanoramaController::isBusy() {
    return currentState != Idle;
}

bool PanoramaController::handle() {
  uint32_t currentTime = micros();
  if (currentState == Idle) {
    if (currentRepeat < repeats && currentTime - nextActionTime >= duration) {
      currentRepeat++;
      planNextMove();
      nextActionTime = currentTime + duration * 1000;
      return true;
    } else {
      return false;
    }
  } else if (currentState == Moving) {
    if (motion->handle()) {
      captureImage();
    }
  } else if (currentState == Shooting) {
    if (cam->handle()) {
      totalImagesShot++;
      if (totalImagesShot >= numHorizontalPanels * numVerticalPanels) {
        currentState = Idle;
        return false;
      } else {
        planNextMove();
      }
    }
  }
  return true;
}

bool PanoramaController::planNextMove() {
  uint32_t currentImage = totalImagesShot;
  uint16_t primaryIndex = currentImage / (rowFirst ? numHorizontalPanels : numVerticalPanels);
  uint16_t secondaryIndex = currentImage % (rowFirst ? numHorizontalPanels : numVerticalPanels);

  if (pattern == Pattern::Serpentine) {
    if (primaryIndex % 2 == 1) {
      secondaryIndex = (rowFirst ? numHorizontalPanels : numVerticalPanels) - 1 - secondaryIndex;
    }
  }

  uint16_t row = rowFirst ? primaryIndex : secondaryIndex;
  uint16_t column = rowFirst ? secondaryIndex : primaryIndex;

  if (!startTop) {
    row = numVerticalPanels - 1 - row;
  }
  if (!startLeft) {
    column = numHorizontalPanels - 1 - column;
  }

  int32_t* keyframeOffset = calculateKeyframeOffset();
  currentTarget[0] = coordinates[0][0] + column * imageDimensions[0] + keyframeOffset[0];
  currentTarget[1] = coordinates[0][1] - row * imageDimensions[1] + keyframeOffset[1];
  motion->moveTo(currentTarget);
  currentState = Moving;
  return true;
}

int32_t* PanoramaController::calculateKeyframeOffset() {
  static int32_t offset[2] = {0, 0};

  if (numKeyframes > 1) {
    // Determine which two keyframes to interpolate
    uint16_t segment = totalImagesShot / repeats;
    uint16_t progress = totalImagesShot % repeats;
    float alpha = float(progress) / repeats;

    // Linear interpolation between the two keyframes
    offset[0] = int32_t(keyframes[segment][0] * (1 - alpha) + keyframes[segment + 1][0] * alpha);
    offset[1] = int32_t(keyframes[segment][1] * (1 - alpha) + keyframes[segment + 1][1] * alpha);
  }

  return offset;
}

void PanoramaController::reset() {
  totalImagesShot = 0;
  currentState = Idle;
}

void PanoramaController::captureImage() {
  cam->shoot();
  currentState = Shooting;
}
