
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com/', {waitUntil:'networkidle', timeout:60000});
  await p.waitForTimeout(4000);
  // move mouse far away so no hover glow
  await p.mouse.move(20, 880);
  await p.waitForTimeout(1500);
  await p.screenshot({path:'/data/scratch/real-nohover.png'});
  // now hover center of triangle
  await p.mouse.move(640, 540);
  await p.waitForTimeout(1500);
  await p.screenshot({path:'/data/scratch/real-hover2.png'});
  await b.close();
})();
