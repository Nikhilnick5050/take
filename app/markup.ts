export const MARKUP = `<header class="header">
  <div class="header-inner">
    <a href="#" class="logo" aria-label="Vercel home">
      <svg viewBox="0 0 76 65" fill="currentColor" aria-hidden="true"><path d="M37.5274 0L75.0548 65H0L37.5274 0Z"/></svg>
    </a>
    <nav class="nav">
      <button class="nav-item">Products
        <svg class="chev" viewBox="0 0 16 16" fill="none" stroke="currentColor" stroke-width="1.6"><path d="M4 6l4 4 4-4"/></svg>
      </button>
      <button class="nav-item">Resources
        <svg class="chev" viewBox="0 0 16 16" fill="none" stroke="currentColor" stroke-width="1.6"><path d="M4 6l4 4 4-4"/></svg>
      </button>
      <a href="#" class="nav-item">Enterprise</a>
      <a href="#" class="nav-item">Pricing</a>
    </nav>
    <div class="header-right">
      <button class="theme-toggle" id="themeToggle" aria-label="Toggle theme">
        <svg class="sun" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round">
          <circle cx="12" cy="12" r="4.2"/>
          <path d="M12 2.5v2M12 19.5v2M2.5 12h2M19.5 12h2M5.2 5.2l1.4 1.4M17.4 17.4l1.4 1.4M18.8 5.2l-1.4 1.4M6.6 17.4l-1.4 1.4"/>
        </svg>
        <svg class="moon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
          <path d="M20 14.5A8.5 8.5 0 019.5 4a8.5 8.5 0 1010.5 10.5z"/>
        </svg>
      </button>
      <a href="#" class="btn btn-ghost">Get a Demo</a>
      <a href="#" class="btn btn-ghost">Log In</a>
      <a href="#" class="btn btn-dark">Sign Up</a>
    </div>
  </div>
</header>

<section class="hero">
  <div class="hero-row">

    <div class="hero-left">
      <h1 class="hero-title anim-title">Agentic<br />Infrastructure</h1>
      <div class="hero-cta anim-cta">
        <a href="#" class="btn btn-dark btn-lg">Deploy now</a>
        <a href="#" class="btn btn-ghost btn-lg">Talk to sales</a>
      </div>
    </div>

    <div class="tri-stage" id="triStage" data-state="off">
      <div class="tri-prism" aria-hidden="true"></div>
      <div class="tri-shade" aria-hidden="true"></div>
      <div class="tri-glow" id="triGlow"><canvas id="glowCanvas"></canvas></div>
      <div class="tri-wrap anim-tri" id="triWrap">
        <svg class="tri" viewBox="0 0 222 222" xmlns="http://www.w3.org/2000/svg" role="img" aria-label="Vercel triangle">
          <polygon points="111,0 207,166 15,166" />
        </svg>
      </div>
    </div>

    <div class="hero-right anim-right">
      <p><a href="#">For coding agents</a></p>
      <p><a href="#">To ship apps and agents</a></p>
      <p><a href="#">Automated by agents</a></p>
    </div>

  </div>

  <div class="logos anim-logos">
    <svg class="lg-meta" viewBox="0 0 88 18" fill="currentColor" aria-label="Meta">
      <text x="0" y="14" font-family="Geist,Helvetica,Arial,sans-serif" font-size="17" font-weight="600" letter-spacing="-.5">Meta</text>
    </svg>
    <svg class="lg-doordash" viewBox="0 0 147 17" fill="currentColor" aria-label="DoorDash">
      <text x="0" y="13" font-family="Geist,Helvetica,Arial,sans-serif" font-size="14" font-weight="700" letter-spacing="1.2">DOORDASH</text>
    </svg>
    <svg class="lg-openai" viewBox="0 0 78 21" fill="currentColor" aria-label="OpenAI">
      <text x="0" y="16" font-family="Geist,Helvetica,Arial,sans-serif" font-size="18" font-weight="600" letter-spacing="-.6">OpenAI</text>
    </svg>
    <svg class="lg-spacex" viewBox="0 0 397 48" aria-label="SpaceXAI"><path fill="currentColor" d="M91.33 17.62c6.66 0 10.840 2.8 10.84 7.78v3.34c0 5.3-3.6 7.6-10.27 7.6H68.01V48h-9.16V17.62zM395 .01v.04C380.51 1.23 323.590 8.3 284.11 48h-14.65l1.64-1.63C279.36 38.38 315.9 4.53 395 0M152.37 47.99h-10.92l-4.83-6.65h-28.25l4.85-5.18h19.63l-9.44-12.97 5.57-6.86zm42.72-30.37c4.88 0 8.35 1.69 9.24 5.3h-34.36v19.37h34.36c-.98 4.01-3.07 5.7-8.98 5.7h-26c-4.49 0-8.71-1.82-8.71-6.67V24.3c0-4.85 4.22-6.67 8.71-6.67zm49.66 15.4h-18.48v9.27h31.68v5.7h-41V27.82h27.8zM325.86 48h-14.5l-12.89-9.390c2.38-1.86 4.9-3.72 7.33-5.4zM36.36 17.62c5.06 0 7.95 2.5 8.83 5.3H11.33v6.54h26.26v-.01c5.3.3 8.5 2.57 8.5 6.84v4.85c0 4.72-2.8 6.85-8.4 6.85h-26.8c-5.11 0-8.040-1.92-8.89-5.7h35.82v-6.95H11.2c-4.94.02-8.27-2.25-8.27-6.25v-4.8c0-4.59 3.24-6.67 8.89-6.67zM68 30.79h22.21c3.6 0 4-1.21 4-3.39v-1.19c0-2.14-.53-3.3-4.44-3.3H68.07zm227.33-5.7a171 171 0 0 0-7.93 4.98l-17-12.45h14.48zm-36.96-2.18h-41.42v-5.3h41.42z"/></svg>
    <svg class="lg-weather" viewBox="0 0 58 45" fill="currentColor" aria-label="The Weather Company">
      <text x="0" y="11" font-family="Geist,Helvetica,Arial,sans-serif" font-size="10" font-weight="600">The</text>
      <text x="0" y="23" font-family="Geist,Helvetica,Arial,sans-serif" font-size="10" font-weight="600">Weather</text>
      <text x="0" y="35" font-family="Geist,Helvetica,Arial,sans-serif" font-size="10" font-weight="600">Company</text>
    </svg>
    <svg class="lg-poly" viewBox="0 0 130 30" fill="currentColor" aria-label="Polymarket">
      <path d="M2 5l13 10-13 10V5z" fill="none" stroke="currentColor" stroke-width="1.7"/>
      <text x="22" y="21" font-family="Geist,Helvetica,Arial,sans-serif" font-size="16" font-weight="500" letter-spacing="-.4">Polymarket</text>
    </svg>
  </div>
</section>

<!-- ================= SECTION 2 ================= -->
<section class="section2">
  <div class="section2-inner">
    <h2 class="section2-title">Your complete platform<br />for the web.</h2>
    <p class="section2-sub">Deploy your first project in seconds. Then scale to millions.</p>
    <div class="section2-grid">
      <div class="s2-card">
        <div class="s2-icon">&#9650;</div>
        <h3>Deploy</h3>
        <p>Push to git and get a production URL. No config, no servers to manage.</p>
      </div>
      <div class="s2-card">
        <div class="s2-icon">&#9672;</div>
        <h3>Scale</h3>
        <p>Automatic scaling across 100+ edge locations worldwide, from day one.</p>
      </div>
      <div class="s2-card">
        <div class="s2-icon">&#9680;</div>
        <h3>Observe</h3>
        <p>Real-time logs, traces and analytics for every request your app serves.</p>
      </div>
    </div>
  </div>
</section>


<!-- ================= DEMO / LEGAL FOOTER ================= -->
<footer class="demo-footer">
  <div class="demo-footer-inner">
    <p><strong>Demo notice.</strong> This page is a design demonstration created by Classgrid AI to show its ability to rebuild a website's design from the browser. It is not the real Vercel website and is not affiliated with, endorsed by, sponsored by, or connected to Vercel Inc. in any way.</p>
    <p><strong>Trademarks.</strong> "Vercel", the Vercel logo, and the Vercel website design are trademarks and property of Vercel Inc. All other company names and logos shown on this page (including Meta, DoorDash, OpenAI, SpaceX, The Weather Company and Polymarket) are trademarks of their respective owners. Their use here does not mean any of these companies are customers of, partners of, or connected to Classgrid.</p>
    <p><strong>Not for real use.</strong> This page is for demonstration and educational purposes only. It is not a real product or service, offers nothing for sale, and is not intended for any commercial use. Its buttons and links do not work, and it does not collect any personal data.</p>
    <p><strong>Removal.</strong> If you are a rights owner and want this demo removed, contact <a href="mailto:support@classgrid.in">support@classgrid.in</a> and we will remove it.</p>
    <p>&copy; Vercel Inc. for the original design. Rebuilt by Classgrid AI for demo purposes only.</p>
  </div>
</footer>

<div class="cookie" id="cookie">
  <div>This site uses tracking technologies. You may opt in or opt out of the use of these technologies.</div>
  <div class="cookie-row">
    <button class="btn btn-ghost" data-cookie>Deny</button>
    <button class="btn btn-ghost" data-cookie>Accept all</button>
    <button class="btn btn-dark" data-cookie style="margin-left:auto">Consent Settings</button>
  </div>
</div>`;
