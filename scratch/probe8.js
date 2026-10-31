
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com/', {waitUntil:'networkidle', timeout:60000});
  await p.waitForTimeout(4000);
  await p.mouse.move(20,880); await p.waitForTimeout(800);
  const R = await p.evaluate(()=>{
    const out=[];
    document.querySelectorAll('*').forEach(el=>{
      const t = el.textContent ? el.textContent.trim() : '';
      if (t === 'For coding agents' || t === 'To ship apps and agents' || t === 'Automated by agents'){
        const r = el.getBoundingClientRect(); const cs = getComputedStyle(el);
        out.push({t, x:Math.round(r.x), y:Math.round(r.y), w:Math.round(r.width), h:Math.round(r.height),
          fs:cs.fontSize, lh:cs.lineHeight, fw:cs.fontWeight, color:cs.color, ff:cs.fontFamily.slice(0,40), tag:el.tagName});
      }
    });
    // hero container
    const hero = document.querySelector('h1').closest('section') || document.querySelector('h1').parentElement.parentElement;
    const hr = hero.getBoundingClientRect();
    return {items:out, hero:{x:Math.round(hr.x),y:Math.round(hr.y),w:Math.round(hr.width),h:Math.round(hr.height)},
      vh:window.innerHeight, scrollH:document.body.scrollHeight};
  });
  console.log(JSON.stringify(R,null,1));
  await b.close();
})();
