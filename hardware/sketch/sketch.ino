#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <AmperkaServo.h>

#ifndef APSSID
#define APSSID "wemos wifi"
#define APPSK  "zalupazalupa"
#endif

#define LEFT_SERVO_PIN D6
#define LEFT_SERVO_PIN_FB A0
#define RIGHT_SERVO_PIN D7
#define RIGHT_SERVO_PIN_FB A0

#define MCS_MIN 500
#define MCS_MAX 2500
#define FEEDBACK_MIN 0.247
#define FEEDBACK_MAX 3.063 

const uint8_t speed = 255;

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

ESP8266WebServer server(80);

AmperkaServo leftServo;
AmperkaServo rightServo;

// void stop(uint16_t milliseconds) {
//   leftServo.writeSpeed(0);
//   rightServo.writeSpeed(0);
//   delay(milliseconds);
// }

// void stop() {
//   stop(0);
// }

// void moveForward(uint16_t milliseconds) {
//   leftServo.writeSpeed(speed);
//   rightServo.writeSpeed(speed);
//   delay(milliseconds);
//   stop();
// }

// void moveBackward(uint16_t milliseconds) {
//   leftServo.writeSpeed(-speed);
//   rightServo.writeSpeed(-speed);
//   delay(milliseconds);
//   stop();
// }

// void turnLeft(uint16_t milliseconds) {
//   leftServo.writeSpeed(speed);
//   rightServo.writeSpeed(-speed);
//   delay(milliseconds);
//   stop();
// }

// void turnRight(uint16_t milliseconds) {
//   leftServo.writeSpeed(-speed);
//   rightServo.writeSpeed(speed);
//   delay(milliseconds);
//   stop();
// }

void handleRoot() {
  server.send(200, "text/html", "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'><meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=0'><title>Controller</title><style>body{margin:0;padding:0}.mark-red{background-color:rgba(255,0,0,.1);border:1px solid red}.mark-cyan{background-color:rgba(0,255,255,.1);border:1px solid #0ff}.buttons-wrapper{position:absolute;width:90%;height:auto;bottom:20px;left:0;right:0;margin:auto}.button,.wrapper{width:100%;height:100%}.box{width:100%;position:relative}.box:before{content:'';display:block;padding-top:100%}.box .content{position:absolute;top:0;left:0;bottom:0;right:0;display:grid;grid-template-columns:33% 33% 33%;grid-template-rows:33% 33% 33%}.wrapper{padding:5%;box-sizing:border-box}.button{border-radius:12%;background-color:#7a8ca0;color:#fff;font-family:Arial;font-size:600%;line-height:92%;text-align:center}.active{background-color:#363a3a}.btn-left{grid-area:2/1/2/1;transform:rotate(-90deg)}.btn-right{grid-area:2/3/2/3;transform:rotate(90deg)}.btn-bottom{grid-area:3/2/3/2;transform:rotate(180deg)}.btn-top{grid-area:1/2/1/2}</style><script type='application/javascript'>function isTouchscreen(){return!!window.matchMedia('(pointer:coarse)').matches}function bind(o,t,e){var o=document.querySelector(o),n=isTouchscreen?'touchend':'mouseup';o.addEventListener(isTouchscreen?'touchstart':'mousedown',o=>{o.target.classList.add('active'),t()}),o.addEventListener(n,o=>{o.target.classList.remove('active'),e()})}document.addEventListener('DOMContentLoaded',()=>{bind('.btn-left .button',()=>{console.log('turn left'),fetch('/left')},()=>{console.log('stop'),fetch('/stop')}),bind('.btn-right .button',()=>{console.log('turn right')},()=>{console.log('stop')}),bind('.btn-top .button',()=>{console.log('move forward')},()=>{console.log('stop')}),bind('.btn-bottom .button',()=>{console.log('move backward')},()=>{console.log('stop')})});</script></head><body><div class='buttons-wrapper'><div class='box'><div class='content'><div class='wrapper btn-left'><div class='button'>&#8593;</div></div><div class='wrapper btn-right'><div class='button'>&#8593;</div></div><div class='wrapper btn-top'><div class='button'>&#8593;</div></div><div class='wrapper btn-bottom'><div class='button'>&#8593;</div></div></div></div></div></body></html>");
}

void handleLeft() {
  server.send(200, "application/json", "{'status': 'ok'}");
  // turnLeft(1000);
}

void handleStop() {
  server.send(200, "application/json", "{'status': 'ok'}");
  // stop(1000);
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  Serial.println("Configuring access point...");
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(gateway, gateway, subnet);
  delay(1000);
  WiFi.softAP(ssid, password);
  delay(1000);

  IPAddress resultIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(resultIP);
  delay(1000);

  server.on("/", handleRoot);
  server.on("/left", handleLeft);
  server.on("/stop", handleStop);
  server.begin();
  Serial.println("HTTP server started");

  // pinMode(13, OUTPUT);

  leftServo.attach(LEFT_SERVO_PIN, MCS_MIN, MCS_MAX);
  // leftServo.attachFB(LEFT_SERVO_PIN_FB,FEEDBACK_MIN, FEEDBACK_MAX);

  rightServo.attach(RIGHT_SERVO_PIN, MCS_MIN, MCS_MAX);
  // rightServo.attachFB(RIGHT_SERVO_PIN_FB, FEEDBACK_MIN, FEEDBACK_MAX);

}

void loop() {
  server.handleClient();
}