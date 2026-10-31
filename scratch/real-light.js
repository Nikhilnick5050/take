const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light', userAgent:'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130 Safari/537.36'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com', {waitUntil:'domcontentloaded', timeout:60000});
  await p.waitForTimeout(5000);
  // spacex logo
  const logo = await p.evaluate(()=>{
    const els=[...document.querySelectorAll('svg')].filter(s=>{const r=s.getBoundingClientRect();return r.y>790&&r.y<880&&r.x>650&&r.x<850});
    return els.map(s=>s.outerHTML);
  });
  require('fs').writeFileSync('spacex-real.svg', logo.join('\n\n'));
  console.log('SPACEX svgs:', logo.length, logo.map(x=>x.length));
  // canvas info + triangle chain styles
  const info = await p.evaluate(()=>{
    const c=document.querySelector('canvas'); const cs=c?getComputedStyle(c):null;
    return {canvas:c?{op:cs.opacity,cls:c.className,parent:c.parentElement.className,mix:cs.mixBlendMode}:null};
  });
  console.log(JSON.stringify(info,null,1));
  await p.screenshot({path:'rl-0.png'});
  await p.mouse.move(640,440);
  for (const t of [150,400,800,1500,3000]) { await p.waitForTimeout(t===150?150:t-[150,400,800,1500,3000][[150,400,800,1500,3000].indexOf(t)-1]); await p.screenshot({path:`rl-h${t}.png`});
    const st=await p.evaluate(()=>{const c=document.querySelector('canvas');const poly=document.querySelector('polygon');const r=poly.getBoundingClientRect();let ch=[];let e=poly;for(let i=0;i<6&&e;i++){const s=getComputedStyle(e);ch.push(s.transform+'|'+s.opacity);e=e.parentElement;}return {canvasOp:c&&getComputedStyle(c).opacity,poly:[r.x,r.y,r.width,r.height].map(Math.round),chain:ch}});
    console.log(t, JSON.stringify(st)); }
  await p.mouse.move(300,250); await p.waitForTimeout(1200); await p.screenshot({path:'rl-move.png'});
  await b.close();
})();
