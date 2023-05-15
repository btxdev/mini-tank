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
#define _DRVST_DELAY 5

#define _PID_DT_MS 50
#define _PID_DT 0.05

class Driver
{
    public:
        Motor leftMotor;
        Motor rightMotor;

        Driver() {}

        void attach(int leftOutput, int leftFeedback,
                    int rightOutput, int rightFeedback)
        {
            leftMotor.attach(leftOutput, leftFeedback);
            rightMotor.attach(rightOutput, rightFeedback);
        }

        void attach(int leftOutput, int leftFeedback, float leftMultiplier,
                    int rightOutput, int rightFeedback, float rightMultiplier)
        {
            // _leftSpeedMultiplier = leftMultiplier;
            // _rightSpeedMultiplier = rightMultiplier;
            leftMotor.attach(leftOutput, leftFeedback);
            rightMotor.attach(rightOutput, rightFeedback);
            leftMotor.speedMultiplier = leftMultiplier;
            rightMotor.speedMultiplier = rightMultiplier;
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

            // leftMotor.setMaxSpeed(speed * _leftSpeedMultiplier);
            // rightMotor.setMaxSpeed(speed * _rightSpeedMultiplier);
            leftMotor.setMaxSpeed(speed);
            rightMotor.setMaxSpeed(speed);
        }

        void usePID(bool state)
        {
             _usePID = state;
        }

        void usePID(bool state, float kp, float ki, float kd)
        {
            _usePID = state;
            _kp = kp;
            _ki = ki;
            _kd = kd;
        }

        void resetPID()
        {
            _PID_integral = 0;
            _previousYaw = _actualYaw;
        }

        void useMultiplier(bool state)
        {
            leftMotor.useMultiplier = state;
            rightMotor.useMultiplier = state;
        }

        void _turnLeftByDegrees()
        {
            useMultiplier(true);
            if (leftMotor.waFlag)
            {
                // rightMotor.writeSpeed(-_speed * _rightSpeedMultiplier);
                rightMotor.writeSpeed(-_speed);
            }
            else
            {
                rightMotor.writeSpeed(0);
                _state = 0;
            }
        }

        void _turnLeftByGyro()
        {
            useMultiplier(false);
            leftMotor.infiniteReverse();
            rightMotor.infiniteReverse();
            if(abs(_actualYaw - _previousYaw) >= 90) {
                _state = 0;
                leftMotor.infiniteStop();
                rightMotor.infiniteStop();
            }
        }

        void _turnRightByDegrees()
        {
            useMultiplier(true);
            if (leftMotor.waFlag)
            {
                // rightMotor.writeSpeed(_speed * _rightSpeedMultiplier);
                rightMotor.writeSpeed(_speed);
            }
            else
            {
                rightMotor.writeSpeed(0);
                _state = 0;
            }
        }

        void _turnRightByGyro()
        {
            useMultiplier(false);
            leftMotor.infiniteRun();
            rightMotor.infiniteRun();
            if(abs(_actualYaw - _previousYaw) >= 90) {
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
                    // rightMotor.writeSpeed(-_speed * _rightSpeedMultiplier);
                    rightMotor.writeSpeed(-_speed);
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
                    // rightMotor.writeSpeed(_speed * _rightSpeedMultiplier);
                    rightMotor.writeSpeed(_speed);
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

            if (_gyroAttached && _usePID && (millis() - _tPID) > _PID_DT_MS)
            {
                _tPID = millis();
                float error = _previousYaw - _actualYaw;
                float proportional = error * _kp;
                _PID_integral += error * _ki * _PID_DT;
                float derivative = -_kd * _actualYaw / _PID_DT;
                float PID = proportional + _PID_integral + derivative;
                if (_state == _DRVST_MBACKWARD) PID *= -1;
                leftMotor.speedMultiplier = 1.00 + PID * 0.1;
                rightMotor.speedMultiplier = 1.00 - PID * 0.1;
                // rightMotor.speedMultiplier = 0;
                // rightMotor.useMultiplier = true;
                // leftMotor.setMaxSpeed(_leftSpeedMultiplier + PID * 0.1);
                // rightMotor.setMaxSpeed(_rightSpeedMultiplier - PID * 0.1);
                // _rightSpeedMultiplier = 1.00 - PID / 2;
                // _yaw_i = _yaw_i * 0.5 + _actualYaw;
                // float d = _yaw_d * _kd;
                // _yaw_d = _actualYaw;
                // Serial.print("error: ");
                // Serial.print(error);
                // Serial.print(", p: ");
                // Serial.print(proportional);
                // Serial.print(", i: ");
                // Serial.print(_PID_integral);
                // Serial.print(", d: ");
                // Serial.print(derivative);
                // Serial.print(", PID: ");
                // Serial.print(PID);
                // Serial.println();
                // Serial.println(rightMotor.useMultiplier);
            }

            if (_state == _DRVST_DELAY)
            {
                if ((millis() - _delayTimer) > _delayTime)
                {
                    _state = 0;
                }
                // do shit
            }

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
            useMultiplier(true);
            _previousYaw = _actualYaw;
            _PID_integral = 0;
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
            useMultiplier(true);
            _previousYaw = _actualYaw;
            _PID_integral = 0;
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

        void delay(int ms)
        {
            _state = _DRVST_DELAY;
            _delayTimer = millis();
            _delayTime = ms;
        }

    private:
        int8_t _speed = 0;
        uint8_t _state = 0;

        uint32_t _delayTimer = millis();
        uint16_t _delayTime = 1000;

        // float _leftSpeedMultiplier = 1;
        // float _rightSpeedMultiplier = 1;

        bool _gyroAttached = false;
        float _actualYaw = 0;
        float _previousYaw = 0;

        bool _usePID = false;
        uint32_t _tPID = millis();
        float _kp = 1;
        float _ki = 0;
        float _kd = 0;
        float _PID_integral = 0;
        // float _yaw_d = 0;
};

#endif