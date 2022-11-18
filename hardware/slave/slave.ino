#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#ifndef APSSID
#define APSSID "Demolisher"
#define APPSK  "QwerZ123"
#endif

const char *ssid = APSSID;
const char *password = APPSK;

AsyncWebServer server(80);

void handleRoot(AsyncWebServerRequest *request) {
  request->send(200, "text/html", "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'><meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=0'><title>Controller</title><style>body{margin:0;padding:0}.mark-red{background-color:rgba(255,0,0,.1);border:1px solid red}.mark-cyan{background-color:rgba(0,255,255,.1);border:1px solid #0ff}.unselectable{-webkit-touch-callout:none;-webkit-user-select:none;-khtml-user-select:none;-moz-user-select:none;-ms-user-select:none;user-select:none}.buttons-wrapper{position:absolute;width:90%;height:auto;bottom:20px;left:0;right:0;margin:auto}.button,.wrapper{width:100%;height:100%}.box{width:100%;position:relative}.box:before{content:'';display:block;padding-top:100%}.box .content{position:absolute;top:0;left:0;bottom:0;right:0;display:grid;grid-template-columns:33% 33% 33%;grid-template-rows:33% 33% 33%}.wrapper{padding:5%;box-sizing:border-box}.button{border-radius:12%;background-color:#7a8ca0;color:#fff;font-family:Arial;font-size:600%;line-height:92%;text-align:center}.active{background-color:#363a3a}.btn-left{grid-area:2/1/2/1;transform:rotate(-90deg)}.btn-right{grid-area:2/3/2/3;transform:rotate(90deg)}.btn-bottom{grid-area:3/2/3/2;transform:rotate(180deg)}.btn-top{grid-area:1/2/1/2}</style><script type='application/javascript'>function isTouchscreen(){return!!window.matchMedia('(pointer:coarse)').matches}function bind(t,e,o){var t=document.querySelector(t),n=isTouchscreen?'touchend':'mouseup';t.addEventListener(isTouchscreen?'touchstart':'mousedown',t=>{t.target.classList.add('active'),e()}),t.addEventListener(n,t=>{t.target.classList.remove('active'),o()})}document.addEventListener('DOMContentLoaded',()=>{bind('.btn-left .button',()=>{console.log('turn left'),fetch('/controller?btn=left&state=1')},()=>{console.log('stop'),fetch('/controller?btn=left&state=0')}),bind('.btn-right .button',()=>{fetch('/controller?btn=right&state=1')},()=>{fetch('/controller?btn=right&state=0')}),bind('.btn-top .button',()=>{fetch('/controller?btn=top&state=1')},()=>{fetch('/controller?btn=top&state=0')}),bind('.btn-bottom .button',()=>{fetch('/controller?btn=bottom&state=1')},()=>{fetch('/controller?btn=bottom&state=0')})});</script></head><body><div class='buttons-wrapper'><div class='box'><div class='content'><div class='wrapper btn-left'><div class='button unselectable'>&#8593;</div></div><div class='wrapper btn-right'><div class='button unselectable'>&#8593;</div></div><div class='wrapper btn-top'><div class='button unselectable'>&#8593;</div></div><div class='wrapper btn-bottom'><div class='button unselectable'>&#8593;</div></div></div></div></div></body></html>");
}

void handleController(AsyncWebServerRequest *request) {
  if(request->hasParam("btn") && request->hasParam("state")) {
    String btn = request->getParam("btn")->value();
    bool state = request->getParam("state")->value() == "1";
    if(btn == "left") {
      Serial.print(1);
      Serial.println(state);
    }
    if(btn == "right") {
      Serial.print(2);
      Serial.println(state);
    }
    if(btn == "top") {
      Serial.print(3);
      Serial.println(state);
    }
    if(btn == "bottom") {
      Serial.print(4);
      Serial.println(state);
    }
    request->send(200, "application/json", "{'status': 'ok'}");
  }
  else {
    request->send(200, "application/json", "{'status': 'error', 'msg': 'wrong parameter'}");
  }
}

void setup() {
  delay(1000);
  Serial.begin(9600);
  Serial.print(1); Serial.println(0);
  Serial.print(2); Serial.println(0);
  Serial.print(3); Serial.println(0);
  Serial.print(4); Serial.println(0);

  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(gateway, gateway, subnet);
  delay(1000);
  WiFi.softAP(ssid, password);
  delay(1000);

  IPAddress resultIP = WiFi.softAPIP();
  delay(1000);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/controller", HTTP_GET, handleController);
  server.begin();
}

void loop() {}