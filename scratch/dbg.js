const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'dark'});
  const p = await ctx.newPage();
  await p.goto('file:///data/vercel-clone/index.html', {waitUntil:'load'});
  await p.waitForTimeout(2000);
  const d = await p.evaluate(()=>{
    const g = document.getElementById('triGlow');
    const cs = getComputedStyle(g);
    const r = g.getBoundingClientRect();
    const root = getComputedStyle(document.documentElement);
    return {
      box:{x:Math.round(r.x),y:Math.round(r.y),w:Math.round(r.width),h:Math.round(r.height)},
      bg: cs.backgroundImage.slice(0,300),
      filter: cs.filter, opacity: cs.opacity, display: cs.display, visibility: cs.visibility,
      zIndex: cs.zIndex, transform: cs.transform,
      vars: {
        core: root.getPropertyValue('--glow-core'),
        edge: root.getPropertyValue('--glow-edge'),
        mid: root.getPropertyValue('--glow-mid'),
        far: root.getPropertyValue('--glow-far'),
        size: root.getPropertyValue('--glow-size'),
        blur: root.getPropertyValue('--glow-blur')
      },
      theme: document.documentElement.getAttribute('data-theme')
    };
  });
  console.log(JSON.stringify(d,null,1));
  await b.close();
})();
