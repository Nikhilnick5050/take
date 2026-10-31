
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com/', {waitUntil:'networkidle', timeout:60000});
  await p.waitForTimeout(2500);

  const canv = await p.evaluate(()=>{
    const out=[];
    document.querySelectorAll('canvas').forEach(c=>{const r=c.getBoundingClientRect();
      out.push({w:Math.round(r.width),h:Math.round(r.height),x:Math.round(r.x),y:Math.round(r.y),cls:c.className.toString().slice(0,120)})});
    return out;
  });
  console.log('=== CANVASES ===', JSON.stringify(canv));

  // all triangles (light + dark variants)
  const tris = await p.evaluate(()=>{
    const out=[];
    document.querySelectorAll('polygon').forEach(pg=>{
      const svg=pg.closest('svg'); const r=svg.getBoundingClientRect();
      if(r.width<50) return;
      out.push({fill:pg.getAttribute('fill'), computedFill:getComputedStyle(pg).fill,
        w:Math.round(r.width),h:Math.round(r.height),
        svgCls:svg.className.baseVal.slice(0,120),
        parentCls:svg.parentElement.className.toString().slice(0,200),
        parentDisplay:getComputedStyle(svg.parentElement).display,
        parentTransform:getComputedStyle(svg.parentElement).transform,
        parentOpacity:getComputedStyle(svg.parentElement).opacity});
    });
    return out;
  });
  console.log('=== TRIANGLES ===');
  console.log(JSON.stringify(tris,null,1));

  // scroll zoom behaviour
  console.log('=== SCROLL ZOOM ===');
  for (const y of [0,100,300,600,900]) {
    await p.evaluate(v=>window.scrollTo(0,v), y);
    await p.waitForTimeout(700);
    const s = await p.evaluate(()=>{
      const svg=[...document.querySelectorAll('svg')].find(s=>{const r=s.getBoundingClientRect();return r.width>150&&r.height>150});
      if(!svg) return null;
      const r=svg.getBoundingClientRect();
      const par=svg.parentElement;
      const root=getComputedStyle(document.documentElement);
      return {scrollY:Math.round(window.scrollY), w:Math.round(r.width), h:Math.round(r.height),
        parTf:getComputedStyle(par).transform,
        freezeH:root.getPropertyValue('--hero-zoom-freeze-h'),
        shaderY:root.getPropertyValue('--hero-shader-y-offset')};
    });
    console.log(JSON.stringify(s));
  }
  await b.close();
})();
