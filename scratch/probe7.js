
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com/', {waitUntil:'networkidle', timeout:60000});
  await p.waitForTimeout(4000);
  await p.mouse.move(20,880); await p.waitForTimeout(800);
  const L = await p.evaluate(()=>{
    const g = el => { const r = el.getBoundingClientRect(); return {x:Math.round(r.x),y:Math.round(r.y),w:Math.round(r.width),h:Math.round(r.height)} };
    const h1 = document.querySelector('h1');
    const deploy = [...document.querySelectorAll('a,button')].find(e=>e.textContent.trim()==='Deploy now');
    const talk = [...document.querySelectorAll('a,button')].find(e=>e.textContent.trim()==='Talk to sales');
    const right = [...document.querySelectorAll('p,div,span')].find(e=>e.textContent.trim()==='For coding agents');
    const svg = [...document.querySelectorAll('svg')].find(s=>{const r=s.getBoundingClientRect();return r.width>150&&r.height>150});
    const poly = svg.querySelector('polygon');
    const pr = poly.getBoundingClientRect();
    const header = document.querySelector('header') || document.querySelector('nav');
    return {
      h1: g(h1), deploy: g(deploy), talk: g(talk), right: g(right),
      svgBox: g(svg), polyBox: {x:Math.round(pr.x),y:Math.round(pr.y),w:Math.round(pr.width),h:Math.round(pr.height)},
      header: header?g(header):null,
      h1Text: h1.textContent.trim(),
      h1Lines: h1.getClientRects().length
    };
  });
  console.log('REAL LAYOUT:', JSON.stringify(L,null,1));
  await b.close();
})();
