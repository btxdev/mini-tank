const waitForResponseTimeout = 2000;
// const gateway = `ws://${window.location.hostname}/ws`;
const gateway = `ws://192.168.0.1/ws`;
const statusElementSelector = '.status';

let websocket;
let $statusElement;
let waitForResponseInterval;
let isWaitForResponse = false;

let buttons = {
  left: false,
  right: false,
  top: false,
  bottom: false,
};

let gyroscope = {
  pitch: 0,
  roll: 0,
  yaw: 0,
};

let motors = {
  left: 0,
  right: 0,
};

function encodeBtn() {
  if (buttons.top) return '<action:w>';
  else if (buttons.bottom) return '<action:s>';
  else if (buttons.left) return '<action:a>';
  else if (buttons.right) return '<action:d>';
  else return false;
  // else return 'b0';
}

function pressBtn(btn, state) {
  if (buttons.hasOwnProperty(btn)) buttons[btn] = Boolean(state);
  let cmd = encodeBtn();
  if (cmd === false) return;
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

function parseString(str) {
  const props = str.replace(/[<>]/g, '').split(';');
  for (const prop of props) {
    const arr = prop.split(':');
    if (arr.length != 2) continue;
    const [key, value] = arr;
    const numeric = Number(value);
    // <yaw:-339.20;FBL:0;FBR:0;x:0;y:0;SF:40;SB:113;SL:57;SR:74>
    if (key == 'yaw') {
      gyroscope.yaw = numeric;
      const $elem = document.querySelector('.indicator-center .text-indicator');
      $elem.innerText = `${numeric} deg`;
    }
    if (key == 'x') {
      motors.left = numeric;
      const $elem = document.querySelector('.indicator-fbl .text-indicator');
      $elem.innerText = `x: ${numeric} deg`;
    }
    if (key == 'y') {
      motors.right = numeric;
      const $elem = document.querySelector('.indicator-fbr .text-indicator');
      $elem.innerText = `y: ${numeric} deg`;
    }
    if (key == 'SF') {
      const $elem = document.querySelector('.indicator-top .text-indicator');
      $elem.innerText = `${numeric} cm`;
      const $cell = document.querySelector('#map tr:nth-child(1) td:nth-child(2)');
      if (numeric <= 30) $cell.style.background = 'red';
      else $cell.style.background = 'initial';
    }
    if (key == 'SB') {
      const $elem = document.querySelector('.indicator-bottom .text-indicator');
      $elem.innerText = `${numeric} cm`;
      const $cell = document.querySelector('#map tr:nth-child(3) td:nth-child(2)');
      if (numeric <= 30) $cell.style.background = 'red';
      else $cell.style.background = 'initial';
    }
    if (key == 'SL') {
      const $elem = document.querySelector('.indicator-left .text-indicator');
      $elem.innerText = `${numeric} cm`;
      const $cell = document.querySelector('#map tr:nth-child(2) td:nth-child(1)');
      if (numeric <= 30) $cell.style.background = 'red';
      else $cell.style.background = 'initial';
    }
    if (key == 'SR') {
      const $elem = document.querySelector('.indicator-right .text-indicator');
      $elem.innerText = `${numeric} cm`;
      const $cell = document.querySelector('#map tr:nth-child(2) td:nth-child(3)');
      if (numeric <= 30) $cell.style.background = 'red';
      else $cell.style.background = 'initial';
    }
  }
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
  parseString(event.data);
}

document.addEventListener('DOMContentLoaded', () => {
  $statusElement = document.querySelector(statusElementSelector);

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
