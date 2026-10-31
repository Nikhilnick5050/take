const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com/', {waitUntil:'networkidle', timeout:60000});
  await p.waitForTimeout(4000);
  const L = await p.evaluate(()=>{
    const out=[];
    document.querySelectorAll('svg').forEach(s=>{
      const r=s.getBoundingClientRect();
      if(r.width>40 && r.width<320 && r.height>8 && r.height<60){
        out.push({y:Math.round(r.y),x:Math.round(r.x),w:Math.round(r.width),h:Math.round(r.height),
          label:(s.getAttribute('aria-label')||'').slice(0,30)});
      }
    });
    out.sort((a,b)=>a.y-b.y);
    return out;
  });
  console.log(JSON.stringify(L,null,1));
  await b.close();
})();
