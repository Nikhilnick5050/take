"use client";
import { useState } from "react";

type Plan = {
  name: string;
  desc: string;
  monthly: number;
  yearly: number;
  featured?: boolean;
  features: string[];
  cta: string;
};

const PLANS: Plan[] = [
  {
    name: "Free",
    desc: "For trying things out and everyday questions.",
    monthly: 0,
    yearly: 0,
    features: ["50 messages a day", "Web search", "File and image reading", "Basic reminders"],
    cta: "Start free",
  },
  {
    name: "Plus",
    desc: "For daily work, study and planning.",
    monthly: 499,
    yearly: 4990,
    featured: true,
    features: [
      "Unlimited messages",
      "All AI models",
      "Schedules and WhatsApp reminders",
      "Email sending and plugins",
      "Projects with code editor",
    ],
    cta: "Choose Plus",
  },
  {
    name: "Teams",
    desc: "For small teams that work together.",
    monthly: 1299,
    yearly: 12990,
    features: [
      "Everything in Plus",
      "5 seats included",
      "Shared projects and files",
      "Admin billing and usage",
      "Priority support",
    ],
    cta: "Talk to us",
  },
];

const inr = (n: number) => "₹" + n.toLocaleString("en-IN");

export default function Pricing() {
  const [yearly, setYearly] = useState(false);

  return (
    <section className="cg-section" id="pricing">
      <div className="cg-wrap">
        <div className="cg-section-head">
          <h2 className="cg-h2">Simple pricing</h2>
          <p className="cg-lead">
            Start free and upgrade when you need more. Cancel any time, no lock-in.
          </p>
        </div>

        <div className="cg-toggle" role="tablist" aria-label="Billing period">
          <button
            role="tab"
            aria-selected={!yearly}
            className={!yearly ? "on" : ""}
            onClick={() => setYearly(false)}
          >
            Monthly
          </button>
          <button
            role="tab"
            aria-selected={yearly}
            className={yearly ? "on" : ""}
            onClick={() => setYearly(true)}
          >
            Yearly <span className="save">save 2 months</span>
          </button>
        </div>

        <div className="cg-plans">
          {PLANS.map((p) => {
            const price = yearly ? p.yearly : p.monthly;
            return (
              <div key={p.name} className={"cg-plan" + (p.featured ? " featured" : "")}>
                {p.featured && <span className="cg-plan-badge">Most popular</span>}
                <div>
                  <h3>{p.name}</h3>
                  <p className="desc">{p.desc}</p>
                </div>
                <div className="cg-price">
                  <span className="amount">{price === 0 ? "Free" : inr(price)}</span>
                  {price !== 0 && <span className="per">{yearly ? "/ year" : "/ month"}</span>}
                </div>
                <ul>
                  {p.features.map((f) => (
                    <li key={f}>
                      <span className="tick">✓</span>
                      <span>{f}</span>
                    </li>
                  ))}
                </ul>
                <a
                  href="#signup"
                  className={"cg-btn " + (p.featured ? "cg-btn-primary" : "cg-btn-ghost")}
                >
                  {p.cta}
                </a>
              </div>
            );
          })}
        </div>
      </div>
    </section>
  );
}
