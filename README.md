# Stochastic Equity Valuation Engine: Ola Electric ⚡

## Project Overview

Traditional Discounted Cash Flow (DCF) models rely on deterministic point-estimates, which inherently fail to capture the volatility and non-linear risk of high-growth technology and EV companies.

This project is a **Stochastic Valuation Engine** built in Python. It simulates **10,000 probabilistic paths** of Ola Electric's future cash flows to generate a **probability density function** of its intrinsic Enterprise Value. By treating core drivers (like revenue growth and commodity costs) as random variables, this model provides a mathematically rigorous quantification of fundamental equity risk.

---

## Core Methodology & Mathematics

### 1. Ornstein-Uhlenbeck (OU) Mean-Reverting Process

Instead of assuming a static cost for battery raw materials or using Geometric Brownian Motion (which allows prices to drift to infinity), this engine models Lithium costs using the **Ornstein-Uhlenbeck stochastic differential equation**:

```math
dX_t = θ(μ - X_t)dt + σdW_t
```

This captures the real-world mean-reverting nature of commodity markets, where price spikes are eventually mitigated by supply increases or shifts in battery chemistry (e.g., NMC to LFP).

### 2. The "Path to Profitability" Dynamics

To accurately value a pre-profitability, high-capex EV manufacturer, the model dynamically links stochastic inputs to structural financial shifts:

- **Operating Leverage:** Base EBITDA margins structurally expand year-over-year as fixed costs are absorbed by higher vehicle volumes.
- **Dynamic Margin Impact:** Simulated drops in lithium costs automatically trigger gross margin improvements in the cash flow loop.
- **Capex Normalization:** Capital Expenditures are modeled to taper off from peak levels (12% of revenue) to maintenance levels (6%) as the Krishnagiri Gigafactory construction completes.

---

## Results (10,000 Iterations)

The Monte Carlo simulation yields the following Enterprise Value distribution *(illustrative)*:

| Scenario | Percentile | Enterprise Value |
|---|---|---|
| 🐻 Bear Case | 5th Percentile | ~₹3,880 Cr |
| ⚖️ Base Case | 50th Percentile | ~₹5,060 Cr |
| 🐂 Bull Case | 95th Percentile | ~₹6,480 Cr |

> **Note:** The massive spread between the Bear and Bull cases mathematically highlights the heavy variance and execution risk inherent in the EV sector, which traditional Excel models mask.

---

## How to Run the Engine

**1. Clone the repository:**
```bash
git clone https://github.com/yourusername/Ola-Electric-Stochastic-Valuation.git
cd Ola-Electric-Stochastic-Valuation
```

**2. Install dependencies:**
```bash
pip install matplotlib
```

**3. Execute the simulation:**
```bash
python3 ola_monte_carlo.py
```

The script will output the percentile valuations in the terminal and automatically generate/save the probability distribution chart.

---

## Key Base Assumptions *(Configurable)*

| Parameter | Value |
|---|---|
| Cost of Capital (WACC) | 12.5% |
| Terminal Growth Rate | 4.0% |
| Revenue Growth | Normal Distribution (μ = 25%, σ = 8%) |
| Lithium Cost — Long-term Mean (OU Model) | $110/kWh |
| Lithium Cost — Volatility | 15% |
| Lithium Cost — Reversion Speed | 0.8 |
