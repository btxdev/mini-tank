#ifndef BTXDEV_GYRO_USED_
#define BTXDEV_GYRO_USED_

#include <Arduino.h>
#include "src/MPU6050/MPU6050.h"

#define _MPU_GYRO_PERIOD_MS 50
#define _MPU_GYRO_PERIOD_S 0.05
#define _MPU_CALIBRATE

// чувствительность акселерометра, меньше = точнее
// #define _MPU_ACCEL_RANGE MPU6050_RANGE_2G
#define _MPU_ACCEL_RANGE MPU6050_RANGE_4G
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

        void attach(int address)
        {
            _ready = MPU6050::begin(_MPU_GYRO_RANGE, _MPU_ACCEL_RANGE, address);
            #ifdef _MPU_CALIBRATE
            MPU6050::calibrateGyro();
            #endif
        }

        void tick()
        {
            if((millis() - _gyroTimer) > _MPU_GYRO_PERIOD_S) {
                _gyroTimer = millis();

                // центральный
                Vector normalized = MPU6050::readNormalizeGyro();

                pitch += normalized.YAxis * _MPU_GYRO_PERIOD_S;
                roll += normalized.XAxis * _MPU_GYRO_PERIOD_S;
                yaw += normalized.ZAxis * _MPU_GYRO_PERIOD_S;

            }
        }

        float pitch = 0;
        float roll = 0;
        float yaw = 0;

    private:
        bool _ready;
        uint32_t _gyroTimer = millis();

};

#endif