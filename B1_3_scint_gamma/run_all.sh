#!/bin/bash

ENERGIES=(1 10 50 100 500 1000)
TRIALS=1
RUNS_PER_TRIAL=10      # 1 run
PARTICLES=5000       # 10000 primaries per run

for E in "${ENERGIES[@]}"; do
    echo "======================================"
    echo " ENERGY = $E eV"
    echo "======================================"

    for T in $(seq 1 $TRIALS); do
        echo "  Trial $T / $TRIALS"
        ./test2.sh "$RUNS_PER_TRIAL" "$E" "$PARTICLES"
    done
done

