#!/bin/bash
# Runs SumMPI and PiMonteCarlo for a list of process counts,
# repeating REPS times each and keeping the minimum time (standard
# practice for benchmarking - minimises noise from OS scheduling).
# Produces timings.csv used by plot_ex4.py

PROCS="1 2 4 8 16"
REPS=5
OUT=timings.csv

echo "program,processes,time_sec" > $OUT

for prog in SumMPI PiMonteCarlo; do
  for p in $PROCS; do
    best=""
    for r in $(seq 1 $REPS); do
      line=$(mpirun --allow-run-as-root --oversubscribe -np $p ./$prog)
      t=$(echo "$line" | grep -oP 'Time:\s*\K[0-9.eE+-]+')
      if [ -z "$best" ] || (( $(echo "$t < $best" | bc -l) )); then
        best=$t
      fi
    done
    echo "$prog,$p,$best" >> $OUT
  done
done
echo "Wrote $OUT"
