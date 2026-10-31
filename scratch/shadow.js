const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox','--use-gl=swiftshader','--enable-webgl','--ignore-gpu-blocklist']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light',
    userAgent:'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130 Safari/537.36'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com', {waitUntil:'domcontentloaded', timeout:60000});
  await p.waitForTimeout(4000);
  await p.screenshot({path:'sh/idle.png'});
  const pts=[['center',640,440],['left',380,440],['right',900,440],['top',640,250],['bottom',640,650],['tl',420,300],['farR',1200,440]];
  await p.mouse.move(640,440,{steps:10});
  // fade timing
  for(const ms of [100,250,400,600,800,1200]){ await p.waitForTimeout(ms===100?100:ms-[100,250,400,600,800,1200][[100,250,400,600,800,1200].indexOf(ms)-1]);
    const o=await p.evaluate(()=>getComputedStyle(document.querySelector('polygon').closest('svg').parentElement.parentElement).opacity); console.log('fade',ms,o);}
  for(const [n,x,y] of pts){ await p.mouse.move(x,y,{steps:15}); await p.waitForTimeout(2500); await p.screenshot({path:`sh/${n}.png`});
    const o=await p.evaluate(()=>getComputedStyle(document.querySelector('polygon').closest('svg').parentElement.parentElement).opacity); console.log(n,x,y,'svgOp',o);}
  await p.mouse.move(1270,890,{steps:5});
  for(const ms of [200,500,900,1500,3000]){await p.waitForTimeout(ms-([0,200,500,900,1500][[200,500,900,1500,3000].indexOf(ms)])); const o=await p.evaluate(()=>getComputedStyle(document.querySelector('polygon').closest('svg').parentElement.parentElement).opacity); console.log('leave',ms,o);}
  // grab hover-related source hints
  const html=await p.evaluate(()=>{const s=document.querySelector('polygon').closest('svg').parentElement.parentElement; return s.parentElement.outerHTML.slice(0,1500)});
  console.log(html);
  await b.close();
})();
