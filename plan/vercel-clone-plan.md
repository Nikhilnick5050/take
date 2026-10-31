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

## Round 3 — light mode watched live in the real browser (WebGL on)
- Idle (light): white halo behind apex + grey drop shadow down-right → `.tri-shade` + `drop-shadow(9px 12px 12px rgba(0,0,0,.30))`.
- Hover (light): shading fades out (0.7s), rainbow prism blooms from triangle centre (magenta top, blue right, cyan bottom-right, green bottom, yellow bottom-left, red left), stays on after the cursor leaves, breathes ~9s → `.tri-prism` conic-gradient + radial mask.
- Triangle does NOT scale on hover on the real site → removed scale(1.035).
- SpaceXAI logo: replaced with the real 397×48 SVG path from vercel.com.

## Cursor-follow light (watched live on vercel.com, light mode)
- The cursor acts as a light source. The grey shadow falls on the side AWAY from the cursor, and the white highlight sits on the side TOWARD it.
- Measured on the real site: cursor bottom-left (380,800) -> shadow centroid (585,591); cursor right (722,608) -> (624,604); default -> (661,608).
- The rainbow starts only when the cursor touches the black triangle itself, then leans slightly toward the cursor.
- Clone: `--sx/--sy` drive the triangle drop-shadow, `--hx/--hy` the white halo, `--px/--py/--rot` the rainbow. All eased with lerp 0.07.
- Clone check: cursor (150,820) -> shadow (690,546); default -> (658,612). Rainbow centre: cursor left -> x=631, cursor right -> x=663.

## Fix: no stretching on hover
- Removed the rainbow's scale bloom (0.85 → 1) and the 9s grow/shrink "breathing" animation.
- Triangle has `transform:none` so nothing can scale it on hover.
- Checked in the browser: triangle is 192 × 166 px before and after hover; the rainbow keeps one size and only moves with the cursor.


## Light mode re-measure (31 Oct 2026, recorded 30s video + timed screenshots)
- Hover: NO effect on the real site (no scale, no colour). Removed the hover/cursor-light code.
- Idle: grey only. Three shadows pointing one direction (wide 20.5px/blur22/23%, contact 7.3px/blur5.5/39%, edge 2.4px/blur1.6/50%), slowly orbiting ~0.075 rad/s. Idle diff vs real: 1.56 grey levels.
- Click: pastel rainbow ellipse fades in ~1s, colours sampled every 15deg, no rotation; second click fades it out. Bbox matches within 1px, mean diff 5.3.
- SpaceXAI: real SVG path, logo row opacity 1 (was .92). Diff 0.02.
