#ifndef BTXDEV_DRIVER_USED_
#define BTXDEV_DRIVER_USED_

#include "Arduino.h"
#include "Motor.h"

// #define _DRV_ROT_MM 140
// #define _DRV_ROT_MM 165
#define _DRV_ROT_MM 145
#define _DRV_MOVE_MM 300

#define _DRVST_TLEFT 1
#define _DRVST_TRIGHT 2
#define _DRVST_MFORWARD 3
#define _DRVST_MBACKWARD 4

class Driver
{
    public:
        Motor leftMotor;
        Motor rightMotor;

        Driver() {}

        void attach(int leftOutput, int leftFeedback, float leftMultiplier,
                    int rightOutput, int rightFeedback, float rightMultiplier)
        {
            _leftSpeedMultiplier = leftMultiplier;
            _rightSpeedMultiplier = rightMultiplier;
            leftMotor.attach(leftOutput, leftFeedback);
            rightMotor.attach(rightOutput, rightFeedback);
        }

        Driver(int leftOutput, int leftFeedback, float leftMultiplier, 
               int rightOutput, int rightFeedback, float rightMultiplier)
        {
            Driver::attach(leftOutput, leftFeedback, leftMultiplier,
                           rightOutput, rightFeedback, rightMultiplier);
        }

        void setSpeed(int8_t speed)
        {
            _speed = speed;

            leftMotor.setMaxSpeed(speed * _leftSpeedMultiplier);
            rightMotor.setMaxSpeed(speed * _rightSpeedMultiplier);
        }

        void usePID(bool state)
        {
            _usePID = state;
        }

        void _turnLeftByDegrees()
        {
            if (leftMotor.waFlag)
            {
                rightMotor.writeSpeed(-_speed * _rightSpeedMultiplier);
            }
            else
            {
                rightMotor.writeSpeed(0);
                _state = 0;
            }
        }

        void _turnLeftByGyro()
        {
            leftMotor.infiniteRun();
            rightMotor.infiniteRun();
            if(abs(_actualYaw - _previousYaw) >= 90) {
                Serial.println("end left");
                _state = 0;
                leftMotor.infiniteStop();
                rightMotor.infiniteStop();
            }
        }

        void _turnRightByDegrees()
        {
            if (leftMotor.waFlag)
            {
                rightMotor.writeSpeed(_speed * _rightSpeedMultiplier);
            }
            else
            {
                rightMotor.writeSpeed(0);
                _state = 0;
            }
        }

        void _turnRightByGyro()
        {
            leftMotor.infiniteReverse();
            rightMotor.infiniteReverse();
            if(abs(_actualYaw - _previousYaw) >= 90) {
                Serial.println("end right");
                _state = 0;
                leftMotor.infiniteStop();
                rightMotor.infiniteStop();
            }
        }

        void tick()
        {
            leftMotor.tick();
            rightMotor.tick();

            if (_state == _DRVST_TLEFT)
            {
                if (_gyroAttached) _turnLeftByGyro();
                else _turnLeftByDegrees();
            }

            if (_state == _DRVST_TRIGHT)
            {
                if (_gyroAttached) _turnRightByGyro();
                else _turnRightByDegrees();
            }

            if (_state == _DRVST_MFORWARD)
            {
                if (leftMotor.waFlag)
                {
                    rightMotor.writeSpeed(-_speed * _rightSpeedMultiplier);
                }
                else
                {
                    rightMotor.writeSpeed(0);
                    _state = 0;
                }
            }

            if (_state == _DRVST_MBACKWARD)
            {
                if (leftMotor.waFlag)
                {
                    rightMotor.writeSpeed(_speed * _rightSpeedMultiplier);
                }
                else
                {
                    rightMotor.writeSpeed(0);
                    _state = 0;
                }
            }

            if (_state == 0)
            {
                leftMotor.infiniteStop();
                rightMotor.infiniteStop();
            }

            // if (_gyroAttached && _usePID)
            // {
            //     float p = 
            // }

        }

        bool isReady()
        {
            return _state == 0;
        }

        void turnLeft()
        {
            if (_state == _DRVST_TLEFT) return;
            _previousYaw = _actualYaw;
            _state = _DRVST_TLEFT;
        }

        void turnRight()
        {
            if (_state == _DRVST_TRIGHT) return;
            _previousYaw = _actualYaw;
            _state = _DRVST_TRIGHT;
        }

        void moveForward(int16_t mm)
        {
            if (_state == _DRVST_MFORWARD) return;
            leftMotor.writeDistance(mm);
            _state = _DRVST_MFORWARD;
        }

        void moveForward()
        {
            moveForward(_DRV_MOVE_MM);
        }

        void moveBackward(int16_t mm)
        {
            if (_state == _DRVST_MBACKWARD) return;
            leftMotor.writeDistance(-mm);
            _state = _DRVST_MBACKWARD;
        }

        void moveBackward()
        {
            moveBackward(_DRV_MOVE_MM);
        }

        void useGyro(bool gyroStatus)
        {
            _gyroAttached = gyroStatus;
        }

        void sendYaw(float yaw)
        {
            _actualYaw = yaw;
        }

    private:
        int8_t _speed = 0;
        uint8_t _state = 0;

        float _leftSpeedMultiplier = 1;
        float _rightSpeedMultiplier = 1;

        bool _gyroAttached = false;
        float _actualYaw = 0;
        float _previousYaw = 0;

        bool _usePID = false;
        float _kp = 1;
        float _ki = 0;
        float _kd = 0;
        float _dt = 0.05;
        float _integralYaw = 0;

};

#endif