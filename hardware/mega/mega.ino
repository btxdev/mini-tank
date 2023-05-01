#include <Wire.h>
#include <NewPing.h>
#include <MPU6050.h>
#include "AmperkaServo.h"


// IR настройки
#define IR_L1 4
#define IR_L2 5
#define IR_R1 6
#define IR_R2 7


// сонар настройки
#define SNR_FRONT_TRIG   8
#define SNR_FRONT_ECHO   9
#define SNR_BACK_TRIG    10
#define SNR_BACK_ECHO    11
#define SNR_LEFT_TRIG    12
#define SNR_LEFT_ECHO    13
#define SNR_RIGHT_TRIG   14
#define SNR_RIGHT_ECHO   15

// максимальное расстояние сонара (см)
#define SNR_MAX_DISTANCE 400
// расстояние до препятствия (см)
#define SNR_THRESHOLD_DISTANCE 30


// серво настройки
#define LEFT_SERVO_PIN 2
#define LEFT_SERVO_PIN_FB A14

#define RIGHT_SERVO_PIN 3
#define RIGHT_SERVO_PIN_FB A15

// выходной диапазон ШИМ
#define MCS_MIN 500
#define MCS_MAX 2500
// входной диапазон напряжения
#define FEEDBACK_MIN 0.247
#define FEEDBACK_MAX 3.063
// максимальное значение скорости
#define SERVO_SPEED 255

// соотношение обороты / дистанция (мм)
#define SERVO_DISTANCE 109

// период между измерениями
#define SERVO_PERIOD_MS 200


// MPU настройки
#define MPU_CENTER_ADDR 0x68
#define MPU_FRONT_ADDR 0x69
// частота замеров
// при значении меньшем, чем время итерации loop, расчеты будут неверны
#define MPU_GYRO_PERIOD_MS 50
#define MPU_GYRO_PERIOD_S 0.05

// чувствительность акселерометра, меньше = точнее
// #define MPU_ACCEL_RANGE MPU6050_RANGE_2G
#define MPU_ACCEL_RANGE MPU6050_RANGE_4G
// #define MPU_ACCEL_RANGE MPU6050_RANGE_8G
// #define MPU_ACCEL_RANGE MPU6050_RANGE_16G

// лимит гироскопа (deg/s)
#define MPU_GYRO_RANGE MPU6050_SCALE_250DPS
// #define MPU_GYRO_RANGE MPU6050_SCALE_500DPS
// #define MPU_GYRO_RANGE MPU6050_SCALE_1000DPS
// #define MPU_GYRO_RANGE MPU6050_SCALE_2000DPS


// коды действий
#define STOP 0
#define MOVE_FORWARD 1
#define MOVE_BACK 2
#define TURN_LEFT 3
#define TURN_RIGHT 4
#define DIR_FORWARD 0
#define DIR_LEFT 1
#define DIR_BACK 2
#define DIR_RIGHT 3


// период отправки данных на ESP (мс)
#define ESP_OUTPUT_PERIOD 2000


#define DEG2RAD(deg) (deg * (M_PI / 180))


AmperkaServo leftServo;
AmperkaServo rightServo;

MPU6050 mpuCenter;
MPU6050 mpuFront;

NewPing sonarFront(SNR_FRONT_TRIG, SNR_FRONT_ECHO, SNR_MAX_DISTANCE);
NewPing sonarBack(SNR_BACK_TRIG, SNR_BACK_ECHO, SNR_MAX_DISTANCE);
NewPing sonarLeft(SNR_LEFT_TRIG, SNR_LEFT_ECHO, SNR_MAX_DISTANCE);
NewPing sonarRight(SNR_RIGHT_TRIG, SNR_RIGHT_ECHO, SNR_MAX_DISTANCE);


void actionFeedback(uint8_t lastAction);
void serial1Flush();
int32_t calcDegrees(int32_t newValue, int32_t oldValue);


void setup() 
{ 
  // debug
  Serial.begin(115200);

  // wifi
  Serial1.begin(115200);


  // MPU6050
  Serial.println(F("Initializing Center MPU6050..."));
  while(!mpuCenter.begin(MPU_GYRO_RANGE, MPU_ACCEL_RANGE, MPU_CENTER_ADDR))
  {
    Serial.println(F("Could not find a valid MPU6050 sensor (center), check wiring!"));
    delay(500);
  }

  Serial.println(F("Calibrating center gyroscope..."));
  delay(400);
  mpuCenter.calibrateGyro();

  Serial.println(F("Initializing Front MPU6050..."));
  while(!mpuFront.begin(MPU_GYRO_RANGE, MPU_ACCEL_RANGE, MPU_FRONT_ADDR))
  {
    Serial.println(F("Could not find a valid MPU6050 sensor (front), check wiring!"));
    delay(500);
  }

  Serial.println(F("Calibrating front gyroscope..."));
  delay(400);
  mpuFront.calibrateGyro();

  Serial.println(F("Gyroscopes are ready!"));


  // сервы
  leftServo.attach(LEFT_SERVO_PIN, MCS_MIN, MCS_MAX);
  leftServo.attachFB(LEFT_SERVO_PIN_FB, FEEDBACK_MIN, FEEDBACK_MAX);

  rightServo.attach(RIGHT_SERVO_PIN, MCS_MIN, MCS_MAX);
  rightServo.attachFB(RIGHT_SERVO_PIN_FB, FEEDBACK_MIN, FEEDBACK_MAX);

  leftServo.writeSpeed(0);
  rightServo.writeSpeed(0);


  // инфракрасные датчики
  pinMode(IR_L1, INPUT);
  pinMode(IR_L2, INPUT);
  pinMode(IR_R1, INPUT);
  pinMode(IR_R2, INPUT);


  Serial.println(F("loop is running:"));
}


void loop() 
{
  // MPU6050
  static float centerPitch = 0;
  static float centerRoll = 0;
  static float centerYaw = 0;

  static float frontPitch = 0;
  static float frontRoll = 0;
  static float frontYaw = 0;

  // среднее значение угла поворота с 2 датчиков
  static float pitch = 0;
  static float roll = 0;
  static float yaw = 0;

  static uint32_t mpuGyroTimer = 0;


  // сервы
  // суммарное значение (градусы)
  static int32_t leftServoDegrees = 0;
  static int32_t rightServoDegrees = 0;

  // значение до сброса (градусы)
  static int32_t leftServoPrev = 0;
  static int32_t rightServoPrev = 0;

  static uint32_t motorsTimer = 0;

  // градусы в момент начала движения
  static int32_t servoSaved = 0; 


  // вычисляемые координаты (мм)
  static int32_t xPos = 0;
  static int32_t yPos = 0;

  // угол в момент начала разворота
  static float yawSaved = 0;


  // парсер
  static bool readyForCmd = true;
  static bool actionReadMode = false;
  static uint8_t lastAction = STOP;


  static uint32_t outputTimer = 0;


  // сервы FB и координаты
  if((millis() - motorsTimer) > SERVO_PERIOD_MS) {
    motorsTimer = millis();

    // сколько было новых градусов спустя SERVO_PERIOD_MS
    const int leftServoNew = leftServo.readAngleFB();
    const int rightServoNew = rightServo.readAngleFB();
    const int32_t leftServoDelta = calcDegrees(leftServoNew, leftServoPrev);
    const int32_t rightServoDelta = calcDegrees(rightServoNew, rightServoPrev);

    leftServoDegrees += leftServoDelta;
    rightServoDegrees += rightServoDelta;

    leftServoPrev = leftServoNew;
    rightServoPrev = rightServoNew;

    // координаты
    const double arad = DEG2RAD(yaw);
    const double ax = cos(arad);
    const double ay = sin(arad);
    const int32_t delta = (leftServoDelta + rightServoDelta) / 2;
    xPos += delta * ax;
    yPos += delta * ay;
  }


  // IR датчики
  const bool ir1 = !digitalRead(IR_L1);
  const bool ir2 = !digitalRead(IR_L2);
  const bool ir3 = !digitalRead(IR_R1);
  const bool ir4 = !digitalRead(IR_R2);


  // сонары
  const uint32_t sonarFrontDistance = sonarFront.ping_cm();
  const uint32_t sonarBackDistance = sonarBack.ping_cm();
  const uint32_t sonarLeftDistance = sonarLeft.ping_cm();
  const uint32_t sonarRightDistance = sonarRight.ping_cm();

  const bool isSonarFront = sonarFrontDistance <= SNR_THRESHOLD_DISTANCE;
  const bool isSonarBack = sonarBackDistance <= SNR_THRESHOLD_DISTANCE;
  const bool isSonarLeft = sonarLeftDistance <= SNR_THRESHOLD_DISTANCE;
  const bool isSonarRight = sonarRightDistance <= SNR_THRESHOLD_DISTANCE;


  // препятствия
  const bool isObstacleFront = ir2 && ir3 && (ir1 || ir4 || isSonarFront);
  const bool isObstacleBack = isSonarBack;
  const bool isObstacleLeft = isSonarLeft;
  const bool isObstacleRight = isSonarRight;


  // гироскоп
  if((millis() - mpuGyroTimer) > MPU_GYRO_PERIOD_MS) {
    mpuGyroTimer = millis();

    // центральный
    Vector normCenter = mpuCenter.readNormalizeGyro();

    centerPitch += normCenter.YAxis * MPU_GYRO_PERIOD_S;
    centerRoll += normCenter.XAxis * MPU_GYRO_PERIOD_S;
    centerYaw += normCenter.ZAxis * MPU_GYRO_PERIOD_S;

    // передний
    Vector normFront = mpuFront.readNormalizeGyro();

    frontPitch += normFront.YAxis * MPU_GYRO_PERIOD_S;
    frontRoll += normFront.XAxis * MPU_GYRO_PERIOD_S;
    frontYaw += normFront.ZAxis * MPU_GYRO_PERIOD_S;

    // общий
    pitch = (centerPitch + frontPitch) / 2;
    roll = (centerRoll + frontRoll) / 2;
    yaw = (centerYaw + frontYaw) / 2;
  }
  

  // пришли данные с ESP
  if(Serial1.available()) {
    uint8_t code = Serial1.read();

    // пришла команда b (button)
    if(code == 98) {
      actionReadMode = true;
    }
    else if(actionReadMode) {
      lastAction = code - 48;
      actionReadMode = false;
      actionFeedback(lastAction);
    }
  }


  // управление двигателями
  // вперед
  if (lastAction == 1) {
    leftServo.writeSpeed(SERVO_SPEED);
    rightServo.writeSpeed(-SERVO_SPEED);
  }

  // назад
  else if (lastAction == 2) {
    leftServo.writeSpeed(-SERVO_SPEED);
    rightServo.writeSpeed(SERVO_SPEED);
  }

  // влево
  else if (lastAction == 3) {
    leftServo.writeSpeed(-SERVO_SPEED);
    rightServo.writeSpeed(-SERVO_SPEED);
  }

  // вправо
  else if (lastAction == 4) {
    leftServo.writeSpeed(SERVO_SPEED);
    rightServo.writeSpeed(SERVO_SPEED);
  }

  // стоп
  else {
    leftServo.writeSpeed(0);
    rightServo.writeSpeed(0);
  }


  // отправка данных на ESP 
  if((millis() - outputTimer) > ESP_OUTPUT_PERIOD) {
    outputTimer = millis();

    // подготовка данных
    String output = "";
    output += "r";

    // угол поворота
    output += "yaw:" + String(yaw) + ";";

    // моторы
    output += "FBL:" + String(leftServoDegrees) + ";";
    output += "FBR:" + String(rightServoDegrees) + ";";

    // координаты
    output += "x:" + String(xPos) + ";";
    output += "y:" + String(yPos) + ";";

    // сонары
    output += "SF:" + String(sonarFrontDistance) + ";";
    output += "SB:" + String(sonarBackDistance) + ";";
    output += "SL:" + String(sonarLeftDistance) + ";";
    output += "SR:" + String(sonarRightDistance) + ";";

    // отправка
    Serial1.println(output);
  }
}


// отправка ответа на ESP
void actionFeedback(uint8_t lastAction) {
  Serial1.write(98);
  Serial1.write(lastAction + 48);
  Serial1.write(13);
  Serial1.write(10);
}


// очистка буфера
void serial1Flush() {
  while(Serial1.available()) { Serial1.read(); }
}


// подсчет градусов двигателей
int32_t calcDegrees(int32_t newValue, int32_t oldValue) {
  int32_t delta = newValue - oldValue;
  if(delta > 90) delta = (newValue - 180) - oldValue;
  else if(delta < -90) delta = newValue - (oldValue - 180);
  return delta;
}