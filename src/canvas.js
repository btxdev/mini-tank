let cnv;
let ctx;
let w = 0;
let h = 0;

let Tank = {
  pos: {
    x: 0,
    y: 0,
  },
  bufpos: {
    x: 0,
    y: 0,
  },
};

let Camera = {
  pos: {
    x: 0,
    y: 0,
  },
  bufpos: {
    x: 0,
    y: 0,
  },
  angle: 0,
  bufangle: 0,
};

let renderInterval;

let pathPoints = [];

function initCanvas() {
  document.querySelector(
    '.canvas-wrapper'
  ).innerHTML = `<canvas id="canvas" width="${w}" height="${h}"></canvas>`;
  setTimeout(() => {
    cnv = document.getElementById('canvas');
    ctx = cnv.getContext('2d');
    renderInterval = setInterval(render, 30);
  }, 100);
  setInterval(() => {
    addPathPoint(Tank.pos.x, Tank.pos.y);
  }, 2000);
}

function smoothCoords(real, current, speed = 0.3) {
  let acc = {
    x: (real.x - current.x) * speed,
    y: (real.y - current.y) * speed,
  };
  return {
    x: current.x + acc.x,
    y: current.y + acc.y,
  };
}

function addPathPoint(x, y) {
  pathPoints.unshift({ x: x, y: y });
  if (pathPoints.length > 200) {
    pathPoints.pop();
  }
}

function render() {
  ctx.fillStyle = 'white';
  ctx.fillRect(0, 0, w, h);

  cw = w / 2;
  ch = h / 2;

  Tank.bufpos = smoothCoords(
    { x: Tank.pos.x, y: Tank.pos.y },
    { x: Tank.bufpos.x, y: Tank.bufpos.y },
    0.3
  );

  Camera.bufpos = smoothCoords(
    { x: Tank.pos.x, y: Tank.pos.y },
    { x: Camera.bufpos.x, y: Camera.bufpos.y },
    0.5
  );

  Camera.angle = deg2rad(gyroscope.yaw);
  Camera.bufangle += (Camera.angle - Camera.bufangle) * 0.04;

  Tank.pos.x += gyroscope.accelX * 0.01;
  Tank.pos.y += gyroscope.accelY * 0.01;

  let point;

  // draw path
  if (pathPoints.length > 2) {
    ctx.strokeStyle = 'rgb(0, 0, 0)';
    ctx.beginPath();
    for (let i = 1; i < pathPoints.length; i++) {
      let p0x = pathPoints[i - 1].x;
      let p0y = pathPoints[i - 1].y;
      let p1x = pathPoints[i].x;
      let p1y = pathPoints[i].y;
      let pxpy0 = getScreenPos(
        p0x,
        p0y,
        { w: w, h: h },
        { x: Camera.bufpos.x, y: Camera.bufpos.y }
      );
      let pxpy1 = getScreenPos(
        p1x,
        p1y,
        { w: w, h: h },
        { x: Camera.bufpos.x, y: Camera.bufpos.y }
      );
      let px0 = pxpy0.x;
      let py0 = pxpy0.y;
      let px1 = pxpy1.x;
      let py1 = pxpy1.y;
      let center0 = { x: px0, y: py0 };
      let center1 = { x: px1, y: py1 };

      point = rotate2d({ x: px0, y: py0 }, center0, 0);
      ctx.moveTo(point.x, point.y);
      point = rotate2d({ x: px1, y: py1 }, center1, 0);
      ctx.lineTo(point.x, point.y);
    }
    ctx.stroke();
  }

  // draw arrow
  let ps = 24;
  pxpy = getScreenPos(
    Tank.bufpos.x,
    Tank.bufpos.y,
    { w: w, h: h },
    { x: Camera.bufpos.x, y: Camera.bufpos.y }
  );
  let px = pxpy.x;
  let py = pxpy.y;
  ctx.strokeStyle = 'rgb(20, 52, 70)';
  ctx.fillStyle = 'rgb(178, 199, 216)';
  ctx.beginPath();
  let center = { x: px, y: py };
  let p_angle = Camera.bufangle;
  point = rotate2d({ x: px, y: py - ps / 2 }, center, p_angle);
  ctx.moveTo(point.x, point.y);
  point = rotate2d({ x: px - (ps / 2) * 0.7, y: py + ps / 2 }, center, p_angle);
  ctx.lineTo(point.x, point.y);
  point = rotate2d({ x: px, y: py + ps / 4 }, center, p_angle);
  ctx.lineTo(point.x, point.y);
  point = rotate2d({ x: px + (ps / 2) * 0.7, y: py + ps / 2 }, center, p_angle);
  ctx.lineTo(point.x, point.y);
  point = rotate2d({ x: px, y: py - ps / 2 }, center, p_angle);
  ctx.lineTo(point.x, point.y);
  ctx.fill();
  ctx.stroke();
}

document.addEventListener('DOMContentLoaded', () => {
  cnv = document.getElementById('canvas');
  w = cnv.clientWidth;
  h = cnv.clientHeight;
  initCanvas();
});
