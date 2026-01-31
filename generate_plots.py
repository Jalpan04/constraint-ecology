import pandas as pd
import matplotlib.pyplot as plt

# Load data
baseline = pd.read_csv('episodes_baseline.csv')
evolving = pd.read_csv('episodes_evolving.csv')

# Plot 1: Spatial Variance
plt.figure(figsize=(8, 6))
plt.plot(baseline['episode'], baseline['spatial_variance'], 'b-o', label='Baseline', linewidth=2, markersize=6)
plt.plot(evolving['episode'], evolving['spatial_variance'], 'r-o', label='Evolving', linewidth=2, markersize=6)
plt.xlabel('Episode', fontsize=12)
plt.ylabel('Spatial Variance', fontsize=12)
plt.title('Spatial Variance: Baseline vs Evolving', fontsize=14)
plt.legend(fontsize=11)
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig('spatial_variance_plot.pdf')
plt.close()

# Plot 2: Mean Strength (Evolving only)
plt.figure(figsize=(8, 6))
plt.plot(evolving['episode'], evolving['mean_strength'], 'g-o', linewidth=2, markersize=6)
plt.xlabel('Episode', fontsize=12)
plt.ylabel('Mean Constraint Strength', fontsize=12)
plt.title('Constraint Adaptation (Evolving Mode)', fontsize=14)
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig('constraint_strength_plot.pdf')
plt.close()

# Plot 3: Entropy
plt.figure(figsize=(8, 6))
plt.plot(baseline['episode'], baseline['entropy'], 'b-o', label='Baseline', linewidth=2, markersize=6)
plt.plot(evolving['episode'], evolving['entropy'], 'r-o', label='Evolving', linewidth=2, markersize=6)
plt.xlabel('Episode', fontsize=12)
plt.ylabel('Entropy', fontsize=12)
plt.title('Entropy: Baseline vs Evolving', fontsize=14)
plt.legend(fontsize=11)
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig('entropy_plot.pdf')
plt.close()

# Plot 4: Stability
plt.figure(figsize=(8, 6))
plt.plot(baseline['episode'], baseline['stability'], 'b-o', label='Baseline', linewidth=2, markersize=6)
plt.plot(evolving['episode'], evolving['stability'], 'r-o', label='Evolving', linewidth=2, markersize=6)
plt.xlabel('Episode', fontsize=12)
plt.ylabel('Stability', fontsize=12)
plt.title('Stability: Baseline vs Evolving', fontsize=14)
plt.legend(fontsize=11)
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig('stability_plot.pdf')
plt.close()

print("All plots generated successfully!")
print("\nGenerated files:")
print("  - spatial_variance_plot.pdf")
print("  - constraint_strength_plot.pdf") 
print("  - entropy_plot.pdf")
print("  - stability_plot.pdf")
