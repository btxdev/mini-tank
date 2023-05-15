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


String toMegaBuffer = "";
String toUserBuffer = "";



void sendAction();
void serialFlush();
void handleRoot(AsyncWebServerRequest *request);


void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    toMegaBuffer = (char*)data;
    ws.textAll("recieved");
  }
}


void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void initWebSocket();


void setup() {
  Serial.begin(115200);

  delay(200);

  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(gateway, gateway, subnet);
  delay(200);
  WiFi.softAP(ssid, password);
  delay(200);

  IPAddress resultIP = WiFi.softAPIP();
  delay(200);

  initWebSocket();

  server.on("/", HTTP_GET, handleRoot);
  server.begin();
}


void loop() {
  // read mode
  if (Serial.available() > 0) {
    char code = Serial.read();
    toUserBuffer += code;
    // send accumulated string to client
    if (code == '>') {
      ws.textAll(toUserBuffer);
      toUserBuffer = "";
    }
  }
  // write mode
  else if(toMegaBuffer.length() > 0) {
    Serial.println(toMegaBuffer);
    toMegaBuffer = "";
  }
}


void handleRoot(AsyncWebServerRequest *request) {
  request->send(200, "text/html", "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'><meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=0'><title>Controller</title><style>.arrow,.button{font-family:Arial;overflow:hidden}.arrow,.button,.status,.text-indicator{font-family:Arial}body{margin:0;padding:0}.mark-red{background-color:rgba(255,0,0,.1);border:1px solid red}.mark-cyan{background-color:rgba(0,255,255,.1);border:1px solid #0ff}.unselectable{-webkit-touch-callout:none;-webkit-user-select:none;-khtml-user-select:none;-moz-user-select:none;-ms-user-select:none;user-select:none}.buttons-wrapper{position:absolute;width:90%;height:auto;bottom:20px;left:0;right:0;margin:auto}.button,.wrapper{height:100%;width:100%}.box{width:100%;position:relative}.box:before{content:'';display:block;padding-top:100%}.box .content{position:absolute;top:0;left:0;bottom:0;right:0;display:grid;grid-template-columns:20% 20% 20% 20% 20%;grid-template-rows:20% 20% 20% 20% 20%}.wrapper{padding:5%;box-sizing:border-box}.button{border-radius:12%;background-color:#7a8ca0;color:#fff;font-size:600%;line-height:92%;text-align:center}.active{background-color:#363a3a}.btn-left{grid-area:3/2/3/2;transform:rotate(-90deg)}.btn-right{grid-area:3/4/3/4;transform:rotate(90deg)}.btn-bottom{grid-area:4/3/4/3;transform:rotate(180deg)}.btn-top{grid-area:2/3/2/3}.indicator-left{grid-area:3/1/3/1}.indicator-right{grid-area:3/5/3/5}.indicator-top{grid-area:1/3/1/3}.indicator-bottom{grid-area:5/3/5/3}.indicator-center{grid-area:3/3/3/3}.indicator-fbl{grid-area:5/1/5/1}.indicator-fbr{grid-area:5/5/5/5}.text-indicator{width:100%;height:40%;margin-top:30%;color:#000;font-size:120%;text-align:center;vertical-align:middle;font-weight:700}.status{font-size:22px;background-color:#c0591e;font-weight:800;line-height:22px;height:26px;color:#fff;padding:5px}.greentext{background-color:#1ec539}.redtext{background-color:#e00808}.map-wrapper{box-sizing:border-box;width:72vw;height:72vw;margin-top:5vw;margin-left:14vw}#map{width:100%;height:100%}#map td{border:1px solid #000;width:33%;height:33%}.arrow{font-size:400%;line-height:100%;text-align:center}</style><script type='application/javascript'>const waitForResponseTimeout=2e3,gateway='ws://192.168.0.1/ws',statusElementSelector='.status';let websocket,$statusElement,waitForResponseInterval,isWaitForResponse=!1,buttons={left:!1,right:!1,top:!1,bottom:!1},gyroscope={pitch:0,roll:0,yaw:0},motors={left:0,right:0};function encodeBtn(){return buttons.top?'<action:w>':buttons.bottom?'<action:s>':buttons.left?'<action:a>':!!buttons.right&&'<action:d>'}function pressBtn(e,t){buttons.hasOwnProperty(e)&&(buttons[e]=Boolean(t));e=encodeBtn();!1!==e&&(websocket.send(e),isWaitForResponse=!0,waitForResponseInterval=setInterval(waitForResponseFunc,waitForResponseTimeout))}function redText(e='Соединение потеряно'){$statusElement.classList.remove('greentext'),$statusElement.classList.add('redtext'),$statusElement.innerText=e}function orangeText(e='Подключение...'){$statusElement.classList.remove('greentext'),$statusElement.classList.remove('redtext'),$statusElement.innerText=e}function greenText(e='Подключен'){$statusElement.classList.add('greentext'),$statusElement.classList.remove('redtext'),$statusElement.innerText=e}function waitForResponseFunc(){clearInterval(waitForResponseInterval),isWaitForResponse&&(redText(),websocket.close(),document.location.reload())}function isTouchscreen(){return!!window.matchMedia('(pointer:coarse)').matches}function bind(e,t,n){var e=document.querySelector(e),o=isTouchscreen?'touchend':'mouseup';e.addEventListener(isTouchscreen?'touchstart':'mousedown',e=>{e.target.classList.add('active'),t()}),e.addEventListener(o,e=>{e.target.classList.remove('active'),n()})}function parseString(e){for(const o of e.replace(/[<>]/g,'').split(';')){var t,n=o.split(':');2==n.length&&([n,t]=n,t=Number(t),'yaw'==n&&(gyroscope.yaw=t,document.querySelector('.indicator-center .text-indicator').innerText=t+' deg'),'x'==n&&(motors.left=t,document.querySelector('.indicator-fbl .text-indicator').innerText=`x: ${t} deg`),'y'==n&&(motors.right=t,document.querySelector('.indicator-fbr .text-indicator').innerText=`y: ${t} deg`),'SF'==n&&(document.querySelector('.indicator-top .text-indicator').innerText=t+' cm',document.querySelector('#map tr:nth-child(1) td:nth-child(2)').style.background=t<=30?'red':'initial'),'SB'==n&&(document.querySelector('.indicator-bottom .text-indicator').innerText=t+' cm',document.querySelector('#map tr:nth-child(3) td:nth-child(2)').style.background=t<=30?'red':'initial'),'SL'==n&&(document.querySelector('.indicator-left .text-indicator').innerText=t+' cm',document.querySelector('#map tr:nth-child(2) td:nth-child(1)').style.background=t<=30?'red':'initial'),'SR'==n)&&(document.querySelector('.indicator-right .text-indicator').innerText=t+' cm',document.querySelector('#map tr:nth-child(2) td:nth-child(3)').style.background=t<=30?'red':'initial')}}function wsInit(){console.log('Trying to open a WS connection...'),orangeText(),(websocket=new WebSocket(gateway)).onopen=wsOnOpen,websocket.onclose=wsOnClose,websocket.onmessage=wsOnMessage}function wsOnOpen(e){console.log('Connection opened'),greenText()}function wsOnClose(e){console.log('Connection closed'),setTimeout(wsInit,1e3)}function json(n){return new Promise((e,t)=>{try{e(JSON.parse(n))}catch(e){t(e)}})}function wsOnMessage(e){isWaitForResponse=!1,clearInterval(waitForResponseInterval),greenText(),console.log(e.data),parseString(e.data)}document.addEventListener('DOMContentLoaded',()=>{$statusElement=document.querySelector(statusElementSelector),wsInit();for(let e of['left','right','top','bottom'])bind(`.btn-${e} .button`,()=>{pressBtn(e,!0)},()=>{pressBtn(e,!1)})});</script></head><body><div class='status redtext'>загрузка...</div><div class='map-wrapper'><table id='map'><tr><td></td><td></td><td></td></tr><tr><td></td><td><div class='arrow'>&#11165;</div></td><td></td></tr><tr><td></td><td></td><td></td></tr></table></div><div class='buttons-wrapper'><div class='box'><div class='content'><!-- кнопки --><div class='wrapper btn-left'><div class='button unselectable'>&#8593;</div></div><div class='wrapper btn-right'><div class='button unselectable'>&#8593;</div></div><div class='wrapper btn-top'><div class='button unselectable'>&#8593;</div></div><div class='wrapper btn-bottom'><div class='button unselectable'>&#8593;</div></div><!-- индикаторы --><div class='wrapper indicator-left'><div class='text-indicator'>0.0 cm</div></div><div class='wrapper indicator-right'><div class='text-indicator'>0.0 cm</div></div><div class='wrapper indicator-top'><div class='text-indicator'>0.0 cm</div></div><div class='wrapper indicator-bottom'><div class='text-indicator'>0.0 cm</div></div><div class='wrapper indicator-center'><div class='text-indicator'>0 deg</div></div><div class='wrapper indicator-fbl'><div class='text-indicator'>FBL: 0</div></div><div class='wrapper indicator-fbr'><div class='text-indicator'>FBR: 0</div></div></div></div></div></body></html>");
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
