const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const p = await (await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light',userAgent:'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130 Safari/537.36'})).newPage();
  await p.goto('https://vercel.com', {waitUntil:'domcontentloaded', timeout:60000});
  await p.waitForTimeout(5000);
  await p.mouse.move(640,450,{steps:5}); await p.waitForTimeout(500);
  const fs=require('fs'); const clip={x:300,y:150,width:700,height:520};
  let t0=Date.now(); await p.mouse.click(640,450);
  const out=[];
  while(Date.now()-t0<9000){ const buf=await p.screenshot({clip}); out.push(['on',Date.now()-t0]); fs.writeFileSync(`tl/on_${out.length}.png`,buf); await p.waitForTimeout(150);}
  // move cursor around to see if rainbow follows
  await p.mouse.move(300,300,{steps:8}); await p.waitForTimeout(1500); fs.writeFileSync('tl/mv_tl.png',await p.screenshot({clip}));
  await p.mouse.move(1000,650,{steps:8}); await p.waitForTimeout(1500); fs.writeFileSync('tl/mv_br.png',await p.screenshot({clip}));
  await p.mouse.move(640,450,{steps:5}); await p.waitForTimeout(500);
  t0=Date.now(); await p.mouse.click(640,450); let n=0;
  while(Date.now()-t0<4000){ const buf=await p.screenshot({clip}); n++; out.push(['off',Date.now()-t0]); fs.writeFileSync(`tl/off_${n}.png`,buf); await p.waitForTimeout(150);}
  fs.writeFileSync('tl/times.json',JSON.stringify(out));
  await b.close();
})();
