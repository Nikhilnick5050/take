
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com/', {waitUntil:'networkidle', timeout:60000});
  await p.waitForTimeout(4000);
  await p.mouse.move(20,880); await p.waitForTimeout(600);

  const info = await p.evaluate(()=>{
    const g = el => { const r = el.getBoundingClientRect(); return {x:Math.round(r.x),y:Math.round(r.y),w:Math.round(r.width),h:Math.round(r.height)} };
    const h1 = document.querySelector('h1');
    // walk up from h1 to the hero grid
    let el = h1, chain=[];
    for (let i=0;i<7 && el;i++){
      const cs = getComputedStyle(el);
      chain.push({tag:el.tagName, cls:el.className.toString().slice(0,150), box:g(el),
        display:cs.display, gridCols:cs.gridTemplateColumns, justify:cs.justifyContent, align:cs.alignItems,
        padding:cs.padding, gap:cs.gap, transform:cs.transform, marginTop:cs.marginTop});
      el = el.parentElement;
    }
    // right column container
    const span = [...document.querySelectorAll('span')].find(e=>e.textContent.trim()==='For coding agents' && e.getBoundingClientRect().width>0);
    let rchain=[]; el = span;
    for (let i=0;i<5 && el;i++){
      const cs=getComputedStyle(el);
      rchain.push({tag:el.tagName, cls:el.className.toString().slice(0,150), box:g(el),
        display:cs.display, justifySelf:cs.justifySelf, alignSelf:cs.alignSelf, textAlign:cs.textAlign});
      el=el.parentElement;
    }
    // measure real text width of h1 line 2
    const range = document.createRange();
    range.selectNodeContents(h1);
    const rects = [...range.getClientRects()].map(r=>({x:Math.round(r.x),y:Math.round(r.y),w:Math.round(r.width),h:Math.round(r.height)}));
    return {chain, rchain, h1Rects:rects};
  });
  console.log(JSON.stringify(info,null,1));
  await b.close();
})();
