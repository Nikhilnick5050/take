# Python Playground — plan

## Goal
Give Nikhil a page he can open in the Studio Browser tab and type Python into directly,
with a Run button. Python must run fully in the browser (no server, no compile endpoint).

## Approach
- Single self-contained page: `playground/index.html` (HTML + CSS + JS inline, no build step).
- Editor: CodeMirror 5 from CDN (Python syntax highlighting, line numbers, bracket closing).
- Python runtime: Pyodide (CPython compiled to WebAssembly) from the jsDelivr CDN.
  Loaded lazily on the first Run so the page opens instantly.
- stdout / stderr captured into an output pane; `input()` wired to a browser prompt.
- Code auto-saved to `localStorage` so a refresh does not lose work.
- Example snippets in a dropdown for quick starts.

## Steps
1. Write `plan/playground-plan.md` (this file).
2. Write `playground/index.html`.
3. Serve the project on port 8080 with a static server.
4. Open `http://localhost:8080/playground/` in the Studio browser.
5. Click Run and confirm Pyodide loads and prints output.
6. Report the URL and how to use it.

## Notes
- No secrets, no API keys, no server code — everything is client-side.
- C++ can be added later behind a small server endpoint (a browser cannot compile C++ alone).
