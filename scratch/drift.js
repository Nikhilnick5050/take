const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light',userAgent:'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130 Safari/537.36'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com', {waitUntil:'domcontentloaded', timeout:60000});
  await p.waitForTimeout(5000);
  const shot=async n=>{const buf=await p.screenshot({clip:{x:440,y:280,width:400,height:300}}); require('fs').writeFileSync(`dr/${n}.png`,buf)};
  await p.mouse.move(100,440,{steps:5});
  for(let i=0;i<6;i++){await p.waitForTimeout(1000);await shot('L'+i);}
  await p.mouse.move(1180,440,{steps:5});
  for(let i=0;i<6;i++){await p.waitForTimeout(1000);await shot('R'+i);}
  await p.mouse.move(640,120,{steps:5}); for(let i=0;i<4;i++){await p.waitForTimeout(1000);await shot('T'+i);}
  await p.mouse.move(640,800,{steps:5}); for(let i=0;i<4;i++){await p.waitForTimeout(1000);await shot('B'+i);}
  await b.close();
})();
