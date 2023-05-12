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

// проезжаемая дистанция (мм) за 1 оборот
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
#define DRIVE_FORWARD 1
#define DRIVE_BACKWARD 2
#define TURN_LEFT 3
#define TURN_RIGHT 4


// период отправки данных на ESP (мс)
#define ESP1_OUTPUT_PERIOD 2000
#define ESP2_OUTPUT_PERIOD 1000


#define DEG2RAD(deg) (deg * (M_PI / 180))


// новый код
#define UNIT_CONDITION_INPUT 1
#define CLOSE_READING_INPUT 2
#define OPEN_WRITING_OUTPUT 5
#define DRIVING_CONDITION_OUTPUT 7
#define MAP_ROWS 6
#define MAP_COLS 6
#define CELL_SIZE_MM 300


AmperkaServo leftServo;
AmperkaServo rightServo;

MPU6050 mpuCenter;
MPU6050 mpuFront;

NewPing sonarFront(SNR_FRONT_TRIG, SNR_FRONT_ECHO, SNR_MAX_DISTANCE);
NewPing sonarBack(SNR_BACK_TRIG, SNR_BACK_ECHO, SNR_MAX_DISTANCE);
NewPing sonarLeft(SNR_LEFT_TRIG, SNR_LEFT_ECHO, SNR_MAX_DISTANCE);
NewPing sonarRight(SNR_RIGHT_TRIG, SNR_RIGHT_ECHO, SNR_MAX_DISTANCE);


String buildUart1Message(
  uint8_t positionX, 
  uint8_t positionY,
  bool obstacleLeft,
  bool obstacleRight,
  bool obstacleBack,
  bool obstacleForward
  );

String buildUart2Message(
  uint8_t positionX, 
  uint8_t positionY,
  bool obstacleLeft,
  bool obstacleRight,
  bool obstacleBack,
  bool obstacleForward
  );

int32_t calcDegrees(int32_t newValue, int32_t oldValue);
void drive(uint8_t direction);

bool unitConditionServer();
bool unitCloseReading();
void setWritingCondition(bool condition);
void setDrivingCondition(bool condition);

void pinModeSetting() {
  pinMode(UNIT_CONDITION_INPUT, INPUT);
  pinMode(CLOSE_READING_INPUT, INPUT);
  pinMode(OPEN_WRITING_OUTPUT, OUTPUT);
  pinMode(DRIVING_CONDITION_OUTPUT, OUTPUT);
}


void setup() 
{ 
  pinModeSetting();

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

  static uint32_t mpuGyroTimer = millis();


  // сервы
  // суммарное значение (градусы)
  static int32_t leftServoDegrees = 0;
  static int32_t rightServoDegrees = 0;

  // значение до сброса (градусы)
  static int32_t leftServoPrev = 0;
  static int32_t rightServoPrev = 0;

  static uint32_t motorsTimer = millis();

  // градусы в момент начала движения
  static int32_t servoSaved = 0; 


  // вычисляемые координаты (мм)
  static int32_t xPos = 0;
  static int32_t yPos = 0;

  // клеточные координаты
  static uint8_t xCell = 0;
  static uint8_t yCell = 0;

  // угол в момент начала разворота
  static float yawSaved = 0;


  // парсер
  static bool readyForCmd = true;
  static bool actionReadMode = false;
  static uint8_t lastAction = STOP;


  static uint32_t output1Timer = millis();
  static uint32_t output2Timer = millis();
  static uint32_t output2Timer2 = millis();


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
    xCell = xPos / CELL_SIZE_MM;
    yCell = yPos / CELL_SIZE_MM;
    if (xCell >= MAP_COLS - 3) xCell = 0;
    if (yCell >= MAP_ROWS - 3) yCell = 0;
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
  

  // чтение данных с ESP
  // готов читать команду и пришли данные с ESP
  if(readyForCmd && Serial1.available()) {
    uint8_t code = Serial1.read();

    // пришла команда b (button)
    if(code == 'b') {
      actionReadMode = true;
    }
    // код кнопки
    else if(actionReadMode) {
      lastAction = code - 48;
      actionReadMode = false;
    }
  }


  // можно ехать
  if (!unitConditionServer()) {
    
    // управление двигателями
    setDrivingCondition(true);

    // вперед
    if (lastAction == DRIVE_FORWARD) {
      readyForCmd = false;
      drive(lastAction);
      // пока не будет подсчитано 230 градусов
      if(abs(leftServoDegrees - servoSaved) > 230) {
        readyForCmd = true;
        lastAction = 0;
        drive(STOP);
        servoSaved = leftServoDegrees;
      }
    }

    // назад
    else if (lastAction == DRIVE_BACKWARD) {
      readyForCmd = false;
      drive(lastAction);
      // пока не будет подсчитано -230 градусов
      if(abs(leftServoDegrees - servoSaved) > 230) {
        readyForCmd = true;
        lastAction = 0;
        drive(STOP);
        servoSaved = leftServoDegrees;
      }
    }

    // влево
    else if (lastAction == TURN_LEFT) {
      readyForCmd = false;
      drive(lastAction);
      // пока не будет достигнут угол в -90
      if(abs(yaw - yawSaved) >= 90) {
        readyForCmd = true;
        lastAction = 0;
        drive(STOP);
        yawSaved = yaw;
      }
    }

    // вправо
    else if (lastAction == TURN_RIGHT) {
      readyForCmd = false;
      yawSaved = yaw;
      drive(lastAction);
      // пока не будет достигнут угол в 90
      if(abs(yaw - yawSaved) >= 90) {
        readyForCmd = true;
        lastAction = 0;
        drive(STOP);
        yawSaved = yaw;
      }
    }

    setDrivingCondition(false);
  }

  // стоп
  if(lastAction == STOP) {
    drive(lastAction);
  }

  // фильтрация: пока едем, чистим буфер
  if(lastAction >= 1 && lastAction <= 4) {
    Serial1.flush();
  }


  // можно отправлять данные на Serial 2
  if (!unitConditionServer()) {
    // прошло достаточно времени
    if( (millis() - output2Timer) > ESP2_OUTPUT_PERIOD) {

      setWritingCondition(true);
      while (!unitCloseReading()) {
        if ((millis() - output2Timer2) > 300) {
          output2Timer2 = millis();
          Serial2.write(buildUart2Message(
            xCell,
            yCell,
            isObstacleLeft,
            isObstacleRight,
            isObstacleBack,
            isObstacleFront
            ).c_str());
        }
      }
      setWritingCondition(false);

      output2Timer = millis();
    }
  }


  // можно отправлять данные на Serial 1
  if(
    !Serial1.available()
    && (millis() - output1Timer) > ESP1_OUTPUT_PERIOD
    ) {
    output1Timer = millis();

    // отправка
    Serial1.println(buildUart1Message(
      yaw,
      leftServoDegrees,
      rightServoDegrees,
      xPos,
      yPos,
      sonarFrontDistance,
      sonarBackDistance,
      sonarLeftDistance,
      sonarRightDistance
      ));
  }

}


// подсчет градусов двигателей
int32_t calcDegrees(int32_t newValue, int32_t oldValue) {
  int32_t delta = newValue - oldValue;
  if(delta > 90) delta = (newValue - 180) - oldValue;
  else if(delta < -90) delta = newValue - (oldValue - 180);
  return delta;
}


void drive(uint8_t direction) {
  switch(direction) {
    case DRIVE_FORWARD:
      leftServo.writeSpeed(SERVO_SPEED);
      rightServo.writeSpeed(-SERVO_SPEED);
      break;

    case DRIVE_BACKWARD:
      leftServo.writeSpeed(-SERVO_SPEED);
      rightServo.writeSpeed(SERVO_SPEED);
      break;

    case TURN_LEFT:
      leftServo.writeSpeed(-SERVO_SPEED);
      rightServo.writeSpeed(-SERVO_SPEED);
      break;

    case TURN_RIGHT:
      leftServo.writeSpeed(SERVO_SPEED);
      rightServo.writeSpeed(SERVO_SPEED);
      break;

    case STOP:
      leftServo.writeSpeed(0);
      rightServo.writeSpeed(0);
      break;
  }
}


String buildUart1Message(
  float yaw, 
  int32_t leftServoDegrees,
  int32_t rightServoDegrees,
  int32_t xPos,
  int32_t yPos,
  uint32_t sonarFrontDistance,
  uint32_t sonarBackDistance,
  uint32_t sonarLeftDistance,
  uint32_t sonarRightDistance
  ) {
    // подготовка данных
    String output = "";
    output += "<";

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
    output += "SR:" + String(sonarRightDistance);

    output += ">";

    return output;
}


// новый код

String buildUart2Message(
  uint8_t positionX, 
  uint8_t positionY,
  bool obstacleLeft,
  bool obstacleRight,
  bool obstacleBack,
  bool obstacleForward
  ) {
    String messageBody;
    messageBody += String(positionX) + ":";
    messageBody += String(positionY) + ":";
    messageBody += String(obstacleForward) + ":";
    messageBody += String(obstacleBack) + ":";
    messageBody += String(obstacleRight) + ":";
    messageBody += String(obstacleLeft);
    String message = String("<") + messageBody + String(">");
    return message;
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
  }
  else {
    digitalWrite(OPEN_WRITING_OUTPUT, LOW);
  }
}


void setDrivingCondition(bool condition) {
  if (condition) {
    digitalWrite(DRIVING_CONDITION_OUTPUT, HIGH);
  }
  else {
    digitalWrite(DRIVING_CONDITION_OUTPUT, LOW);
  }
}