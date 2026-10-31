import asyncio, os
from playwright.async_api import async_playwright
from PIL import Image

async def main():
    async with async_playwright() as p:
        b = await p.chromium.launch(args=["--force-color-profile=srgb"])
        pg = await b.new_page(viewport={"width":1280,"height":800}, device_scale_factor=1)
        await pg.goto("file:///data/vercel-clone/index.html")
        await pg.wait_for_timeout(900)

        async def probe(tag):
            await pg.wait_for_timeout(900)
            path=f"/data/scratch/{tag}.png"
            await pg.screenshot(path=path)
            im=Image.open(path).convert("RGB")
            # sample a ring around the triangle apex area
            pts=[(640,300),(600,340),(680,340),(640,380),(560,300),(720,300)]
            px=[im.getpixel(pt) for pt in pts]
            colorful=any(max(c)-min(c)>12 for c in px)
            bright=sum(sum(c) for c in px)/len(px)/3
            print(tag, px, "colorful:",colorful, "avg:",round(bright,1))

        # dark mode
        await pg.click("#themeToggle")
        await pg.wait_for_timeout(900)
        print("theme:", await pg.evaluate("document.documentElement.getAttribute('data-theme')"))
        await probe("d_initial")
        await pg.click(".tri polygon", force=True)
        await probe("d_click1")
        await pg.click(".tri polygon", force=True)
        await probe("d_click2")
        print("stage class dark:", await pg.evaluate("document.getElementById('triStage').className"))

        # light mode
        await pg.click("#themeToggle")
        await pg.wait_for_timeout(900)
        print("theme:", await pg.evaluate("document.documentElement.getAttribute('data-theme')"))
        await probe("l_initial")
        await pg.click(".tri polygon", force=True)
        await probe("l_click1")
        await pg.click(".tri polygon", force=True)
        await probe("l_click2")
        print("stage class light:", await pg.evaluate("document.getElementById('triStage').className"))
        await b.close()

asyncio.run(main())
