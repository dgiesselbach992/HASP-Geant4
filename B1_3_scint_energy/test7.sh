#!/bin/bash

NUM_RUNS=${1:-10}        # number of runs
ENERGY=${2:-5}           # GeV
NUM_PARTICLES=${3:-5000} # primaries per run

mkdir -p batch_runs

RESULTS_DIR="results_E${ENERGY}"
mkdir -p "$RESULTS_DIR"

# Output CSVs
CSV12="$RESULTS_DIR/scint12.csv"
CSV123="$RESULTS_DIR/scint123.csv"
CSVTUNG="$RESULTS_DIR/tungsten.csv"
CSVALL="$RESULTS_DIR/scintAll.csv"

# Write headers
echo "run,neutron,electron,positron,muon,pion,gamma,proton,other" > "$CSV12"
echo "run,neutron,electron,positron,muon,pion,gamma,proton,other" > "$CSV123"
echo "run,neutron,electron,positron,muon,pion,gamma,proton,other" > "$CSVTUNG"
echo "run,neutron,electron,positron,muon,pion,gamma,proton,other" > "$CSVALL"

# Map particle names to categories
map_particle() {
    case "$1" in
        neutron) echo "neutron" ;;
        e-)      echo "electron" ;;
        e+)      echo "positron" ;;
        mu-|mu+|mu0) echo "muon" ;;
        pi-|pi+|pi0) echo "pion" ;;
        gamma)   echo "gamma" ;;
        proton)  echo "proton" ;;
        *)       echo "other" ;;
    esac
}

# Loop over runs
for i in $(seq 1 $NUM_RUNS); do
    echo "Running simulation $i..."

    SEED1=$(od -An -N4 -tu4 < /dev/urandom)
    SEED2=$(od -An -N4 -tu4 < /dev/urandom)

    sed "s/SEED1/$SEED1/; s/SEED2/$SEED2/; s/ENERGY_VALUE/$ENERGY/; s/PARTICLES/$NUM_PARTICLES/" \
        myrun.mac > batch_runs/run_$i.mac

    OUTPUT=$(./exampleB1 batch_runs/run_$i.mac 2>&1)

    # ------------------------------------------------------------
    # Robust block extraction (stops at next header)
    # ------------------------------------------------------------

    BLOCKALL=$(echo "$OUTPUT" | awk '
        /Particles that scintillated \(any detector\)/ {flag=1; next}
        /^---/ && flag {flag=0}
        /^===/ && flag {flag=0}
        flag
    ')

    BLOCK12=$(echo "$OUTPUT" | awk '
        /scintillated in Scintillators 1 & 2/ {flag=1; next}
        /^---/ && flag {flag=0}
        /^===/ && flag {flag=0}
        flag
    ')

    BLOCK123=$(echo "$OUTPUT" | awk '
        /scintillated in Scintillators 1, 2 & 3/ {flag=1; next}
        /^---/ && flag {flag=0}
        /^===/ && flag {flag=0}
        flag
    ')

    BLOCKTUNG=$(echo "$OUTPUT" | awk '
        /Tungsten\(Target\)/ {flag=1; next}
        /^---/ && flag {flag=0}
        /^===/ && flag {flag=0}
        flag
    ')

    # ------------------------------------------------------------
    # Initialize counters
    # ------------------------------------------------------------
    declare -A COUNT12 COUNT123 COUNTTUNG COUNTALL
    for p in neutron electron positron muon pion gamma proton other; do
        COUNT12[$p]=0
        COUNT123[$p]=0
        COUNTTUNG[$p]=0
        COUNTALL[$p]=0
    done

    # ------------------------------------------------------------
    # Parse blocks
    # ------------------------------------------------------------

    parse_block() {
        local block="$1"
        local -n arr=$2
        while read -r line; do
            name=$(echo "$line" | cut -d: -f1)
            val=$(echo "$line" | cut -d: -f2)
            col=$(map_particle "$name")
            arr[$col]=$((arr[$col] + val))
        done <<< "$(echo "$block" | grep :)"
    }

    parse_block "$BLOCK12"  COUNT12
    parse_block "$BLOCK123" COUNT123
    parse_block "$BLOCKTUNG" COUNTTUNG
    parse_block "$BLOCKALL" COUNTALL

    # ------------------------------------------------------------
    # Append rows
    # ------------------------------------------------------------
    echo "$i,${COUNT12[neutron]},${COUNT12[electron]},${COUNT12[positron]},${COUNT12[muon]},${COUNT12[pion]},${COUNT12[gamma]},${COUNT12[proton]},${COUNT12[other]}" >> "$CSV12"

    echo "$i,${COUNT123[neutron]},${COUNT123[electron]},${COUNT123[positron]},${COUNT123[muon]},${COUNT123[pion]},${COUNT123[gamma]},${COUNT123[proton]},${COUNT123[other]}" >> "$CSV123"

    echo "$i,${COUNTTUNG[neutron]},${COUNTTUNG[electron]},${COUNTTUNG[positron]},${COUNTTUNG[muon]},${COUNTTUNG[pion]},${COUNTTUNG[gamma]},${COUNTTUNG[proton]},${COUNTTUNG[other]}" >> "$CSVTUNG"

    echo "$i,${COUNTALL[neutron]},${COUNTALL[electron]},${COUNTALL[positron]},${COUNTALL[muon]},${COUNTALL[pion]},${COUNTALL[gamma]},${COUNTALL[proton]},${COUNTALL[other]}" >> "$CSVALL"

done

# ------------------------------------------------------------
# Add sum rows
# ------------------------------------------------------------
sum_csv() {
    file="$1"
    sums=$(awk -F, 'NR>1 {for(i=2;i<=NF;i++) s[i]+=$i} END {printf "sum"; for(i=2;i<=NF;i++) printf ",%d", s[i]; printf "\n"}' "$file")
    echo "$sums" >> "$file"
}

sum_csv "$CSV12"
sum_csv "$CSV123"
sum_csv "$CSVTUNG"
sum_csv "$CSVALL"

echo "===== Completed Batch ====="
