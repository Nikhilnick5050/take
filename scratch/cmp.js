const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const go=async(url,cs)=>{const p=await (await b.newContext({viewport:{width:1280,height:900}, colorScheme:cs})).newPage(); await p.goto(url,{waitUntil:'load',timeout:30000}); await p.waitForTimeout(2500); return p;};
  let urls=['file:///data/vercel-clone/index.html'];
  let p; for(const u of urls){ try{ p=await go(u,'light'); break;}catch(e){console.log('fail',u,e.message.split('\n')[0]);} }
  await p.evaluate(()=>{document.documentElement.setAttribute('data-theme','light'); const c=document.getElementById('cookie'); if(c) c.style.display='none';});
  await p.mouse.move(100,700); await p.waitForTimeout(800);
  await p.screenshot({path:'cm/mine-idle.png'});
  await p.mouse.click(640,450); await p.waitForTimeout(1800); await p.screenshot({path:'cm/mine-on.png'});
  await p.mouse.click(640,450); await p.waitForTimeout(1200); await p.screenshot({path:'cm/mine-off.png'});
  const logo=await p.evaluate(()=>{const r=document.querySelector('.lg-spacex').getBoundingClientRect();return [r.x,r.y,r.width,r.height]});
  console.log('mine spacex box',logo);
  await b.close();
})();
