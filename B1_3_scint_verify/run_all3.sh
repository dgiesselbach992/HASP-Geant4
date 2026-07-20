#!/bin/bash

# -----------------------------
# Experimental design parameters
# -----------------------------
ENERGIES=(5 15)          # GeV
TRIALS=2                 # number of trials per energy
RUNS_PER_TRIAL=10        # number of runs per trial
PARTICLES=5000           # primaries per run

# -----------------------------
# Loop over energies
# -----------------------------
for E in "${ENERGIES[@]}"; do
    echo "======================================"
    echo " ENERGY = ${E} GeV"
    echo "======================================"

    # Create a unique directory for this energy
    RESULTS_DIR="results_E${E}"
    mkdir -p "$RESULTS_DIR"

    # -----------------------------
    # Loop over trials
    # -----------------------------
    for T in $(seq 1 $TRIALS); do
        echo "  Trial $T / $TRIALS"

        # Create a unique CSV for this trial
        csvfile="$RESULTS_DIR/trial_${T}.csv"

        # Correct CSV header for your new simulation
        echo "event,track,particle,tungsten_entry,tungsten_exit,sc1_entry,sc1_exit,sc2_entry,sc2_exit,sc3_entry,sc3_exit" > "$csvfile"

        # -----------------------------
        # Loop over runs inside trial
        # -----------------------------
        for i in $(seq 1 $RUNS_PER_TRIAL); do
            echo "    Running simulation $i..."

            # Random seeds
            SEED1=$(od -An -N4 -tu4 < /dev/urandom)
            SEED2=$(od -An -N4 -tu4 < /dev/urandom)

            # Generate macro file for this run
            sed "s/SEED1/$SEED1/; s/SEED2/$SEED2/; s/ENERGY_VALUE/$E/; s/PARTICLES/$PARTICLES/" \
                myrun.mac > batch_runs/run_${E}_${T}_${i}.mac

            # Run simulation
            OUTPUT=$(./exampleB1 batch_runs/run_${E}_${T}_${i}.mac 2>&1)

            # Extract only valid CSV lines (11 columns)
            echo "$OUTPUT" | grep -E '^[0-9]+,[0-9]+,[^,]+(,[01]){8}$' >> "$csvfile"
        done

        echo "  ===== Completed Trial $T ====="
    done

    echo "===== Completed Energy ${E} GeV ====="
done
