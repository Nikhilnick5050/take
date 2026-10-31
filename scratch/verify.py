from playwright.sync_api import sync_playwright
from PIL import Image
import pathlib

url = "file:///data/vercel-clone/index.html"
out = pathlib.Path("/data/scratch"); out.mkdir(exist_ok=True)

def sample(path):
    im = Image.open(path).convert("RGB")
    w,h = im.size
    pts = [(int(w*0.5), int(h*0.30)), (int(w*0.42), int(h*0.42)), (int(w*0.58), int(h*0.42)), (int(w*0.5), int(h*0.55))]
    return [im.getpixel(p) for p in pts]

def colorful(px):
    return any(max(p)-min(p) > 18 for p in px)

with sync_playwright() as p:
    b = p.chromium.launch()
    pg = b.new_page(viewport={"width":1440,"height":900})
    pg.goto(url); pg.wait_for_timeout(1200)

    # ---- DARK MODE ----
    pg.evaluate("document.documentElement.setAttribute('data-theme','dark')")
    pg.wait_for_timeout(900)
    pg.screenshot(path=str(out/"d0.png"))
    pg.click(".tri polygon", force=True); pg.wait_for_timeout(1000)
    pg.screenshot(path=str(out/"d1.png"))
    pg.click(".tri polygon", force=True); pg.wait_for_timeout(1000)
    pg.screenshot(path=str(out/"d2.png"))
    cls = pg.evaluate("document.getElementById('triStage').className")
    print("dark stage class after clicks:", cls)
    for n in ["d0","d1","d2"]:
        px = sample(out/f"{n}.png"); print(n, px, "colorful:", colorful(px))

    # ---- LIGHT MODE ----
    pg.evaluate("document.documentElement.setAttribute('data-theme','light')")
    pg.wait_for_timeout(900)
    pg.screenshot(path=str(out/"l0.png"))
    pg.click(".tri polygon", force=True); pg.wait_for_timeout(1000)
    pg.screenshot(path=str(out/"l1.png"))
    pg.click(".tri polygon", force=True); pg.wait_for_timeout(1000)
    pg.screenshot(path=str(out/"l2.png"))
    print("light stage class after clicks:", pg.evaluate("document.getElementById('triStage').className"))
    for n in ["l0","l1","l2"]:
        px = sample(out/f"{n}.png"); print(n, px, "colorful:", colorful(px))
    b.close()
