#include <Wire.h>
#include <MPU6050.h>

// MPU настройки
#define MPU_CENTER_ADDR 0x68
#define MPU_FRONT_ADDR 0x69
// частота замеров
// при значении меньшем, чем время итерации loop, расчеты будут неверны
#define MPU_GYRO_PERIOD_MS 50
#define MPU_GYRO_PERIOD_S 0.05

// чувствительность акселерометра, меньше = чувствительнее
// #define MPU_ACCEL_RANGE MPU6050_RANGE_2G
#define MPU_ACCEL_RANGE MPU6050_RANGE_4G
// #define MPU_ACCEL_RANGE MPU6050_RANGE_8G
// #define MPU_ACCEL_RANGE MPU6050_RANGE_16G

// лимит гироскопа (deg/s)
#define MPU_GYRO_RANGE MPU6050_SCALE_250DPS
// #define MPU_GYRO_RANGE MPU6050_SCALE_500DPS
// #define MPU_GYRO_RANGE MPU6050_SCALE_1000DPS
// #define MPU_GYRO_RANGE MPU6050_SCALE_2000DPS

MPU6050 mpuCenter;
MPU6050 mpuFront;

uint32_t mpuGyroTimer = 0;

void setup() {
  // debug
  Serial.begin(115200);

  // MPU6050
  Serial.println(F("Initializing Center MPU6050..."));
  while(!mpuCenter.begin(MPU_GYRO_RANGE, MPU_ACCEL_RANGE, MPU_CENTER_ADDR))
  {
    Serial.println(F("Could not find a valid MPU6050 sensor (center), check wiring!"));
    delay(500);
  }

  Serial.println(F("Calibrating center gyroscope..."));
  mpuCenter.calibrateGyro();

  Serial.println(F("Initializing Front MPU6050..."));
  while(!mpuFront.begin(MPU_GYRO_RANGE, MPU_ACCEL_RANGE, MPU_FRONT_ADDR))
  {
    Serial.println(F("Could not find a valid MPU6050 sensor (front), check wiring!"));
    delay(500);
  }

  Serial.println(F("Calibrating front gyroscope..."));
  mpuFront.calibrateGyro();

  Serial.println(F("Gyroscopes are ready!"));

  Serial.println(F("loop is running:"));
}

void loop() 
{
  // static
  // MPU6050 pitch, roll and yaw values
  static float centerPitch = 0;
  static float centerRoll = 0;
  static float centerYaw = 0;

  static float frontPitch = 0;
  static float frontRoll = 0;
  static float frontYaw = 0;

  // check gyro
  if((millis() - mpuGyroTimer) > MPU_GYRO_PERIOD_MS) {
    mpuGyroTimer = millis();

    // center gyroscope
    Vector normCenter = mpuCenter.readNormalizeGyro();
    // calculate pitch, roll and yaw
    centerPitch += normCenter.YAxis * MPU_GYRO_PERIOD_S;
    centerRoll += normCenter.XAxis * MPU_GYRO_PERIOD_S;
    centerYaw += normCenter.ZAxis * MPU_GYRO_PERIOD_S;
    // output
    Serial.print(centerPitch);
    Serial.print(F(" "));
    Serial.print(centerRoll);
    Serial.print(F(" "));
    Serial.print(centerYaw);
    Serial.print(F(" "));

    // front gyroscope
    Vector normFront = mpuFront.readNormalizeGyro();
    // calculate pitch, roll and yaw
    frontPitch += normFront.YAxis * MPU_GYRO_PERIOD_S;
    frontRoll += normFront.XAxis * MPU_GYRO_PERIOD_S;
    frontYaw += normFront.ZAxis * MPU_GYRO_PERIOD_S;
    // output
    Serial.print(frontPitch);
    Serial.print(F(" "));
    Serial.print(frontRoll);
    Serial.print(F(" "));
    Serial.print(frontYaw);
    Serial.print(F(" "));

    Serial.println();
  }

}