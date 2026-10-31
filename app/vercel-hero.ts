// @ts-nocheck
/* Reference HTML script copied 1:1 (cdn.classgrid.in/ai-generated/1793453291115-index.html).
   Only changes: click = 1-click toggle light <-> rainbow (never off), theme switch resets it, hover flow added at the end. */
let started = false;
export function initPage() {
  if (started) return;
  started = true;

/* 1. THEME -------------------------------------------------------
   The light/dark switch is gone: the page is always dark, exactly like
   vercel.com. Nothing to toggle, nothing saved in localStorage.        */
(function(){
  document.documentElement.setAttribute('data-theme', 'dark');
})();

/* 2. GLOW — drawn on a canvas so the falloff is perfectly smooth
   (CSS radial-gradients band on near-white backgrounds).
   Light theme: peak +1 grey level, exactly like the real page.
   Dark theme : bright halo, matching the measured profile.        */
(function(){
  var stage  = document.getElementById('triStage');
  var glow   = document.getElementById('triGlow');
  var canvas = document.getElementById('glowCanvas');
  if (!stage || !glow || !canvas) return;
  var ctx = canvas.getContext('2d');
  var SIZE = 900;
  var dpr = Math.min(window.devicePixelRatio || 1, 2);

  function isDark(){
    return document.documentElement.getAttribute('data-theme') === 'dark';
  }

  function draw(){
    canvas.width  = SIZE * dpr;
    canvas.height = SIZE * dpr;
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, SIZE, SIZE);

    var r = SIZE / 2;
    var g = ctx.createRadialGradient(r, r, 0, r, r, r);

    if (!isDark()) return;   // light theme: flat #fafafa, exactly like the reference
    // dark: reference halo (alpha = 0.42 * exp(-r/140)), copied 1:1
    for (var i = 0; i <= 30; i++){
      var t = i / 30;
      var a = 0.42 * Math.exp(-(t * r) / 140) * Math.pow(1 - t, 1.6);
      if (i === 30) a = 0;
      g.addColorStop(t, 'rgba(255,255,255,' + a.toFixed(4) + ')');
    }
    ctx.fillStyle = g;
    ctx.beginPath();
    ctx.arc(r, r, r, 0, Math.PI * 2);
    ctx.fill();
  }

  draw();
  new MutationObserver(draw).observe(document.documentElement, {attributes:true, attributeFilter:['data-theme']});

  var tx=0, ty=0, cx=0, cy=0, raf=null, MAX=60;
  function loop(){
    cx += (tx-cx)*0.08;
    cy += (ty-cy)*0.08;
    glow.style.transform = 'translate3d('+cx.toFixed(2)+'px,'+cy.toFixed(2)+'px,0)';
    if (Math.abs(tx-cx)>0.1 || Math.abs(ty-cy)>0.1) raf = requestAnimationFrame(loop);
    else raf = null;
  }
  function kick(){ if(!raf) raf = requestAnimationFrame(loop); }
  window.addEventListener('pointermove', function(e){
    var r = stage.getBoundingClientRect();
    var dx = e.clientX - (r.left + r.width/2);
    var dy = e.clientY - (r.top + r.height/2);
    var len = Math.sqrt(dx*dx+dy*dy) || 1;
    var k = Math.min(1, MAX/len);
    tx = dx*k; ty = dy*k; kick();
  }, {passive:true});
  window.addEventListener('pointerleave', function(){ tx=0; ty=0; kick(); });
})();

/* 2b. PRISM — clicking the triangle toggles the rainbow on and off. */
(function(){
  var stage = document.getElementById('triStage');
  if (!stage) return;
  var toggle = function(){
    var dark = document.documentElement.getAttribute('data-theme') === 'dark';
    if (!dark) {                                   // LIGHT MODE: 1 click = rainbow on / off
      stage.classList.toggle('prism-on');
      return;
    }
    // DARK MODE: white light <-> rainbow on every click. There is NO off state: the light never goes out.
    stage.classList.toggle('prism-on');
  };
  // only the black triangle itself is the trigger
  var poly = stage.querySelector('.tri polygon');
  poly.addEventListener('click', toggle);
  poly.addEventListener('touchstart', function(e){ e.preventDefault(); toggle(); }, {passive:false});
})();

/* 2c. CURSOR LIGHT — the cursor acts as the light source (watched live on
   vercel.com): the grey shadow falls on the side AWAY from the cursor, the
   white highlight sits on the side TOWARD it, and the rainbow leans toward
   the cursor and swings its colours. Everything eases with a soft lag.   */
(function(){
  var stage = document.getElementById('triStage');
  var tri   = stage && stage.querySelector('.tri');
  if (!stage || !tri) return;
  var root = document.documentElement.style;
  var tx=-0.6, ty=-0.8, cx=tx, cy=ty, tr=0, cr=0, raf=null;
  function frame(){
    cx += (tx-cx)*0.07; cy += (ty-cy)*0.07; cr += (tr-cr)*0.07;
    var s = stage.style;
    s.setProperty('--hx', (cx*16).toFixed(2)+'px');   // highlight toward cursor
    s.setProperty('--hy', (cy*16).toFixed(2)+'px');
    tri.style.setProperty('--sx', (-cx*13).toFixed(2)+'px'); // shadow away
    tri.style.setProperty('--sy', (-cy*13 + 3).toFixed(2)+'px');
    s.setProperty('--px', (cx*45).toFixed(2)+'px');   // rainbow leans in
    s.setProperty('--py', (cy*35).toFixed(2)+'px');
    s.setProperty('--rot', cr.toFixed(2)+'deg');      // colours swing
    if (Math.abs(tx-cx)>0.002 || Math.abs(ty-cy)>0.002 || Math.abs(tr-cr)>0.05)
      raf = requestAnimationFrame(frame);
    else raf = null;
  }
  function kick(){ if(!raf) raf = requestAnimationFrame(frame); }
  window.addEventListener('pointermove', function(e){
    var r  = tri.getBoundingClientRect();
    var dx = e.clientX - (r.left + r.width/2);
    var dy = e.clientY - (r.top  + r.height*0.62);
    var len = Math.sqrt(dx*dx+dy*dy) || 1;
    var reach = Math.min(1, len/350);          // stronger the farther away
    tx = dx/len*reach; ty = dy/len*reach;
    tr = Math.max(-18, Math.min(18, dx/25));   // up to ±18deg colour swing
    kick();
  }, {passive:true});
  document.addEventListener('pointerleave', function(){ tx=-0.6; ty=-0.8; tr=0; kick(); });
  kick();
})();

/* 3. COOKIE BANNER ----------------------------------------------- */
(function(){
  var box = document.getElementById('cookie');
  if(!box) return;
  box.querySelectorAll('[data-cookie]').forEach(function(b){
    b.addEventListener('click', function(){ box.classList.add('hide'); });
  });
})();

/* ADDED: FLOW ON HOVER - rainbow colours keep turning while the pointer is on the triangle */
(function(){
  var stage = document.getElementById('triStage');
  if (!stage) return;
  var poly = stage.querySelector('.tri polygon');
  var hover = false, flow = 0, speed = 0, last = 0, raf = 0;
  function step(t){
    var dt = last ? Math.min(50, t - last) : 16; last = t;
    speed += ((hover ? 0.09 : 0) - speed) * Math.min(1, dt / 250);
    flow = (flow + speed * dt) % 360;
    stage.style.setProperty('--flow', flow.toFixed(2) + 'deg');
    if (hover || speed > 0.001) raf = requestAnimationFrame(step); else { raf = 0; last = 0; }
  }
  // hover area = the whole triangle box (not only the edge of the shape)
  var area = document.getElementById('triWrap') || poly;
  area.addEventListener('pointerenter', function(){ hover = true; stage.classList.add('flowing'); if (!raf) raf = requestAnimationFrame(step); });
  area.addEventListener('pointerleave', function(){ hover = false; stage.classList.remove('flowing'); });
})();

}
