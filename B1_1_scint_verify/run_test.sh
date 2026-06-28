#!/bin/bash

# Determine next sequential test folder
n=1
while [ -d "test_output_run$(printf "%03d" $n)" ]; do
    n=$((n+1))
done
OUTDIR="test_output_run$(printf "%03d" $n)"
mkdir -p "$OUTDIR"

# Generate random seeds
SEED1=$(od -An -N4 -tu4 < /dev/urandom)
SEED2=$(od -An -N4 -tu4 < /dev/urandom)
SEED3=$(od -An -N4 -tu4 < /dev/urandom)
SEED4=$(od -An -N4 -tu4 < /dev/urandom)

ENERGY=15
PARTICLES=10000

# Create macro file
sed \
    -e "s/SEED1/$SEED1/" \
    -e "s/SEED2/$SEED2/" \
    -e "s/SEED3/$SEED3/" \
    -e "s/SEED4/$SEED4/" \
    -e "s/ENERGY_VALUE/${ENERGY} GeV/" \
    -e "s/PARTICLES/$PARTICLES/" \
    testrun.mac > "$OUTDIR/run_test.mac"

# Run simulation
./exampleB1 "$OUTDIR/run_test.mac"

# Move CSV outputs into this test folder
mv entered.csv scint.csv exited.csv "$OUTDIR/"
