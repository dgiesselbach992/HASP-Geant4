\#!/bin/bash

ENERGIES=(5 15)
TRIALS=1
RUNS_PER_TRIAL=10
PARTICLES=5000

for E in "${ENERGIES[@]}"; do
    echo "======================================"
    echo " ENERGY = $E GeV"
    echo "======================================"

    for T in $(seq 1 $TRIALS); do
        echo "  Trial $T / $TRIALS"
        ./test7.sh "$RUNS_PER_TRIAL" "$E" "$PARTICLES"
    done
done



