# Classgrid AI — Landing Page Plan (approved)

Goal: a single-page marketing site for Classgrid AI with hero, features, pricing and a sign-up CTA.

Route: `/classgrid` (kept separate so the existing Vercel hero at `/` is untouched).

## Steps
1. **Set up** — new route folder `app/classgrid/`, own stylesheet `landing.css`, no new dependencies (Next 16 + React 19 already installed).
2. **Hero** — headline, subtext, primary CTA, secondary CTA, product mock panel.
3. **Features grid** — six cards with icons and short copy.
4. **Pricing** — three tiers with a monthly / yearly toggle (client component).
5. **Footer + CTA** — sign-up form (client component), social links, legal links.
6. **Verify** — `npm run build` and a dev-server smoke test.

## Files
- `app/classgrid/page.tsx` — page shell + sections
- `app/classgrid/landing.css` — all styles, light/dark aware
- `app/classgrid/pricing.tsx` — pricing toggle
- `app/classgrid/signup.tsx` — sign-up form
