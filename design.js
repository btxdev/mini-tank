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

document.addEventListener('DOMContentLoaded', () => {
  bind(
    '.btn-left .button',
    () => {
      console.log('turn left');
    },
    () => {
      console.log('stop');
    }
  );
  bind(
    '.btn-right .button',
    () => {
      console.log('turn right');
    },
    () => {
      console.log('stop');
    }
  );
  bind(
    '.btn-top .button',
    () => {
      console.log('move forward');
    },
    () => {
      console.log('stop');
    }
  );
  bind(
    '.btn-bottom .button',
    () => {
      console.log('move backward');
    },
    () => {
      console.log('stop');
    }
  );
});
