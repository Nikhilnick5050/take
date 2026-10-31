
const { chromium } = require('playwright');
(async () => {
  const b = await chromium.launch({args:['--no-sandbox']});
  const ctx = await b.newContext({viewport:{width:1280,height:900}, colorScheme:'light'});
  const p = await ctx.newPage();
  await p.goto('https://vercel.com/', {waitUntil:'networkidle', timeout:60000});
  await p.waitForTimeout(2500);

  // BUTTONS
  const btns = await p.evaluate(()=>{
    const out=[];
    document.querySelectorAll('a,button').forEach(el=>{
      const t=el.textContent.trim();
      if(['Deploy now','Talk to sales','Sign Up','Log In','Get a Demo'].includes(t)){
        const cs=getComputedStyle(el);
        out.push({t, bg:cs.backgroundColor, color:cs.color, border:cs.border, radius:cs.borderRadius,
          h:Math.round(el.getBoundingClientRect().height), pad:cs.padding, fs:cs.fontSize, fw:cs.fontWeight,
          transition:cs.transition.slice(0,160), shadow:cs.boxShadow.slice(0,120)});
      }
    });
    return out;
  });
  console.log('=== BUTTONS ==='); console.log(JSON.stringify(btns,null,1));

  // hover Deploy now
  const dn = await p.evaluate(()=>{const el=[...document.querySelectorAll('a,button')].find(e=>e.textContent.trim()==='Deploy now');const r=el.getBoundingClientRect();return{x:r.x+r.width/2,y:r.y+r.height/2}});
  await p.mouse.move(dn.x,dn.y); await p.waitForTimeout(400);
  const hov = await p.evaluate(()=>{const el=[...document.querySelectorAll('a,button')].find(e=>e.textContent.trim()==='Deploy now');const cs=getComputedStyle(el);return{bg:cs.backgroundColor,color:cs.color,transform:cs.transform,shadow:cs.boxShadow.slice(0,150)}});
  console.log('=== Deploy now HOVER ===', JSON.stringify(hov));

  // NAV LINK hover
  const nav = await p.evaluate(()=>{const el=[...document.querySelectorAll('a,button')].find(e=>e.textContent.trim()==='Pricing');const r=el.getBoundingClientRect();return{x:r.x+r.width/2,y:r.y+r.height/2}});
  await p.mouse.move(nav.x,nav.y); await p.waitForTimeout(400);
  const nh = await p.evaluate(()=>{const el=[...document.querySelectorAll('a,button')].find(e=>e.textContent.trim()==='Pricing');const cs=getComputedStyle(el);return{color:cs.color,bg:cs.backgroundColor,transition:cs.transition.slice(0,120)}});
  console.log('=== Pricing HOVER ===', JSON.stringify(nh));

  // does the glow follow the mouse? sample canvas pixels at two mouse positions
  const sample = async (mx,my,label)=>{
    await p.mouse.move(mx,my); await p.waitForTimeout(900);
    const buf = await p.screenshot({clip:{x:100,y:88,width:1080,height:720}});
    const fs=require('fs'); fs.writeFileSync('/data/scratch/glow_'+label+'.png', buf);
    return buf.length;
  };
  await sample(640,540,'center');
  await sample(300,300,'topleft');
  console.log('=== glow screenshots saved ===');

  // typography
  const typo = await p.evaluate(()=>{
    const h1=document.querySelector('h1'); const cs=h1?getComputedStyle(h1):null;
    const out={h1: cs?{text:h1.textContent.trim().slice(0,60),fs:cs.fontSize,fw:cs.fontWeight,lh:cs.lineHeight,ls:cs.letterSpacing,ff:cs.fontFamily.slice(0,80),color:cs.color}:null};
    const right=[...document.querySelectorAll('p,div,span')].find(e=>e.textContent.trim()==='For coding agents');
    if(right){const rc=getComputedStyle(right);out.right={fs:rc.fontSize,fw:rc.fontWeight,lh:rc.lineHeight,color:rc.color,ff:rc.fontFamily.slice(0,60)}}
    return out;
  });
  console.log('=== TYPO ===', JSON.stringify(typo,null,1));
  await b.close();
})();
