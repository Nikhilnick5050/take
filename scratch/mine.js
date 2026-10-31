
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}});
  const p = await ctx.newPage();
  await p.goto('file:///data/vercel-clone/index.html', {waitUntil:'load'});
  await p.waitForTimeout(2000);
  const m = await p.evaluate(()=>{
    const svg=document.querySelector('.tri'); const r=svg.getBoundingClientRect();
    const h1=document.querySelector('.hero-title'); const hc=getComputedStyle(h1);
    const right=document.querySelector('.hero-right'); const rc=getComputedStyle(right);
    const rr=right.getBoundingClientRect();
    const glow=document.querySelector('.tri-glow'); const gr=glow.getBoundingClientRect();
    return {
      tri:{w:Math.round(r.width),h:Math.round(r.height),x:Math.round(r.x),y:Math.round(r.y)},
      h1:{fs:hc.fontSize,lh:hc.lineHeight,fw:hc.fontWeight,ls:hc.letterSpacing,x:Math.round(h1.getBoundingClientRect().x),y:Math.round(h1.getBoundingClientRect().y)},
      right:{fs:rc.fontSize,lh:rc.lineHeight,x:Math.round(rr.x),w:Math.round(rr.width),y:Math.round(rr.y)},
      glow:{w:Math.round(gr.width),h:Math.round(gr.height)},
      bodyBg:getComputedStyle(document.body).backgroundColor
    };
  });
  console.log('MINE:', JSON.stringify(m,null,1));
  await p.screenshot({path:'/data/scratch/mine-light.png'});
  await b.close();
})();
