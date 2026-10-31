"use client";
import { useEffect } from "react";
import { MARKUP } from "./markup";
import { initPage } from "./vercel-hero";

export default function Hero() {
  useEffect(() => { initPage(); }, []);
  return <div style={{ display: "contents" }} dangerouslySetInnerHTML={{ __html: MARKUP }} />;
}
