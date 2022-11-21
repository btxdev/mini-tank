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

String serialBuffer = "";
String lastAction = "b0";

void sendAction() {
  Serial.println(lastAction);
}

void serialFlush() {
  Serial.flush();
  // while(Serial.available()) { Serial.read(); }
}

void handleRoot(AsyncWebServerRequest *request) {
  request->send(200, "text/html", "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'><meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=0'><title>Controller</title><style>.button,.status{font-family:Arial;color:#fff}body{margin:0;padding:0}.mark-red{background-color:rgba(255,0,0,.1);border:1px solid red}.mark-cyan{background-color:rgba(0,255,255,.1);border:1px solid #0ff}.unselectable{-webkit-touch-callout:none;-webkit-user-select:none;-khtml-user-select:none;-moz-user-select:none;-ms-user-select:none;user-select:none}.buttons-wrapper{position:absolute;width:90%;height:auto;bottom:20px;left:0;right:0;margin:auto}.button,.wrapper{height:100%;width:100%}.box{width:100%;position:relative}.box:before{content:'';display:block;padding-top:100%}.box .content{position:absolute;top:0;left:0;bottom:0;right:0;display:grid;grid-template-columns:33% 33% 33%;grid-template-rows:33% 33% 33%}.wrapper{padding:5%;box-sizing:border-box}.button{border-radius:12%;background-color:#7a8ca0;font-size:600%;line-height:92%;text-align:center}.active{background-color:#363a3a}.btn-left{grid-area:2/1/2/1;transform:rotate(-90deg)}.btn-right{grid-area:2/3/2/3;transform:rotate(90deg)}.btn-bottom{grid-area:3/2/3/2;transform:rotate(180deg)}.btn-top{grid-area:1/2/1/2}.status{font-size:22px;background-color:#c0591e;font-weight:800;line-height:22px;height:26px;padding:5px}.greentext{background-color:#1ec539}.redtext{background-color:#e00808}#canvas{width:100%;height:100%}.canvas-wrapper{box-sizing:border-box;width:100%;height:400px}</style><script type='application/javascript'>const Const_G=6.6743*Math.pow(10,-11),Const_c=299792458,Const_c2=Math.pow(Const_c,2),Const_solarM=1.989*Math.pow(10,30),Const_solarR=696340;function reload(){try{document.location.reload()}catch(n){console.error(n),alert('Произошла ошибка, перезагрузите страницу')}}function randomInt(n,t){let o,r;return r=void 0===t?(o=0,n):(o=n,t),Math.ceil(Math.random()*(r-o)+o)}function randomChar(){var n='abcdefghijklmnopqrstuvwxyz';return n[randomInt(n.length-1)]}function randomCharSpecial(n){var t='bcdfghjklmnpqrstvwxyz',o='aeiou';return'consonants'==n||'c'==n?t[randomInt(t.length-1)]:'vowels'==n||'v'==n?o[randomInt(o.length-1)]:'x'}function deg2rad(n){return n/180*Math.PI}function rad2deg(n){return 180/Math.PI*n}function getScreenPos(n,t,o,r){return{x:n-r.x+o.w/2,y:t-r.y+o.h/2}}function clamp(n,t,o){return n=o<(n=n<t?t:n)?o:n}function rotate2d(n,t,o){var r={x:0,y:0};return r.x=t.x+(n.x-t.x)*Math.cos(o)-(n.y-t.y)*Math.sin(o),r.y=t.y+(n.y-t.y)*Math.cos(o)+(n.x-t.x)*Math.sin(o),r}function distanceToAngle(n,t){return 0<=n?!(0<=t)&&n-t>=Math.PI?2*Math.PI-(n-t):t-n:0<=t&&t-n>=Math.PI?t-n-2*Math.PI:t-n}</script><script type='application/javascript'>const waitForResponseTimeout=2e3,gateway='ws://192.168.0.1/ws',statusElementSelector='.status';let websocket,$statusElement,waitForResponseInterval,isWaitForResponse=!1,simpleN=[3,5,7,11],buttons={left:!1,right:!1,top:!1,bottom:!1},mapPoints=[],gyroscope={pitch:0,roll:0,yaw:0},motors={left:0,right:0};function saveMapPointsToStorage(){}function addMapPoints(e){}function loadMapPointsFromStorage(){}function encodeBtn(){return buttons.top?'b1':buttons.bottom?'b2':buttons.left?'b3':buttons.right?'b4':'b0'}function compareBtn(e){return e===encodeBtn()}function pressBtn(e,t){buttons.hasOwnProperty(e)&&(buttons[e]=Boolean(t));e=encodeBtn();websocket.send(e),isWaitForResponse=!0,waitForResponseInterval=setInterval(waitForResponseFunc,waitForResponseTimeout)}function redText(e='Соединение потеряно'){$statusElement.classList.remove('greentext'),$statusElement.classList.add('redtext'),$statusElement.innerText=e}function orangeText(e='Подключение...'){$statusElement.classList.remove('greentext'),$statusElement.classList.remove('redtext'),$statusElement.innerText=e}function greenText(e='Подключен'){$statusElement.classList.add('greentext'),$statusElement.classList.remove('redtext'),$statusElement.innerText=e}function waitForResponseFunc(){clearInterval(waitForResponseInterval),isWaitForResponse&&(redText(),websocket.close(),document.location.reload())}function isTouchscreen(){return!!window.matchMedia('(pointer:coarse)').matches}function bind(e,t,o){var e=document.querySelector(e),n=isTouchscreen?'touchend':'mouseup';e.addEventListener(isTouchscreen?'touchstart':'mousedown',e=>{e.target.classList.add('active'),t()}),e.addEventListener(n,e=>{e.target.classList.remove('active'),o()})}function wsInit(){console.log('Trying to open a WS connection...'),orangeText(),(websocket=new WebSocket(gateway)).onopen=wsOnOpen,websocket.onclose=wsOnClose,websocket.onmessage=wsOnMessage}function wsOnOpen(e){console.log('Connection opened'),greenText()}function wsOnClose(e){console.log('Connection closed'),setTimeout(wsInit,1e3)}function json(o){return new Promise((e,t)=>{try{e(JSON.parse(o))}catch(e){t(e)}})}function wsOnMessage(e){isWaitForResponse=!1,clearInterval(waitForResponseInterval),greenText(),console.log(e.data),json(e.data).then(e=>{'feedback'==e.msgType&&(gyroscope.pitch=e?.pitch,gyroscope.roll=e?.roll,gyroscope.yaw=e?.yaw,motors.left=e?.motLeft,motors.right=e?.motRight),'map'==e.msgType&&e.hasOwnProperty('points')&&addMapPoints(e.points)}).catch(e=>{})}document.addEventListener('DOMContentLoaded',()=>{$statusElement=document.querySelector(statusElementSelector),loadMapPointsFromStorage(),wsInit();for(let e of['left','right','top','bottom'])bind(`.btn-${e} .button`,()=>{pressBtn(e,!0)},()=>{pressBtn(e,!1)})});</script><script type='application/javascript'>let cnv,ctx,w=0,h=0,Tank={pos:{x:0,y:0},bufpos:{x:0,y:0}},Camera={pos:{x:0,y:0},bufpos:{x:0,y:0},angle:0,bufangle:0},renderInterval,pathPoints=[];function initCanvas(){document.querySelector('.canvas-wrapper').innerHTML=`<canvas id='canvas' width='${w}' height='${h}'></canvas>`,setTimeout(()=>{cnv=document.getElementById('canvas'),ctx=cnv.getContext('2d'),renderInterval=setInterval(render,30)},100),setInterval(()=>{addPathPoint(Tank.pos.x,Tank.pos.y)},2e3)}function smoothCoords(t,e,a=.3){let o=(t.x-e.x)*a,n=(t.y-e.y)*a;return{x:e.x+o,y:e.y+n}}function addPathPoint(t,e){pathPoints.unshift({x:t,y:e}),200<pathPoints.length&&pathPoints.pop()}function render(){ctx.fillStyle='white',ctx.fillRect(0,0,w,h),cw=w/2,ch=h/2,Tank.bufpos=smoothCoords({x:Tank.pos.x,y:Tank.pos.y},{x:Tank.bufpos.x,y:Tank.bufpos.y},.3),Camera.bufpos=smoothCoords({x:Tank.pos.x,y:Tank.pos.y},{x:Camera.bufpos.x,y:Camera.bufpos.y},.5),Camera.angle=deg2rad(gyroscope.yaw),Camera.bufangle+=.04*(Camera.angle-Camera.bufangle),Tank.pos.x+=.01*gyroscope.accelX,Tank.pos.y+=.01*gyroscope.accelY;let e;if(2<pathPoints.length){ctx.strokeStyle='rgb(0, 0, 0)',ctx.beginPath();for(let t=1;t<pathPoints.length;t++){var a=pathPoints[t-1].x,o=pathPoints[t-1].y,n=pathPoints[t].x,x=pathPoints[t].y,a=getScreenPos(a,o,{w:w,h:h},{x:Camera.bufpos.x,y:Camera.bufpos.y}),o=getScreenPos(n,x,{w:w,h:h},{x:Camera.bufpos.x,y:Camera.bufpos.y}),n=a.x,x=a.y,a=o.x,o=o.y,s={x:a,y:o};e=rotate2d({x:n,y:x},{x:n,y:x},0),ctx.moveTo(e.x,e.y),e=rotate2d({x:a,y:o},s,0),ctx.lineTo(e.x,e.y)}ctx.stroke()}var t=(pxpy=getScreenPos(Tank.bufpos.x,Tank.bufpos.y,{w:w,h:h},{x:Camera.bufpos.x,y:Camera.bufpos.y})).x,r=pxpy.y,y=(ctx.strokeStyle='rgb(20, 52, 70)',ctx.fillStyle='rgb(178, 199, 216)',ctx.beginPath(),{x:t,y:r}),c=Camera.bufangle;e=rotate2d({x:t,y:r-12},y,c),ctx.moveTo(e.x,e.y),e=rotate2d({x:t-12*.7,y:r+12},y,c),ctx.lineTo(e.x,e.y),e=rotate2d({x:t,y:r+6},y,c),ctx.lineTo(e.x,e.y),e=rotate2d({x:t+12*.7,y:r+12},y,c),ctx.lineTo(e.x,e.y),e=rotate2d({x:t,y:r-12},y,c),ctx.lineTo(e.x,e.y),ctx.fill(),ctx.stroke()}</script></head><body><div class='status redtext'>загрузка...</div><div class='canvas-wrapper'><canvas id='canvas'></canvas></div><div class='buttons-wrapper'><div class='box'><div class='content'><div class='wrapper btn-left'><div class='button unselectable'>&#8593;</div></div><div class='wrapper btn-right'><div class='button unselectable'>&#8593;</div></div><div class='wrapper btn-top'><div class='button unselectable'>&#8593;</div></div><div class='wrapper btn-bottom'><div class='button unselectable'>&#8593;</div></div></div></div></div></body></html>");
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;
    
    if(message.substring(0, 1) == "b") {
      lastAction = message.substring(0, 2);
      sendAction();
      // ws.textAll(encodedButtons);
    }

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
  Serial.begin(115200);
  sendAction();

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
  if (Serial.available() > 0) {
    uint8_t code = Serial.read();

    if(code == 'b') {
      String buffer = "b";
      bool reading = true;
      while(reading) {
        uint8_t code1 = Serial.read();
        buffer += (char)code1;
        if(code == 10 || code == 13) reading = false;
        if(Serial.available() == 0) reading = false;
      }
      if(buffer.substring(0, 2) != lastAction) {
        sendAction();
      }
      serialFlush();
    }
  }
    
  //   bool reading = true;
  //   // uint32_t waitTimer = millis();
  //   while(reading) {
  //     // wait for slowly incoming data
  //     // uint32_t waitTimer = millis();
  //     // while(!(Serial.available() || ((millis() - waitTimer) > 100))) {}
  //     // read byte
  //     uint8_t code = Serial.read();
  //     buffer += (char)code;
  //     // end of message
  //     // if(code == 10 || code == 13) {
  //     //   reading = false;
  //     //   break;
  //     // }
  //     if(code == 10 || code == 13) reading = false;
  //     if(Serial.available() == 0) reading = false;
  //     // if((Serial.available() == 0) && ((millis() - waitTimer) > 1000)) {
  //     //   reading = false;
  //     //   break;
  //     // }
  //   }
  //   // ws.textAll(buffer);
  //   // is action code
  //   if(buffer.substring(0, 1) == "b") {
  //     if(buffer.substring(0, 2) != lastAction) {
  //       // serialFlush();
  //       sendAction();
  //       // ws.textAll("some shit on response: " + buffer);
  //     }
  //     else {
  //       // ws.textAll("good response: " + buffer);
  //     }
  //     // serialFlush();
  //   }
  //   // is json data
  //   // else {
  //   //   ws.textAll(buffer);
  //   // }
  //   serialFlush();
  // }

    

    // action feedback
    // if(serialBuffer.length() == 0 && code == 98) {
    //   serialBuffer += (char)code;
    //   code = Serial1.read();
    //   serialBuffer += (char)code;
    //   if(serialBuffer != lastAction) {
    //     sendAction();
    //     ws.textAll("some shit on response: " + serialBuffer);
    //   }
    //   else {
    //     ws.textAll("good response: " + serialBuffer);
    //   }
    //   serialBuffer = "";
    // }

    // // parse
    // else if(code == 13 || code == 10) {
    //   serialBuffer = "";
    // }

    // // save
    // else {
    //   // serialBuffer += (char)code;
    // }    
  // }
    // else if(actionReadMode) {
    //   lastAction = code - 48;
    //   actionReadMode = false;
    //   Serial.println(lastAction);
    //   actionFeedback();
    // }

    // read data from Mega
    // String incomingSerialString = Serial.readString();

  //   // check buttons
  //   if(incomingSerialString.substring(0, 1) == "b") {
  //     if(incomingSerialString.substring(0, 2) != lastAction) {
  //       serialFlush();
  //       sendAction();
  //       ws.textAll("some shit on response: " + incomingSerialString);
  //     }
  //     else {
  //       ws.textAll("good response: " + incomingSerialString);
  //     }
  //   }
  //   // send json data
  //   else {
  //     ws.textAll(incomingSerialString);
  //   }
}