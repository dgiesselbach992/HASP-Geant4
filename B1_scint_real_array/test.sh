#!/bin/bash

### ================================
### USER PARAMETERS
### ================================
NUM_RUNS=${1:-10}
ENERGY=${2:-2.5}
NUM_PARTICLES=${3:-5000}

### ================================
### SETUP
### ================================
mkdir -p batch_runs
RESULTS_DIR="results_E${ENERGY}"
mkdir -p "$RESULTS_DIR"

count=$(ls $RESULTS_DIR/results_*.csv 2>/dev/null | wc -l)
next=$((count + 1))
csvfile="$RESULTS_DIR/results_${next}.csv"

echo "run,\
d1_proton,d1_electron,d1_gamma,d1_pion,d1_muon,d1_kaon,d1_other,d1_total,d1_photons,\
d2_proton,d2_electron,d2_gamma,d2_pion,d2_muon,d2_kaon,d2_other,d2_total,d2_photons,\
d3_proton,d3_electron,d3_gamma,d3_pion,d3_muon,d3_kaon,d3_other,d3_total,d3_photons,\
d4_proton,d4_electron,d4_gamma,d4_pion,d4_muon,d4_kaon,d4_other,d4_total,d4_photons,"\
> "$csvfile"

### ================================
### ACCUMULATORS
### ================================
sum1_proton=0; sum1_electron=0; sum1_gamma=0; sum1_pion=0
sum1_muon=0;  sum1_kaon=0;    sum1_other=0;  sum1_total=0; sum1_photons=0

sum2_proton=0; sum2_electron=0; sum2_gamma=0; sum2_pion=0
sum2_muon=0;  sum2_kaon=0;    sum2_other=0;  sum2_total=0; sum2_photons=0

sum3_proton=0; sum3_electron=0; sum3_gamma=0; sum3_pion=0
sum3_muon=0;  sum3_kaon=0;    sum3_other=0;  sum3_total=0; sum3_photons=0

sum4_proton=0; sum4_electron=0; sum4_gamma=0; sum4_pion=0
sum4_muon=0;  sum4_kaon=0;    sum4_other=0;  sum4_total=0; sum4_photons=0

### ================================
### MAIN LOOP
### ================================
for i in $(seq 1 $NUM_RUNS); do
    echo "Running simulation $i..."

    SEED1=$(od -An -N4 -tu4 < /dev/urandom)
    SEED2=$(od -An -N4 -tu4 < /dev/urandom)

    sed "s/SEED1/$SEED1/; s/SEED2/$SEED2/; s/ENERGY_VALUE/$ENERGY/; s/PARTICLES/$NUM_PARTICLES/" \
        myrun.mac > batch_runs/run_$i.mac

    OUTPUT=$(./exampleB1 batch_runs/run_$i.mac 2>&1)

    # Parse both detectors in one awk pass
    mapfile -t rows < <(
        echo "$OUTPUT" | awk '
            /^================ Detector / {
                # line like: "================ Detector 1 ================"
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
                for (d=1; d<=4; d++) {
                    # default missing to 0
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

    # rows[0] -> detector 1, rows[1] -> detector 2
    read _ d1_proton d1_electron d1_gamma d1_pion d1_muon d1_kaon d1_other d1_total d1_photons <<< "${rows[0]}"
    read _ d2_proton d2_electron d2_gamma d2_pion d2_muon d2_kaon d2_other d2_total d2_photons <<< "${rows[1]}"
    read _ d3_proton d3_electron d3_gamma d3_pion d3_muon d3_kaon d3_other d3_total d3_photons <<< "${rows[2]}"
    read _ d4_proton d4_electron d4_gamma d4_pion d4_muon d4_kaon d4_other d4_total d4_photons <<< "${rows[3]}"

    echo "$i,\
$d1_proton,$d1_electron,$d1_gamma,$d1_pion,$d1_muon,$d1_kaon,$d1_other,$d1_total,$d1_photons,\
$d2_proton,$d2_electron,$d2_gamma,$d2_pion,$d2_muon,$d2_kaon,$d2_other,$d2_total,$d2_photons,\
$d3_proton,$d3_electron,$d3_gamma,$d3_pion,$d3_muon,$d3_kaon,$d3_other,$d3_total,$d3_photons,\
$d4_proton,$d4_electron,$d4_gamma,$d4_pion,$d4_muon,$d4_kaon,$d4_other,$d4_total,$d4_photons,"\
>> "$csvfile"

    sum1_proton=$((sum1_proton + d1_proton))
    sum1_electron=$((sum1_electron + d1_electron))
    sum1_gamma=$((sum1_gamma + d1_gamma))
    sum1_pion=$((sum1_pion + d1_pion))
    sum1_muon=$((sum1_muon + d1_muon))
    sum1_kaon=$((sum1_kaon + d1_kaon))
    sum1_other=$((sum1_other + d1_other))
    sum1_total=$((sum1_total + d1_total))
    sum1_photons=$((sum1_photons + d1_photons))

    sum2_proton=$((sum2_proton + d2_proton))
    sum2_electron=$((sum2_electron + d2_electron))
    sum2_gamma=$((sum2_gamma + d2_gamma))
    sum2_pion=$((sum2_pion + d2_pion))
    sum2_muon=$((sum2_muon + d2_muon))
    sum2_kaon=$((sum2_kaon + d2_kaon))
    sum2_other=$((sum2_other + d2_other))
    sum2_total=$((sum2_total + d2_total))
    sum2_photons=$((sum2_photons + d2_photons))

    sum3_proton=$((sum3_proton + d3_proton))
    sum3_electron=$((sum3_electron + d3_electron))
    sum3_gamma=$((sum3_gamma + d3_gamma))
    sum3_pion=$((sum3_pion + d3_pion))
    sum3_muon=$((sum3_muon + d3_muon))
    sum3_kaon=$((sum3_kaon + d3_kaon))
    sum3_other=$((sum3_other + d3_other))
    sum3_total=$((sum3_total + d3_total))
    sum3_photons=$((sum3_photons + d3_photons))

    sum4_proton=$((sum4_proton + d4_proton))
    sum4_electron=$((sum4_electron + d4_electron))
    sum4_gamma=$((sum4_gamma + d4_gamma))
    sum4_pion=$((sum4_pion + d4_pion))
    sum4_muon=$((sum4_muon + d4_muon))
    sum4_kaon=$((sum4_kaon + d4_kaon))
    sum4_other=$((sum4_other + d4_other))
    sum4_total=$((sum4_total + d4_total))
    sum4_photons=$((sum4_photons + d4_photons))
done

avg1_proton=$((sum1_proton / NUM_RUNS))
avg1_electron=$((sum1_electron / NUM_RUNS))
avg1_gamma=$((sum1_gamma / NUM_RUNS))
avg1_pion=$((sum1_pion / NUM_RUNS))
avg1_muon=$((sum1_muon / NUM_RUNS))
avg1_kaon=$((sum1_kaon / NUM_RUNS))
avg1_other=$((sum1_other / NUM_RUNS))
avg1_total=$((sum1_total / NUM_RUNS))
avg1_photons=$((sum1_photons / NUM_RUNS))

avg2_proton=$((sum2_proton / NUM_RUNS))
avg2_electron=$((sum2_electron / NUM_RUNS))
avg2_gamma=$((sum2_gamma / NUM_RUNS))
avg2_pion=$((sum2_pion / NUM_RUNS))
avg2_muon=$((sum2_muon / NUM_RUNS))
avg2_kaon=$((sum2_kaon / NUM_RUNS))
avg2_other=$((sum2_other / NUM_RUNS))
avg2_total=$((sum2_total / NUM_RUNS))
avg2_photons=$((sum2_photons / NUM_RUNS))

avg3_proton=$((sum3_proton / NUM_RUNS))
avg3_electron=$((sum3_electron / NUM_RUNS))
avg3_gamma=$((sum3_gamma / NUM_RUNS))
avg3_pion=$((sum3_pion / NUM_RUNS))
avg3_muon=$((sum3_muon / NUM_RUNS))
avg3_kaon=$((sum3_kaon / NUM_RUNS))
avg3_other=$((sum3_other / NUM_RUNS))
avg3_total=$((sum3_total / NUM_RUNS))
avg3_photons=$((sum3_photons / NUM_RUNS))

avg4_proton=$((sum4_proton / NUM_RUNS))
avg4_electron=$((sum4_electron / NUM_RUNS))
avg4_gamma=$((sum4_gamma / NUM_RUNS))
avg4_pion=$((sum4_pion / NUM_RUNS))
avg4_muon=$((sum4_muon / NUM_RUNS))
avg4_kaon=$((sum4_kaon / NUM_RUNS))
avg4_other=$((sum4_other / NUM_RUNS))
avg4_total=$((sum4_total / NUM_RUNS))
avg4_photons=$((sum4_photons / NUM_RUNS))

norm1_proton=$((avg1_proton / NUM_PARTICLES))
norm1_electron=$((avg1_electron / NUM_PARTICLES))
norm1_gamma=$((avg1_gamma / NUM_PARTICLES))
norm1_pion=$((avg1_pion / NUM_PARTICLES))
norm1_muon=$((avg1_muon / NUM_PARTICLES))
norm1_kaon=$((avg1_kaon / NUM_PARTICLES))
norm1_other=$((avg1_other / NUM_PARTICLES))
norm1_total=$((avg1_total / NUM_PARTICLES))
norm1_photons=$((avg1_photons / NUM_PARTICLES))

norm2_proton=$((avg2_proton / NUM_PARTICLES))
norm2_electron=$((avg2_electron / NUM_PARTICLES))
norm2_gamma=$((avg2_gamma / NUM_PARTICLES))
norm2_pion=$((avg2_pion / NUM_PARTICLES))
norm2_muon=$((avg2_muon / NUM_PARTICLES))
norm2_kaon=$((avg2_kaon / NUM_PARTICLES))
norm2_other=$((avg2_other / NUM_PARTICLES))
norm2_total=$((avg2_total / NUM_PARTICLES))
norm2_photons=$((avg2_photons / NUM_PARTICLES))

norm3_proton=$((avg3_proton / NUM_PARTICLES))
norm3_electron=$((avg3_electron / NUM_PARTICLES))
norm3_gamma=$((avg3_gamma / NUM_PARTICLES))
norm3_pion=$((avg3_pion / NUM_PARTICLES))
norm3_muon=$((avg3_muon / NUM_PARTICLES))
norm3_kaon=$((avg3_kaon / NUM_PARTICLES))
norm3_other=$((avg3_other / NUM_PARTICLES))
norm3_total=$((avg3_total / NUM_PARTICLES))
norm3_photons=$((avg3_photons / NUM_PARTICLES))

norm4_proton=$((avg4_proton / NUM_PARTICLES))
norm4_electron=$((avg4_electron / NUM_PARTICLES))
norm4_gamma=$((avg4_gamma / NUM_PARTICLES))
norm4_pion=$((avg4_pion / NUM_PARTICLES))
norm4_muon=$((avg4_muon / NUM_PARTICLES))
norm4_kaon=$((avg4_kaon / NUM_PARTICLES))
norm4_other=$((avg4_other / NUM_PARTICLES))
norm4_total=$((avg4_total / NUM_PARTICLES))
norm4_photons=$((avg4_photons / NUM_PARTICLES))
{
    echo
    echo "Total,$sum1_proton,$sum1_electron,$sum1_gamma,$sum1_pion,$sum1_muon,$sum1_kaon,$sum1_other,$sum1_total,$sum1_photons,$sum2_proton,$sum2_electron,$sum2_gamma,$sum2_pion,$sum2_muon,$sum2_kaon,$sum2_other,$sum2_total,$sum2_photons,$sum3_proton,$sum3_electron,$sum3_gamma,$sum3_pion,$sum3_muon,$sum3_kaon,$sum3_other,$sum3_total,$sum3_photons,$sum4_proton,$sum4_electron,$sum4_gamma,$sum4_pion,$sum4_muon,$sum4_kaon,$sum4_other,$sum4_total,$sum4_photons"
    echo "Average,$avg1_proton,$avg1_electron,$avg1_gamma,$avg1_pion,$avg1_muon,$avg1_kaon,$avg1_other,$avg1_total,$avg1_photons,$avg2_proton,$avg2_electron,$avg2_gamma,$avg2_pion,$avg2_muon,$avg2_kaon,$avg2_other,$avg2_total,$avg2_photons,$avg3_proton,$avg3_electron,$avg3_gamma,$avg3_pion,$avg3_muon,$avg3_kaon,$avg3_other,$avg3_total,$avg3_photons,$avg4_proton,$avg4_electron,$avg4_gamma,$avg4_pion,$avg4_muon,$avg4_kaon,$avg4_other,$avg4_total,$avg4_photons"
    echo "PerPrimary,$norm1_proton,$norm1_electron,$norm1_gamma,$norm1_pion,$norm1_muon,$norm1_kaon,$norm1_other,$norm1_total,$norm1_photons,$norm2_proton,$norm2_electron,$norm2_gamma,$norm2_pion,$norm2_muon,$norm2_kaon,$norm2_other,$norm2_total,$norm2_photons,$norm3_proton,$norm3_electron,$norm3_gamma,$norm3_pion,$norm3_muon,$norm3_kaon,$norm3_other,$norm3_total,$norm3_photons,$norm4_proton,$norm4_electron,$norm4_gamma,$norm4_pion,$norm4_muon,$norm4_kaon,$norm4_other,$norm4_total,$norm4_photons"
} >> "$csvfile"

echo "===== Completed Batch $next ====="
column -s, -t < "$csvfile" | less -S
