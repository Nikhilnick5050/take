
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  for (const scheme of ['light','dark']) {
    const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:scheme});
    const p = await ctx.newPage();
    await p.goto('file:///data/vercel-clone/index.html', {waitUntil:'load'});
    await p.waitForTimeout(2500);
    await p.mouse.move(20,880); await p.waitForTimeout(700);
    await p.screenshot({path:'/data/scratch/mine-'+scheme+'.png'});
    const m = await p.evaluate(()=>{
      const g = el => { const r = el.getBoundingClientRect(); return {x:Math.round(r.x),y:Math.round(r.y),w:Math.round(r.width),h:Math.round(r.height)} };
      const h1=document.querySelector('.hero-title');
      const poly=document.querySelector('.tri polygon');
      const svg=document.querySelector('.tri');
      const right=document.querySelector('.hero-right');
      const d=[...document.querySelectorAll('.btn')].find(e=>e.textContent.trim()==='Deploy now');
      const t=[...document.querySelectorAll('.btn')].find(e=>e.textContent.trim()==='Talk to sales');
      const range=document.createRange(); range.selectNodeContents(h1);
      const rects=[...range.getClientRects()].map(r=>({x:Math.round(r.x),y:Math.round(r.y),w:Math.round(r.width)}));
      return {h1:g(h1), h1lines:rects, svg:g(svg), poly:g(poly), right:g(right), deploy:g(d), talk:g(t),
        bg:getComputedStyle(document.body).backgroundColor};
    });
    console.log(scheme.toUpperCase(), JSON.stringify(m));
    await ctx.close();
  }
  await b.close();
})();
