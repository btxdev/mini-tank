#include "Driver.h"


// серво настройки
#define LEFT_SERVO_PIN 2
#define LEFT_SERVO_PIN_FB A14
#define LEFT_SERVO_K 0.85

#define RIGHT_SERVO_PIN 3
#define RIGHT_SERVO_PIN_FB A15
#define RIGHT_SERVO_K 1.15


Driver driver;
String taskQueue = "w_a_w_d_w_d_d_w_a_w_d_w_";
// String taskQueue = "a_d_d_a___aa_dd";


void setup()
{
    Serial.begin(115200);

    driver.attach(LEFT_SERVO_PIN, LEFT_SERVO_PIN_FB, LEFT_SERVO_K, 
                  RIGHT_SERVO_PIN, RIGHT_SERVO_PIN_FB, RIGHT_SERVO_K);
    driver.setSpeed(70);

    delay(4000);
}

void loop()
{
    driver.tick();

    if (!driver.isReady()) return;
    if (taskQueue.length() == 0) return;
    char task = taskQueue[0];
    taskQueue.remove(0, 1);
    if (task == 'w') driver.moveForward();
    if (task == 'a') driver.turnLeft();
    if (task == 's') driver.moveBackward();
    if (task == 'd') driver.turnRight();
    if (task == '_') delay(1000);
}