#ifndef __AMPERKA_SERVO_H__
#define __AMPERKA_SERVO_H__

#include <Arduino.h>
#include <Servo.h>

#if defined(__AVR__)
#define BOARD_OPERATING_VOLTAGE 5.0
#else
#define BOARD_OPERATING_VOLTAGE 3.3
#endif

class AmperkaServo : public Servo {
public:
    AmperkaServo()
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
    int _pin;
    int _pinFB;
    int _minPulseWidth;
    int _maxPulseWidth;
    int _minAngle;
    int _maxAngle;
    float _voltageFBCW;
    float _voltageFBCCW;
    uint8_t _bits;
};

#endif // __AMPERKA_SERVO_H__