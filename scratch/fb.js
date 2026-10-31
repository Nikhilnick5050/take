const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const p = await (await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light',userAgent:'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130 Safari/537.36'})).newPage();
  await p.goto('https://vercel.com', {waitUntil:'domcontentloaded', timeout:60000});
  await p.waitForTimeout(3000);
  const r=await p.evaluate(()=>{const f=document.querySelector('[data-hero-static-fallback]');
    return {attr:f.getAttribute('data-hero-static-fallback'), imgs:[...f.querySelectorAll('img')].map(i=>({src:i.currentSrc, wrap:i.parentElement.className, style:i.getAttribute('style')})), gl: (()=>{const c=document.createElement('canvas');const g=c.getContext('webgl2')||c.getContext('webgl');if(!g)return 'none';const d=g.getExtension('WEBGL_debug_renderer_info');return d?g.getParameter(d.UNMASKED_RENDERER_WEBGL):'yes';})()}});
  console.log(JSON.stringify(r,null,1));
  for (const [i,im] of r.imgs.entries()){ const res=await p.request.get(im.src); require('fs').writeFileSync(`fb${i}.webp`, await res.body()); }
  await b.close();
})();
