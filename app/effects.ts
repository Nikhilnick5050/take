/* Ported 1:1 from the approved HTML (cdn.classgrid.in/ai-generated/1793453291115-index.html)
   + 3-click cycle (off -> light -> rainbow -> off) + flow on hover. */
let started = false;

export function initPage() {
  if (started) return;
  started = true;
  const root = document.documentElement;
  const isDark = () => root.getAttribute("data-theme") === "dark";

  /* 1. THEME — the light/dark switch is gone; the page is always dark. */
  root.setAttribute("data-theme", "dark");

  const stage = document.getElementById("triStage") as HTMLElement | null;
  const glow = document.getElementById("triGlow") as HTMLElement | null;
  const canvas = document.getElementById("glowCanvas") as HTMLCanvasElement | null;
  const tri = stage?.querySelector(".tri") as SVGElement | null;
  if (!stage || !glow || !canvas || !tri) return;

  /* 2. GLOW canvas (dark theme white halo) */
  const ctx = canvas.getContext("2d")!;
  const SIZE = 900;
  const dpr = Math.min(window.devicePixelRatio || 1, 2);
  function draw() {
    canvas!.width = SIZE * dpr; canvas!.height = SIZE * dpr;
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, SIZE, SIZE);
    if (!isDark()) return;
    const r = SIZE / 2;
    const g = ctx.createRadialGradient(r, r, 0, r, r, r);
    for (let i = 0; i <= 30; i++) {
      const t = i / 30;
      let a = 0.42 * Math.exp(-(t * r) / 140) * Math.pow(1 - t, 1.6);
      if (i === 30) a = 0;
      g.addColorStop(t, "rgba(255,255,255," + a.toFixed(4) + ")");
    }
    ctx.fillStyle = g; ctx.beginPath(); ctx.arc(r, r, r, 0, Math.PI * 2); ctx.fill();
  }
  draw();
  new MutationObserver(draw).observe(root, { attributes: true, attributeFilter: ["data-theme"] });

  /* 2b. CLICK CYCLE: off -> light -> rainbow -> off (both themes) */
  const ORDER = ["off", "light", "rainbow"];
  const cycle = () => {
    const cur = stage.getAttribute("data-state") || "off";
    stage.setAttribute("data-state", ORDER[(ORDER.indexOf(cur) + 1) % ORDER.length]);
    kick();
  };
  const poly = stage.querySelector(".tri polygon")!;
  poly.addEventListener("click", cycle);
  poly.addEventListener("touchstart", (e) => { e.preventDefault(); cycle(); }, { passive: false });

  /* 2c. FLOW ON HOVER */
  let hovering = false;
  poly.addEventListener("pointerenter", () => { hovering = true; stage.classList.add("flowing"); kick(); });
  poly.addEventListener("pointerleave", () => { hovering = false; stage.classList.remove("flowing"); kick(); });

  /* 2d. CURSOR LIGHT (light theme only; dark theme does not follow the cursor) */
  let tx = -0.6, ty = -0.8, cx = tx, cy = ty, tr = 0, cr = 0;
  let gtx = 0, gty = 0, gcx = 0, gcy = 0;
  let flow = 0, flowSpeed = 0, orbit = 0, last = performance.now();
  let raf: number | null = null;

  function frame(now: number) {
    const dt = Math.min(64, now - last); last = now;
    const dark = isDark();
    const state = stage!.getAttribute("data-state");
    // flow: speeds up smoothly while hovering, eases to a stop when the cursor leaves
    const target = hovering && state !== "off" ? 0.09 : 0; // deg per ms (~90deg/s)
    flowSpeed += (target - flowSpeed) * 0.06;
    flow = (flow + flowSpeed * dt) % 360;
    orbit += flowSpeed * dt * (Math.PI / 180);
    stage!.style.setProperty("--flow", flow.toFixed(2) + "deg");

    let ex = cx, ey = cy;
    if (!dark) {
      cx += (tx - cx) * 0.07; cy += (ty - cy) * 0.07; cr += (tr - cr) * 0.07;
      // in the LIGHT state, hovering makes the light orbit the triangle (shadow flows around)
      const amp = Math.min(1, flowSpeed / 0.09) * (state === "light" ? 0.55 : 0);
      ex = cx + Math.cos(orbit) * amp; ey = cy + Math.sin(orbit) * amp;
      gcx += (gtx - gcx) * 0.08; gcy += (gty - gcy) * 0.08;
      glow!.style.transform = "translate3d(" + gcx.toFixed(2) + "px," + gcy.toFixed(2) + "px,0)";
    } else {
      cr += (0 - cr) * 0.07;
    }
    const s = stage!.style;
    s.setProperty("--hx", (ex * 16).toFixed(2) + "px");
    s.setProperty("--hy", (ey * 16).toFixed(2) + "px");
    tri!.style.setProperty("--sx", (-ex * 13).toFixed(2) + "px");
    tri!.style.setProperty("--sy", (-ey * 13 + 3).toFixed(2) + "px");
    s.setProperty("--px", (cx * 45).toFixed(2) + "px");
    s.setProperty("--py", (cy * 35).toFixed(2) + "px");
    s.setProperty("--rot", cr.toFixed(2) + "deg");

    const moving = Math.abs(tx - cx) > 0.002 || Math.abs(ty - cy) > 0.002 || Math.abs(tr - cr) > 0.05 ||
      Math.abs(gtx - gcx) > 0.1 || Math.abs(gty - gcy) > 0.1 || flowSpeed > 0.0005 || hovering;
    raf = moving ? requestAnimationFrame(frame) : null;
  }
  function kick() { if (!raf) { last = performance.now(); raf = requestAnimationFrame(frame); } }

  window.addEventListener("pointermove", (e) => {
    if (isDark()) return; // dark: no cursor movement
    const r = tri.getBoundingClientRect();
    const dx = e.clientX - (r.left + r.width / 2);
    const dy = e.clientY - (r.top + r.height * 0.62);
    const len = Math.sqrt(dx * dx + dy * dy) || 1;
    const reach = Math.min(1, len / 350);
    tx = (dx / len) * reach; ty = (dy / len) * reach;
    tr = Math.max(-18, Math.min(18, dx / 25));
    const sr = stage.getBoundingClientRect();
    const gx = e.clientX - (sr.left + sr.width / 2), gy = e.clientY - (sr.top + sr.height / 2);
    const gl = Math.sqrt(gx * gx + gy * gy) || 1, k = Math.min(1, 60 / gl);
    gtx = gx * k; gty = gy * k;
    kick();
  }, { passive: true });
  document.addEventListener("pointerleave", () => { tx = -0.6; ty = -0.8; tr = 0; gtx = 0; gty = 0; kick(); });
  new MutationObserver(() => { if (isDark()) glow.style.transform = ""; kick(); })
    .observe(root, { attributes: true, attributeFilter: ["data-theme"] });
  kick();

  /* 3. COOKIE BANNER */
  const box = document.getElementById("cookie");
  box?.querySelectorAll("[data-cookie]").forEach((b) =>
    b.addEventListener("click", () => box.classList.add("hide")));
}
