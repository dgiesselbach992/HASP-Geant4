#!/bin/bash

ENERGIES=(5 15)
TRIALS=2
RUNS_PER_TRIAL=10      # 1 run
PARTICLES=5000       #5000 primaries per run

for E in "${ENERGIES[@]}"; do
    echo "======================================"
    echo " ENERGY = $E GeV"
    echo "======================================"

    for T in $(seq 1 $TRIALS); do
        echo "  Trial $T / $TRIALS"
        ./test2.sh "$RUNS_PER_TRIAL" "$E" "$PARTICLES"
    done
done

