const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light',
    recordVideo:{dir:'/data/scratch/rec', size:{width:1280,height:900}},
    userAgent:'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130 Safari/537.36'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com', {waitUntil:'domcontentloaded', timeout:60000});
  await p.waitForTimeout(4000);
  const log=[];
  const snap=async(tag)=>{log.push(tag+' '+JSON.stringify(await p.evaluate(()=>{
    const c=document.querySelector('canvas');const poly=document.querySelector('polygon');
    const r=poly.getBoundingClientRect();
    let ch=[];let e=poly.closest('svg');for(let i=0;i<5&&e;i++){const s=getComputedStyle(e);ch.push((e.className.baseVal??e.className).toString().slice(0,60)+' T='+s.transform+' O='+s.opacity+' F='+s.filter);e=e.parentElement;}
    return {canvasOp:c&&getComputedStyle(c).opacity, fill:getComputedStyle(poly).fill, poly:[r.x,r.y,r.width,r.height].map(Math.round), chain:ch};
  })))};
  await snap('t0 idle');
  // approach and hover the triangle for ~15s with small movement
  await p.mouse.move(100,700); await p.waitForTimeout(500);
  const t0=Date.now();
  await p.mouse.move(640,450,{steps:25});
  for(let i=0;i<15;i++){
    await p.waitForTimeout(1000);
    const a=i*0.6; await p.mouse.move(640+Math.cos(a)*120, 450+Math.sin(a)*80,{steps:10});
    await snap('hover+'+(i+1)+'s');
  }
  await p.mouse.move(1200,800,{steps:20}); await p.waitForTimeout(1500); await snap('left');
  await ctx.close(); await b.close();
  require('fs').writeFileSync('/data/scratch/rec/log.txt', log.join('\n'));
})();
