# Vercel hero clone — measured spec

All numbers read off vercel.com at 1280x900 via computed styles + pixel scans.

## Layout (light and dark identical)
| Element | x | y | w | h |
|---|---|---|---|---|
| header | 0 | 0 | 1280 | 64 |
| h1 box | 24 | 328 | 444 | 128 |
| h1 line 1 "Agentic" | 24 | 318 | 195 | 83 |
| h1 line 2 "Infrastructure" | 24 | 382 | 344 | 83 |
| Deploy now | 24 | 488 | 124 | 40 |
| Talk to sales | 160 | 488 | 133 | 40 |
| triangle svg | 529 | 337 | 222 | 222 |
| triangle polygon | 544 | 337 | 192 | 166 |
| right nav | 892 | 352 | 364 | 152 |
| right items | 900 | 385 / 417 / 449 | — | 21 |

Hero row: flex, justify-content:space-between, align-items:center, padding 40px 0.
Triangle is absolutely centred in the row (not a grid column).

## Type
- h1: 64px / 64px / weight 400 / letter-spacing -3.84px / #171717
- right column: 16px / 24px / weight 450 / #171717 (Geist Sans, not Mono)
- nav: 14px / weight 400 / #4d4d4d
- buttons: 0.15s cubic-bezier(0.4,0,0.2,1)

## Colours
- light bg #fafafa, dark bg #000000
- light fg #171717, dark fg #ededed
- light muted #4d4d4d, dark muted #a1a1a1
- border #ebebeb / #1f1f1f

## Glow (the hard part)
- Light theme: peak is 251 against a 250 background — ONE grey level.
  Imperceptible. Drawing it on canvas quantises into visible rings, so the
  clone renders nothing in light mode and keeps flat #fafafa.
- Dark theme: alpha = 0.43 * exp(-r / 110px), radius 450px.
  Measured: 110 at apex, 45 at 100px, 20 at 160px, 8 at 240px, 2 at 320px.
- Rendered on a 900x900 canvas (1 canvas px = 1 CSS px) so the falloff is smooth.
- Follows the cursor with a lerp (0.08), clamped to 60px travel.

## Motion
- Entrance: slide-in (h1), fade-in (cta, right, logos), soft-fade-in (triangle)
- Triangle hover: scale(1.035), 0.5s cubic-bezier(.22,1,.36,1)
- Scroll zoom: scale 1 -> 1.6 over the first 600px of scroll
- Theme toggle: 0.4s colour transition, saved to localStorage

## Known differences from the real site
1. Geist is loaded from Google Fonts, not Vercel's own CDN files.
2. Logos are text-based approximations (real ones are licensed trademarks).
3. The real hero glow is a WebGL shader; this is a 2D canvas gradient.
4. No drag-and-drop "Drop to deploy" widget (needs their backend).
5. Section 2 is a simplified stand-in, not the real Zapier/Global Delivery blocks.
