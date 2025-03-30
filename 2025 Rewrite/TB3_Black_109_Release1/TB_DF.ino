/*
  DFMoco version 1.2.6
  
  Multi-axis motion control.
  For use with the Arc motion control system in Dragonframe 3.
  Generates step and direction signals, which can be sent to stepper motor drivers.
   
  Control up to four axes with an Uno or Duemilanove board.
  Control up to eight axes with a Mega or Mega 2560, chipKit MAX32 or LeafLabs Maple.

  Version History
  
  Version 1.2.6 Add PINOUT_VERSION option to use older pinout.
  Version 1.2.5 Fix jogging with low pulse rate.
  Version 1.2.4 Fix pin assignments
  Version 1.2.3 New Position command
  Version 1.2.2 Jog and Inch commands
  Version 1.2.1 Moved step/direction pins for motions 5-8.
                Detects board type automatically.
  Version 1.2.0 Basic go-motion capabilities
  Version 1.1.2 Smooth transitions when changing direction
  Version 1.1.1 Save/restore motor position
  Version 1.1.0 Major rework 
  Version 1.0.2 Moved pulses into interrupt handler
  Version 1.0.1 Added delay for pulse widths  
  Version 1.0.0 Initial public release.

  Pin configuration for TB3 Orange:
  
  #define MOTOR_EN     17
  #define MOTOR_EN_2   11
  
  #define MS1           15
  #define MS2_3         16  
  
  Pin configuration for TB3 Black:
      
  #define MOTOR_EN     A3
  #define MOTOR_EN_2   11 
  
  #define MS1           A1
  #define MS2           A2 
  #define MS3           A2   
      
 */

char txBuf[32];
char *txBufPtr;

/**
 * Serial output specialization
 */
#if defined(UBRRH)
#define TX_UCSRA UCSRA
#define TX_UDRE UDRE
#define TX_UDR UDR
#else
#define TX_UCSRA UCSR0A
#define TX_UDRE UDRE0
#define TX_UDR UDR0
#endif

#define TX_MSG_BUF_SIZE 16

enum class MessageState : uint8_t {
    MSG_STATE_START   = 0,
    MSG_STATE_CMD   = 1,
    MSG_STATE_DATA   = 2,
    MSG_STATE_ERR   = 3,
    MSG_STATE_DONE   = 100,
};

/*
 * Command codes from user
 */
#define USER_CMD_ARGS 40

enum class Command : uint8_t {
    CMD_NONE = 0,
    CMD_HI   = 10,   // initiate communication
    CMD_MS   = 30,   // requests the motors state
    CMD_NP   = 31,   // sets the motor's current position to a new value (no movement)
    CMD_MM   = 40,   // move motor
    CMD_PR   = 41,   // pulse rate
    CMD_SM   = 42,   // stop motor
    CMD_MP   = 43,   // motor position
    CMD_ZM   = 44,   // zero motor
    CMD_SA   = 50,   // stop all (hard)
    CMD_BF   = 60,   // blur frame
    CMD_GO   = 61,   // go!
    CMD_JM   = 70,   // jog motor
    CMD_IM   = 71,   // inch motor
};

enum Command processUserMessage(const char data);

enum class Message : uint8_t {
    MSG_HI, // initiate communication
    MSG_MM, // check moving status of all motors
    MSG_MP, // request motor position
    MSG_MS, // requests the motors state
    MSG_PR, // set the maximum steps/second of motor
    MSG_SM, // stop a motor fairly quickly
    MSG_SA, // stop all motors fairly quickly
    MSG_BF, // blur frame
    MSG_GO, // go!
    MSG_JM, // move the motor at a reasonable speed (pulse rate) towards destination
    MSG_IM, // move the in very small increments towards destination
};

void sendMessage(const enum Message msg, const uint8_t motorIndex);


struct UserCmd {
  Command command;
  byte argCount;
  int32_t args[USER_CMD_ARGS];
};

/*
 * Message state machine variables.
 */
byte lastUserData;
MessageState msgState;
int msgNumberSign;
UserCmd userCmd;


struct txMsg {
  byte msg;
  byte motor;
};


#ifdef KILL_SWITCH_INTERRUPT
void killSwitch() {
  hardStop();
}
#endif




void DFSetup() {
  goMoReady = false;
  lastUserData = 0;
  msgState = MessageState::MSG_STATE_START;
  velocityUpdateCounter = 0;
  sendPositionCounter = 10;
  nextMoveLoaded = false;
  motorMoving = 0;  //eMotimo added -
  
  for (char& index : txBuf) {
    index = 0;
  }

  txBufPtr = txBuf;

#ifdef KILL_SWITCH_INTERRUPT
  attachInterrupt(KILL_SWITCH_INTERRUPT, killSwitch, CHANGE);
#endif

  // initialize motor structures
  setPulsesPerSecond(config.MAX_JOG_STEPS_PER_SEC);
  for (uint8_t i = 0; i < MOTORS; i++) {
    Motor *motor = &motors[i];
// setup motor pins - you can customize/modify these after loop
// default sets step/dir pairs together, with first four motors at 4/5, 6/7, 8/9, 10/11
// then, for the Mega boards, it jumps to 28/29, 30/31, 32/33, 34/35

    motor->dir = true;  // forward
    motor->position = 0L;
    motor->destination = 0L;

    motor->nextMotorMoveSteps = 0;
    motor->nextMotorMoveSpeed = 0;

    //Hardcode Max Jog Speeds to start
    motor->jogMaxVelocity = config.MAX_JOG_STEPS_PER_SEC[i];
    motor->jogMaxAcceleration = config.MAX_JOG_STEPS_PER_SEC[i] / 2;

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

    // disable PWM
    switch (motorStepPins[i]) {
#if defined(TCCR3A) && defined(COM3B1)
      case 4:
        TCCR3A &= static_cast<uint8_t>(~COM3B1);
        break;
#endif

#if defined(TCCR4A) && defined(COM4A1)
      case 6:
        TCCR4A &= static_cast<uint8_t>(~COM4A1);
        break;
#endif

#if defined(TCCR4A) && defined(COM4C1)
      case 8:
        TCCR4A &= static_cast<uint8_t>(~COM4C1);
        break;
#endif

#if defined(TCCR2A) && defined(COM2A1)
      case 10:
        TCCR2A &= static_cast<uint8_t>(~COM2A1);
        break;
#endif
    }

#else

    switch (motorStepPins[i]) {
#if defined(TCCR1A) && defined(COM1B1)
      case 10:
        TCCR1A &= static_cast<uint8_t>(~COM1B1);
        break;
#endif
    }

#endif
  }

  // set initial direction
  for (uint8_t i = 0; i < MOTORS; i++) {
    PIN_ON(MOTOR_DIR_PORT[i], MOTOR_DIR_PIN[i]);  // Forward
  }


  //Setup TB3 Specific Parameters

  //Setup Microstepping
  /*  
pinMode(MS1,OUTPUT); //MS1
pinMode(MS2,OUTPUT); //MS2 for Motor 2
pinMode(MS3,OUTPUT); //MS3 for Motor 2
 
digitalWrite(MS1, HIGH); //set to microstep 16
digitalWrite(MS2, HIGH); //set to microstep 16
digitalWrite(MS3, HIGH); //set to microstep 16

//Turn off LED's  
pinMode(CAMERA_PIN,OUTPUT);
CameraShutter(LOW);
pinMode(FOCUS_PIN,OUTPUT);
CameraFocus(LOW);
  
//Enable the Motors
pinMode(MOTOR_EN,OUTPUT);
digitalWrite(MOTOR_EN, LOW); //Enable the motors 0 and 1
pinMode(MOTOR_EN2,OUTPUT);
digitalWrite(MOTOR_EN2, LOW); //Enable the motors 2 

*/
  //comment variable redefinition out as this is setup on start.

  //turn on the motors
  UpdatePowersave(Powersave::Shoot_Minimum); // Need to Move

// setup serial connection
#if (BOARD == ARDUINO) || (BOARD == ARDUINOMEGA) || (BOARD == CHIPKITMAX32)
  Serial.begin(57600);
#endif

  sendMessage(Message::MSG_HI, 0);

  // SET UP interrupt timer

#if (BOARD == ARDUINO) || (BOARD == ARDUINOMEGA)

  TCCR1A = 0;
  TCCR1B = _BV(WGM13);

  ICR1 = (F_CPU / 4000000) * TIME_CHUNK;  // goes twice as often as time chunk, but every other event turns off pins
  TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
  TIMSK1 = _BV(TOIE1);
  TCCR1B |= _BV(CS10);


#endif
}


ISR(TIMER1_OVF_vect)  //timer interrupt
{
  static bool toggleStep = 0;
  toggleStep = !toggleStep;

  if (toggleStep) {
    // Handle stepping for each motor
    for (uint8_t i = 0; i < MOTORS; i++) {
      Motor *motor = &motors[i];
      if (motor->motorMoveSteps) {
        uint16_t prevAccumulator = motor->motorAccumulator;
        motor->motorAccumulator += motor->motorMoveSpeed;
        if (motor->motorAccumulator < prevAccumulator) {
          (motor->motorMoveSteps)--;

          // Activate the step pin
          PIN_ON(MOTOR_STEP_PORT[i], MOTOR_STEP_PIN[i]);
        
          // Update the current position based on direction
          if (motor->dir) { motor->position++; }
          else            { motor->position--; }
        }
      }
    }
  }
  else {
    velocityUpdateCounter++;
    if (velocityUpdateCounter == VELOCITY_UPDATE_RATE) {
      velocityUpdateCounter = 0;

      if (sendPositionCounter) { sendPositionCounter--; }

      for (uint8_t i = 0; i < MOTORS; i++) {
        Motor *motor = &motors[i];
        if (motor->motorMoveSpeed && motor->nextMotorMoveSpeed == 0) {
          sendPosition = ubitSet(sendPosition, i);
        }

        motor->motorMoveSteps = motor->nextMotorMoveSteps;
        motor->motorMoveSpeed = motor->nextMotorMoveSpeed;
        digitalWrite(motorDirPins[i], motor->dir);
        motor->motorAccumulator = 65535;
      }
      nextMoveLoaded = false;  // ready for new move
    }
    for (uint8_t i = 0; i < MOTORS; i++) {
      PIN_OFF(MOTOR_STEP_PORT[i], MOTOR_STEP_PIN[i]);
    }
  }
}

/*
 * For stepper-motor timing, every clock cycle counts.
 */



void DFloop() {
  while (true) {
    if (!nextMoveLoaded)
      updateMotorVelocities();

    processSerialCommand();
    if (sendPositionCounter == 0) {
      sendPositionCounter = 20;

      for (uint8_t i = 0; i < MOTORS; i++) {
        if (bitRead(motorMoving, i) || bitRead(sendPosition, i)) {
          sendMessage(Message::MSG_MP, i);
        }
      }
      sendPosition = 0;
    }
  }
}  //end of loop


/**
 * Update velocities.
 */

void updateMotorVelocities() {
  for (uint8_t m = 0; m < MOTORS; m++) {
    Motor *motor = &motors[m];
    motor->nextMotorMoveSteps = 0;
    motor->nextMotorMoveSpeed = 0;

    if (bitRead(motorMoving, m)) {
      uint8_t seg = motor->currentMove;

      if (motor->moveTime[seg] == 0) {
        motor->nextMotorMoveSpeed = 0;
        motorMoving = ubitClear(motorMoving, m);
      }
      else {
        motor->currentMoveTime += (1.0f / TIME_CHUNK);   // 20 updates per second, or 20 Hz

        if (motor->currentMoveTime >= motor->moveTime[seg]) {
          motor->currentMoveTime -= motor->moveTime[seg];
          motor->currentMove++;
          seg++;
        }
        float t = motor->currentMoveTime;
        float nextPosition = static_cast<float>(motor->movePosition[seg]) + motor->moveVelocity[seg] * t + motor->moveAcceleration[seg] * t * t;  // accel was already multiplied * 0.5
        lcd.empty();
        lcd.at(1,1,int32_t(nextPosition));
        lcd.at(2,1,motor->movePosition[seg]);
        uint32_t stepsToNextPosition = static_cast<uint32_t>(fabs(nextPosition - static_cast<float>(motor->position)) * (65635.0f / 1000.0f));

        motor->nextMotorMoveSpeed = static_cast<uint16_t>(constrain(stepsToNextPosition, 1, motor->jogMaxVelocity));
        motor->nextMotorMoveSteps = stepsToNextPosition;



        bool forward = static_cast<int32_t>(nextPosition) > motor->position;
        motor->dir = forward;
        //motor->position = static_cast<int32_t>(nextPosition);
      }
    }  //end if motor moving

  }  //end motor count routine
  nextMoveLoaded = true;
}

/*
 * Set up the axis for pulses per second (approximate)
 */
void setPulsesPerSecond(const uint16_t pulsesPerSecond[3]) {
  for (uint8_t m = 0; m < MOTORS; m++) {
    Motor *motor = &motors[m];
    uint16_t intpulsesPerSecond = constrain(pulsesPerSecond[m], 100, config.MAX_JOG_STEPS_PER_SEC[m]);
    motor->jogMaxVelocity = intpulsesPerSecond;
    motor->jogMaxAcceleration = intpulsesPerSecond / 2;
  }
}

void setupMotorMove(const int32_t destination[3]) {
  for (uint8_t m = 0; m < MOTORS; m++) {
    Motor *motor = &motors[m];
    motor->destination = destination[m];
    if (destination[m] != motor->position) {
      calculatePointToPoint(m, destination[m]);
      motorMoving = ubitSet(motorMoving, m);
    }
  }
}

void setupMotorMove(uint8_t motor_index, const int32_t destination) {
  motors[motor_index].destination = destination;
  if (destination != motors[motor_index].position) {
    calculatePointToPoint(motor_index, destination);
    motorMoving = ubitSet(motorMoving, motor_index);
  }
}


void hardStop() {
  // set the destination to the current location, so they won't move any more
  for (uint8_t i = 0; i < MOTORS; i++) {
    stopMotor(i);
  }
}

void stopMotor(const uint8_t motorIndex) {
  Motor *motor = &motors[motorIndex];
  if (motor->destination == motor->position)
    return;

  for (uint8_t i = 0; i < P2P_MOVE_COUNT; i++) {
    motor->moveTime[i] = 0;
    motor->moveVelocity[i] = 0;
    motor->movePosition[i] = 0;
  }

float v = float(1000000 * motor->nextMotorMoveSpeed) / float(65535 * TIME_CHUNK);

  float maxA = static_cast<float>(motor->jogMaxAcceleration);
  float maxV = static_cast<float>(motor->jogMaxVelocity);

  if (v > maxV)
    v = maxV;

  if (motor->dir == 0)
    v = -v;

  float t = fabs(v / maxA);

  motor->moveTime[0] = t;
  motor->movePosition[0] = motor->position;
  motor->moveVelocity[0] = v;
  motor->moveAcceleration[0] = (v > 0) ? -maxA : maxA;

  motor->moveTime[1] = 0;
  motor->movePosition[1] = static_cast<int32_t>(static_cast<float>(motor->movePosition[0]) + motor->moveVelocity[0] * t + 0.5f * motor->moveAcceleration[0] * t * t);
  motor->moveVelocity[1] = 0;
  motor->moveAcceleration[1] = 0;

  motor->moveAcceleration[0] *= 0.5f;

  motor->destination = motor->movePosition[1];

  motor->currentMoveTime = 0;
  motor->currentMove = 0;
}

__attribute__ ((const))
__attribute__ ((warn_unused_result))
bool isValidMotor(const uint8_t motorIndex) {
  return (motorIndex < MOTORS);
}


void processGoPosition(const uint8_t motorIndex, const int32_t pos) {
  Motor *motor = &motors[motorIndex];
  if (motor->position != pos) {
    setupMotorMove(motorIndex, pos);
    sendMessage(Message::MSG_MM, motorIndex);
  }
  else {
    sendMessage(Message::MSG_MP, motorIndex);
  }
}

/*

Command format

ASCII
[command two bytes]

Version
"hi"
-> "hi 1"

zero motor
"zm 1"
-> "z 1"

move motor
"mm 1 +1111111111

motor position?
mp 1

MOTOR STATUS
"ms"
-> "ms [busy motor count]"

SET PULSE PER SECOND
pr 1 200

STOP MOTOR
sm 1

STOP ALL
sa

*/

/*
 * int processUserMessage(char data)
 *
 * Read user data (from virtual com port), processing one byte at a time.
 * Implemented with a state machine to reduce memory overhead.
 *
 * Returns command code for completed command.
 */
enum Command processUserMessage(const char data) {
  Command cmd = Command::CMD_NONE;

  switch (msgState) {
    case MessageState::MSG_STATE_START:
      if (data != '\r' && data != '\n') {
        msgState = MessageState::MSG_STATE_CMD;
        msgNumberSign = 1;
        userCmd.command = Command::CMD_NONE;
        userCmd.argCount = 0;
        userCmd.args[0] = 0;
      }
      break;

    case MessageState::MSG_STATE_CMD:
      if (lastUserData == 'h' && data == 'i') {
        userCmd.command = Command::CMD_HI;
        msgState = MessageState::MSG_STATE_DONE;
      }
      else if (lastUserData == 'm' && data == 's') {
        userCmd.command = Command::CMD_MS;
        msgState = MessageState::MSG_STATE_DONE;
      }
      else if (lastUserData == 's' && data == 'a') {
        userCmd.command = Command::CMD_SA;
        msgState = MessageState::MSG_STATE_DONE;
      }
      else if (lastUserData == 'm' && data == 'm') {
        userCmd.command = Command::CMD_MM;
        msgState = MessageState::MSG_STATE_DATA;
      }
      else if (lastUserData == 'n' && data == 'p') {
        userCmd.command = Command::CMD_NP;
        msgState = MessageState::MSG_STATE_DATA;
      }
      else if (lastUserData == 'm' && data == 'p') {
        userCmd.command = Command::CMD_MP;
        msgState = MessageState::MSG_STATE_DATA;
      }
      else if (lastUserData == 'z' && data == 'm') {
        userCmd.command = Command::CMD_ZM;
        msgState = MessageState::MSG_STATE_DATA;
      } 
      else if (lastUserData == 's' && data == 'm') {
        userCmd.command = Command::CMD_SM;
        msgState = MessageState::MSG_STATE_DATA;
      } 
      else if (lastUserData == 'p' && data == 'r') {
        userCmd.command = Command::CMD_PR;
        msgState = MessageState::MSG_STATE_DATA;
      } 
      else if (lastUserData == 'b' && data == 'f') {
        userCmd.command = Command::CMD_BF;
        msgState = MessageState::MSG_STATE_DATA;
      }
      else if (lastUserData == 'g' && data == 'o') {
        userCmd.command = Command::CMD_GO;
        msgState = MessageState::MSG_STATE_DONE;
      }
      else if (lastUserData == 'j' && data == 'm') { // jm [motor] [%speed]
        userCmd.command = Command::CMD_JM;
        msgState = MessageState::MSG_STATE_DATA;
      }
      else if (lastUserData == 'i' && data == 'm') { // im [motor] [%speed]
        userCmd.command = Command::CMD_IM;
        msgState = MessageState::MSG_STATE_DATA;
      }
      else {
        // error msg? unknown command?
        msgState = MessageState::MSG_STATE_START;
      }
      break;

    case MessageState::MSG_STATE_DATA:
      if (((data >= '0' && data <= '9') || data == '-') && lastUserData == ' ') {
        userCmd.argCount++;
        if (userCmd.argCount >= USER_CMD_ARGS) {
          Serial.print(F_STR("error: too many args\r\n"));
          msgState = MessageState::MSG_STATE_ERR;
        }
        else {
          userCmd.args[userCmd.argCount - 1] = 0;
          if (data == '-') {
            msgNumberSign = -1;
          }
          else {
            msgNumberSign = 1;
            userCmd.args[userCmd.argCount - 1] = (data - '0');
          }
        }
      }
      else if (data >= '0' && data <= '9') {
        userCmd.args[userCmd.argCount - 1] = userCmd.args[userCmd.argCount - 1] * 10 + (data - '0');
      }
      else if (data == ' ' || data == '\r') {
        if (lastUserData >= '0' && lastUserData <= '9') {
          if (userCmd.argCount > 0)
            userCmd.args[userCmd.argCount - 1] *= msgNumberSign;
        }
        if (data == '\r') {
          msgState = MessageState::MSG_STATE_DONE;
        }
      }
      break;


    case MessageState::MSG_STATE_ERR:
      userCmd.command = Command::CMD_NONE;
      msgState = MessageState::MSG_STATE_DONE;
      break;

    case MessageState::MSG_STATE_DONE:
      // wait for newline, then reset
      if (data == '\n' && lastUserData == '\r') {
        cmd = userCmd.command;
        msgState = MessageState::MSG_STATE_START;
        lastUserData = 0;
      }
      break;

    default:  // unknown state -> revert to begin
      msgState = MessageState::MSG_STATE_START;
      lastUserData = 0;
  }

  lastUserData = data;

  return cmd;
}

void processSerialCommand() {
  byte avail = Serial.available();
  byte motor;

  for (uint8_t i = 0; i < avail; i++) {
    Command cmd = processUserMessage(Serial.read());

    if (cmd != Command::CMD_NONE) {
      bool parseError = false;

      motor = userCmd.args[0] - 1;

      switch (cmd) {
        case Command::CMD_HI:
          sendMessage(Message::MSG_HI, 0);
          break;

        case Command::CMD_ZM:
          parseError = (userCmd.argCount != 1 || !isValidMotor(motor));
          if (!parseError) {
            motors[motor].position = 0;
            setupMotorMove(motor, 0);
            processGoPosition(motor, 0);
            sendPosition = ubitSet(sendPosition, motor);
          }
          break;

        case Command::CMD_MM:
          parseError = (userCmd.argCount != 2 || !isValidMotor(motor));
          if (!parseError) {
            processGoPosition(motor, userCmd.args[1]);
          }
          break;

        case Command::CMD_NP:
          parseError = (userCmd.argCount != 2 || !isValidMotor(motor));
          if (!parseError) {
            motors[motor].position = userCmd.args[1];
            sendMessage(Message::MSG_MP, motor);
          }
          break;


        case Command::CMD_MP:
          parseError = (userCmd.argCount != 1 || !isValidMotor(motor));
          if (!parseError) {
            sendMessage(Message::MSG_MP, motor);
          }
          break;

        case Command::CMD_MS:
          parseError = (userCmd.argCount != 0);
          if (!parseError) {
            sendMessage(Message::MSG_MS, 0);
          }
          break;

        case Command::CMD_SM:
          parseError = (userCmd.argCount != 1 || !isValidMotor(motor));
          if (!parseError) {
            stopMotor(motor);
            sendMessage(Message::MSG_SM, motor);
            sendMessage(Message::MSG_MP, motor);
          }
          break;

        case Command::CMD_SA:
          parseError = (userCmd.argCount != 0);
          if (!parseError) {
            hardStop();
            sendMessage(Message::MSG_SA, 0);
          }
          break;

        case Command::CMD_PR:
          parseError = (userCmd.argCount != 2 || !isValidMotor(motor));
          if (!parseError) {
            config.MAX_JOG_STEPS_PER_SEC[motor] = static_cast<uint16_t>(userCmd.args[1]);
            setPulsesPerSecond(config.MAX_JOG_STEPS_PER_SEC);
            sendMessage(Message::MSG_PR, motor);
          }
          break;

        case Command::CMD_BF:
          parseError = motorMoving || userCmd.argCount < 5 || ((userCmd.argCount - 2) % 4) != 0;
          if (!parseError) {
            goMoDelayTime = 500;

            int motorCount = (userCmd.argCount - 2) / 4;

            for (uint8_t m = 0; m < MOTORS; m++) {
              motors[m].gomoMoveTime[0] = 0.0f;
            }

            for (uint8_t m = 0; m < motorCount; m++) {
              int offset = 2 + m * 4;
              motor = userCmd.args[offset] - 1;
              if (!isValidMotor(motor)) {
                parseError = true;
                break;
              }
              setupBlur(motor, userCmd.args[0], userCmd.args[1], userCmd.args[offset + 1], userCmd.args[offset + 2], userCmd.args[offset + 3]);
            }
            goMoReady = true;
            sendMessage(Message::MSG_BF, 0);
          }
          break;

        case Command::CMD_GO:
          parseError = motorMoving || (userCmd.argCount > 0) || !goMoReady;
          if (!parseError) {
            for (uint8_t m = 0; m < MOTORS; m++) {
              Motor *motorx = &motors[m];
              if (motorx->gomoMoveTime[0] != 0) {
                for (uint8_t j = 0; j < P2P_MOVE_COUNT; j++) {
                  motorx->moveTime[j] = motorx->gomoMoveTime[j];
                  motorx->movePosition[j] = motorx->gomoMovePosition[j];
                  motorx->moveVelocity[j] = motorx->gomoMoveVelocity[j];
                  motorx->moveAcceleration[j] = motorx->gomoMoveAcceleration[j];
                }
                motorx->destination = motorx->gomoMovePosition[4];  // TODO change this!
                motorx->currentMove = 0;
                motorMoving = ubitSet(motorMoving, m);
              }
            }
            updateMotorVelocities();
            noInterrupts();
            velocityUpdateCounter = VELOCITY_UPDATE_RATE - 1;
            interrupts();
          }
          break;

        case Command::CMD_JM:
          parseError = (userCmd.argCount != 2 || !isValidMotor(motor));
          if (!parseError) {
            int32_t destination = 0;
            if (jogMotor(motor, userCmd.args[1], &destination)) {
              if (!bitRead(motorMoving, motor) || destination != motors[motor].destination) {
                setupMotorMove(motor, destination);
              }
            }
            sendMessage(Message::MSG_JM, motor);
          }
          break;

        case Command::CMD_IM:
          parseError = (userCmd.argCount != 2 || !isValidMotor(motor));
          if (!parseError) {
            inchMotor(motor, userCmd.args[1]);
            sendMessage(Message::MSG_IM, motor);
          }
          break;

        default:
          parseError = true;
          break;
      }

      if (parseError) {
        Serial.print(F_STR("parse error\r\n"));
      }
    }
  }
}


/*
 *
 * Serial transmission.
 *
 */
void sendMessage(const enum Message msg, const uint8_t motorIndex) {
  switch (msg) {
    case Message::MSG_HI:
      Serial.print(F_STR("hi "));
      Serial.print(DFMOCO_VERSION);
      Serial.print(' ');
      Serial.print(MOTORS);
      Serial.print(' ');
      Serial.print(DFMOCO_VERSION_STRING);
      Serial.print(F_STR("\r\n"));
      break;
    case Message::MSG_MM:
      Serial.print(F_STR("mm "));
      Serial.print(motorIndex + 1);
      Serial.print(' ');
      Serial.print(motors[motorIndex].destination);
      Serial.print(F_STR("\r\n"));
      break;
    case Message::MSG_MP:
      Serial.print(F_STR("mp "));
      Serial.print(motorIndex + 1);
      Serial.print(' ');
      Serial.print(motors[motorIndex].position);
      Serial.print(F_STR("\r\n"));
      break;
    case Message::MSG_MS:
      Serial.print(F_STR("ms "));
      for (uint8_t i = 0; i < MOTORS; i++)
        Serial.print(bitRead(motorMoving, i) ? '1' : '0');
      Serial.print(F_STR("\r\n"));
      break;
    case Message::MSG_PR:
      Serial.print(F_STR("pr "));
      Serial.print(motorIndex + 1);
      Serial.print(' ');
      Serial.print(motors[motorIndex].jogMaxVelocity);
      Serial.print(F_STR("\r\n"));
      break;
    case Message::MSG_SM:
      Serial.print(F_STR("sm "));
      Serial.print(motorIndex + 1);
      Serial.print(F_STR("\r\n"));
      break;
    case Message::MSG_SA:
      Serial.print(F_STR("sa\r\n"));
      break;
    case Message::MSG_BF:
      Serial.print(F_STR("bf "));
      Serial.print(goMoDelayTime);
      Serial.print(F_STR("\r\n"));
      break;
    case Message::MSG_JM:
      Serial.print(F_STR("jm "));
      Serial.print(motorIndex + 1);
      Serial.print(F_STR("\r\n"));
      break;
    case Message::MSG_IM:
      Serial.print(F_STR("im "));
      Serial.print(motorIndex + 1);
      Serial.print(F_STR("\r\n"));
      break;
    case Message::MSG_GO: // Unimplemented
      break;
  }
}


__attribute__ ((warn_unused_result))
bool jogMotor(const uint8_t motorIndex, const int32_t target, int32_t *destination) {
    Motor *motor = &motors[motorIndex];
  // ideally send motor to distance where decel happens after 2 seconds
  float vi = (motor->dir ? 1 : -1) * (static_cast<float>(1000000 * motor->nextMotorMoveSpeed) / static_cast<float>(TIME_CHUNK * 65535));

  

  int8_t dir = (target > motor->position) ? 1 : -1;
  // if switching direction, just stop
  if (motor->nextMotorMoveSpeed && motor->dir * dir < 0) {
    stopMotor(motorIndex);
    return false;
  }
  if (target == motor->position) {
    return false;
  }

  float maxVelocity = static_cast<float>(motor->jogMaxVelocity);
  float maxAcceleration = static_cast<float>(motor->jogMaxAcceleration);


  // given current velocity vi
  // compute distance so that decel starts after 0.5 seconds
  // time to accel
  // time at maxvelocity
  // time to decel
  float accelTime = 0, atMaxVelocityTime = 0;
  if (fabs(vi) < maxVelocity) {
    accelTime = (maxVelocity - fabs(vi)) / maxAcceleration;
    if (accelTime < 0.5f) {
      atMaxVelocityTime = 0.5f - accelTime;
    } else {
      accelTime = 0.5f;
    }
  } else {
    atMaxVelocityTime = 0.5f;
  }
  float maxVelocityReached = fabs(vi) + maxAcceleration * accelTime;

  float delta = fabs(vi) * accelTime + (0.5f * maxAcceleration * accelTime * accelTime);
  delta += atMaxVelocityTime * maxVelocityReached;
  delta += 0.5f * (maxVelocityReached * maxVelocityReached) / maxAcceleration;  // = 0.5 * a * t^2 -> t = (v/a)

  int32_t dest = motor->position + dir * static_cast<int32_t>(delta);

  // now clamp to target
  if ((dir == 1 && dest > target) || (dir == -1 && dest < target)) {
    dest = target;
  }
  *destination = dest;
  return true;
}

void inchMotor(const uint8_t motorIndex, const int32_t target) {
  Motor *motor = &motors[motorIndex];
  // ideally send motor to distance where decel happens after 2 seconds

  // if switching direction, just stop
  int dir = (target > motor->destination) ? 1 : -1;

  if (motor->nextMotorMoveSpeed)  // && motor->dir * dir < 0)
  {
    stopMotor(motorIndex);
    return;
  }

  int32_t dest = motor->destination + dir * 2;

  // now clamp to target
  if ((dir == 1 && dest > target) || (dir == -1 && dest < target)) {
    dest = target;
  }
  //setupMotorMove(motorIndex, dest);

  for (uint8_t i = 0; i < P2P_MOVE_COUNT; i++) {
    motor->moveTime[i] = 0;
    motor->moveVelocity[i] = 0;
    motor->moveAcceleration[i] = 0;
  }
  motor->currentMoveTime = 0;
  motor->moveTime[0] = 0.01f;
  motor->movePosition[0] = motor->position;
  motor->movePosition[1] = motor->position + dir * 2;
  motor->currentMove = 0;

  motor->destination = dest;

  if (dest != motor->position) {
    motorMoving = ubitSet(motorMoving, motorIndex);
  }
}

void calculatePointToPoint(const uint8_t motorIndex, const int32_t destination) {
  Motor *motor = &motors[motorIndex];

  uint8_t moveCount = 0;

  for (uint8_t i = 0; i < P2P_MOVE_COUNT; i++) {
    motor->moveTime[i] = 0;
    motor->moveVelocity[i] = 0;
    motor->moveAcceleration[i] = 0;
  }
  motor->currentMoveTime = 0;
  motor->movePosition[0] = motor->position;

  float tmax = static_cast<float>(motor->jogMaxVelocity) / static_cast<float>(motor->jogMaxAcceleration);
  int32_t dmax = static_cast<int32_t>(static_cast<float>(motor->jogMaxVelocity) * tmax);

  int32_t dist = uabs(-destination - motor->position);
  int16_t dir = -destination > motor->position ? 1 : -1;

  if (motor->nextMotorMoveSpeed > 5)  // we need to account for existing velocity
  {
    float vi = (motor->dir ? 1 : -1) * (static_cast<float>(1000000 * motor->nextMotorMoveSpeed) / static_cast<float>(TIME_CHUNK * 65535));
    vi = constrain(vi, -motor->jogMaxVelocity, motor->jogMaxVelocity);
    float ti = fabs(vi / static_cast<float>(motor->jogMaxAcceleration));
    int32_t di = static_cast<int32_t>(0.5f * static_cast<float>(motor->jogMaxAcceleration) * ti * ti);

    if (vi * dir < 0)  // switching directions
    {
      motor->moveTime[moveCount] = ti;
      motor->moveAcceleration[moveCount] = dir * static_cast<float>(motor->jogMaxAcceleration);
      motor->moveVelocity[moveCount] = vi;
      moveCount++;

      dist += di;
    } else if (dist < di)  // must decelerate and switch directions
    {
      motor->moveTime[moveCount] = ti;
      motor->moveAcceleration[moveCount] = -dir * static_cast<float>(motor->jogMaxAcceleration);
      motor->moveVelocity[moveCount] = vi;
      moveCount++;

      dist = di - dist;
      dir = -dir;
    } else  // further on in same direction
    {
      dist += di;
      motor->movePosition[0] -= dir * di;
      motor->currentMoveTime = ti;
    }
  }

  float t = tmax;
  if (dist <= dmax) {
    t = sqrt(static_cast<float>(dist) / static_cast<float>(motor->jogMaxAcceleration));
  }

  motor->moveTime[moveCount] = t;
  motor->moveAcceleration[moveCount] = dir * static_cast<float>(motor->jogMaxAcceleration);

  if (dist > dmax) {
    moveCount++;
    dist -= dmax;
    float tconst = static_cast<float>(dist) / static_cast<float>(motor->jogMaxVelocity);
    motor->moveTime[moveCount] = tconst;
    motor->moveAcceleration[moveCount] = 0;
  }

  moveCount++;
  motor->moveTime[moveCount] = t;
  motor->moveAcceleration[moveCount] = dir * -static_cast<float>(motor->jogMaxAcceleration);


  for (uint8_t i = 1; i <= moveCount; i++) {
    t = motor->moveTime[i - 1];
    motor->movePosition[i] = static_cast<int32_t>(static_cast<float>(motor->movePosition[i - 1]) + motor->moveVelocity[i - 1] * t + 0.5f * motor->moveAcceleration[i - 1] * t * t);
    motor->moveVelocity[i] = motor->moveVelocity[i - 1] + motor->moveAcceleration[i - 1] * t;
  }
  motor->movePosition[moveCount + 1] = destination;
  for (uint8_t i = 0; i <= moveCount; i++) {
    motor->moveAcceleration[i] *= 0.5f;  // pre-multiply here for later position calculation
  }
  motor->currentMove = 0;

  return;
}

void setupBlur(const uint8_t motorIndex, const uint32_t exposure_ms, const int16_t blur_percent, const int32_t p0, const int32_t p1, const int32_t p2) {
    Motor *motor = &motors[motorIndex];

    // Convert exposure time and blur percentage to float
    const float blurFactor = static_cast<float>(blur_percent) / 1000.0f;
    const float exposureTime = static_cast<float>(exposure_ms) / 1000.0f;

    // Calculate blurred positions using integer math first
    const int32_t blurredP0 = p1 + static_cast<int32_t>(blurFactor * static_cast<float>(p0 - p1));
    const int32_t blurredP2 = p1 + static_cast<int32_t>(blurFactor * static_cast<float>(p2 - p1));

    // Initialize all move parameters to zero
    for (uint8_t i = 0; i < P2P_MOVE_COUNT; i++) {
        motor->gomoMoveTime[i] = 0;
        motor->gomoMoveVelocity[i] = 0;
        motor->gomoMoveAcceleration[i] = 0;
    }

    // Setup move positions and times
    motor->gomoMovePosition[1] = blurredP0;
    motor->gomoMoveTime[1] = exposureTime * 0.5f;
    motor->gomoMoveVelocity[1] = static_cast<float>(p1 - blurredP0) / (exposureTime * 0.5f);

    motor->gomoMovePosition[2] = p1;
    motor->gomoMoveTime[2] = exposureTime * 0.5f;
    motor->gomoMoveVelocity[2] = static_cast<float>(blurredP2 - p1) / (exposureTime * 0.5f);

    // Acceleration calculations (use consistent float operations for these)
    const float accelTime = 1.0f;
    const float acceleration1 = motor->gomoMoveVelocity[1] / accelTime;
    const float displacement1 = 0.5f * acceleration1 * accelTime * accelTime;
    const float startPosition = static_cast<float>(blurredP0) - displacement1;

    motor->gomoMovePosition[0] = static_cast<int32_t>(startPosition);
    motor->gomoMoveTime[0] = accelTime;
    motor->gomoMoveAcceleration[0] = acceleration1 * 0.5f; // Pre-multiplied

    const float acceleration2 = motor->gomoMoveVelocity[2] / accelTime;
    const float displacement2 = 0.5f * acceleration2 * accelTime * accelTime;
    const float finalPosition = static_cast<float>(blurredP2) + displacement2;

    motor->gomoMovePosition[3] = blurredP2;
    motor->gomoMoveTime[3] = accelTime;
    motor->gomoMoveVelocity[3] = motor->gomoMoveVelocity[2];
    motor->gomoMoveAcceleration[3] = -acceleration2 * 0.5f; // Pre-multiplied

    motor->gomoMovePosition[4] = static_cast<int32_t>(finalPosition);

    // Call to finalize motor setup
    setupMotorMove(motorIndex, static_cast<int32_t>(startPosition));
}


__attribute__ ((warn_unused_result))
float calculateVelocityMotor(const uint8_t motorIndex, const float local_time, const float local_ramp) {
  Motor *motor = &motors[motorIndex];
  float new_time = local_time;
  //set

  motor->moveMaxVelocity = static_cast<uint16_t>(static_cast<float>(uabs(motor->destination - motor->position)) / static_cast<float>(local_time * (1.0 - local_ramp)));
  //enforce the max velocity
  if (motor->moveMaxVelocity > motor->jogMaxVelocity)  // we need to figure out new times
  {
    new_time = static_cast<float>(uabs(motor->destination - motor->position)) / (static_cast<float>(motor->jogMaxVelocity) * (1.0 - local_ramp));
    motor->moveMaxVelocity = motor->jogMaxVelocity;
  }


  //we can do anything for acceleration
  motor->moveMaxAcceleration = static_cast<uint16_t>(static_cast<float>(motor->moveMaxVelocity) / (local_ramp * local_time));
  //return velocitytemp;
  if (DEBUG_MOTOR) {
    Serial.print(F_STR("moveMaxVelocity:"));
    Serial.println(motor->moveMaxVelocity);

    Serial.print(F_STR("moveMaxAcceleration:"));
    Serial.println(motor->moveMaxAcceleration);
  }
  return new_time;  //returns the time
}

void synched3PtMove_origin() {
  int32_t home[MOTORS] = { 0, 0, 0 };
  synched3PtMove_max(home);
}

void synched3PtMove_max(const int32_t targets[3])  //
{
  float MotorTotalMoveTime[3] = { 0.0, 0.0, 0.0 };
  float LongestMoveTime = MotorTotalMoveTime[0];
  uint8_t DominantMotor = 0;

  for (uint8_t mot = 0; mot < MOTORS; mot++) {
    Motor *motor = &motors[mot];
    motor->destination = targets[mot];
    calculatePointToPoint_jog(mot, targets[mot]);  //Calculate the fastest move times based on max speeds and accelerations, we will recalc later after figuring out dominant axix (limiting axis)

    for (uint8_t seg = 0; seg < MAX_MOVE_POINTS; seg++)  //segments
    {
      MotorTotalMoveTime[mot] += motor->moveTime[seg];
    }

    if (MotorTotalMoveTime[mot] > LongestMoveTime) {  //Determine dominant Axis  - start with Pan or motor 0
      LongestMoveTime = MotorTotalMoveTime[mot];
      DominantMotor = mot;
    }
  }

  //create branch to eiether jog back max speed, or use local time if it is longer..
  float quickest_ramp = motors[DominantMotor].moveTime[0] / LongestMoveTime;

  for (uint8_t mot = 0; mot < MOTORS; mot++) {
    Motor *motor = &motors[mot];
    (void) calculateVelocityMotor(mot, LongestMoveTime, quickest_ramp);  //this sets the velocity & accel for a movecalc
    calculatePointToPoint_move(mot);
    if (motor->destination != motor->position) motorMoving = ubitSet(motorMoving, mot);
    else motorMoving = ubitClear(motorMoving, mot);
  }

}

void synched3AxisMove_timed(const int32_t targets[3], const float local_time, const float local_ramp)  //
{
  for (uint8_t mot = 0; mot < MOTORS; mot++) {
    Motor *motor = &motors[mot];
    motor->destination = targets[mot];
    calculatePointToPoint_jog(mot, targets[mot]);  //Calculate the timing for the total move with max velocities
  }

  float limited_motor_move_time_max = 0.0;
  maxVelLimit = false;
  for (uint8_t mot = 0; mot < MOTORS; mot++) {
    Motor *motor = &motors[mot];
    float limited_motor_move_time = calculateVelocityMotor(mot, local_time, local_ramp);
    if (limited_motor_move_time > limited_motor_move_time_max) limited_motor_move_time_max = limited_motor_move_time;
    calculatePointToPoint_move(mot);
    if (motor->destination != motor->position) motorMoving = ubitSet(motorMoving, mot);
    else motorMoving = ubitClear(motorMoving, mot);
  }

  if (limited_motor_move_time_max > local_time) {  //run the routine again with new max time if the real calculated values hit limits.
    maxVelLimit = true;
    for (uint8_t mot = 0; mot < MOTORS; mot++) {
      Motor *motor = &motors[mot];
      (void) calculateVelocityMotor(mot, limited_motor_move_time_max, local_ramp);
      calculatePointToPoint_move(mot);
      if (motor->destination != motor->position) motorMoving = ubitSet(motorMoving, mot);
      else motorMoving = ubitClear(motorMoving, mot);
    }
  }
}


void calculatePointToPoint_move(const uint8_t motorIndex) {
  Motor *motor = &motors[motorIndex];
  uint8_t moveCount = 0;

  for (uint8_t i = 0; i < P2P_MOVE_COUNT; i++) {
    motor->moveTime[i] = 0;
    motor->moveVelocity[i] = 0;
    motor->moveAcceleration[i] = 0;
  }
  motor->currentMoveTime = 0;
  motor->movePosition[0] = motor->position;

  float tmax = static_cast<float>(motor->moveMaxVelocity) / static_cast<float>(motor->moveMaxAcceleration);
  int32_t dmax = static_cast<int32_t>(static_cast<float>(motor->moveMaxVelocity) * tmax);

  int32_t dist = uabs(motor->destination - motor->position);
  int dir = motor->destination > motor->position ? 1 : -1;

  /*
  if (motor->nextMotorMoveSpeed > 5) // we need to account for existing velocity
  {
    float vi = (motor->dir ? 1 : -1) * (static_cast<float>(1000000 * motor->nextMotorMoveSpeed) / static_cast<float>(TIME_CHUNK * 65535));
    float ti = fabs(vi / motor->moveMaxAcceleration);
    float di = 0.5f * motor->moveMaxAcceleration * ti * ti;
    
    if (vi * dir < 0) // switching directions
    {
      motor->moveTime[moveCount] = ti;
      motor->moveAcceleration[moveCount] = dir * motor->moveMaxAcceleration;
      motor->moveVelocity[moveCount] = vi;
      moveCount++;
      
      dist += di;
    }
    else if (dist < di) // must decelerate and switch directions
    {
      motor->moveTime[moveCount] = ti;
      motor->moveAcceleration[moveCount] = -dir * motor->moveMaxAcceleration;
      motor->moveVelocity[moveCount] = vi;
      moveCount++;

      dist = (di - dist);
      dir = -dir;
    }
    else // further on in same direction
    {
      dist += di;
      motor->movePosition[0] -= dir * di;

      motor->currentMoveTime = ti;
    }
  }
  */


  float t = tmax;
  if (dist <= dmax) {
    t = sqrt(static_cast<float>(dist) / static_cast<float>(motor->moveMaxAcceleration));
  }

  motor->moveTime[moveCount] = t;
  motor->moveAcceleration[moveCount] = dir * motor->moveMaxAcceleration;

  if (dist > dmax) {
    moveCount++;
    dist -= dmax;
    float tconst = static_cast<float>(dist) / static_cast<float>(motor->moveMaxVelocity);
    motor->moveTime[moveCount] = tconst;
    motor->moveAcceleration[moveCount] = 0;
  }

  moveCount++;
  motor->moveTime[moveCount] = t;
  motor->moveAcceleration[moveCount] = dir * -static_cast<float>(motor->moveMaxAcceleration);


  for (uint8_t i = 1; i <= moveCount; i++) {
    t = motor->moveTime[i - 1];
    motor->movePosition[i] = static_cast<int32_t>(static_cast<float>(motor->movePosition[i - 1]) + motor->moveVelocity[i - 1] * t + 0.5f * motor->moveAcceleration[i - 1] * t * t);
    motor->moveVelocity[i] = motor->moveVelocity[i - 1] + motor->moveAcceleration[i - 1] * t;
  }
  motor->movePosition[moveCount + 1] = motor->destination;
  for (uint8_t i = 0; i <= moveCount; i++) {
    motor->moveAcceleration[i] *= 0.5f;  // pre-multiply here for later position calculation
  }
  motor->currentMove = 0;

  return;
}

void calculatePointToPoint_jog(const uint8_t motorIndex, const int32_t destination) {
  Motor *motor = &motors[motorIndex];

  uint8_t moveCount = 0;

  for (uint8_t i = 0; i < P2P_MOVE_COUNT; i++) {
    motor->moveTime[i] = 0;
    motor->moveVelocity[i] = 0;
    motor->moveAcceleration[i] = 0;
  }
  motor->currentMoveTime = 0;
  motor->movePosition[0] = motor->position;

  float tmax = static_cast<float>(motor->jogMaxVelocity) / static_cast<float>(motor->jogMaxAcceleration);
  int32_t dmax = static_cast<int32_t>(static_cast<float>(motor->jogMaxVelocity) * tmax);

  int32_t dist = uabs(destination - motor->position);
  int dir = destination > motor->position ? 1 : -1;

  /* 
  if (motor->nextMotorMoveSpeed > 5) // we need to account for existing velocity
  {
    float vi = (motor->dir ? 1 : -1) * (static_cast<float>(1000000 * motor->nextMotorMoveSpeed) / static_cast<float>(TIME_CHUNK * 65535));
    float ti = fabs(vi / static_cast<float>(motor->jogMaxAcceleration));
    float di = 0.5f * static_cast<float>(motor->jogMaxAcceleration) * ti * ti;
    
    if (vi * dir < 0) // switching directions
    {
      motor->moveTime[moveCount] = ti;
      motor->moveAcceleration[moveCount] = dir * static_cast<float>(motor->jogMaxAcceleration);
      motor->moveVelocity[moveCount] = vi;
      moveCount++;
      
      dist += di;
    }
    else if (dist < di) // must decelerate and switch directions
    {
      motor->moveTime[moveCount] = ti;
      motor->moveAcceleration[moveCount] = -dir * static_cast<float>(motor->jogMaxAcceleration);
      motor->moveVelocity[moveCount] = vi;
      moveCount++;

      dist = (di - dist);
      dir = -dir;
    }
    else // further on in same direction
    {
      dist += di;
      motor->movePosition[0] -= dir * di;

      motor->currentMoveTime = ti;
    }
  }
  */


  float t = tmax;
  if (dist <= dmax) {
    t = sqrt(static_cast<float>(dist) / static_cast<float>(motor->jogMaxAcceleration));
  }

  motor->moveTime[moveCount] = t;
  motor->moveAcceleration[moveCount] = dir * static_cast<int16_t>(motor->jogMaxAcceleration);

  if (dist > dmax) {
    moveCount++;
    dist -= dmax;
    float tconst = static_cast<float>(dist) / static_cast<float>(motor->jogMaxVelocity);
    motor->moveTime[moveCount] = tconst;
    motor->moveAcceleration[moveCount] = 0;
  }

  moveCount++;
  motor->moveTime[moveCount] = t;
  motor->moveAcceleration[moveCount] = dir * -static_cast<int16_t>(motor->jogMaxAcceleration);


  for (uint8_t i = 1; i <= moveCount; i++) {
    t = motor->moveTime[i - 1];
    motor->movePosition[i] = static_cast<int32_t>(static_cast<float>(motor->movePosition[i - 1]) + motor->moveVelocity[i - 1] * t + 0.5f * motor->moveAcceleration[i - 1] * t * t);
    motor->moveVelocity[i] = motor->moveVelocity[i - 1] + motor->moveAcceleration[i - 1] * t;
  }
  motor->movePosition[moveCount + 1] = destination;
  for (uint8_t i = 0; i <= moveCount; i++) {
    motor->moveAcceleration[i] *= 0.5f;  // pre-multiply here for later position calculation
  }
  motor->currentMove = 0;

  return;
}
