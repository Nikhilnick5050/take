"use client";
import { useState } from "react";

export default function Signup() {
  const [email, setEmail] = useState("");
  const [state, setState] = useState<"idle" | "ok" | "err">("idle");
  const [msg, setMsg] = useState("");

  function submit(e: React.FormEvent) {
    e.preventDefault();
    const value = email.trim();
    if (!/^[^\s@]+@[^\s@]+\.[^\s@]{2,}$/.test(value)) {
      setState("err");
      setMsg("Please enter a valid email address.");
      return;
    }
    setState("ok");
    setMsg("Thanks! We'll send your invite to " + value + " shortly.");
    setEmail("");
  }

  return (
    <section className="cg-section" id="signup">
      <div className="cg-wrap">
        <div className="cg-cta">
          <h2 className="cg-h2">Get started with Classgrid AI</h2>
          <p className="cg-lead">
            Drop your email and we&apos;ll send you an invite. No card needed to try it.
          </p>
          <form className="cg-form" onSubmit={submit} noValidate>
            <input
              className="cg-input"
              type="email"
              placeholder="you@example.com"
              aria-label="Email address"
              value={email}
              onChange={(e) => {
                setEmail(e.target.value);
                if (state !== "idle") {
                  setState("idle");
                  setMsg("");
                }
              }}
            />
            <button className="cg-btn cg-btn-primary cg-btn-lg" type="submit">
              Get my invite
            </button>
          </form>
          <p className={"cg-form-msg " + (state === "ok" ? "ok" : state === "err" ? "err" : "")}>
            {msg}
          </p>
        </div>
      </div>
    </section>
  );
}
