#ifndef __MY_SERVO_H__
#define __MY_SERVO_H__

#include <Arduino.h>
#include <AmperkaServo.h>

class MyServo : public AmperkaServo {
public:
    MyServo()
        : Servo() { }
    uint8_t attach(int pin, int minPulseWidth = 544, int maxPulseWidth = 2400,
                   int minAngle = 0, int maxAngle = 180);
    void writeAngle(int angle);
    void writeSpeed(int speed);
    int getMinPulseWidth() { return _minPulseWidth; };
    int getMaxPulseWidth() { return _maxPulseWidth; };
    int getMinAngle() { return _minAngle; };
    int getMaxAngle() { return _maxAngle; };
    int getMidAngle() { return (_minAngle + _maxAngle) / 2; };
    void attachFB(int pinFB, float voltageFBCW, float voltageFBCCW);
    float readVoltageFB();
    int readPulseFB();
    int readAngleFB();
    int readSpeedFB();
    void setAnalogReadResolution(uint8_t bits = 10);

private:
    int _summaryAngle;
    int _
};

#endif // __MY_SERVO_H__