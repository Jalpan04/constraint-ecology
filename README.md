# Constraint Ecology: Evolving Constraints for Procedural Generation

A research implementation exploring constraint evolution as a mechanism for procedural generation in simulation environments.

## Project Structure

### Paper
- `main.tex` - Main LaTeX document
- `main.pdf` - Compiled research paper (13 pages)
- `methodology.tex`, `experiments.tex`, `metrics.tex` - LaTeX includes

### Simulation
- `constraint_ecology.c` - Core simulation code (raylib-based)
- Compile: `gcc constraint_ecology.c -o constraint_ecology -lraylib -lm`
- Run modes: Set `EVOLVING_MODE` to 0 (baseline) or 1 (evolving)

### Results & Analysis
- `episodes_baseline.csv` - Baseline experiment results (10 episodes)
- `episodes_evolving.csv` - Evolving constraint results (10 episodes)
- `generate_plots.py` - Python script to generate comparison plots
- `figures/` - All plots and diagrams (PDF/PNG format)

### Key Parameters
- **Agents**: 50
- **Episodes**: 10 per run
- **Timesteps**: 1000 per episode
- **Random Seed**: 12345 (fixed for reproducibility)
- **Constraints**: 5 spatial constraints with variable strength

## Running Experiments

1. Compile simulation with raylib
2. Set mode (baseline or evolving) in `constraint_ecology.c`
3. Run to generate CSV data
4. Use `generate_plots.py` to create figures
5. LaTeX compiles automatically include figures from `figures/`

## Citation

See `main.pdf` for full paper with results and references.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

