var globalInterval = {
  left: undefined,
  right: undefined,
  top: undefined,
  bottom: undefined,
};

let intervalMs = 500;

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

function sendCmd(direction, stateBoolean) {
  let stateNumeric = stateBoolean ? 1 : 0;
  let run = () => {
    console.log(`fetch btn=${direction} state=${stateNumeric}`);
    fetch(`/controller?btn=${direction}&state=${stateNumeric}`);
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
