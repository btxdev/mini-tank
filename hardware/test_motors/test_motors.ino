#include "Motor.h"
#include "src/Timer/timer.h"
#include "src/Timer/timerManager.h"


// серво настройки
#define LEFT_SERVO_PIN 2
#define LEFT_SERVO_PIN_FB A14

#define RIGHT_SERVO_PIN 3
#define RIGHT_SERVO_PIN_FB A15


Motor leftServo;
Motor rightServo;

Timer frame1Timer;
// Timer frame2Timer;


void frame1()
{
    leftServo.writeDistance(155);
}

// void frame2()
// {
//     leftServo.writeAngle(-90);
//     rightServo.writeAngle(-90);
// }
// Timer rotateTimer;


// void rotateCallback()
// {
//     leftServo.writeAngle(180);
//     // Serial.println(leftServo.degrees);
//     // Serial.println(analogRead(LEFT_SERVO_PIN_FB));
//     // Serial.println(leftServo.readVoltageFB());
//     // Serial.println(leftServo.degrees);
// }


void setup()
{
    Serial.begin(115200);

    // pinMode(LEFT_SERVO_PIN_FB, INPUT);
    leftServo.attach(LEFT_SERVO_PIN, LEFT_SERVO_PIN_FB);
    rightServo.attach(RIGHT_SERVO_PIN, RIGHT_SERVO_PIN_FB);
    leftServo.setMaxSpeed(70);
    rightServo.setMaxSpeed(70);

    frame1Timer.setTimeout(4000);
    frame1Timer.setCallback(frame1);

    // frame2Timer.setTimeout(20000);
    // frame2Timer.setCallback(frame2);

    // rotateTimer.setInterval(8000);
    // rotateTimer.setInterval(50);
    // rotateTimer.setCallback(rotateCallback);
    // rotateTimer.start();

    TimerManager::instance().start();
}

void loop()
{
    TimerManager::instance().update();
    leftServo.tick();
    rightServo.tick();
    if (leftServo.waFlag)
    {
        rightServo.writeSpeed(70);
    }
    else
    {
        rightServo.writeSpeed(0);
    }
}