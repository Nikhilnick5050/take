
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}});
  const p = await ctx.newPage();
  await p.goto('file:///data/vercel-clone/index.html', {waitUntil:'load'});
  await p.waitForTimeout(3000);
  const f = await p.evaluate(async ()=>{
    await document.fonts.ready;
    const loaded = [];
    document.fonts.forEach(f=>loaded.push(f.family+' '+f.weight+' '+f.status));
    const h1=document.querySelector('.hero-title');
    const cs=getComputedStyle(h1);
    // measure text width with canvas
    const c=document.createElement('canvas').getContext('2d');
    c.font='400 64px Geist';
    const wGeist=c.measureText('Infrastructure').width;
    c.font='400 64px sans-serif';
    const wSans=c.measureText('Infrastructure').width;
    return {loaded, ff:cs.fontFamily, wGeist:Math.round(wGeist), wSans:Math.round(wSans),
      h1w:Math.round(h1.getBoundingClientRect().width)};
  });
  console.log(JSON.stringify(f,null,1));
  await b.close();
})();
