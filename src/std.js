const Const_G = 6.6743 * Math.pow(10, -11);
const Const_c = 299792458;
const Const_c2 = Math.pow(Const_c, 2);
const Const_solarM = 1.989 * Math.pow(10, 30);
const Const_solarR = 696340;

function reload() {
  try {
    document.location.reload();
  } catch (error) {
    console.error(error);
    alert('Произошла ошибка, перезагрузите страницу');
  }
}

function randomInt(arg1, arg2) {
  let min, max;
  if (typeof arg2 == 'undefined') {
    min = 0;
    max = arg1;
  } else {
    min = arg1;
    max = arg2;
  }
  return Math.ceil(Math.random() * (max - min) + min);
}

function randomChar() {
  let charMap = 'abcdefghijklmnopqrstuvwxyz';
  return charMap[randomInt(charMap.length - 1)];
}

function randomCharSpecial(type) {
  let consonants = 'bcdfghjklmnpqrstvwxyz';
  let vowels = 'aeiou';
  if (type == 'consonants' || type == 'c') {
    return consonants[randomInt(consonants.length - 1)];
  } else if (type == 'vowels' || type == 'v') {
    return vowels[randomInt(vowels.length - 1)];
  } else {
    return 'x';
  }
}

function deg2rad(deg) {
  return (deg / 180) * Math.PI;
}
function rad2deg(rad) {
  return (180 / Math.PI) * rad;
}

function getScreenPos(x, y, Canvas, Camera) {
  return { x: x - Camera.x + Canvas.w / 2, y: y - Camera.y + Canvas.h / 2 };
}

function clamp(val, min, max) {
  if (val < min) val = min;
  if (val > max) val = max;
  return val;
}

function rotate2d(point, center, angle) {
  var output = { x: 0, y: 0 };
  output.x =
    center.x +
    (point.x - center.x) * Math.cos(angle) -
    (point.y - center.y) * Math.sin(angle);
  output.y =
    center.y +
    (point.y - center.y) * Math.cos(angle) +
    (point.x - center.x) * Math.sin(angle);
  return output;
}

function distanceToAngle(from, to) {
  if (from >= 0) {
    if (to >= 0) {
      return to - from;
    } else {
      if (from - to >= Math.PI) {
        return Math.PI * 2 - (from - to);
      } else {
        return to - from;
      }
    }
  } else {
    if (to >= 0) {
      if (to - from >= Math.PI) {
        return to - from - Math.PI * 2;
      } else {
        return to - from;
      }
    } else {
      return to - from;
    }
  }
}
