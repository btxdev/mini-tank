#include "AmperkaServo.h"

float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint8_t AmperkaServo::attach(int pin, int minPulseWidth, int maxPulseWidth,
                             int minAngle, int maxAngle) {
    _pin = pin;
    _minPulseWidth = minPulseWidth;
    _maxPulseWidth = maxPulseWidth;
    _minAngle = minAngle;
    _maxAngle = maxAngle;
    Servo::attach(pin, minPulseWidth, maxPulseWidth);
}

void AmperkaServo::attachFB(int pinFB, float voltageFBCW, float voltageFBCCW) {
    _pinFB = pinFB;
    _bits = 10;
    _voltageFBCW = voltageFBCW;
    _voltageFBCCW = voltageFBCCW;
}

void AmperkaServo::writeAngle(int angle) {
    int value
        = map(angle, _minAngle, _maxAngle, _minPulseWidth, _maxPulseWidth);
    Servo::writeMicroseconds(value);
}

void AmperkaServo::writeSpeed(int speed) {
    int value
        = map(speed, 255, -255, _minPulseWidth, _maxPulseWidth);
    Servo::writeMicroseconds(value);
}

float AmperkaServo::readVoltageFB() {
    int valueADC = analogRead(_pinFB);
    float valueVoltage = valueADC * BOARD_OPERATING_VOLTAGE / pow(2, _bits);
    return valueVoltage;
}

int AmperkaServo::readPulseFB() {
    float valueVoltage = readVoltageFB();
    if(_voltageFBCW < _voltageFBCCW) {
      if(valueVoltage < _voltageFBCW) {
        valueVoltage = _voltageFBCW;
      } else if (valueVoltage > _voltageFBCCW) {
        valueVoltage = _voltageFBCCW;
      }
    } else {
      if(valueVoltage < _voltageFBCCW) {
        valueVoltage = _voltageFBCCW;
      } else if(valueVoltage > _voltageFBCW) {
        valueVoltage = _voltageFBCW;
      }
    }
    uint16_t pulseWidth = fmap(valueVoltage, _voltageFBCW, _voltageFBCCW,
                               _minPulseWidth, _maxPulseWidth);                         
    return pulseWidth;
}

int AmperkaServo::readAngleFB() {
    uint16_t pulseWidth = readPulseFB();
    uint16_t angle
        = map(pulseWidth, _minPulseWidth, _maxPulseWidth, _minAngle, _maxAngle);
    return angle;
}

void AmperkaServo::setAnalogReadResolution(uint8_t bits) { _bits = bits; }
