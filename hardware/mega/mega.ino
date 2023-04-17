#include <Wire.h>
#include <NewPing.h>
#include <MPU6050.h>
#include <AmperkaServo.h>


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
#define LEFT_SERVO_PIN_FB A15

#define RIGHT_SERVO_PIN 3
#define RIGHT_SERVO_PIN_FB A14

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


AmperkaServo leftServo;
AmperkaServo rightServo;

MPU6050 mpuCenter;
MPU6050 mpuFront;

NewPing sonarFront(SNR_FRONT_TRIG, SNR_FRONT_ECHO, SNR_MAX_DISTANCE);
NewPing sonarBack(SNR_BACK_TRIG, SNR_BACK_ECHO, SNR_MAX_DISTANCE);
NewPing sonarLeft(SNR_LEFT_TRIG, SNR_LEFT_ECHO, SNR_MAX_DISTANCE);
NewPing sonarRight(SNR_RIGHT_TRIG, SNR_RIGHT_ECHO, SNR_MAX_DISTANCE);


// void actionFeedback(uint8_t lastAction);
// void serial1Flush();
int calcDegrees(int newValue, int oldValue);


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
  mpuCenter.calibrateGyro();

  Serial.println(F("Initializing Front MPU6050..."));
  while(!mpuFront.begin(MPU_GYRO_RANGE, MPU_ACCEL_RANGE, MPU_FRONT_ADDR))
  {
    Serial.println(F("Could not find a valid MPU6050 sensor (front), check wiring!"));
    delay(500);
  }

  Serial.println(F("Calibrating front gyroscope..."));
  mpuFront.calibrateGyro();

  Serial.println(F("Gyroscopes are ready!"));


  // сервы
  leftServo.attach(LEFT_SERVO_PIN, MCS_MIN, MCS_MAX);
  leftServo.attachFB(LEFT_SERVO_PIN_FB,FEEDBACK_MIN, FEEDBACK_MAX);

  rightServo.attach(RIGHT_SERVO_PIN, MCS_MIN, MCS_MAX);
  rightServo.attachFB(RIGHT_SERVO_PIN_FB, FEEDBACK_MIN, FEEDBACK_MAX);


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
  static float yaw = 0;

  static uint32_t mpuGyroTimer = 0;


  // сервы
  // предыдущее значение
  static int leftServoPrev = 0;
  static int rightServoPrev = 0;

  // градусы
  static int32_t leftServoDegrees = 0;
  static int32_t rightServoDegrees = 0;

  static uint32_t motorsTimer = 0;


  // вычисляемые координаты (мм)
  static int32_t xPos = 0;
  static int32_t yPos = 0;


  // парсер
  static bool readyForCmd = true;
  static bool actionReadMode = false;
  static uint8_t lastAction = STOP;


  // отправка данных
  static uint32_t outputTimer = 0;


  // двигатели
  // ...


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


  // гироскоп и координаты
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
    yaw = (centerYaw + frontYaw) / 2;

    // координаты
    // ...
  }
  

  // чтение данных с ESP
  while(Serial1.available()) {
    Serial1.read();
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


  // // read data from WiFi module
  // if(readyForCmd) {
  //   if(Serial1.available()) {
  //     uint8_t code = Serial1.read();
  //     Serial.println(code);

  //     // read action
  //     if(code == 98) {
  //       actionReadMode = true;
  //     }
  //     else if(actionReadMode) {
  //       lastAction = code - 48;
  //       actionReadMode = false;
  //       // Serial.println(lastAction);
  //       actionFeedback(lastAction);
  //     }
  //   }
  // }

  // // control servo motors
  // if (lastAction == 1) {
  //   readyForCmd = false;
  //   leftServo.writeSpeed(SERVO_SPEED);
  //   rightServo.writeSpeed(-SERVO_SPEED);
  //   if(leftServoDegrees > 230) {
  //     leftServoDegrees = 0;
  //     readyForCmd = true;
  //     lastAction = 0;
  //     leftServo.writeSpeed(0);
  //     rightServo.writeSpeed(0);
  //   }
  // }
  // else if (lastAction == 2) {
  //   leftServo.writeSpeed(-SERVO_SPEED);
  //   rightServo.writeSpeed(SERVO_SPEED);
  // }
  // else if (lastAction == 3) {
  //   readyForCmd = false;
  //   leftServo.writeSpeed(-SERVO_SPEED);
  //   rightServo.writeSpeed(-SERVO_SPEED);
  //   if(yaw < -80) {
  //     yaw = 0;
  //     readyForCmd = true;
  //     lastAction = 0;
  //     leftServo.writeSpeed(0);
  //     rightServo.writeSpeed(0);
  //   }
  // }
  // else if (lastAction == 4) {
  //   readyForCmd = false;
  //   leftServo.writeSpeed(SERVO_SPEED);
  //   rightServo.writeSpeed(SERVO_SPEED);
  //   if(yaw > 80) {
  //     yaw = 0;
  //     readyForCmd = true;
  //     lastAction = 0;
  //     leftServo.writeSpeed(0);
  //     rightServo.writeSpeed(0);
  //   }
  // }
  // else {
  //   leftServo.writeSpeed(0);
  //   rightServo.writeSpeed(0);
  // }
}

// // отправка ответа на ESP
// void actionFeedback(uint8_t lastAction) {
//   Serial1.write(98);
//   Serial1.write(lastAction + 48);
//   Serial1.write(13);
//   Serial1.write(10);
// }

// // очистка буфера
// void serial1Flush() {
//   while(Serial1.available()) { Serial1.read(); }
// }

// подсчет градусов двигателей
int calcDegrees(int newValue, int oldValue) {
  int delta = newValue - oldValue;
  if(delta > 90) delta = (newValue - 180) - oldValue;
  else if(delta < -90) delta = newValue - (oldValue - 180);
  return delta;
}