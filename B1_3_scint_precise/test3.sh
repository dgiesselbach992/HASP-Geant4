#!/bin/bash

### ================================
### USER PARAMETERS
### ================================
NUM_RUNS=10
ENERGY=12
NUM_PARTICLES=1000

### ================================
### SETUP
### ================================
mkdir -p batch_runs2
mkdir -p results2

count=$(ls results2/results_*.csv 2>/dev/null | wc -l)
next=$((count + 1))
csvfile="results2/results_${next}.csv"

echo "run,\
d1_proton,d1_electron,d1_gamma,d1_pion,d1_muon,d1_kaon,d1_other,d1_total,d1_photons,\
d2_proton,d2_electron,d2_gamma,d2_pion,d2_muon,d2_kaon,d2_other,d2_total,d2_photons,\
d3_proton,d3_electron,d3_gamma,d3_pion,d3_muon,d3_kaon,d3_other,d3_total,d3_photons" \
> "$csvfile"

### ================================
### ACCUMULATORS
### ================================
init_sums() {
    eval sum$1_proton=0
    eval sum$1_electron=0
    eval sum$1_gamma=0
    eval sum$1_pion=0
    eval sum$1_muon=0
    eval sum$1_kaon=0
    eval sum$1_other=0
    eval sum$1_total=0
    eval sum$1_photons=0
}

init_sums 1
init_sums 2
init_sums 3

### ================================
### MAIN LOOP
### ================================
for i in $(seq 1 $NUM_RUNS); do
    echo "Running simulation $i..."

    SEED1=$(od -An -N4 -tu4 < /dev/urandom)
    SEED2=$(od -An -N4 -tu4 < /dev/urandom)

    sed "s/SEED1/$SEED1/; s/SEED2/$SEED2/; s/ENERGY_VALUE/$ENERGY/; s/PARTICLES/$NUM_PARTICLES/" \
        myrun.mac > batch_runs2/run_$i.mac

    OUTPUT=$(./exampleB1 batch_runs2/run_$i.mac 2>&1)

    # Parse all 3 detectors in one AWK pass
    mapfile -t rows < <(
        echo "$OUTPUT" | awk '
            /^================ Detector / {
                for (i=1; i<=NF; i++) {
                    if ($i == "Detector") {
                        det = $(i+1) + 0
                        break
                    }
                }
            }
            /^proton:/          { proton[det]=$2 }
            /^electron:/        { electron[det]=$2 }
            /^gamma:/           { gamma[det]=$2 }
            /^pion:/            { pion[det]=$2 }
            /^muon:/            { muon[det]=$2 }
            /^kaon:/            { kaon[det]=$2 }
            /^other:/           { other[det]=$2 }
            /^total:/           { total[det]=$2 }
            /^optical_photons:/ { photons[det]=$2 }

            END {
                for (d=1; d<=3; d++) {
                    if (proton[d]   == "") proton[d]=0
                    if (electron[d] == "") electron[d]=0
                    if (gamma[d]    == "") gamma[d]=0
                    if (pion[d]     == "") pion[d]=0
                    if (muon[d]     == "") muon[d]=0
                    if (kaon[d]     == "") kaon[d]=0
                    if (other[d]    == "") other[d]=0
                    if (total[d]    == "") total[d]=0
                    if (photons[d]  == "") photons[d]=0

                    print d, proton[d], electron[d], gamma[d], pion[d], \
                             muon[d], kaon[d], other[d], total[d], photons[d]
                }
            }
        '
    )

    # rows[0] = detector 1
    # rows[1] = detector 2
    # rows[2] = detector 3
    read _ d1_proton d1_electron d1_gamma d1_pion d1_muon d1_kaon d1_other d1_total d1_photons <<< "${rows[0]}"
    read _ d2_proton d2_electron d2_gamma d2_pion d2_muon d2_kaon d2_other d2_total d2_photons <<< "${rows[1]}"
    read _ d3_proton d3_electron d3_gamma d3_pion d3_muon d3_kaon d3_other d3_total d3_photons <<< "${rows[2]}"

    echo "$i,\
$d1_proton,$d1_electron,$d1_gamma,$d1_pion,$d1_muon,$d1_kaon,$d1_other,$d1_total,$d1_photons,\
$d2_proton,$d2_electron,$d2_gamma,$d2_pion,$d2_muon,$d2_kaon,$d2_other,$d2_total,$d2_photons,\
$d3_proton,$d3_electron,$d3_gamma,$d3_pion,$d3_muon,$d3_kaon,$d3_other,$d3_total,$d3_photons" \
>> "$csvfile"

    # Accumulate for each detector
    for det in 1 2 3; do
        for key in proton electron gamma pion muon kaon other total photons; do
            eval val=\$d${det}_${key}
            eval sum${det}_${key}=\$(( sum${det}_${key} + val ))
        done
    done

done

### ================================
### TOTALS, AVERAGES, NORMALIZED
### ================================
write_stats() {
    det=$1
    prefix=$2

    echo -n "${prefix}_D${det}," >> "$csvfile"
    for key in proton electron gamma pion muon kaon other total photons; do
        eval v=\$${prefix}${det}_${key}
        echo -n "$v," >> "$csvfile"
    done
    echo "" >> "$csvfile"
}

# Compute averages and normalized values
for det in 1 2 3; do
    for key in proton electron gamma pion muon kaon other total photons; do
        eval sum=\$sum${det}_${key}
        eval avg${det}_${key}=\$(( sum / NUM_RUNS ))
        eval norm${det}_${key}=\$(( avg${det}_${key} / NUM_PARTICLES ))
    done
done

echo "" >> "$csvfile"

write_stats 1 sum
write_stats 2 sum
write_stats 3 sum

write_stats 1 avg
write_stats 2 avg
write_stats 3 avg

write_stats 1 norm
write_stats 2 norm
write_stats 3 norm

echo "===== Completed Batch $next ====="
column -s, -t < "$csvfile" | less -S
