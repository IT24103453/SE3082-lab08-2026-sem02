"""
plot_ex4.py
Exercise 4 - For Exercise 2 (parallel sum) and Exercise 3 (Monte Carlo Pi):
  1. Plot a graph that shows Time vs Number of Processors
  2. Plot a graph that shows Speedup ( = Time(1 process) / Time(P processes) )

Reads timings.csv (produced by run_timings.sh) and writes:
  time_vs_processors.png
  speedup.png

Usage: python3 plot_ex4.py
"""
import csv
from collections import defaultdict
import matplotlib.pyplot as plt

data = defaultdict(dict)  # data[program][processes] = time
with open("timings.csv") as f:
    reader = csv.DictReader(f)
    for row in reader:
        data[row["program"]][int(row["processes"])] = float(row["time_sec"])

labels = {
    "SumMPI": "Exercise 2: Sum 1..10,000,000",
    "PiMonteCarlo": "Exercise 3: Monte Carlo Pi",
}

# ---- Graph 1: Time vs Number of Processors ----
plt.figure(figsize=(7, 5))
for prog, label in labels.items():
    procs = sorted(data[prog].keys())
    times = [data[prog][p] for p in procs]
    plt.plot(procs, times, marker="o", label=label)
plt.xlabel("Number of processors (MPI processes)")
plt.ylabel("Time (seconds)")
plt.title("Time vs Number of Processors")
plt.xscale("log", base=2)
plt.legend()
plt.grid(True, which="both", linestyle="--", alpha=0.5)
plt.tight_layout()
plt.savefig("time_vs_processors.png", dpi=150)
plt.close()

# ---- Graph 2: Speedup = T(1) / T(P) ----
plt.figure(figsize=(7, 5))
max_p = 1
for prog, label in labels.items():
    procs = sorted(data[prog].keys())
    t1 = data[prog][1]
    speedup = [t1 / data[prog][p] for p in procs]
    plt.plot(procs, speedup, marker="o", label=label)
    max_p = max(max_p, max(procs))

ideal = list(range(1, max_p + 1))
plt.plot(ideal, ideal, linestyle="--", color="gray", label="Ideal (linear) speedup")

plt.xlabel("Number of processors (MPI processes)")
plt.ylabel("Speedup  T(1) / T(P)")
plt.title("Speedup vs Number of Processors")
plt.legend()
plt.grid(True, linestyle="--", alpha=0.5)
plt.tight_layout()
plt.savefig("speedup.png", dpi=150)
plt.close()

print("Saved time_vs_processors.png and speedup.png")
