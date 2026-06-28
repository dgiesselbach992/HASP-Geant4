#!/bin/bash

# Number of particles per run
PARTICLES=50000

# Energies to test
ENERGIES=(5 15)

# Loop over energies
for E in "${ENERGIES[@]}"; do

    # Determine next sequential folder name
    n=1
    while [ -d "output_${E}GeV_run$(printf "%02d" $n)" ]; do
        n=$((n+1))
    done
    OUTDIR="output_${E}GeV_run$(printf "%02d" $n)"
    mkdir -p "$OUTDIR"

    # Generate random seeds
    SEED1=$(od -An -N4 -tu4 < /dev/urandom)
    SEED2=$(od -An -N4 -tu4 < /dev/urandom)
    SEED3=$(od -An -N4 -tu4 < /dev/urandom)
    SEED4=$(od -An -N4 -tu4 < /dev/urandom)

    # Create macro file
    sed \
        -e "s/SEED1/$SEED1/" \
        -e "s/SEED2/$SEED2/" \
        -e "s/ENERGY_VALUE/${E} GeV/" \
        -e "s/PARTICLES/$PARTICLES/" \
        myrun.mac > "$OUTDIR/run.mac"

    # Run simulation
    ./exampleB1 "$OUTDIR/run.mac"

    # Move CSV outputs into this run folder
    mv tungsten.csv scint123.csv "$OUTDIR/"

done
