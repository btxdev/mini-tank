#include <NewPing.h>
 
#define SNR_FRONT_TRIG   8
#define SNR_FRONT_ECHO   9
#define SNR_BACK_TRIG    10
#define SNR_BACK_ECHO    11
#define SNR_LEFT_TRIG    12
#define SNR_LEFT_ECHO    13
#define SNR_RIGHT_TRIG   14
#define SNR_RIGHT_ECHO   15
#define SNR_MAX_DISTANCE 400
 
NewPing sonarFront(SNR_FRONT_TRIG, SNR_FRONT_ECHO, SNR_MAX_DISTANCE);
NewPing sonarBack(SNR_BACK_TRIG, SNR_BACK_ECHO, SNR_MAX_DISTANCE);
NewPing sonarLeft(SNR_LEFT_TRIG, SNR_LEFT_ECHO, SNR_MAX_DISTANCE);
NewPing sonarRight(SNR_RIGHT_TRIG, SNR_RIGHT_ECHO, SNR_MAX_DISTANCE);

void setup() {
  Serial.begin(9600);
  Serial.println("Sonar");
}
 
void loop() {
  delay(100);
  Serial.print(sonarFront.ping_cm());
  Serial.print(" ");
  Serial.print(sonarBack.ping_cm());
  Serial.print(" ");
  Serial.print(sonarLeft.ping_cm());
  Serial.print(" ");
  Serial.print(sonarRight.ping_cm());
  Serial.println("");
}