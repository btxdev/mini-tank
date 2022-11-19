#include <Wire.h>
#include <ArduinoJson.h>
#include <AmperkaServo.h>
#include <MPU6050.h>

// servo settings
#define LEFT_SERVO_PIN 6
#define LEFT_SERVO_PIN_FB A14
#define RIGHT_SERVO_PIN 7
#define RIGHT_SERVO_PIN_FB A15
#define MCS_MIN 500
#define MCS_MAX 2500
#define FEEDBACK_MIN 0.247
#define FEEDBACK_MAX 3.063 

const uint8_t SERVO_SPEED = 255;

// mpu6050 output interval
#define MPU_INTERVAL 3000

AmperkaServo leftServo;
AmperkaServo rightServo;

MPU6050 mpu;

StaticJsonDocument<200> doc;

// MPU6050 timers
uint32_t mpuGyroTimer = 0;
uint32_t mpuAccelTimer = 0;
uint32_t mpuOutputTimer = 0;
float mpuGyroTimeStep = 0.01;
float mpuAccelTimeStep = 0.01;
uint8_t mpuAccelCounter = 0;

// MPU6050 pitch, roll and yaw values
float pitch = 0;
float roll = 0;
float yaw = 0;

// MPU6050 accel
float accelX = 0;
float accelXb = 0;
float accelY = 0;
float accelYb = 0;
float accelZ = 0;
float accelZb = 0;

// state of buttons from WiFi App
bool BTN_LEFT_FLAG = false;
bool BTN_RIGHT_FLAG = false;
bool BTN_TOP_FLAG = false;
bool BTN_BOTTOM_FLAG = false;

void setup() 
{ 
  // debug
  Serial.begin(115200);
  // wifi
  Serial1.begin(115200);

  // Initialize MPU6050
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  mpu.calibrateGyro();
  mpu.setThreshold(3);
  mpu.setAccelOffsetX(-735);
  mpu.setAccelOffsetY(-2300);
  mpu.setAccelOffsetZ(3330);

  leftServo.attach(LEFT_SERVO_PIN, MCS_MIN, MCS_MAX);
  leftServo.attachFB(LEFT_SERVO_PIN_FB,FEEDBACK_MIN, FEEDBACK_MAX);

  rightServo.attach(RIGHT_SERVO_PIN, MCS_MIN, MCS_MAX);
  rightServo.attachFB(RIGHT_SERVO_PIN_FB, FEEDBACK_MIN, FEEDBACK_MAX);
} 

int looper = 0;

void loop() 
{ 
  // read codes from WiFI module
  if (Serial1.available()) {
    uint8_t inByte = Serial1.read();
    if (inByte == 1) BTN_LEFT_FLAG = true;
    else if (inByte == 2) BTN_LEFT_FLAG = false;
    else if (inByte == 3) BTN_RIGHT_FLAG = true;
    else if (inByte == 4) BTN_RIGHT_FLAG = false;
    else if (inByte == 5) BTN_TOP_FLAG = true;
    else if (inByte == 6) BTN_TOP_FLAG = false;
    else if (inByte == 7) BTN_BOTTOM_FLAG = true;
    else if (inByte == 8) BTN_BOTTOM_FLAG = false;
    else {
      BTN_LEFT_FLAG = false;
      BTN_RIGHT_FLAG = false;
      BTN_TOP_FLAG = false;
      BTN_BOTTOM_FLAG = false;
    }
    Serial.println(inByte);
  }

  // control servo motors
  if (BTN_TOP_FLAG) {
    leftServo.writeSpeed(-SERVO_SPEED);
    rightServo.writeSpeed(SERVO_SPEED);
  }
  else if (BTN_BOTTOM_FLAG) {
    leftServo.writeSpeed(SERVO_SPEED);
    rightServo.writeSpeed(-SERVO_SPEED);
  }
  else if (BTN_LEFT_FLAG) {
    leftServo.writeSpeed(-SERVO_SPEED);
    rightServo.writeSpeed(-SERVO_SPEED);
  }
  else if (BTN_RIGHT_FLAG) {
    leftServo.writeSpeed(SERVO_SPEED);
    rightServo.writeSpeed(SERVO_SPEED);
  }
  else {
    leftServo.writeSpeed(0);
    rightServo.writeSpeed(0);
  }

  // gyro MPU6050
  if((millis() - mpuGyroTimer) > (mpuGyroTimeStep * 1000)) {
    mpuGyroTimer = millis();
    // read normalized values
    Vector norm = mpu.readNormalizeGyro();
    // calculate pitch, roll and yaw
    pitch = pitch + norm.YAxis * mpuGyroTimeStep;
    roll = roll + norm.XAxis * mpuGyroTimeStep;
    yaw = yaw + norm.ZAxis * mpuGyroTimeStep;
  }

  // accel MPU6050
  if((millis() - mpuAccelTimer) > (mpuAccelTimeStep * 1000)) {
    mpuAccelTimer = millis();
    // read normalized values
    Vector normAccel = mpu.readNormalizeAccel();
    // calculate values
    accelXb += normAccel.XAxis;
    accelYb += normAccel.YAxis;
    accelZb += normAccel.ZAxis;
    mpuAccelCounter++;
  }
  if(mpuAccelCounter > 10) {
    accelX = accelXb / 10;
    accelY = accelYb / 10;
    accelZ = accelZb / 10;
    accelXb = 0;
    accelYb = 0;
    accelZb = 0;
    mpuAccelCounter = 0;
  }

  // output accel/gyro data
  if((millis() - mpuOutputTimer) > MPU_INTERVAL) {
    doc["msgType"] = "gyroscope";
    doc["pitch"] = pitch;
    doc["roll"] = roll;
    doc["yaw"] = yaw;
    doc["accX"] = accelX;
    doc["accY"] = accelY;
    doc["accZ"] = accelZ;
    char jsonOutput[200];
    serializeJson(doc, jsonOutput);
    Serial.println(jsonOutput);
    Serial1.println(jsonOutput);
    mpuOutputTimer = millis();
  }

}