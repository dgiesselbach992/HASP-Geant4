#!/bin/bash

ENERGIES=(2 7 12 17 22)     # <-- your 4 energies (edit as needed)
TRIALS=5
RUNS_PER_TRIAL=10
PARTICLES=5000

for E in "${ENERGIES[@]}"; do
    echo "======================================"
    echo " ENERGY = $E GeV"
    echo "======================================"

    for T in $(seq 1 $TRIALS); do
        echo "  Trial $T / $TRIALS"

        ./test.sh "$RUNS_PER_TRIAL" "$E" "$PARTICLES"
    done
done
