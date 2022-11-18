#include <AmperkaServo.h>

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

AmperkaServo leftServo;
AmperkaServo rightServo;

// state of buttons from WiFi App
uint8_t SELECTED_BTN = 0;
bool BTN_LEFT_FLAG = false;
bool BTN_RIGHT_FLAG = false;
bool BTN_TOP_FLAG = false;
bool BTN_BOTTOM_FLAG = false;

void setup() 
{ 
  // debug
  Serial.begin(115200);
  // wifi
  Serial1.begin(9600);

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
    Serial.print(inByte / 10);
    if (inByte > 0 && inByte <= 4) SELECTED_BTN = inByte;
    bool state = inByte == 6;
    if(inByte == 5 || inByte == 6) {
      if (SELECTED_BTN == 1) BTN_LEFT_FLAG = state;
      if (SELECTED_BTN == 2) BTN_RIGHT_FLAG = state;
      if (SELECTED_BTN == 3) BTN_TOP_FLAG = state;
      if (SELECTED_BTN == 4) BTN_BOTTOM_FLAG = state;
      // SELECTED_BTN = 0;
    }
    // Serial.write(inByte);
  }

  // looper++;
  // if(looper > 1000) {
  //   Serial.println(SELECTED_BTN);
  //   looper = 0;
  // }
  

  // control servo motors
  if (BTN_TOP_FLAG) {
    leftServo.writeSpeed(SERVO_SPEED);
    rightServo.writeSpeed(SERVO_SPEED);
  }
  else if (BTN_BOTTOM_FLAG) {
    leftServo.writeSpeed(-SERVO_SPEED);
    rightServo.writeSpeed(-SERVO_SPEED);
  }
  else if (BTN_LEFT_FLAG) {
    leftServo.writeSpeed(-SERVO_SPEED);
    rightServo.writeSpeed(SERVO_SPEED);
  }
  else if (BTN_RIGHT_FLAG) {
    leftServo.writeSpeed(SERVO_SPEED);
    rightServo.writeSpeed(-SERVO_SPEED);
  }
  else {
    leftServo.writeSpeed(0);
    rightServo.writeSpeed(0);
  }

}