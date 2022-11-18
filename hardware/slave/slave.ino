#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#ifndef APSSID
#define APSSID "Demolisher"
#define APPSK  "QwerZ123"
#endif

const char *ssid = APSSID;
const char *password = APPSK;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

StaticJsonDocument<100> doc;

bool leftBtnState = false;
bool rightBtnState = false;
bool topBtnState = false;
bool bottomBtnState = false;
uint32_t lastSentCmd = 0;

void sendStates() {
  if(leftBtnState) Serial.write(1);
  else Serial.write(2);
  if(rightBtnState) Serial.write(3);
  else Serial.write(4);
  if(topBtnState) Serial.write(5);
  else Serial.write(6);
  if(bottomBtnState) Serial.write(7);
  else Serial.write(8);
  lastSentCmd = millis();
}

void handleRoot(AsyncWebServerRequest *request) {
  request->send(200, "text/html", "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'><meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=0'><title>Controller</title><style>.button,.status{font-family:Arial;color:#fff}body{margin:0;padding:0}.mark-red{background-color:rgba(255,0,0,.1);border:1px solid red}.mark-cyan{background-color:rgba(0,255,255,.1);border:1px solid #0ff}.unselectable{-webkit-touch-callout:none;-webkit-user-select:none;-khtml-user-select:none;-moz-user-select:none;-ms-user-select:none;user-select:none}.buttons-wrapper{position:absolute;width:90%;height:auto;bottom:20px;left:0;right:0;margin:auto}.button,.wrapper{width:100%;height:100%}.box{width:100%;position:relative}.box:before{content:'';display:block;padding-top:100%}.box .content{position:absolute;top:0;left:0;bottom:0;right:0;display:grid;grid-template-columns:33% 33% 33%;grid-template-rows:33% 33% 33%}.wrapper{padding:5%;box-sizing:border-box}.button{border-radius:12%;background-color:#7a8ca0;font-size:600%;line-height:92%;text-align:center}.active{background-color:#363a3a}.btn-left{grid-area:2/1/2/1;transform:rotate(-90deg)}.btn-right{grid-area:2/3/2/3;transform:rotate(90deg)}.btn-bottom{grid-area:3/2/3/2;transform:rotate(180deg)}.btn-top{grid-area:1/2/1/2}.status{font-size:22px;background-color:#c0591e;font-weight:800;line-height:22px;height:26px;padding:5px}.greentext{background-color:#1ec539}.redtext{background-color:#e00808}</style><script type='application/javascript'>let globalInterval={left:void 0,right:void 0,top:void 0,bottom:void 0},intervalMs=500,gateway=`ws://${window.location.hostname}/ws`,websocket,statusElementSelector='.status',$statusElement;function isTouchscreen(){return!!window.matchMedia('(pointer:coarse)').matches}function bind(e,t,s){var e=document.querySelector(e),n=isTouchscreen?'touchend':'mouseup';e.addEventListener(isTouchscreen?'touchstart':'mousedown',e=>{e.target.classList.add('active'),t()}),e.addEventListener(n,e=>{e.target.classList.remove('active'),s()})}function wsInit(){console.log('Trying to open a WS connection...'),$statusElement.classList.remove('greentext'),$statusElement.classList.remove('redtext'),$statusElement.innerText='Подключение...',(websocket=new WebSocket(gateway)).onopen=wsOnOpen,websocket.onclose=wsOnClose,websocket.onmessage=wsOnMessage}function wsOnOpen(e){console.log('Connection opened'),$statusElement.classList.add('greentext'),$statusElement.classList.remove('redtext'),$statusElement.innerText='Подключен'}function wsOnClose(e){console.log('Connection closed'),$statusElement.classList.remove('greentext'),$statusElement.classList.add('redtext'),$statusElement.innerText='Соединение потеряно',setTimeout(wsInit,2e3)}function wsOnMessage(e){console.log('recieved msg')}function sendCmd(t,e){let s=e?1:0,n=()=>{var e={btn:t,state:s};console.log(`ws btn=${t} state=`+s),console.log(e),websocket.send(JSON.stringify(e))};e?(n(),globalInterval[t]=setInterval(()=>{n()},intervalMs)):(clearInterval(globalInterval[t]),n())}document.addEventListener('DOMContentLoaded',()=>{$statusElement=document.querySelector(statusElementSelector),wsInit();for(let e of['left','right','top','bottom'])bind(`.btn-${e} .button`,()=>{sendCmd(e,!0)},()=>{sendCmd(e,!1)})});</script></head><body><div class='status redtext'>загрузка...</div><div class='buttons-wrapper'><div class='box'><div class='content'><div class='wrapper btn-left'><div class='button unselectable'>&#8593;</div></div><div class='wrapper btn-right'><div class='button unselectable'>&#8593;</div></div><div class='wrapper btn-top'><div class='button unselectable'>&#8593;</div></div><div class='wrapper btn-bottom'><div class='button unselectable'>&#8593;</div></div></div></div></div></body></html>");
}

// void handleController(AsyncWebServerRequest *request) {
//   if(request->hasParam("btn") && request->hasParam("state")) {
//     String btn = request->getParam("btn")->value();
//     bool state = request->getParam("state")->value() == "1";
//     if(btn == "left") leftBtnState = state;
//     if(btn == "right") rightBtnState = state;
//     if(btn == "top") topBtnState = state;
//     if(btn == "bottom") bottomBtnState = state;
//     sendStates();
//     request->send(200, "application/json", "{'status': 'ok'}");
//   }
//   else {
//     request->send(200, "application/json", "{'status': 'error', 'msg': 'wrong parameter'}");
//   }
// }

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {

  AwsFrameInfo *info = (AwsFrameInfo*)arg;

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;
    DeserializationError error = deserializeJson(doc, message);

    String btn = doc["btn"];
    String state = doc["state"];

    if (btn.equals("left")) {
      if (state.equals("1")) leftBtnState = true;
      else leftBtnState = false;
    }
    if (btn.equals("right")) {
      if (state.equals("1")) rightBtnState = true;
      else rightBtnState = false;
    }
    if (btn.equals("top")) {
      if (state.equals("1")) topBtnState = true;
      else topBtnState = false;
    }
    if (btn.equals("bottom")) {
      if (state.equals("1")) bottomBtnState = true;
      else bottomBtnState = false;
    }

    sendStates();

    ws.textAll("recieved");
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
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

  initWebSocket();

  server.on("/", HTTP_GET, handleRoot);
  // server.on("/controller", HTTP_GET, handleController);
  server.begin();
}

void loop() {
  if((millis() - lastSentCmd) > 500) {
    sendStates();
  }
}