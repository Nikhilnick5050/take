
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com/', {waitUntil:'networkidle', timeout:60000});
  await p.waitForTimeout(2500);

  // inspect the triangle's ancestor chain
  const chain = await p.evaluate(() => {
    const svg = [...document.querySelectorAll('svg')].find(s=>{const r=s.getBoundingClientRect();return r.width>150&&r.height>150});
    if(!svg) return 'no svg';
    const out=[]; let el=svg;
    for(let i=0;i<6&&el;i++){
      const cs=getComputedStyle(el); const r=el.getBoundingClientRect();
      out.push({tag:el.tagName, cls:(el.className.baseVal!==undefined?el.className.baseVal:el.className).toString().slice(0,180),
        w:Math.round(r.width),h:Math.round(r.height),
        transform:cs.transform, transition:cs.transition.slice(0,200), filter:cs.filter,
        boxShadow:cs.boxShadow.slice(0,200), animation:cs.animation.slice(0,150),
        bg:cs.backgroundColor, opacity:cs.opacity, mixBlend:cs.mixBlendMode});
      el=el.parentElement;
    }
    return out;
  });
  console.log('=== ANCESTOR CHAIN ===');
  console.log(JSON.stringify(chain,null,1));

  // hover the triangle and sample transform/scale over time
  const box = await p.evaluate(()=>{const s=[...document.querySelectorAll('svg')].find(s=>{const r=s.getBoundingClientRect();return r.width>150&&r.height>150});const r=s.getBoundingClientRect();return {x:r.x+r.width/2,y:r.y+r.height/2}});
  await p.mouse.move(box.x, box.y);
  console.log('=== HOVER SAMPLES (triangle svg) ===');
  for (let t=0;t<8;t++){
    const s = await p.evaluate(()=>{
      const svg=[...document.querySelectorAll('svg')].find(s=>{const r=s.getBoundingClientRect();return r.width>150&&r.height>150});
      const r=svg.getBoundingClientRect(); const cs=getComputedStyle(svg);
      const par=svg.parentElement; const pcs=getComputedStyle(par);
      return {w:Math.round(r.width),h:Math.round(r.height),tf:cs.transform,parTf:pcs.transform,parFilter:pcs.filter,parShadow:pcs.boxShadow.slice(0,120)};
    });
    console.log(t*200+'ms', JSON.stringify(s));
    await p.waitForTimeout(200);
  }
  await p.screenshot({path:'/data/scratch/hover.png'});

  // dark mode
  await p.emulateMedia({colorScheme:'dark'});
  await p.waitForTimeout(1200);
  const dark = await p.evaluate(()=>{
    const root=getComputedStyle(document.documentElement);
    const svg=[...document.querySelectorAll('svg')].find(s=>{const r=s.getBoundingClientRect();return r.width>150&&r.height>150});
    const poly=svg?svg.querySelector('polygon'):null;
    return {bodyBg:getComputedStyle(document.body).backgroundColor,
      htmlClass:document.documentElement.className,
      bg100:root.getPropertyValue('--ds-background-100'), bg200:root.getPropertyValue('--ds-background-200'),
      gray1000:root.getPropertyValue('--ds-gray-1000'),
      polyFill: poly?getComputedStyle(poly).fill:null,
      polyAttr: poly?poly.getAttribute('fill'):null,
      svgFilter: svg?getComputedStyle(svg).filter:null,
      parShadow: svg?getComputedStyle(svg.parentElement).boxShadow.slice(0,150):null};
  });
  console.log('=== DARK MODE ===');
  console.log(JSON.stringify(dark,null,1));
  await p.screenshot({path:'/data/scratch/dark.png'});
  await b.close();
})();
