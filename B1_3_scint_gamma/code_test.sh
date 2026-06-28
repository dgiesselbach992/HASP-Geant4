#!/bin/bash

NUM_RUNS=10        # 10 runs
ENERGY=10            # energy in MeV
NUM_PARTICLES=1000   # 1000 primaries per run

RESULTS_DIR="code_test"
mkdir  -p  "$RESULTS_DIR"

count=$(ls $RESULTS_DIR/results_*.csv 2>/dev/null | wc -l)
next=$((count + 1))
csvfile="$RESULTS_DIR/results_${next}.csv"

# CSV header
echo "event,track,particle,d1,d2,d3" > "$csvfile"

for i in $(seq 1 $NUM_RUNS); do
    echo "Running simulation $i..."

    SEED1=$(od -An -N4 -tu4 < /dev/urandom)
    SEED2=$(od -An -N4 -tu4 < /dev/urandom)

    sed "s/SEED1/$SEED1/; s/SEED2/$SEED2/; s/ENERGY_VALUE/$ENERGY/; s/PARTICLES/$NUM_PARTICLES/" \
        myrun.mac > batch_runs/run_$i.mac

    OUTPUT=$(./exampleB1 batch_runs/run_$i.mac 2>&1)

    # Extract only the CSV lines from Geant4 output
    echo "$OUTPUT" | grep -E '^[0-9]+,[0-9]+,[^,]+,[0-1],[0-1],[0-1]$' >> "$csvfile"
done

echo "===== Completed Batch $next ====="
