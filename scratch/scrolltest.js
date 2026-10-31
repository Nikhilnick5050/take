const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}});
  const p = await ctx.newPage();
  await p.goto('file:///data/vercel-clone/index.html', {waitUntil:'load'});
  await p.waitForTimeout(1500);
  const h = await p.evaluate(()=>document.body.scrollHeight);
  console.log('scrollHeight:', h, 'viewport: 900');
  for (const y of [0,150,300,600]) {
    await p.evaluate(v=>window.scrollTo(0,v), y);
    await p.waitForTimeout(400);
    const t = await p.evaluate(()=>({sy:Math.round(window.scrollY), tf:document.getElementById('triWrap').style.transform}));
    console.log('scrollY', t.sy, '->', t.tf);
  }
  await b.close();
})();
