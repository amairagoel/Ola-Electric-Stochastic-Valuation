#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iomanip>

// ---------------------------------------------------------
// 1. Ornstein-Uhlenbeck Process for Commodity Modeling
// ---------------------------------------------------------
class OrnsteinUhlenbeck {
private:
    double mu;     // Long-term mean cost (e.g., structural floor)
    double theta;  // Speed of mean reversion
    double sigma;  // Volatility
    double dt;     // Time step size (1/12 for monthly)

public:
    OrnsteinUhlenbeck(double _mu, double _theta, double _sigma, double _dt)
        : mu(_mu), theta(_theta), sigma(_sigma), dt(_dt) {}

    // Generate a price path over 'steps' periods
    std::vector<double> generatePath(double current_price, int steps, std::mt19937& gen) {
        std::vector<double> path(steps);
        path[0] = current_price;
        std::normal_distribution<double> norm_dist(0.0, 1.0);

        for (int i = 1; i < steps; ++i) {
            double Z = norm_dist(gen);
            double drift = theta * (mu - path[i - 1]) * dt;
            double diffusion = sigma * std::sqrt(dt) * Z;
            
            path[i] = path[i - 1] + drift + diffusion;
            // Floor the commodity price so it doesn't go negative
            if (path[i] < 10.0) path[i] = 10.0; 
        }
        return path;
    }
};

// ---------------------------------------------------------
// 2. Main Monte Carlo DCF Engine
// ---------------------------------------------------------
int main() {
    // --- Global Parameters ---
    int num_paths = 10000;
    int years = 5;
    int steps_per_year = 12; // Monthly simulation
    int total_steps = years * steps_per_year;
    double dt = 1.0 / steps_per_year;

    // --- Valuation Assumptions (Illustrative for Ola FY27-FY31) ---
    double wacc = 0.125;           // Cost of Capital (12.5%)
    double tax_rate = 0.25;        // Corporate Tax Rate
    double terminal_growth = 0.04; // Terminal Growth Rate (4%)
    
    // --- Stochastic Generators ---
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Revenue Growth follows a normal distribution (Mean 25% YoY, Volatility 8%)
    std::normal_distribution<double> rev_growth_dist(0.25, 0.08);

    // Lithium Cost via Ornstein-Uhlenbeck (Mean $110/kWh, Reversion 0.8, Volatility 15%)
    OrnsteinUhlenbeck lithiumCost(110.0, 0.8, 15.0, dt);
    double initial_lithium_cost = 135.0;

    std::vector<double> enterprise_values;
    enterprise_values.reserve(num_paths);

    std::cout << "Running " << num_paths << " simulated paths for Ola Electric..." << std::endl;

    // --- Monte Carlo Loop ---
    for (int p = 0; p < num_paths; ++p) {
        double current_revenue = 3000.0; // Starting Revenue in INR Crores
        double ebitda_margin = 0.05;     // Starting EBITDA Margin
        
        std::vector<double> lithium_path = lithiumCost.generatePath(initial_lithium_cost, total_steps, gen);
        
        double present_value_ufcf = 0.0;
        double final_year_ufcf = 0.0;

        for (int y = 1; y <= years; ++y) {
            // 1. Simulate Annual Revenue
            double annual_growth = rev_growth_dist(gen);
            current_revenue *= (1.0 + annual_growth);

            // 2. Calculate average lithium cost for the year from the monthly OU path
            double avg_lithium_cost = 0.0;
            for(int m = (y-1)*12; m < y*12; ++m) {
                avg_lithium_cost += lithium_path[m];
            }
            avg_lithium_cost /= 12.0;

            // 3. Margin Impact (Lower lithium costs = higher margins)
            // If lithium cost drops below initial, margin improves
            double margin_improvement = (initial_lithium_cost - avg_lithium_cost) * 0.0005; 
            double current_ebitda_margin = ebitda_margin + margin_improvement;
            
            // CapEx is heavy for the Gigafactory (Assume 10% of revenue)
            double capex = current_revenue * 0.10; 
            double d_and_a = current_revenue * 0.05; // Depreciation
            
            // 4. Calculate EBIT & UFCF
            double ebitda = current_revenue * current_ebitda_margin;
            double ebit = ebitda - d_and_a;
            
            // UFCF = EBIT*(1-t) + D&A - CapEx (Ignoring NWC for simplicity in this script)
            double ufcf = (ebit * (1.0 - tax_rate)) + d_and_a - capex;
            
            // 5. Discount to Present Value
            double discount_factor = std::pow(1.0 + wacc, y);
            present_value_ufcf += (ufcf / discount_factor);

            if (y == years) {
                final_year_ufcf = ufcf;
            }
        }

        // --- Terminal Value Calculation (Gordon Growth Method) ---
        double terminal_value = (final_year_ufcf * (1.0 + terminal_growth)) / (wacc - terminal_growth);
        double pv_terminal_value = terminal_value / std::pow(1.0 + wacc, years);

        double total_enterprise_value = present_value_ufcf + pv_terminal_value;
        enterprise_values.push_back(total_enterprise_value);
    }

    // --- Statistical Output ---
    std::sort(enterprise_values.begin(), enterprise_values.end());

    double percentile_5 = enterprise_values[num_paths * 0.05];
    double median_ev = enterprise_values[num_paths * 0.50];
    double percentile_95 = enterprise_values[num_paths * 0.95];

    std::cout << "\n=== Monte Carlo DCF Results (INR Crores) ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "5th Percentile (Bear Case)  : INR " << percentile_5 << " Cr" << std::endl;
    std::cout << "50th Percentile (Base Case) : INR " << median_ev << " Cr" << std::endl;
    std::cout << "95th Percentile (Bull Case) : INR " << percentile_95 << " Cr" << std::endl;
    std::cout << "===========================================" << std::endl;

    return 0;
}