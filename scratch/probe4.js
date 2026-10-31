
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'dark'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com/', {waitUntil:'networkidle', timeout:60000});
  await p.waitForTimeout(3500);
  await p.screenshot({path:'/data/scratch/real-dark.png'});
  // what is visible in dark: check the dark variant triangle
  const d = await p.evaluate(()=>{
    const out=[];
    document.querySelectorAll('svg').forEach(s=>{const r=s.getBoundingClientRect();
      if(r.width>100&&r.height>100){const pg=s.querySelector('polygon');
      out.push({w:Math.round(r.width),h:Math.round(r.height),fill:pg?pg.getAttribute('fill'):null,
        computed:pg?getComputedStyle(pg).fill:null,
        parentDisplay:getComputedStyle(s.parentElement).display,
        parentCls:s.parentElement.className.toString().slice(0,220)})}});
    return out;
  });
  console.log(JSON.stringify(d,null,1));
  await b.close();
})();
