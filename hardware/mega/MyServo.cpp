#include "MyServo.h"

uint8_t MyServo::attach(int pin, int minPulseWidth, int maxPulseWidth,
                             int minAngle, int maxAngle) {
    _summaryAngle = 0;
    AmperkaServo::attach(pin, minPulseWidth, maxPulseWidth, minAngle, maxAngle);
}