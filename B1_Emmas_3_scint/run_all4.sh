#!/bin/bash

# Energies to run
ENERGIES=(1)       # example: run 5 GeV and 10 GeV
NUM_RUNS=200           # runs per energy
NUM_PARTICLES=5000     # primaries per run

RESULTS_DIR="output"
mkdir -p "$RESULTS_DIR"

# Auto-increment output file index
count=$(ls $RESULTS_DIR/results_*.csv 2>/dev/null | wc -l)
next=$((count + 1))
csvfile="$RESULTS_DIR/results_${next}.csv"

# New calorimeter CSV header
echo "event,track,particle,Edep_scint1,Edep_scint2,Edep_scint3" > "$csvfile"

for ENERGY in "${ENERGIES[@]}"; do
    echo "===== Starting ENERGY ${ENERGY} GeV ====="

    for i in $(seq 1 $NUM_RUNS); do
        echo "Running simulation $i at ${ENERGY} GeV..."

        # Random seeds
        SEED1=$(od -An -N4 -tu4 < /dev/urandom)
        SEED2=$(od -An -N4 -tu4 < /dev/urandom)

        # Create macro file for this run
        sed "s/SEED1/$SEED1/; s/SEED2/$SEED2/; s/ENERGY_VALUE/$ENERGY/; s/PARTICLES/$NUM_PARTICLES/" \
            myrun.mac > batch_runs/run_${ENERGY}_$i.mac

        # Run simulation
        OUTPUT=$(./exampleB1 batch_runs/run_${ENERGY}_$i.mac 2>&1)

        # Extract calorimeter CSV rows (6 columns)
        echo "$OUTPUT" | grep -E '^[0-9]+,[0-9]+,[^,]+,[0-9.eE+-]+,[0-9.eE+-]+,[0-9.eE+-]+$' >> "$csvfile"
    done
done

echo "===== Completed Batch $next ====="
