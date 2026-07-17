#!/bin/bash

ENERGIES=(5 15)        # energies to run
NUM_RUNS=10            # runs per energy
NUM_PARTICLES=5000     # primaries per run

RESULTS_DIR="output"
mkdir -p "$RESULTS_DIR"

count=$(ls $RESULTS_DIR/results_*.csv 2>/dev/null | wc -l)
next=$((count + 1))
csvfile="$RESULTS_DIR/results_${next}.csv"

# Correct CSV header for your new simulation
echo "event,track,particle,tungsten_entry,tungsten_exit,sc1_entry,sc1_exit,sc2_entry,sc2_exit,sc3_entry,sc3_exit" > "$csvfile"

for ENERGY in "${ENERGIES[@]}"; do
    echo "===== Starting ENERGY $ENERGY GeV ====="

    for i in $(seq 1 $NUM_RUNS); do
        echo "Running simulation $i at $ENERGY GeV..."

        SEED1=$(od -An -N4 -tu4 < /dev/urandom)
        SEED2=$(od -An -N4 -tu4 < /dev/urandom)

        sed "s/SEED1/$SEED1/; s/SEED2/$SEED2/; s/ENERGY_VALUE/$ENERGY/; s/PARTICLES/$NUM_PARTICLES/" \
            myrun.mac > batch_runs/run_${ENERGY}_$i.mac

        OUTPUT=$(./exampleB1 batch_runs/run_${ENERGY}_$i.mac 2>&1)

        # Extract only the CSV lines from Geant4 output (11 columns)
        echo "$OUTPUT" | grep -E '^[0-9]+,[0-9]+,[^,]+(,[01]){8}$' >> "$csvfile"
    done
done

echo "===== Completed Batch $next ====="

