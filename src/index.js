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
      fetch('/controller?btn=left&state=1');
    },
    () => {
      console.log('stop');
      fetch('/controller?btn=left&state=0');
    }
  );
  bind(
    '.btn-right .button',
    () => {
      fetch('/controller?btn=right&state=1');
    },
    () => {
      fetch('/controller?btn=right&state=0');
    }
  );
  bind(
    '.btn-top .button',
    () => {
      fetch('/controller?btn=top&state=1');
    },
    () => {
      fetch('/controller?btn=top&state=0');
    }
  );
  bind(
    '.btn-bottom .button',
    () => {
      fetch('/controller?btn=bottom&state=1');
    },
    () => {
      fetch('/controller?btn=bottom&state=0');
    }
  );
});
