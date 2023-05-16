#include "Driver.h"
#include "Gyro.h"
#include "src/NewPing/NewPing.h"


// серво настройки
#define LEFT_SERVO_PIN 2
#define LEFT_SERVO_PIN_FB A14
// #define LEFT_SERVO_K 0.85

#define RIGHT_SERVO_PIN 3
#define RIGHT_SERVO_PIN_FB A15
// #define RIGHT_SERVO_K 1.15

// MPU настройки
#define MPU_CENTER_ADDR 0x68
#define MPU_FRONT_ADDR 0x69

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

// период отправки данных на ESP (мс)
#define ESP1_OUTPUT_PERIOD 2000
#define ESP2_OUTPUT_PERIOD 1000

#define UNIT_CONDITION_INPUT 22
#define CLOSE_READING_INPUT 23
#define OPEN_WRITING_OUTPUT 24
#define DRIVING_CONDITION_OUTPUT 25
#define MAP_ROWS 6
#define MAP_COLS 6
#define CELL_SIZE_MM 300

#define DEG2RAD(deg) (deg * (M_PI / 180))


Driver driver;
Gyro frontGyro;
Gyro centerGyro;
NewPing sonarFront(SNR_FRONT_TRIG, SNR_FRONT_ECHO, SNR_MAX_DISTANCE);
NewPing sonarBack(SNR_BACK_TRIG, SNR_BACK_ECHO, SNR_MAX_DISTANCE);
NewPing sonarLeft(SNR_LEFT_TRIG, SNR_LEFT_ECHO, SNR_MAX_DISTANCE);
NewPing sonarRight(SNR_RIGHT_TRIG, SNR_RIGHT_ECHO, SNR_MAX_DISTANCE);


String buildUart2Message(
  uint8_t positionX, 
  uint8_t positionY,
  bool obstacleLeft,
  bool obstacleRight,
  bool obstacleBack,
  bool obstacleForward
  );
bool unitConditionServer();
bool unitCloseReading();
void setWritingCondition(bool condition);
void setDrivingCondition(bool condition);


void setup()
{
    delay(500);

    Serial.begin(115200);
    Serial1.begin(115200);
    Serial2.begin(115200);

    pinMode(UNIT_CONDITION_INPUT, INPUT);
    pinMode(CLOSE_READING_INPUT, INPUT);
    pinMode(OPEN_WRITING_OUTPUT, OUTPUT);
    pinMode(DRIVING_CONDITION_OUTPUT, OUTPUT);

    // если гироскопы сломались и нужно ехать/поворачивать по градусам
    // driver.attach(LEFT_SERVO_PIN, LEFT_SERVO_PIN_FB, LEFT_SERVO_K, 
    //               RIGHT_SERVO_PIN, RIGHT_SERVO_PIN_FB, RIGHT_SERVO_K);
    driver.attach(LEFT_SERVO_PIN, LEFT_SERVO_PIN_FB, 
                  RIGHT_SERVO_PIN, RIGHT_SERVO_PIN_FB);
    // основная скорость для моторов
    driver.setSpeed(80);
    driver.usePID(true, 0.8, 0, 0);

    // коэффициент для угла гироскопа (не накопительный)
    frontGyro.attach(0x68, 1.065);
    centerGyro.attach(0x69, 1.097);

    // по двум
    bool allGyroReady = frontGyro.ready && centerGyro.ready;
    // по одному
    // bool allGyroReady = frontGyro.ready;
    driver.useGyro(allGyroReady);

    delay(500);
}

void loop()
{
    frontGyro.tick();
    centerGyro.tick();
    driver.tick();

    // по двум
    float yaw = (frontGyro.yaw + centerGyro.yaw) / 2;
    // по одному
    // float yaw = frontGyro.yaw;
    driver.sendYaw(yaw);
    // для отладки коэфов гироскопа через плоттер
    // Serial.print("front: ");
    // Serial.print(frontGyro.yaw);
    // Serial.print(", center: ");
    // Serial.print(centerGyro.yaw);
    // Serial.print(", mean: ");
    // Serial.print(yaw);
    // Serial.println();
    // return;

    // координаты танка
    static uint8_t xCell = 0;
    static uint8_t yCell = 0;
    static int8_t direction = 0;

    // таймеры для отправки на ESP
    static uint32_t output1Timer = millis();
    static uint32_t output2Timer = millis();
    static uint32_t output2Timer2 = millis();

    // препятствия
    const uint32_t sonarFrontDistance = sonarFront.ping_cm();
    const uint32_t sonarBackDistance = sonarBack.ping_cm();
    const uint32_t sonarLeftDistance = sonarLeft.ping_cm();
    const uint32_t sonarRightDistance = sonarRight.ping_cm();

    const bool isSonarFront = sonarFrontDistance <= SNR_THRESHOLD_DISTANCE;
    const bool isSonarBack = sonarBackDistance <= SNR_THRESHOLD_DISTANCE;
    const bool isSonarLeft = sonarLeftDistance <= SNR_THRESHOLD_DISTANCE;
    const bool isSonarRight = sonarRightDistance <= SNR_THRESHOLD_DISTANCE;

    // готов читать команду и пришли данные с ESP (controller)
    if (!unitConditionServer() && Serial1.available() && driver.isReady())
    {
        // читаем в этот раз сразу строкой
        String task = Serial1.readString().substring(0, 10);
        // Serial.println(task);
        // коды WASD
        if (task == "<action:w>")
        {
            driver.moveForward();
            if (direction == 0) yCell++;
            if (direction == 1) xCell++;
            if (direction == 2) yCell--;
            if (direction == 3) xCell--;
        }
        if (task == "<action:a>")
        {
            driver.turnLeft();
            direction--;
        }
        if (task == "<action:s>")
        {
            driver.moveBackward();
            if (direction == 0) yCell--;
            if (direction == 1) xCell--;
            if (direction == 2) yCell++;
            if (direction == 3) xCell++;
        }
        if (task == "<action:d>")
        {
            driver.turnRight();
            direction++;
        }
        if (task == "<action:_>")
        {
            driver.delay(1000);
        }
        Serial1.flush();
    }
    if (direction < 0) direction = 3;
    if (direction > 3) direction = 0;

    // говорим ESP (monitor) что мы едем
    setDrivingCondition(!driver.isReady());

    // можно отправлять данные на ESP (monitor)
    if (!unitConditionServer())
    {
        // прошло достаточно времени
        if( (millis() - output2Timer) > ESP2_OUTPUT_PERIOD)
        {
            setWritingCondition(true);
            while (!unitCloseReading())
            {
                if ((millis() - output2Timer2) > 300)
                {
                    output2Timer2 = millis();
                    Serial2.write(buildUart2Message(
                    xCell,
                    yCell,
                    isSonarLeft,
                    isSonarRight,
                    isSonarBack,
                    isSonarFront
                    ).c_str());
                }
            }
            setWritingCondition(false);

            output2Timer = millis();
        }
    }


    // можно отправлять данные на ESP (controller)
    if (
        !Serial1.available()
        && (millis() - output1Timer) > ESP1_OUTPUT_PERIOD
        )
    {
        output1Timer = millis();

        // отправка
        Serial1.println(buildUart1Message(
        yaw,
        xCell,
        yCell,
        sonarFrontDistance,
        sonarBackDistance,
        sonarLeftDistance,
        sonarRightDistance
        ));
    }
}

String buildUart1Message(
    float yaw,
    int32_t xPos,
    int32_t yPos,
    uint32_t sonarFrontDistance,
    uint32_t sonarBackDistance,
    uint32_t sonarLeftDistance,
    uint32_t sonarRightDistance
    )
{
    // подготовка данных
    String output = "";
    output += "<";

    // угол поворота
    output += "yaw:" + String(yaw) + ";";

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

String buildUart2Message(
    uint8_t positionX, 
    uint8_t positionY,
    bool obstacleLeft,
    bool obstacleRight,
    bool obstacleBack,
    bool obstacleForward
    )
{
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

bool unitConditionServer()
{
    return digitalRead(UNIT_CONDITION_INPUT);
}

bool unitCloseReading()
{
    return digitalRead(CLOSE_READING_INPUT);
}

void setWritingCondition(bool condition)
{
    digitalWrite(OPEN_WRITING_OUTPUT, condition);
}

void setDrivingCondition(bool condition)
{
    digitalWrite(DRIVING_CONDITION_OUTPUT, condition);
}