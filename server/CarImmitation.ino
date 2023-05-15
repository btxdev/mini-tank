#include <SoftwareSerial.h>

#define UNIT_CONDITION_INPUT D1
#define CLOSE_READING_INPUT D2
#define OPEN_WRITING_OUTPUT D5
#define DRIVING_CONDITION_OUTPUT D7

uint8_t positionX = 0, positionY = 0;
uint8_t mapRows = 6;
uint8_t mapCols = 6;

uint16_t delayMovement = 5000;

bool obstacleLeft = false, 
     obstacleRight = false,
     obstacleBack = false,
     obstacleForward = false;

bool wasBraken = false;

char startChar = '<',
     endChar = '>';

SoftwareSerial softSerial(D9, D10);

String buildUartMessage(){
  String messageBody = String(positionX) + ":" + String(positionY) + ":" + String(obstacleForward) + ":" + String(obstacleBack)
                        + ":" + String(obstacleRight) + ":" + String(obstacleLeft);
  String message = String(startChar) + messageBody + String(endChar);
  return message;
}

void pinModeSetting() {
  pinMode(UNIT_CONDITION_INPUT, INPUT);
  pinMode(CLOSE_READING_INPUT, INPUT);
  pinMode(OPEN_WRITING_OUTPUT, OUTPUT);
  pinMode(DRIVING_CONDITION_OUTPUT, OUTPUT);
}

bool unitConditionServer() {
  if (digitalRead(UNIT_CONDITION_INPUT) == HIGH) {
    return true;
  }
  return false;
}

bool unitCloseReading() {
  if (digitalRead(CLOSE_READING_INPUT) == HIGH) {
    return true;
  }
  return false;
}

void setWritingCondition(bool condition) {
  if (condition) {
    digitalWrite(OPEN_WRITING_OUTPUT, HIGH);
  } else {
    digitalWrite(OPEN_WRITING_OUTPUT, LOW);
  }
}

void setDrivingCondition(bool condition){
  if (condition) {
    digitalWrite(DRIVING_CONDITION_OUTPUT, HIGH);
  } else {
    digitalWrite(DRIVING_CONDITION_OUTPUT, LOW);
  }
}

void changePosition() {
  if (positionX < mapRows - 3) {
    positionX++;
  } else if (positionY < mapCols - 3) {
    positionY ++;
  } else {
    positionX = 0, positionY = 0;
  }
}

void drive() {
  delay(delayMovement);
  changePosition();
}

void setup() {
  pinModeSetting();
  // Serial.begin(115200);
  softSerial.begin(115200);
}

void loop() {
  // Serial.println("My position: (" + String(positionX) + ":" + String(positionY) + ")");
  // Serial.println("D1 " + String(digitalRead(UNIT_CONDITION_INPUT)));

  // Serial.println("D6 " + String(digitalRead(CLOSE_READING_INPUT)));
  // delay(100);
  delay(1000);
  if (!unitConditionServer()) {
    setDrivingCondition(true);
    drive();
    setDrivingCondition(false);

    setWritingCondition(true);
    while (!unitCloseReading()) {
      softSerial.write(buildUartMessage().c_str());
      delay(300);
    }
    setWritingCondition(false); 
  }
/*
  if () {

  }

  if (!unitConditionServer()) {
   



    while (!unitCloseReading()) {
      softSerial.write(buildUartMessage().c_str());
      delay(200);
      setWritingCondition(true);
      if (unitConditionServer()){
        wasBraken = true;
        break;
      }
      wasBraken = false;
    }
    if (!wasBraken) {
      setWritingCondition(false);
      
      delay(1000);
    }
   
  } 
  */
}