#include <Wire.h>
#include <ArduinoJson.h>
#include <AmperkaServo.h>
#include <MPU6050.h>

// servo settings
#define LEFT_SERVO_PIN 7
#define LEFT_SERVO_PIN_FB A15
#define RIGHT_SERVO_PIN 6
#define RIGHT_SERVO_PIN_FB A14
#define MCS_MIN 500
#define MCS_MAX 2500
#define FEEDBACK_MIN 0.247
#define FEEDBACK_MAX 3.063 
#define STOP 0
#define MOVE_FORWARD 1
#define MOVE_BACK 2
#define TURN_LEFT 3
#define TURN_RIGHT 4

#define MAP_INTERVAL 500

const uint8_t SERVO_SPEED = 255;

AmperkaServo leftServo;
AmperkaServo rightServo;

MPU6050 mpu;

bool readyForCmd = true;

// MPU6050 timers
uint32_t mpuGyroTimer = 0;
uint32_t motorsTimer = 0;
float mpuGyroTimeStep = 0.01;

// MPU6050 pitch, roll and yaw values
float pitch = 0;
float roll = 0;
float yaw = 0;

// draw map interval
uint32_t mapTimer = 0;

// servo action from WiFi App
uint8_t lastAction = STOP;
bool actionReadMode = false;

// servo degrees
int leftServoPrev = 0;
int rightServoPrev = 0;
int32_t leftServoDegrees = 0;
int32_t rightServoDegrees = 0;

// map
#define POINTS_LIM 10
int points[POINTS_LIM + 1][2];
uint8_t pointsN = 0;

const int mapJsonCapacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(POINTS_LIM) + POINTS_LIM * JSON_OBJECT_SIZE(2);

void actionFeedback() {
  Serial1.write(98);
  Serial1.write(lastAction + 48);
  Serial1.write(13);
  Serial1.write(10);
}

void serial1Flush() {
  while(Serial1.available()) { Serial1.read(); }
}

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

  Serial.println("run loop");
}

// 180 - 27


void loop() 
{
  // // read data from WiFi module
  if(readyForCmd) {
    if(Serial1.available()) {
      uint8_t code = Serial1.read();

      // read action
      if(code == 98) {
        actionReadMode = true;
      }
      else if(actionReadMode) {
        lastAction = code - 48;
        actionReadMode = false;
        // Serial.println(lastAction);
        actionFeedback();
      }
    }
  }

  // control servo motors
  if (lastAction == 1) {
    readyForCmd = false;
    leftServo.writeSpeed(SERVO_SPEED);
    rightServo.writeSpeed(-SERVO_SPEED);
    if(leftServoDegrees > 230) {
      leftServoDegrees = 0;
      readyForCmd = true;
      lastAction = 0;
      leftServo.writeSpeed(0);
      rightServo.writeSpeed(0);
    }
  }
  else if (lastAction == 2) {
    leftServo.writeSpeed(-SERVO_SPEED);
    rightServo.writeSpeed(SERVO_SPEED);
  }
  else if (lastAction == 3) {
    readyForCmd = false;
    leftServo.writeSpeed(-SERVO_SPEED);
    rightServo.writeSpeed(-SERVO_SPEED);
    if(yaw < -80) {
      yaw = 0;
      readyForCmd = true;
      lastAction = 0;
      leftServo.writeSpeed(0);
      rightServo.writeSpeed(0);
    }
  }
  else if (lastAction == 4) {
    readyForCmd = false;
    leftServo.writeSpeed(SERVO_SPEED);
    rightServo.writeSpeed(SERVO_SPEED);
    if(yaw > 80) {
      yaw = 0;
      readyForCmd = true;
      lastAction = 0;
      leftServo.writeSpeed(0);
      rightServo.writeSpeed(0);
    }
  }
  else {
    leftServo.writeSpeed(0);
    rightServo.writeSpeed(0);
  }

  // // gyro MPU6050
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
}