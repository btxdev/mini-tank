#ifndef BTXDEV_GYRO_USED_
#define BTXDEV_GYRO_USED_

#include <Arduino.h>
#include "src/MPU6050/MPU6050.h"

#define _MPU_GYRO_PERIOD_MS 50
#define _MPU_GYRO_PERIOD_S 0.05
#define _MPU_CALIBRATE

// чувствительность акселерометра, меньше = точнее
#define _MPU_ACCEL_RANGE MPU6050_RANGE_2G
// #define _MPU_ACCEL_RANGE MPU6050_RANGE_4G
// #define _MPU_ACCEL_RANGE MPU6050_RANGE_8G
// #define _MPU_ACCEL_RANGE MPU6050_RANGE_16G

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
                _gyroTimer = millis();

                // центральный
                Vector normalized = MPU6050::readNormalizeGyro();

                _pitch += normalized.YAxis * _MPU_GYRO_PERIOD_S;
                _roll += normalized.XAxis * _MPU_GYRO_PERIOD_S;
                _yaw += normalized.ZAxis * _MPU_GYRO_PERIOD_S;

                pitch = _pitch * _multiplier;
                roll = _roll * _multiplier;
                yaw = _yaw * _multiplier;

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