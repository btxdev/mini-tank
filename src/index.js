const waitForResponseTimeout = 2000;
// const gateway = `ws://${window.location.hostname}/ws`;
const gateway = `ws://192.168.0.1/ws`;
const statusElementSelector = '.status';

let websocket;
let $statusElement;
let waitForResponseInterval;
let isWaitForResponse = false;

let simpleN = [3, 5, 7, 11];

let buttons = {
  left: false,
  right: false,
  top: false,
  bottom: false,
};

let mapPoints = [];

let gyroscope = {
  pitch: 0,
  roll: 0,
  yaw: 0,
};

let motors = {
  left: 0,
  right: 0,
};

function saveMapPointsToStorage() {}

function addMapPoints(points) {}

function loadMapPointsFromStorage() {}

function encodeBtn() {
  // let str = 'b';
  // // let i = 1;
  // let j = 0;
  // for (let value of Object.values(buttons)) {
  //   let byte = (value ? simpleN[j++] : 0) + 48;
  //   str += String.fromCharCode(byte);
  //   // str += value ? String(i) : String(i + 1);
  //   // i += 2;
  // }
  // return str;
  if (buttons.top) return 'b1';
  else if (buttons.bottom) return 'b2';
  else if (buttons.left) return 'b3';
  else if (buttons.right) return 'b4';
  else return 'b0';
}

function compareBtn(testStr) {
  return testStr === encodeBtn();
}

function pressBtn(btn, state) {
  if (buttons.hasOwnProperty(btn)) buttons[btn] = Boolean(state);
  let cmd = encodeBtn();
  websocket.send(cmd);
  isWaitForResponse = true;
  waitForResponseInterval = setInterval(
    waitForResponseFunc,
    waitForResponseTimeout
  );
}

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
  clearInterval(waitForResponseInterval);
  if (!isWaitForResponse) return;
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
  isWaitForResponse = false;
  clearInterval(waitForResponseInterval);
  greenText();
  console.log(event.data);
  json(event.data)
    .then((jsonData) => {
      if (jsonData.msgType == 'feedback') {
        gyroscope.pitch = jsonData?.pitch;
        gyroscope.roll = jsonData?.roll;
        gyroscope.yaw = jsonData?.yaw;
        motors.left = jsonData?.motLeft;
        motors.right = jsonData?.motRight;
      }
      if (jsonData.msgType == 'map') {
        if (jsonData.hasOwnProperty('points')) {
          addMapPoints(jsonData.points);
        }
      }
    })
    .catch((exc) => {
      //
    });
}

document.addEventListener('DOMContentLoaded', () => {
  $statusElement = document.querySelector(statusElementSelector);

  loadMapPointsFromStorage();
  wsInit();

  let btns = ['left', 'right', 'top', 'bottom'];
  for (let btn of btns) {
    bind(
      `.btn-${btn} .button`,
      () => {
        pressBtn(btn, true);
      },
      () => {
        pressBtn(btn, false);
      }
    );
  }
});
