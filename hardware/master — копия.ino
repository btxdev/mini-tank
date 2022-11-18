#include <AmperkaServo.h>

#define LEFT_SERVO_PIN 6
#define RIGHT_SERVO_PIN 7

#define BTN_LEFT_PIN A15
#define BTN_RIGHT_PIN A14
#define BTN_TOP_PIN A13
#define BTN_BOTTOM_PIN A12

#define MCS_MIN 500
#define MCS_MAX 2500
#define FEEDBACK_MIN 0.247
#define FEEDBACK_MAX 3.063 

const uint8_t SERVO_SPEED = 255;

AmperkaServo leftServo;
AmperkaServo rightServo;

void setup() {
  Serial.begin(115200);
  Serial.println("Serial init");

  pinMode(BTN_LEFT_PIN, INPUT);
  pinMode(BTN_RIGHT_PIN, INPUT);
  pinMode(BTN_TOP_PIN, INPUT);
  pinMode(BTN_BOTTOM_PIN, INPUT);

  leftServo.attach(LEFT_SERVO_PIN, MCS_MIN, MCS_MAX);
  rightServo.attach(RIGHT_SERVO_PIN, MCS_MIN, MCS_MAX);
}

void loop() {

  // if(analogRead(BTN_TOP_PIN) > 100) {
  //   leftServo.writeSpeed(SERVO_SPEED);
  //   rightServo.writeSpeed(SERVO_SPEED);
  // }
  // else if(analogRead(BTN_BOTTOM_PIN) > 100) {
  //   leftServo.writeSpeed(-SERVO_SPEED);
  //   rightServo.writeSpeed(-SERVO_SPEED);
  // }
  // else if(analogRead(BTN_LEFT_PIN) > 100) {
  //   leftServo.writeSpeed(-SERVO_SPEED);
  //   rightServo.writeSpeed(SERVO_SPEED);
  // }
  // else if(analogRead(BTN_RIGHT_PIN) > 100) {
  //   leftServo.writeSpeed(SERVO_SPEED);
  //   rightServo.writeSpeed(-SERVO_SPEED);
  // }
  // else {
  //   leftServo.writeSpeed(0);
  //   rightServo.writeSpeed(0);
  // }

  Serial.print(analogRead(BTN_TOP_PIN));
  Serial.print(" ");
  Serial.print(analogRead(BTN_TOP_PIN));
  Serial.print(" ");
  Serial.print(analogRead(BTN_TOP_PIN));
  Serial.print(" ");
  Serial.print(analogRead(BTN_TOP_PIN));
  Serial.println();

}

