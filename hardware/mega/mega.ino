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
#define STOP 0
#define MOVE_FORWARD 1
#define MOVE_BACK 2
#define TURN_LEFT 3
#define TURN_RIGHT 4

const uint8_t SERVO_SPEED = 255;

// feedback interval
#define MPU_INTERVAL 3000

AmperkaServo leftServo;
AmperkaServo rightServo;

MPU6050 mpu;

StaticJsonDocument<200> doc;

// MPU6050 timers
uint32_t mpuGyroTimer = 0;
uint32_t mpuOutputTimer = 0;
float mpuGyroTimeStep = 0.01;

// MPU6050 pitch, roll and yaw values
float pitch = 0;
float roll = 0;
float yaw = 0;

// servo action from WiFi App
uint8_t lastAction = STOP;
bool actionReadMode = false;

void actionFeedback() {
  Serial1.write(98);
  Serial1.write(lastAction + 48);
  Serial1.write(13);
  Serial1.write(10);
}

void serial1Flush() {
  while(Serial1.available()) { Serial1.read(); }
}

void setup() 
{ 
  // debug
  Serial.begin(115200);
  // wifi
  Serial1.begin(115200);

  // Initialize MPU6050
  // Serial.print("Initializing MPU6050...");
  // while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  // {
  //   Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
  //   delay(500);
  // }

  // Serial.println("Calibrating gyroscope...");
  // mpu.calibrateGyro();

  // initialize servos
  leftServo.attach(LEFT_SERVO_PIN, MCS_MIN, MCS_MAX);
  leftServo.attachFB(LEFT_SERVO_PIN_FB,FEEDBACK_MIN, FEEDBACK_MAX);

  rightServo.attach(RIGHT_SERVO_PIN, MCS_MIN, MCS_MAX);
  rightServo.attachFB(RIGHT_SERVO_PIN_FB, FEEDBACK_MIN, FEEDBACK_MAX);
}

void loop() 
{
  // // read data from WiFi module
  // if (Serial1.available() > 0) {
  //   uint8_t inByte = Serial1.read();
  //   // read codes from WiFI module
  //   if(inByte == 98) {
  //     lastAction = (int)Serial1.read() - 48;
  //     Serial.println(lastAction);
  //     actionFeedback();
  //   }
  // }
  if(Serial1.available()) {
    uint8_t code = Serial1.read();
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
  // while(Serial1.available()) {
  //   // String incomingData = Serial1.readString();
  //   // Serial.println(incomingData);
  //   uint8_t code = Serial1.read();
  //   Serial.print(code);
  //   Serial.print(" ");
  //   if(code == 98) {
  //     code = Serial1.read();
  //     lastAction = code - 48;
  //     serial1Flush();
  //     actionFeedback();
  //   }
  // }

  // control servo motors
  if (lastAction == 1) {
    leftServo.writeSpeed(-SERVO_SPEED);
    rightServo.writeSpeed(SERVO_SPEED);
  }
  else if (lastAction == 2) {
    leftServo.writeSpeed(SERVO_SPEED);
    rightServo.writeSpeed(-SERVO_SPEED);
  }
  else if (lastAction == 3) {
    leftServo.writeSpeed(-SERVO_SPEED);
    rightServo.writeSpeed(-SERVO_SPEED);
  }
  else if (lastAction == 4) {
    leftServo.writeSpeed(SERVO_SPEED);
    rightServo.writeSpeed(SERVO_SPEED);
  }
  else {
    leftServo.writeSpeed(0);
    rightServo.writeSpeed(0);
  }

  // gyro MPU6050
  // if((millis() - mpuGyroTimer) > (mpuGyroTimeStep * 1000)) {
  //   mpuGyroTimer = millis();
  //   // read normalized values
  //   Vector norm = mpu.readNormalizeGyro();
  //   // calculate pitch, roll and yaw
  //   pitch = pitch + norm.YAxis * mpuGyroTimeStep;
  //   roll = roll + norm.XAxis * mpuGyroTimeStep;
  //   yaw = yaw + norm.ZAxis * mpuGyroTimeStep;
  // }

  // // output accel/gyro data
  // if((millis() - mpuOutputTimer) > MPU_INTERVAL) {
  //   doc["msgType"] = "gyroscope";
  //   doc["pitch"] = pitch;
  //   doc["roll"] = roll;
  //   doc["yaw"] = yaw;
  //   doc["accX"] = accelX;
  //   doc["accY"] = accelY;
  //   doc["accZ"] = accelZ;
  //   char jsonOutput[200];
  //   serializeJson(doc, jsonOutput);
  //   Serial.println(jsonOutput);
  //   Serial1.println(jsonOutput);
  //   mpuOutputTimer = millis();
  // }

}