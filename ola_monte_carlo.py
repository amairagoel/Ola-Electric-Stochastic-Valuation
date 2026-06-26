import random
import math
import matplotlib.pyplot as plt

class OrnsteinUhlenbeck:
    def __init__(self, mu, theta, sigma, dt):
        self.mu = mu
        self.theta = theta
        self.sigma = sigma
        self.dt = dt

    def generate_path(self, current_price, steps):
        path = [0.0] * steps
        path[0] = current_price
        
        for i in range(1, steps):
            Z = random.gauss(0.0, 1.0)
            drift = self.theta * (self.mu - path[i - 1]) * self.dt
            diffusion = self.sigma * math.sqrt(self.dt) * Z
            
            path[i] = path[i - 1] + drift + diffusion
            if path[i] < 10.0:  # Absolute floor
                path[i] = 10.0
                
        return path

def main():
    print("Running 10,000 simulated paths for Ola Electric...")
    
    num_paths = 10000
    years = 5
    steps_per_year = 12
    total_steps = years * steps_per_year
    dt = 1.0 / steps_per_year

    # Valuation Assumptions
    wacc = 0.125
    tax_rate = 0.25
    terminal_growth = 0.04

    # Lithium Cost via Ornstein-Uhlenbeck 
    lithium_cost_model = OrnsteinUhlenbeck(mu=110.0, theta=0.8, sigma=15.0, dt=dt)
    initial_lithium_cost = 135.0

    enterprise_values = []

    for p in range(num_paths):
        current_revenue = 3000.0  # Starting Revenue in INR Crores
        ebitda_margin = 0.05      # Starting EBITDA Margin
        
        lithium_path = lithium_cost_model.generate_path(initial_lithium_cost, total_steps)
        
        present_value_ufcf = 0.0
        final_year_ufcf = 0.0

        for y in range(1, years + 1):
            # 1. Simulate Annual Revenue (Mean 25% YoY growth, 8% Volatility)
            annual_growth = random.gauss(0.25, 0.08)
            current_revenue *= (1.0 + annual_growth)

            # 2. Calculate average lithium cost for the year
            start_month = (y - 1) * 12
            end_month = y * 12
            avg_lithium_cost = sum(lithium_path[start_month:end_month]) / 12.0

            # 3. Margin Impact & Structural Expansion
            # Lithium savings impact
            margin_improvement = (initial_lithium_cost - avg_lithium_cost) * 0.0005
            # Operating leverage: Base margin grows structurally by 2% each year (from 5% to 15%)
            base_ebitda_margin = ebitda_margin + (y * 0.02)
            current_ebitda_margin = base_ebitda_margin + margin_improvement
            
            # 4. Capex Normalization (Gigafactory completes, capex scales down)
            # Capex drops from 11% of rev in year 1 to ~6% by year 5
            capex_percent = 0.12 - (y * 0.012)
            capex = current_revenue * capex_percent
            d_and_a = current_revenue * 0.05
            
            # 5. Calculate EBIT & UFCF
            ebitda = current_revenue * current_ebitda_margin
            ebit = ebitda - d_and_a
            ufcf = (ebit * (1.0 - tax_rate)) + d_and_a - capex
            
            # 6. Discount to Present Value
            discount_factor = math.pow(1.0 + wacc, y)
            present_value_ufcf += (ufcf / discount_factor)

            if y == years:
                final_year_ufcf = ufcf

        # Terminal Value Calculation
        terminal_value = (final_year_ufcf * (1.0 + terminal_growth)) / (wacc - terminal_growth)
        pv_terminal_value = terminal_value / math.pow(1.0 + wacc, years)

        enterprise_values.append(present_value_ufcf + pv_terminal_value)

    # Statistical Output
    enterprise_values.sort()
    
    percentile_5 = enterprise_values[int(num_paths * 0.05)]
    median_ev = enterprise_values[int(num_paths * 0.50)]
    percentile_95 = enterprise_values[int(num_paths * 0.95)]

    print("\n=== Monte Carlo DCF Results (INR Crores) ===")
    print(f"5th Percentile (Bear Case)  : INR {percentile_5:,.2f} Cr")
    print(f"50th Percentile (Base Case) : INR {median_ev:,.2f} Cr")
    print(f"95th Percentile (Bull Case) : INR {percentile_95:,.2f} Cr")
    print("===========================================")

    # --- Visualization for GitHub Portfolio ---
    print("\nGenerating probability distribution chart...")
    plt.figure(figsize=(10, 6))
    
    # Create histogram
    plt.hist(enterprise_values, bins=100, color='#3498db', edgecolor='black', alpha=0.7)
    
    # Add vertical lines for percentiles
    plt.axvline(percentile_5, color='#e74c3c', linestyle='dashed', linewidth=2, label=f'5th Percentile (Bear): ₹{percentile_5:,.0f} Cr')
    plt.axvline(median_ev, color='#2ecc71', linestyle='dashed', linewidth=2, label=f'50th Percentile (Base): ₹{median_ev:,.0f} Cr')
    plt.axvline(percentile_95, color='#f1c40f', linestyle='dashed', linewidth=2, label=f'95th Percentile (Bull): ₹{percentile_95:,.0f} Cr')
    
    # Formatting
    plt.title('Ola Electric: Stochastic DCF Valuation Distribution\n(10,000 Monte Carlo Iterations)', fontsize=14, fontweight='bold')
    plt.xlabel('Enterprise Value (INR Crores)', fontsize=12)
    plt.ylabel('Frequency (Number of Scenarios)', fontsize=12)
    plt.grid(axis='y', alpha=0.3)
    plt.legend(loc='upper right')
    
    # Save the plot for GitHub README and display it
    plt.tight_layout()
    plt.savefig('valuation_distribution.png', dpi=300)
    print("Chart saved successfully as 'valuation_distribution.png'. Close the chart window to exit.")
    plt.show()

if __name__ == "__main__":
    main()