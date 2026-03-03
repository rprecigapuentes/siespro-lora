# SIESPRO — Project Documentation

[![UNAL](https://img.shields.io/badge/UNAL-Bogotá-8B0000)](https://unal.edu.co/)
[![TPI](https://img.shields.io/badge/TPI-EXPOIDEAS_2025-orange)](https://unal.edu.co/)
[![Methodology](https://img.shields.io/badge/Methodology-Design_Thinking_+_PESTEL-4B9CD3)]()
[![Stage](https://img.shields.io/badge/Stage-Convergence-green)]()

SIESPRO (*Sistema Estudiantil de Protección*) is a student
project from the **National University of Colombia — Bogotá campus** that goes
beyond a purely technical implementation. The project was structured around two
complementary axes: a **Design Thinking process** centered on the real needs of
stakeholders, and a **PESTEL analysis** to evaluate the viability and context of
the solution. The technical development (an RF-based perimeter classification
system using LoRa and Machine Learning) is the direct result of that process,
not the starting point.

The core problem addressed is **school child safety without compromising student
privacy**: a perimeter monitoring system that detects whether a student is inside
or outside a defined zone using RF signal features (RSSI, SNR), with no cameras
or biometric data involved.

---

## Project Phases

### Divergence — Problem Discovery

The divergence stage applied Design Thinking tools to identify and validate the
problem before committing to any technical solution. A survey was conducted with
the primary stakeholders — parents and school administrators — confirming both
the existence of the safety concern and the rejection of surveillance-based
approaches (cameras, GPS tracking). The outputs of this stage are:

- [**`siespro_infografia.pdf`**](assets/siespro_infografia.pdf) — Visual summary of the problem space, the
  stakeholder map and the key insight that motivated the solution: families want
  safety guarantees without exposing children to invasive monitoring.

- [**`siespro_slides.pdf`**](assets/siespro_slides.pdf) — Presentation used during the divergence review.
  Contains the PESTEL analysis across the six dimensions (Political, Economic,
  Social, Technological, Environmental, Legal), the survey methodology, results
  and the opportunity framing that bridged the problem into the solution space.

### Convergence — Solution & Validation

The convergence stage produced a defined, implementable solution and evaluated
its real-world viability as a product. The outputs of this stage are:

- [**`siespro_poster.pdf`**](siespro_poster.pdf) — Academic poster presented at **TPI EXPOIDEAS 2025**,
  the semiannual innovation fair of the National University of Colombia. Summarizes
  the full project — problem, methodology, technical architecture, results and
  conclusions — in a single-page format for a general audience.

- [**`siespro_plan_negocios.pdf`**](siespro_plan_negocios.pdf) — Business plan developed to assess the
  commercial and institutional viability of SIESPRO. Covers market analysis,
  value proposition, cost structure, revenue model and go-to-market strategy
  targeting Colombian educational institutions.

---

## Technical Reference

- [**`siespro_architecture_diagram.png`**](siespro_architecture_diagram.png) — System architecture diagram showing
  the data flow across all components: the On Field Node (ESP32-C3 Mini +
  SX1278), the Hub Node (ESP32 + sensors), the host-side ML inference engine
  (Random Forest classifier), the Supabase cloud backend and the parent-facing
  web dashboard. This diagram is the canonical reference for understanding how
  the hardware, firmware, ML model and backend interact

---

## Document Index

| File | Phase | Type |
|---|---|---|
| `siespro_infografia.pdf` | Divergence | Infographic |
| `siespro_slides.pdf` | Divergence | Presentation + PESTEL |
| `siespro_poster.pdf` | Convergence | Academic poster — TPI EXPOIDEAS 2025 |
| `siespro_plan_negocios.pdf` | Convergence | Business plan |
| `siespro_architecture_diagram.png` | Technical | System architecture diagram |
