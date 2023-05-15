#include "Driver.h"
#include "Gyro.h"


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


Driver driver;
Gyro frontGyro;
Gyro centerGyro;
// String taskQueue = "w_a_w_d_w_d_d_w_a_w_d_w_";
// String taskQueue = "a_d_d_a___aa_dd";
// String taskQueue = "a_d_d_a_";
// String taskQueue = "w_s_w_s";
// String taskQueue = "a_d_w_s_";
String taskQueue = "ww_a_w_d_w_aa_w_a_w_d_ww_dd";
// String taskQueue = "wwww";


void setup()
{
    Serial.begin(115200);

    // driver.attach(LEFT_SERVO_PIN, LEFT_SERVO_PIN_FB, LEFT_SERVO_K, 
    //               RIGHT_SERVO_PIN, RIGHT_SERVO_PIN_FB, RIGHT_SERVO_K);
    driver.attach(LEFT_SERVO_PIN, LEFT_SERVO_PIN_FB, 
                  RIGHT_SERVO_PIN, RIGHT_SERVO_PIN_FB);
    driver.setSpeed(80);
    // driver.usePID(true, 0.8, 0.4, 0);
    driver.usePID(true, 0.8, 0, 0);


    // frontGyro.attach(0x68, 1.03);
    // centerGyro.attach(0x69, 1.05);

    frontGyro.attach(0x68, 1.065);
    centerGyro.attach(0x69, 1.097);

    bool allGyroReady = frontGyro.ready && centerGyro.ready;
    // bool allGyroReady = frontGyro.ready;
    // bool allGyroReady = centerGyro.ready;
    driver.useGyro(allGyroReady);

    delay(1000);
}

void loop()
{
    frontGyro.tick();
    centerGyro.tick();
    driver.tick();

    float yaw = (frontGyro.yaw + centerGyro.yaw) / 2;
    // float yaw = frontGyro.yaw;
    // float yaw = centerGyro.yaw;
    driver.sendYaw(yaw);

    // Serial.print("front: ");
    // Serial.print(frontGyro.yaw);
    // Serial.print(" , center: ");
    // Serial.print(centerGyro.yaw);
    // Serial.print(" , average: ");
    // Serial.print(yaw);
    // Serial.println();

    if (!driver.isReady()) return;
    if (taskQueue.length() == 0) return;
    char task = taskQueue[0];
    taskQueue.remove(0, 1);
    if (task == 'w') driver.moveForward();
    if (task == 'a') driver.turnLeft();
    if (task == 's') driver.moveBackward();
    if (task == 'd') driver.turnRight();
    if (task == '_') driver.delay(1000);
}