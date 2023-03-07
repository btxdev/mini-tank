#include <Wire.h>
#include <NewPing.h>
#include <MPU6050.h>
#include <AmperkaServo.h>


// servo settings
#define LEFT_SERVO_PIN 7
#define LEFT_SERVO_PIN_FB A15

#define RIGHT_SERVO_PIN 6
#define RIGHT_SERVO_PIN_FB A14

#define MCS_MIN 500
#define MCS_MAX 2500
#define FEEDBACK_MIN 0.247
#define FEEDBACK_MAX 3.063 


// how often to send the map (in ms) 
#define MAP_INTERVAL 5000


// sonar settings
#define SNR_FRONT_TRIG   3
#define SNR_FRONT_ECHO   4
#define SNR_BACK_TRIG    5
#define SNR_BACK_ECHO    6
#define SNR_LEFT_TRIG    7
#define SNR_LEFT_ECHO    8
#define SNR_RIGHT_TRIG   9
#define SNR_RIGHT_ECHO   10

#define SNR_MAX_DISTANCE 400
#define SNR_THRESHOLD_DISTANCE 30


// IR settings
#define IR_L1 4
#define IR_L2 5
#define IR_R1 6
#define IR_R2 7


// cmds
#define STOP 0
#define MOVE_FORWARD 1
#define MOVE_BACK 2
#define TURN_LEFT 3
#define TURN_RIGHT 4
#define DIR_FORWARD 0
#define DIR_LEFT 1
#define DIR_BACK 2
#define DIR_RIGHT 3


// const
const uint8_t SERVO_SPEED = 255;


// class
AmperkaServo leftServo;
AmperkaServo rightServo;

MPU6050 mpu;

NewPing sonarFront(SNR_FRONT_TRIG, SNR_FRONT_ECHO, SNR_MAX_DISTANCE);
NewPing sonarBack(SNR_BACK_TRIG, SNR_BACK_ECHO, SNR_MAX_DISTANCE);
NewPing sonarLeft(SNR_LEFT_TRIG, SNR_LEFT_ECHO, SNR_MAX_DISTANCE);
NewPing sonarRight(SNR_RIGHT_TRIG, SNR_RIGHT_ECHO, SNR_MAX_DISTANCE);


// MPU6050 timers
uint32_t mpuGyroTimer = 0;
uint32_t motorsTimer = 0;
float mpuGyroTimeStep = 0.01;

// MPU6050 pitch, roll and yaw values
float pitch = 0;
float roll = 0;
float yaw = 0;


// action from WiFi App
bool readyForCmd = true;
uint8_t lastAction = STOP;
bool actionReadMode = false;


// servo degrees
uint8_t leftServoPrev = 0;
uint8_t rightServoPrev = 0;
int32_t leftServoDegrees = 0;
int32_t rightServoDegrees = 0;


// millis for map
uint32_t mapTimer = 0;

// map
#define MAP_SIZE_X         100
#define MAP_SIZE_Y         100
#define INITIAL_TANK_POS_X 49
#define INITIAL_TANK_POS_Y 49
#define MAP_ARRAY_SIZE     ((MAP_SIZE_X * MAP_SIZE_Y) + 1)

#define UNKNOWN_CELL  0
#define FREE_CELL     1
#define OBSTACLE_CELL 2

int theMap[MAP_ARRAY_SIZE];

// current coordinates of tank
uint8_t tankX = INITIAL_TANK_POS_X;
uint8_t tankY = INITIAL_TANK_POS_Y;
uint8_t tankDirection = DIR_FORWARD;


// convert x, y coordinates to array element index
uint16_t map_2D_to_1D(uint8_t x, uint8_t y) {
  if(x < 0 || x >= MAP_SIZE_X || y < 0 || y >= MAP_SIZE_Y) return 65535;
  return (y * MAP_SIZE_X) + x;
}


// paste data to the map
void mapSetTo(uint8_t x, uint8_t y, uint8_t mark) {
  uint16_t index = map_2D_to_1D(x, y);
  if(index == 65535) return;
  theMap[index] = mark;
}


// reset map
void initMap() {
  for(int i = 0; i < MAP_ARRAY_SIZE; i++) {
    theMap[i] = UNKNOWN_CELL;
  }
  mapSetTo(INITIAL_TANK_POS_X, INITIAL_TANK_POS_Y, FREE_CELL);
}


// shift tank coordinates on the map depending on the actual tank direction
struct CoordinatesShift {
  int x = 0;
  int y = 0;
  CoordinatesShift(bool moveForward) {
    switch(tankDirection) {
      case DIR_FORWARD:
        this->y = -1;
        break;
      case DIR_BACK:
        this->y = 1;
        break;
      case DIR_LEFT:
        this->x = -1;
        break;
      case DIR_RIGHT:
        this->x = 1;
        break;
    }
    if(!moveForward) {
      this->x *= -1;
      this->y *= -1;
    }
  }
};
void moveTankCoordinates(bool forward) {
  CoordinatesShift shift(forward);
  tankX += shift.x;
  tankY += shift.y;
}


// send feedback to WiFi module
void actionFeedback() {
  Serial1.write(98);
  Serial1.write(lastAction + 48);
  Serial1.write(13);
  Serial1.write(10);
}


// kostyle & velosipedy
void serial1Flush() {
  while(Serial1.available()) { Serial1.read(); }
}


// calculate servo degrees
int calcDegrees(int newValue, int oldValue) {
  int delta = newValue - oldValue;
  if(delta > 90) delta = (newValue - 180) - oldValue;
  else if(delta < -90) delta = newValue - (oldValue - 180);
  return delta;
}


void setup() 
{ 
  // debug
  Serial.begin(115200);
  // wifi
  Serial1.begin(115200);


  initMap();


  // Initialize MPU6050
  Serial.println("Initializing MPU6050...");
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  Serial.println("Calibrating gyroscope...");
  mpu.calibrateGyro();


  // initialize servos
  leftServo.attach(LEFT_SERVO_PIN, MCS_MIN, MCS_MAX);
  leftServo.attachFB(LEFT_SERVO_PIN_FB,FEEDBACK_MIN, FEEDBACK_MAX);

  rightServo.attach(RIGHT_SERVO_PIN, MCS_MIN, MCS_MAX);
  rightServo.attachFB(RIGHT_SERVO_PIN_FB, FEEDBACK_MIN, FEEDBACK_MAX);


  // IR sensor
  pinMode(IR_L1, INPUT);
  pinMode(IR_L2, INPUT);
  pinMode(IR_R1, INPUT);
  pinMode(IR_R2, INPUT);


  Serial.println("run loop");
}


void loop() 
{
  // // read data from WiFi module
  if(Serial1.available()) {
    uint8_t code = Serial1.read();

    // read action
    if(code == 98) {
      actionReadMode = true;
    }
    else if(actionReadMode) {
      lastAction = code - 48;
      actionReadMode = false;
      Serial.println(lastAction);
      actionFeedback();
    }
  }


  // control servo motors
  switch(lastAction) {
    // recieved 'move forward' cmd
    case MOVE_FORWARD:
      // when cmd recieved
      readyForCmd = false;
      leftServo.writeSpeed(SERVO_SPEED);
      rightServo.writeSpeed(-SERVO_SPEED);
      // when the action is done
      if(leftServoDegrees > 230) {
        // reset
        leftServoDegrees = 0;
        readyForCmd = true;
        lastAction = 0;
        // stop
        leftServo.writeSpeed(0);
        rightServo.writeSpeed(0);
        // change tank coordinates on the map
        moveTankCoordinates(true);
      }
      break;
    case MOVE_BACK:
      leftServo.writeSpeed(-SERVO_SPEED);
      rightServo.writeSpeed(SERVO_SPEED);
      if(leftServoDegrees < -230) {
        leftServoDegrees = 0;
        readyForCmd = true;
        lastAction = 0;
        leftServo.writeSpeed(0);
        rightServo.writeSpeed(0);
        moveTankCoordinates(false);
      }
      break;
    case TURN_LEFT:
      readyForCmd = false;
      leftServo.writeSpeed(-SERVO_SPEED);
      rightServo.writeSpeed(-SERVO_SPEED);
      if(yaw < -80) {
        yaw = 0;
        readyForCmd = true;
        lastAction = 0;
        leftServo.writeSpeed(0);
        rightServo.writeSpeed(0);
        tankDirection--;
      }
      break;
    case TURN_RIGHT:
      readyForCmd = false;
      leftServo.writeSpeed(SERVO_SPEED);
      rightServo.writeSpeed(SERVO_SPEED);
      if(yaw > 80) {
        yaw = 0;
        readyForCmd = true;
        lastAction = 0;
        leftServo.writeSpeed(0);
        rightServo.writeSpeed(0);
        tankDirection++;
      }
      break;
    default:
      leftServo.writeSpeed(0);
      rightServo.writeSpeed(0);
      break;
  }
  // tank direction is like WASD
  if(tankDirection < 0) tankDirection = 3;
  if(tankDirection > 3) tankDirection = 0;


  // check motors every second
  if((millis() - motorsTimer) > 1000) {
    motorsTimer = millis();

    int leftServoNew = leftServo.readAngleFB();
    int rightServoNew = rightServo.readAngleFB();
    int leftServoDelta = calcDegrees(leftServoNew, leftServoPrev);
    int rightServoDelta = calcDegrees(rightServoNew, rightServoPrev);

    leftServoDegrees += leftServoDelta;
    rightServoDegrees += rightServoDelta;

    leftServoPrev = leftServoNew;
    rightServoPrev = rightServoNew;
  }


  // check gyro
  if((millis() - mpuGyroTimer) > (mpuGyroTimeStep * 1000)) {
    mpuGyroTimer = millis();

    Vector norm = mpu.readNormalizeGyro();
    // calculate pitch, roll and yaw
    pitch = pitch + norm.YAxis * mpuGyroTimeStep;
    roll = roll + norm.XAxis * mpuGyroTimeStep;
    yaw = yaw + norm.ZAxis * mpuGyroTimeStep;
    Serial.print(pitch);
    Serial.print(" ");
    Serial.print(roll);
    Serial.print(" ");
    Serial.println(yaw);
  }


  // current tank location cannot be an obstacle !!!
  mapSetTo(tankX, tankY, FREE_CELL);

  // detect front obstacle
  if(sonarFront.ping_cm() < SNR_THRESHOLD_DISTANCE || digitalRead(IR_L1) || digitalRead(IR_L2) || digitalRead(IR_R1) || digitalRead(IR_R2)) {
    // mark next cell as obstacle
    CoordinatesShift shift(true);
    mapSetTo(tankX + shift.x, tankY + shift.y, OBSTACLE_CELL);
    // stop
    leftServoDegrees = 0;
    readyForCmd = true;
    lastAction = 0;
    leftServo.writeSpeed(0);
    rightServo.writeSpeed(0);
  }
  else {
    // mark next cell as free
    CoordinatesShift shift(true);
    mapSetTo(tankX + shift.x, tankY + shift.y, FREE_CELL);
  }
  // detect back obstacle
  if(sonarBack.ping_cm() < SNR_THRESHOLD_DISTANCE) {
    // mark pevious cell as obstacle
    CoordinatesShift shift(false);
    mapSetTo(tankX + shift.x, tankY + shift.y, OBSTACLE_CELL);
  }
  else {
    // mark pevious cell as free
    CoordinatesShift shift(false);
    mapSetTo(tankX + shift.x, tankY + shift.y, FREE_CELL);
  }
  // detect left obstacle
  if(sonarLeft.ping_cm() < SNR_THRESHOLD_DISTANCE) {
    CoordinatesShift shift(false);
    mapSetTo(tankX + shift.y, tankY + shift.x, OBSTACLE_CELL);
  }
  else {
    CoordinatesShift shift(false);
    mapSetTo(tankX + shift.y, tankY + shift.x, OBSTACLE_FREE);
  }
  // detect right obstacle
  if(sonarRight.ping_cm() < SNR_THRESHOLD_DISTANCE) {
    CoordinatesShift shift(true);
    mapSetTo(tankX + shift.y, tankY + shift.x, OBSTACLE_CELL);
  }
  else {
    CoordinatesShift shift(true);
    mapSetTo(tankX + shift.y, tankY + shift.x, OBSTACLE_FREE);
  }


  // send map every MAP_INTERVAL
  if((millis() - mapTimer) > MAP_INTERVAL) {
    mapTimer = millis();

    Serial1.print('type:map;');
    for(int i = 0; i < MAP_ARRAY_SIZE; i++) {
      Serial1.print(theMap[i]);
      Serial1.print(",");
    }
    Serial1.println();
  }

}