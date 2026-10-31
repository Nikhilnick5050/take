
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com/', {waitUntil:'networkidle', timeout:60000});
  await p.waitForTimeout(3000);
  const info = await p.evaluate(() => {
    const out = {svgs:[], keyframes:[], cssVars:{}};
    document.querySelectorAll('svg').forEach((s,i)=>{
      const r = s.getBoundingClientRect();
      if (r.width > 80 && r.height > 60) {
        const cs = getComputedStyle(s);
        out.svgs.push({i, w:Math.round(r.width), h:Math.round(r.height), x:Math.round(r.x), y:Math.round(r.y),
          html: s.outerHTML.slice(0,500),
          transform: cs.transform, filter: cs.filter, transition: cs.transition,
          animation: cs.animation, willChange: cs.willChange, opacity: cs.opacity});
      }
    });
    for (const sheet of document.styleSheets) {
      let rules; try { rules = sheet.cssRules } catch(e){ continue }
      if (!rules) continue;
      for (const r of rules) {
        if (r.type === CSSRule.KEYFRAMES_RULE) out.keyframes.push(r.cssText.slice(0,700));
      }
    }
    const root = getComputedStyle(document.documentElement);
    ['--ds-gray-100','--ds-background-100','--ds-gray-1000','--ds-background-200'].forEach(v=>{
      out.cssVars[v]=root.getPropertyValue(v);
    });
    out.bodyBg = getComputedStyle(document.body).backgroundColor;
    out.htmlClass = document.documentElement.className;
    return out;
  });
  console.log('=== SVGS ===');
  console.log(JSON.stringify(info.svgs, null, 1));
  console.log('=== BODY BG ===', info.bodyBg, '| html class:', info.htmlClass);
  console.log('=== VARS ===', JSON.stringify(info.cssVars));
  console.log('=== KEYFRAMES (' + info.keyframes.length + ') ===');
  info.keyframes.forEach(k=>console.log(k));
  await b.close();
})();
