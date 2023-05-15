#ifndef BTXDEV_GYRO_USED_
#define BTXDEV_GYRO_USED_

#include <Arduino.h>
#include "src/MPU6050/MPU6050.h"

#define _MPU_GYRO_PERIOD_MS 200
#define _MPU_CALIBRATE

// чувствительность акселерометра, меньше = точнее
// #define _MPU_ACCEL_RANGE MPU6050_RANGE_2G
// #define _MPU_ACCEL_RANGE MPU6050_RANGE_4G
// #define _MPU_ACCEL_RANGE MPU6050_RANGE_8G
#define _MPU_ACCEL_RANGE MPU6050_RANGE_16G

// лимит гироскопа (deg/s)
#define _MPU_GYRO_RANGE MPU6050_SCALE_250DPS
// #define _MPU_GYRO_RANGE MPU6050_SCALE_500DPS
// #define _MPU_GYRO_RANGE MPU6050_SCALE_1000DPS
// #define _MPU_GYRO_RANGE MPU6050_SCALE_2000DPS

class Gyro : public MPU6050
{
    public:

        Gyro() : MPU6050() {}

        void attach(int address, float multiplier)
        {
            ready = MPU6050::begin(_MPU_GYRO_RANGE, _MPU_ACCEL_RANGE, address);
            _multiplier = multiplier;
            if (!ready)
            {
                Serial.println(" <!> gyroscope error");
                return;
            }

            #ifdef _MPU_CALIBRATE
            MPU6050::calibrateGyro();
            #endif
        }

        void tick()
        {
            if (!ready) return;

            if((millis() - _gyroTimer) > _MPU_GYRO_PERIOD_MS) {
                
                Vector normalized = MPU6050::readNormalizeGyro();
                float dt = (float)(millis() - _gyroTimer) / 1000.00;

                _pitch += normalized.YAxis * dt;
                _roll += normalized.XAxis * dt;
                _yaw += normalized.ZAxis * dt;

                pitch = _pitch * _multiplier;
                roll = _roll * _multiplier;
                yaw = _yaw * _multiplier;

                _gyroTimer = millis();
            }
        }

        float pitch = 0;
        float roll = 0;
        float yaw = 0;

        bool ready;

    private:
        float _pitch = 0;
        float _roll = 0;
        float _yaw = 0;
        uint32_t _gyroTimer = millis();
        float _multiplier = 1;

};

#endif