const waitForResponseTimeout = 2000;
const gateway = `ws://${window.location.hostname}/ws`;
const statusElementSelector = '.status';

let websocket;
let waitForResponse;
let $statusElement;

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
  redText();
  websocket.close();
  document.location.reload();
  clearInterval(waitForResponse);
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

function wsOnMessage(event) {
  clearInterval(waitForResponse);
  greenText();
  console.log('msg recieved');
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
