// Vercel site footer, rebuilt 1:1 from vercel.com (measured live at 1280px):
// one 12-column grid (each column spans 2) with the triangle logo as the 13th cell
// on its own row, then a bottom bar: status on the left, icon buttons on the right.
// The demo/legal notice stays as a small strip underneath.
export const MARKUP_FOOTER = `<footer class="vf">
  <nav class="vf-nav" aria-label="Vercel Directory">
    <div class="vf-grid">
      <div class="vf-col">
        <h2>Agent Stack</h2>
        <ul>
          <li><a href="#">AI SDK</a></li>
          <li><a href="#">AI Gateway</a></li>
          <li><a href="#">Sandbox</a></li>
          <li><a href="#">Workflows</a></li>
          <li><a href="#">Connect <span class="vf-badge">New</span></a></li>
          <li><a href="#">Passport <span class="vf-badge">New</span></a></li>
          <li><a href="#">eve <span class="vf-badge">New</span></a></li>
        </ul>
      </div>
      <div class="vf-col">
        <h2>Core Platform</h2>
        <ul>
          <li><a href="#">CI/CD</a></li>
          <li><a href="#">Content Delivery</a></li>
          <li><a href="#">Fluid Compute</a></li>
          <li><a href="#">Observability</a></li>
        </ul>
      </div>
      <div class="vf-col">
        <h2>Security</h2>
        <ul>
          <li><a href="#">Platform Security</a></li>
          <li><a href="#">WAF</a></li>
          <li><a href="#">Bot Management</a></li>
          <li><a href="#">BotID</a></li>
        </ul>
      </div>
      <div class="vf-col">
        <h2>Tools</h2>
        <ul>
          <li><a href="#">Vercel Drop</a></li>
          <li><a href="#">Vercel Agent</a></li>
          <li><a href="#">Vercel Plugin</a></li>
          <li><a href="#">Agent Skills</a></li>
          <li><a href="#">Domains</a></li>
          <li><a href="#">v0</a></li>
        </ul>
      </div>
      <div class="vf-col">
        <h2>Frameworks</h2>
        <ul>
          <li><a href="#">eve <span class="vf-badge">New</span></a></li>
          <li><a href="#">Next.js</a></li>
          <li><a href="#">Nuxt</a></li>
          <li><a href="#">SvelteKit</a></li>
          <li><a href="#">Nitro</a></li>
          <li><a href="#">Turborepo</a></li>
          <li><a href="#">Tanstack Start</a></li>
          <li><a href="#">FastAPI</a></li>
          <li><a href="#">All frameworks</a></li>
        </ul>
      </div>
      <div class="vf-col">
        <h2>SDKs</h2>
        <ul>
          <li><a href="#">Vercel SDK</a></li>
          <li><a href="#">Workflow SDK</a></li>
          <li><a href="#">Flags SDK</a></li>
          <li><a href="#">Chat SDK</a></li>
          <li><a href="#">Queues SDK</a></li>
          <li><a href="#">Streamdown</a></li>
        </ul>
      </div>
      <div class="vf-col">
        <h2>Build</h2>
        <ul>
          <li><a href="#">AI Apps</a></li>
          <li><a href="#">Web Apps</a></li>
          <li><a href="#">Marketing Sites</a></li>
          <li><a href="#">Platforms</a></li>
          <li><a href="#">Commerce</a></li>
          <li><a href="#">Platform Engineers</a></li>
          <li><a href="#">Design Engineers</a></li>
        </ul>
      </div>
      <div class="vf-col">
        <h2>Learn</h2>
        <ul>
          <li><a href="#">Docs</a></li>
          <li><a href="#">Blog</a></li>
          <li><a href="#">Changelog</a></li>
          <li><a href="#">Knowledge Base</a></li>
          <li><a href="#">Academy</a></li>
          <li><a href="#">Articles</a></li>
          <li><a href="#">Community</a></li>
          <li><a href="#">Is Agentic</a></li>
        </ul>
      </div>
      <div class="vf-col">
        <h2>Explore</h2>
        <ul>
          <li><a href="#">Customers</a></li>
          <li><a href="#">Marketplace</a></li>
          <li><a href="#">Templates</a></li>
          <li><a href="#">Partner Finder</a></li>
          <li><a href="#">Vercel + AWS</a></li>
        </ul>
      </div>
      <div class="vf-col">
        <h2>Company</h2>
        <ul>
          <li><a href="#">About</a></li>
          <li><a href="#">Careers</a></li>
          <li><a href="#">Press</a></li>
          <li><a href="#">Events</a></li>
          <li><a href="#">Startups</a></li>
          <li><a href="#">Shipped on Vercel</a></li>
          <li><a href="#">Open Source Program</a></li>
          <li><a href="#">Enterprise</a></li>
          <li><a href="#">Pricing</a></li>
          <li><a href="#">Help</a></li>
        </ul>
      </div>
      <div class="vf-col">
        <h2>Legal &amp; Trust</h2>
        <ul>
          <li><a href="#">Privacy Policy</a></li>
          <li><a href="#">Terms of Service</a></li>
          <li><a href="#">Cookie Policy</a></li>
          <li><a href="#">DPA</a></li>
          <li><a href="#">Acceptable Use Policy</a></li>
          <li><a href="#">Legal (all documents)</a></li>
          <li><a href="#">Trust Center</a></li>
          <li><a href="#">Status</a></li>
          <li><a href="#">Cookie Preferences</a></li>
        </ul>
      </div>
      <div class="vf-col">
        <h2>Social</h2>
        <ul>
          <li><a href="#">GitHub</a></li>
          <li><a href="#">X</a></li>
          <li><a href="#">LinkedIn</a></li>
          <li><a href="#">YouTube</a></li>
          <li><a href="#">Instagram</a></li>
        </ul>
      </div>
      <div class="vf-logo-cell">
        <a href="#" aria-label="Vercel home">
          <svg viewBox="0 0 76 65" fill="currentColor" aria-hidden="true"><path d="M37.5274 0L75.0548 65H0L37.5274 0Z"/></svg>
        </a>
      </div>
    </div>
  </nav>

  <div class="vf-bar">
    <a href="#" class="vf-status">
      <span class="vf-dot"></span>
      <p class="vf-status-text">All systems normal.</p>
    </a>
    <div class="vf-bar-right">
      <button class="vf-agent" type="button" aria-label="Deployments"><svg viewBox="0 0 16 16" width="16" height="16" fill="none" stroke="currentColor" stroke-width="1.4" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M8 1.6l6 3.1-6 3.1-6-3.1 6-3.1z"/><path d="M2 8.2l6 3.1 6-3.1"/><path d="M2 11.5l6 3.1 6-3.1"/></svg></button>
      <button class="vf-agent" type="button" aria-label="Regions"><svg viewBox="0 0 16 16" width="16" height="16" fill="none" stroke="currentColor" stroke-width="1.4" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><circle cx="8" cy="8" r="6.2"/><path d="M1.8 8h12.4M8 1.8c1.7 1.8 2.6 3.9 2.6 6.2S9.7 12.4 8 14.2c-1.7-1.8-2.6-3.9-2.6-6.2S6.3 3.6 8 1.8z"/></svg></button>
      <button class="vf-agent" type="button" aria-label="Feedback"><svg viewBox="0 0 16 16" width="16" height="16" fill="none" stroke="currentColor" stroke-width="1.4" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M14 7.7A5.9 5.9 0 0 1 8.1 13.6H5.6L2.2 15.4l1-3.2A5.9 5.9 0 1 1 14 7.7z"/></svg></button>
      <button class="vf-agent" type="button" aria-label="Keyboard shortcuts"><svg viewBox="0 0 16 16" width="16" height="16" fill="none" stroke="currentColor" stroke-width="1.4" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><rect x="1.6" y="4.2" width="12.8" height="7.6" rx="1.6"/><path d="M4.5 6.7h.01M7 6.7h.01M9.5 6.7h.01M12 6.7h.01M4.5 9.4h7"/></svg></button>
    </div>
  </div>
</footer>

<footer class="demo-footer">
  <div class="demo-footer-inner">
    <p><strong>Demo notice.</strong> This page is a design demonstration created by Classgrid AI to show its ability to rebuild a website's design from the browser. It is not the real Vercel website and is not affiliated with, endorsed by, sponsored by, or connected to Vercel Inc. in any way.</p>
    <p><strong>Trademarks.</strong> "Vercel", the Vercel logo, and the Vercel website design are trademarks and property of Vercel Inc. All other company names and logos shown on this page (including Meta, DoorDash, OpenAI, SpaceX, The Weather Company and Polymarket) are trademarks of their respective owners. Their use here does not mean any of these companies are customers of, partners of, or connected to Classgrid.</p>
    <p><strong>Not for real use.</strong> This page is for demonstration and educational purposes only. It is not a real product or service, offers nothing for sale, and is not intended for any commercial use. Its buttons and links do not work, and it does not collect any personal data.</p>
    <p><strong>Removal.</strong> If you are a rights owner and want this demo removed, contact <a href="mailto:support@classgrid.in">support@classgrid.in</a> and we will remove it.</p>
    <p>&copy; Vercel Inc. for the original design. Rebuilt by Classgrid AI for demo purposes only.</p>
  </div>
</footer>`;