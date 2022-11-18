let globalInterval = {
  left: undefined,
  right: undefined,
  top: undefined,
  bottom: undefined,
};

let intervalMs = 500;

let gateway = `ws://${window.location.hostname}/ws`;
let websocket;

let statusElementSelector = '.status';
let $statusElement;

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
  $statusElement.classList.remove('greentext');
  $statusElement.classList.remove('redtext');
  $statusElement.innerText = 'Подключение...';

  websocket = new WebSocket(gateway);
  websocket.onopen = wsOnOpen;
  websocket.onclose = wsOnClose;
  websocket.onmessage = wsOnMessage;
}

function wsOnOpen(event) {
  console.log('Connection opened');
  $statusElement.classList.add('greentext');
  $statusElement.classList.remove('redtext');
  $statusElement.innerText = 'Подключен';
}

function wsOnClose(event) {
  console.log('Connection closed');
  $statusElement.classList.remove('greentext');
  $statusElement.classList.add('redtext');
  $statusElement.innerText = 'Соединение потеряно';
  setTimeout(wsInit, 2000);
}

function wsOnMessage(event) {
  console.log('recieved msg');
}

function sendCmd(direction, stateBoolean) {
  let stateNumeric = stateBoolean ? 1 : 0;
  let run = () => {
    let obj = {
      btn: direction,
      state: stateNumeric,
    };
    console.log(`ws btn=${direction} state=${stateNumeric}`);
    console.log(obj);
    websocket.send(JSON.stringify(obj));
  };
  if (stateBoolean) {
    run();
    globalInterval[direction] = setInterval(() => {
      run();
    }, intervalMs);
  } else {
    clearInterval(globalInterval[direction]);
    run();
  }
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
