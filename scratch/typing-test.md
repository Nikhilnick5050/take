# Typing Test — 200 Lines

Written on 31 October 2026, on request, to check that the editor streams
text live while the assistant is still typing. Every line below was typed
in order. This is a scratch note, not part of the app.

---

## 1. What we built today

The Vercel clone lives in `vercel-clone/index.html` and is a single file
with no build step, so it opens straight from disk or from a CDN.

The Python playground lives in `playground/index.html` and runs Pyodide in
the browser, which means `input()` works without any server at all.

Both pages are pushed to the `classgrid/20261031-1121` branch and sit in
pull request #1 on GitHub, waiting for a review and a merge.

---

## 2. The triangle, in plain words

The hero of the page is a single triangle drawn with CSS borders, not an
image, so it stays sharp at any size and can be animated cheaply.

In light mode the triangle sits on white with a soft grey shadow under it.
Hovering makes a rainbow glow follow the cursor around the shape.

Clicking the triangle turns the rainbow on and keeps it on; clicking again
turns it off. That toggle is the whole interaction, and it is deliberate.

In dark mode the triangle sits on black with a white glow behind it.
Clicking turns the glow off, clicking again brings it back. No rainbow.

---

## 3. Rules I keep forgetting

Never add a feature to dark mode that was only asked for in light mode.
Never let the triangle scale on hover; the shape must not stretch.
Never put secrets in front-end code, only in the hosting environment.
Never push straight to main; always open a pull request instead.
Never claim a fix works without checking pixels in a headless browser.

---

## 4. How the check works

A small Playwright script opens the page, sets the colour scheme, clicks
the triangle, and samples the pixels around it before and after.

If the average brightness drops to zero in dark mode, the glow is off.
If colour pixels appear in light mode, the rainbow is on.

That is the whole test: numbers, not opinions, and no guessing.

---

## 5. Open threads

- Push the dark-mode glow toggle to pull request #1.
- Deploy the clone on Vercel for a real `vercel.app` link.
- Build the rest of the page below the hero: features, pricing, footer.
- Add a C++ compile endpoint to the playground.
- Write a short README so the repo explains itself to a stranger.

---

## 6. A note on typing

Typing two hundred lines is not hard; typing them in the right order is.
The editor shows each line as it arrives, so a long file looks like it is
being written by hand, which is exactly what this test is checking.

If you are reading this in the Files tab while it is still growing, the
streaming works. If it appeared all at once, it did not.

---

## 7. Colours and theme variables

Every colour in the page comes from a CSS variable, never a hard-coded
hex value, so switching the theme is a single class change on the body.

The light theme sets a white background, near-black text, and a grey
shadow under the triangle. The dark theme sets black and off-white.

The rainbow is a conic gradient masked to the triangle's shape, which is
why it can follow the cursor: the gradient's centre is the pointer.

Because the mask is the shape itself, the glow never spills outside the
triangle, and the page never gets a coloured rectangle behind it.

---

## 8. The playground

Pyodide is loaded from a CDN on first run, which takes a few seconds and
then stays in memory for the rest of the session.

`input()` is patched so that it pauses the program and waits for a line
typed into the output panel, then resumes where it stopped.

Code is saved to `localStorage` on every keystroke, so a refresh never
loses work, and six example programs are included for a quick start.

The number guessing game is the friendliest of the six, because it shows
a loop, a condition, and user input in about twenty lines.

---

## 9. Git and branches

The project is `Nikhilnick5050/take`, and the working branch is
`classgrid/20261031-1121`, which is where every change lands.

Pull request #1 holds the clone, the playground, the C++ and DSA files,
and the plan documents. It has not been merged yet.

Each push adds a commit with a plain message, so the history reads like a
short diary of the work rather than a list of hashes.

---

## 10. Deployment

Vercel is connected, so a push can become a preview link in under a
minute, and the production domain can be attached later.

The clone needs no build step, which means the deploy is a static upload
and cannot fail on a missing dependency.

The playground is the same: static files, one CDN script, no server.

---

## 11. Things that broke

The first dark-mode build had a rainbow that nobody asked for, because a
rule written for light mode was applied to both themes.

The second build removed the glow entirely in dark mode, which was also
wrong: the glow was wanted, only the rainbow was not.

The third build got it right by keeping two separate behaviours and
resetting the state whenever the theme changes.

That is the pattern to remember: one behaviour per theme, never shared.

---

## 12. Small wins

The triangle no longer zooms when the page scrolls, which was distracting.
The logo no longer stretches when the cursor passes over it.
The hover glow now follows the pointer instead of sitting in the centre.
The toggle survives a theme switch without leaving stale colour behind.

---

## 13. Next week

Finish the sections below the hero so the page reads like a real site.
Add a pricing table with three columns and a monthly and yearly switch.
Add a footer with four link columns and a language picker.
Test the whole page on a phone at 390 pixels wide.
Write the README and take two screenshots for it.

---

## 14. A short checklist before any push

- Does the page open from disk with no server?
- Does light mode still toggle the rainbow on and off?
- Does dark mode still toggle the white glow on and off?
- Is there any colour left in dark mode after two clicks?
- Does the triangle stay the same size on hover and on scroll?
- Does the logo keep its shape when the cursor passes over it?
- Does a theme switch reset the state cleanly?
- Does the page still look right at 390 pixels wide?
- Are there any secrets in the front-end files?
- Is the commit message readable by a stranger?

If every line is a yes, the push is safe.

---

## 15. Closing

That is the file. Two hundred lines, one at a time, no shortcuts.
If the count is off by a line or two, the point still stands: it streamed.

Typing this slowly on purpose is a strange exercise, but it proves the
editor keeps up with a long answer and never drops a line in the middle.

Thanks for the test, Nikhil. It was a fair one.

End of file.
Line 199.
Line 200.
