import type { Metadata } from "next";
import "./landing.css";
import Pricing from "./pricing";
import Signup from "./signup";

export const metadata: Metadata = {
  title: "Classgrid AI — your personal AI assistant",
  description:
    "Classgrid AI helps you study, write, plan and get things done. Ask anything, get real answers, and let it handle the boring parts.",
};

const FEATURES = [
  {
    icon: "💬",
    title: "Ask anything",
    text: "Questions, explanations, summaries and drafts. Plain answers, no jargon, in the language you prefer.",
  },
  {
    icon: "🔎",
    title: "Real web answers",
    text: "It searches the live web, opens the actual pages and cites the source, so you are not reading guesses.",
  },
  {
    icon: "📅",
    title: "Plans and reminders",
    text: "Turn a syllabus or a deadline into a day-by-day plan, then get the reminders by email or WhatsApp.",
  },
  {
    icon: "📄",
    title: "Reads your files",
    text: "Drop in a PDF, a spreadsheet, an image or a voice note and ask questions about it directly.",
  },
  {
    icon: "🛠️",
    title: "Does the work",
    text: "Sends emails, books meetings, updates sheets and builds small apps, all from one chat.",
  },
  {
    icon: "🔒",
    title: "Private by default",
    text: "Your chats and files stay yours. Nothing is used to train models, and you can delete anything any time.",
  },
];

const FOOTER_LINKS = [
  {
    heading: "Product",
    links: ["Features", "Pricing", "Changelog", "Roadmap"],
  },
  {
    heading: "Company",
    links: ["About", "Blog", "Careers", "Contact"],
  },
  {
    heading: "Resources",
    links: ["Help centre", "Guides", "Status", "Community"],
  },
];

export default function ClassgridLanding() {
  return (
    <div className="cg">
      <header className="cg-nav">
        <div className="cg-nav-inner">
          <a className="cg-brand" href="#top">
            <span className="cg-brand-mark">C</span>
            Classgrid AI
          </a>
          <nav className="cg-nav-links">
            <a href="#features">Features</a>
            <a href="#pricing">Pricing</a>
            <a href="#signup">Get started</a>
          </nav>
          <div className="cg-nav-right">
            <a className="cg-btn cg-btn-ghost cg-btn-sm" href="#signup">
              Sign in
            </a>
            <a className="cg-btn cg-btn-primary cg-btn-sm" href="#signup">
              Try it free
            </a>
          </div>
        </div>
      </header>

      <main id="top">
        {/* ---------------- HERO ---------------- */}
        <section className="cg-hero">
          <div className="cg-wrap cg-hero-inner">
            <div>
              <span className="cg-eyebrow">
                <span className="dot" />
                Now with schedules and WhatsApp reminders
              </span>
              <h1 className="cg-h1">
                Your personal AI assistant, <em>for everything you do.</em>
              </h1>
              <p className="cg-sub">
                Classgrid AI answers your questions, reads your files, makes your plans and
                actually gets things done. One chat, no setup, works on your phone.
              </p>
              <div className="cg-cta-row">
                <a className="cg-btn cg-btn-primary cg-btn-lg" href="#signup">
                  Start for free
                </a>
                <a className="cg-btn cg-btn-ghost cg-btn-lg" href="#features">
                  See what it does
                </a>
              </div>
              <p className="cg-note">Free plan available · No card needed · Cancel any time</p>
            </div>

            <div className="cg-mock" aria-hidden="true">
              <div className="cg-mock-bar">
                <i />
                <i />
                <i />
                <span className="cg-mock-title">Classgrid AI</span>
              </div>
              <div className="cg-mock-body">
                <div className="cg-bubble user">
                  I have my Physics exam on 15 Feb. Make me a study plan.
                </div>
                <div className="cg-bubble ai">
                  Done — <b>14 days</b>, 2 chapters a day, with the last 2 days kept for revision.
                  I&apos;ve added it to your planner and set a daily 7 pm reminder.
                </div>
                <div className="cg-bubble user">Also email the plan to my teacher.</div>
                <div className="cg-bubble ai">
                  Sent ✅ Subject: &ldquo;Study plan — Physics, Feb 15&rdquo;. Want me to copy it to
                  your calendar too?
                </div>
              </div>
              <div className="cg-mock-foot">
                <span className="pill">Planner</span>
                <span className="pill">Reminders</span>
                <span className="pill">Email</span>
              </div>
            </div>
          </div>
        </section>

        {/* ---------------- FEATURES ---------------- */}
        <section className="cg-section" id="features">
          <div className="cg-wrap">
            <div className="cg-section-head">
              <h2 className="cg-h2">One assistant, a lot of jobs</h2>
              <p className="cg-lead">
                Everything below works from the same chat. You never have to learn a new tool for
                each task.
              </p>
            </div>
            <div className="cg-features">
              {FEATURES.map((f) => (
                <article className="cg-feature" key={f.title}>
                  <div className="cg-feature-icon" aria-hidden="true">
                    {f.icon}
                  </div>
                  <h3>{f.title}</h3>
                  <p>{f.text}</p>
                </article>
              ))}
            </div>
          </div>
        </section>

        {/* ---------------- PRICING ---------------- */}
        <Pricing />

        {/* ---------------- SIGN-UP ---------------- */}
        <Signup />
      </main>

      {/* ---------------- FOOTER ---------------- */}
      <footer className="cg-footer">
        <div className="cg-wrap">
          <div className="cg-footer-grid">
            <div className="cg-footer-brand">
              <a className="cg-brand" href="#top">
                <span className="cg-brand-mark">C</span>
                Classgrid AI
              </a>
              <p>
                A personal AI assistant for study, work and everyday life. Built in India, made to
                be simple.
              </p>
              <div className="cg-social">
                <a href="#top" aria-label="X">
                  𝕏
                </a>
                <a href="#top" aria-label="LinkedIn">
                  in
                </a>
                <a href="#top" aria-label="Instagram">
                  ◎
                </a>
                <a href="#top" aria-label="YouTube">
                  ▶
                </a>
              </div>
            </div>
            {FOOTER_LINKS.map((col) => (
              <div key={col.heading}>
                <h4>{col.heading}</h4>
                <ul>
                  {col.links.map((l) => (
                    <li key={l}>
                      <a href="#top">{l}</a>
                    </li>
                  ))}
                </ul>
              </div>
            ))}
          </div>
          <div className="cg-legal">
            <span>© {new Date().getFullYear()} Classgrid. All rights reserved.</span>
            <div className="cg-legal-links">
              <a href="#top">Privacy</a>
              <a href="#top">Terms</a>
              <a href="#top">Security</a>
              <a href="#top">Cookies</a>
            </div>
          </div>
        </div>
      </footer>
    </div>
  );
}
