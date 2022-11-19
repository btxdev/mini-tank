const waitForResponseTimeout = 2000;
// const gateway = `ws://${window.location.hostname}/ws`;
const gateway = `ws://192.168.0.1/ws`;
const statusElementSelector = '.status';

let websocket;
let waitForResponse;
let $statusElement;
let imWaitingForResponse = false;

let gyroscope = {
  pitch: 0,
  roll: 0,
  yaw: 0,
  accelX: 0,
  accelY: 0,
  accelZ: 0,
};

function redText(msg = 'Соединение потеряно') {
  $statusElement.classList.remove('greentext');
  $statusElement.classList.add('redtext');
  $statusElement.innerText = msg;
}

function orangeText(msg = 'Подключение...') {
  $statusElement.classList.remove('greentext');
  $statusElement.classList.remove('redtext');
  $statusElement.innerText = msg;
}

function greenText(msg = 'Подключен') {
  $statusElement.classList.add('greentext');
  $statusElement.classList.remove('redtext');
  $statusElement.innerText = msg;
}

function waitForResponseFunc() {
  clearInterval(waitForResponse);
  if (!imWaitingForResponse) return;
  redText();
  websocket.close();
  document.location.reload();
}

function isTouchscreen() {
  if (window.matchMedia('(pointer:coarse)').matches) {
    return true;
  } else {
    return false;
  }
}

function bind(selector, on, off) {
  let elem = document.querySelector(selector);
  let onEvent = isTouchscreen ? 'touchstart' : 'mousedown';
  let offEvent = isTouchscreen ? 'touchend' : 'mouseup';
  elem.addEventListener(onEvent, (e) => {
    e.target.classList.add('active');
    on();
  });
  elem.addEventListener(offEvent, (e) => {
    e.target.classList.remove('active');
    off();
  });
}

function wsInit() {
  console.log('Trying to open a WS connection...');
  orangeText();

  websocket = new WebSocket(gateway);
  websocket.onopen = wsOnOpen;
  websocket.onclose = wsOnClose;
  websocket.onmessage = wsOnMessage;
}

function wsOnOpen(event) {
  console.log('Connection opened');
  greenText();
}

function wsOnClose(event) {
  console.log('Connection closed');
  setTimeout(wsInit, 1000);
}

function json(strData) {
  return new Promise((resolve, reject) => {
    try {
      resolve(JSON.parse(strData));
    } catch (exc) {
      reject(exc);
    }
  });
}

function wsOnMessage(event) {
  imWaitingForResponse = false;
  clearInterval(waitForResponse);
  greenText();
  console.log(event.data);
  json(event.data)
    .then((jsonData) => {
      if (jsonData.msgType == 'gyroscope') {
        gyroscope.pitch = jsonData?.pitch;
        gyroscope.roll = jsonData?.roll;
        gyroscope.yaw = jsonData?.yaw;
        gyroscope.accelX = jsonData?.accX;
        gyroscope.accelY = jsonData?.accY;
        gyroscope.accelZ = jsonData?.accZ;
      }
    })
    .catch((exc) => {
      //
    });
}

function sendCmd(direction, stateBoolean) {
  let stateNumeric = stateBoolean ? 1 : 0;
  let obj = {
    btn: direction,
    state: stateNumeric,
  };
  console.log(`ws send btn=${direction} state=${stateNumeric}`);
  console.log(obj);
  websocket.send(JSON.stringify(obj));
  imWaitingForResponse = true;
  waitForResponse = setInterval(waitForResponseFunc, waitForResponseTimeout);
}

document.addEventListener('DOMContentLoaded', () => {
  $statusElement = document.querySelector(statusElementSelector);

  wsInit();

  let btns = ['left', 'right', 'top', 'bottom'];
  for (let btn of btns) {
    bind(
      `.btn-${btn} .button`,
      () => {
        sendCmd(btn, true);
      },
      () => {
        sendCmd(btn, false);
      }
    );
  }
});
