#ifndef BTXDEV_SERVO_USED_
#define BTXDEV_SERVO_USED_

#include <Arduino.h>
#include "src/AmperkaServo/AmperkaServo.h"

class Motor : public AmperkaServo
{
    public:
        int32_t degrees = 0;
        bool waFlag = false;

        Motor() : AmperkaServo() {}

        uint16_t readAngle()
        {
            uint16_t raw = analogRead(_feedbackPin);
            float angle = ((float) raw - 7) / 678 * 360;
            return (uint16_t) angle;
        }

        void attach(int outputAnalogPin, int feedbackAnalogPin)
        {
            AmperkaServo::attach(outputAnalogPin, 500, 2500);
            AmperkaServo::attachFB(feedbackAnalogPin, 0, 2.5);
            _feedbackPin = feedbackAnalogPin;
            _prevAngle = readAngle();
        }

        Motor(int outputAnalogPin, int feedbackAnalogPin) : AmperkaServo()
        {
            Motor::attach(outputAnalogPin, feedbackAnalogPin);
        }

        int8_t speedLimiter(int8_t speed)
        {
            if (speed > 100) speed = 100;
            if (speed < -100) speed = -100;
            return speed;
        }

        void setSpeed(int8_t speed)
        {
            _speed = speedLimiter(speed);
        }

        void setMaxSpeed(int8_t speed)
        {
            _maxSpeed = speedLimiter(speed);
        }

        void tick()
        {
            if ((millis() - _angleTimer) > 2)
            {
                uint16_t rawAngle = readAngle();
                int16_t diff = _prevAngle - rawAngle;
                if (abs(diff) > 100) diff = 0;
                degrees += diff;
                _prevAngle = rawAngle;
                _angleTimer = millis();
            }

            if (waFlag)
            {
                int32_t targetAngle = _waPrevAngle + _waTargetAngle;
                int32_t distance = degrees - targetAngle;
                int8_t direction = 1;
                if (degrees > targetAngle) direction = -1;
                float speed = abs(_maxSpeed) * direction;
                if (useMultiplier) speed *= speedMultiplier;

                AmperkaServo::writeSpeed(speed);
                if(abs(distance) < 10)
                {
                    waFlag = false;
                    AmperkaServo::writeSpeed(0);
                }
            }
            else
            {
                Motor::writeSpeed(_speed);
            }
        }

        void writeSpeed(float speed)
        {
            float multiplier = 1;
            if (useMultiplier) multiplier = speedMultiplier;
            float newSpeed = (float)speed * 2.55 * multiplier;
            AmperkaServo::writeSpeed(newSpeed);
        }

        void writeAngle(int16_t angle)
        {
            if (!waFlag)
            {
                _waTargetAngle = angle;
                _waPrevAngle = degrees;
                waFlag = true;
            }
        }

        void writeDistance(int16_t mm)
        {
            float distance = (float) mm / 99.21 * 360;
            writeAngle(distance);
        }

        void infiniteRun()
        {
            waFlag = false;
            _speed = _maxSpeed;
        }

        void infiniteReverse()
        {
            waFlag = false;
            _speed = -_maxSpeed;
        }

        void infiniteStop()
        {
            waFlag = false;
            _speed = 0;
            AmperkaServo::writeSpeed(0);
        }

        float speedMultiplier = 1;
        bool useMultiplier = false;
    
    private:
        uint8_t _feedbackPin;
        int8_t _speed = 0;
        int8_t _maxSpeed = 0;
        uint16_t _prevAngle = 0;
        uint32_t _angleTimer = millis();
        int32_t _waPrevAngle = 0;
        int16_t _waTargetAngle = 0;
};

#endif