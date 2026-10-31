const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light'});
  const p = await ctx.newPage();
  await p.goto('file:///data/vercel-clone/index.html', {waitUntil:'load'});
  await p.waitForTimeout(2000);

  // HOVER test on triangle
  const before = await p.evaluate(()=>getComputedStyle(document.querySelector('.tri')).transform);
  await p.mouse.move(640, 420);
  await p.waitForTimeout(700);
  const after = await p.evaluate(()=>getComputedStyle(document.querySelector('.tri')).transform);
  console.log('HOVER  before:', before, '-> after:', after);

  // GLOW follow test
  const g1 = await p.evaluate(()=>document.getElementById('triGlow').style.transform);
  await p.mouse.move(300, 200);
  await p.waitForTimeout(700);
  const g2 = await p.evaluate(()=>document.getElementById('triGlow').style.transform);
  console.log('GLOW   at centre:', g1, '-> moved:', g2);

  // SCROLL zoom
  await p.evaluate(()=>window.scrollTo(0,300));
  await p.waitForTimeout(500);
  const sc = await p.evaluate(()=>document.getElementById('triWrap').style.transform);
  console.log('SCROLL at 300px:', sc);

  // THEME toggle
  await p.evaluate(()=>window.scrollTo(0,0));
  await p.click('#themeToggle');
  await p.waitForTimeout(600);
  const th = await p.evaluate(()=>({theme:document.documentElement.getAttribute('data-theme'), bg:getComputedStyle(document.body).backgroundColor}));
  console.log('THEME  after toggle:', JSON.stringify(th));

  // button hover
  await p.click('#themeToggle');
  await p.waitForTimeout(400);
  const btn = await p.evaluate(()=>{const e=[...document.querySelectorAll('.btn')].find(x=>x.textContent.trim()==='Deploy now');const r=e.getBoundingClientRect();return {x:r.x+r.width/2,y:r.y+r.height/2}});
  await p.mouse.move(btn.x, btn.y);
  await p.waitForTimeout(400);
  const bo = await p.evaluate(()=>{const e=[...document.querySelectorAll('.btn')].find(x=>x.textContent.trim()==='Deploy now');return getComputedStyle(e).opacity});
  console.log('BTN    Deploy now hover opacity:', bo);

  await b.close();
})();
