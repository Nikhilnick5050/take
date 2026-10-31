const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const p = await (await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light',userAgent:'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130 Safari/537.36'})).newPage();
  await p.goto('https://vercel.com', {waitUntil:'domcontentloaded', timeout:60000});
  await p.waitForTimeout(5000);
  await p.mouse.move(640,450,{steps:10}); await p.waitForTimeout(800);
  await p.mouse.down(); await p.waitForTimeout(150); await p.screenshot({path:'ck/down.png'}); await p.mouse.up();
  for (const t of [300,1000,2500,5000]) { await p.waitForTimeout(t===300?300:t-[300,1000,2500,5000][[300,1000,2500,5000].indexOf(t)-1]); await p.screenshot({path:`ck/c${t}.png`}); }
  await p.mouse.click(640,450); await p.waitForTimeout(1500); await p.screenshot({path:'ck/c2nd.png'});
  await b.close();
})();
