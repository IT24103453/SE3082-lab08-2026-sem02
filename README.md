# SE3082 - Parallel Computing - Lab 08 (MPI)

BSc (Hons) in Information Technology
SE3082 - Parallel Computing

Solutions to Lab Sheet 8 (MPI). `message1.cc` and `message2.cc` are the
given starter files; everything else is under `Lab 08 Exercises/`.

## Build & run

```bash
mpicxx -O2 <file>.cc -o <name>
mpirun --oversubscribe -np <P> ./<name>
```

`--oversubscribe` is only needed if you run more MPI processes than
physical cores.

## Contents

| Path | Exercise | What it does |
|---|---|---|
| `message1.cc`, `message2.cc` | given | starter files |
| `Lab 08 Exercises/Exercise1/HelloMPI.c` | 1 | Hello World MPI + GitHub repo setup notes |
| `Lab 08 Exercises/Exercise2/SumMPI.cc` | 2 | parallel sum of 1..10,000,000 |
| `Lab 08 Exercises/Exercise3/PiMonteCarlo.cc` | 3 | Monte Carlo Pi, explicit Send/Recv |
| `Lab 08 Exercises/Exercise4/` | 4 | `run_timings.sh`, `plot_ex4.py`, `timings.csv` -> graphs |
| `Lab 08 Exercises/Exercise5/SourceMismatch.cc` | 5.1 | deliberate source mismatch -> deadlock demo |
| `Lab 08 Exercises/Exercise5/Message2BSend.cc` | 5.2 | message2.cc rewritten with MPI_Bsend |
| `Lab 08 Exercises/Exercise6/PiAnySource.cc` | 6 | Exercise 3 with MPI_ANY_SOURCE |
| `Lab 08 Exercises/Exercise7/PiAnySourceBSend.cc` | 7 | Exercise 6 + MPI_Bsend |
| `Lab 08 Exercises/Graphs/` | 4 | `time_vs_processors.png`, `speedup.png` |
| `Lab 08 Exercises/Answers.txt` | 5.1, 6, 5.2/7 | written answers to the conceptual questions |

## Exercise 4 note

The included `timings.csv` and graphs were benchmarked in a single-core
sandbox, so they're flat/noisy rather than showing real speedup. Re-run
`run_timings.sh` on an actual multi-core machine to get a meaningful
Time-vs-Processors / Speedup curve.
