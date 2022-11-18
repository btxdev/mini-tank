#include <AmperkaServo.h>

// #define LEFT_SERVO_PIN D3
#define LEFT_SERVO_PIN 5
#define RIGHT_SERVO_PIN 6

#define MCS_MIN 500
#define MCS_MAX 2500
#define FEEDBACK_MIN 0.247
#define FEEDBACK_MAX 3.063 

const uint8_t speed = 255;

AmperkaServo leftServo;
AmperkaServo rightServo;

void stop(uint16_t milliseconds) {
  leftServo.writeSpeed(0);
  rightServo.writeSpeed(0);
  delay(milliseconds);
}

void stop() {
  stop(0);
}

void moveForward(uint16_t milliseconds) {
  leftServo.writeSpeed(speed);
  rightServo.writeSpeed(speed);
  delay(milliseconds);
  stop();
}

void moveBackward(uint16_t milliseconds) {
  leftServo.writeSpeed(-speed);
  rightServo.writeSpeed(-speed);
  delay(milliseconds);
  stop();
}

void turnLeft(uint16_t milliseconds) {
  leftServo.writeSpeed(speed);
  rightServo.writeSpeed(-speed);
  delay(milliseconds);
  stop();
}

void turnRight(uint16_t milliseconds) {
  leftServo.writeSpeed(-speed);
  rightServo.writeSpeed(speed);
  delay(milliseconds);
  stop();
}

void setup() {
  Serial.begin(9600);

  leftServo.attach(LEFT_SERVO_PIN, MCS_MIN, MCS_MAX);
  rightServo.attach(RIGHT_SERVO_PIN, MCS_MIN, MCS_MAX);
}

void loop() {
  unsigned int t = 3000;
  moveForward(t);
  stop(t / 2);
  turnLeft(t / 2);
  stop(t / 2);
  turnRight(t / 2);
  stop(t / 2);
  moveBackward(t);
  stop(t);
}
