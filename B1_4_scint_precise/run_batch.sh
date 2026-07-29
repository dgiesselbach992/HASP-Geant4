#!/bin/bash

mkdir -p basic_runs
mkdir -p runs

# determine next CSV index
count=$(ls basic_runs/results_*.csv 2>/dev/null | wc -l)
next=$((count + 1))

# set output CSV name
csvfile="basic_runs/results_${next}.csv"

echo "run,total,proton,electron,gamma,pion,muon,kaon,other,photons" > "$csvfile"

# initialize accumulators
sum_total=0
sum_proton=0
sum_electron=0
sum_gamma=0
sum_pion=0
sum_muon=0
sum_kaon=0
sum_other=0
sum_photons=0

num_runs=10

energy=12

num_parts=1000

for i in {1..10}
do
    # Print Run Number
    echo "Running simulation $i..."

    # generate high‑quality random seeds
    SEED1=$(od -An -N4 -tu4 < /dev/urandom)
    SEED2=$(od -An -N4 -tu4 < /dev/urandom)

    # create a per‑run macro with unique seeds
    sed "s/SEED1/$SEED1/; s/SEED2/$SEED2/; s/ENERGY_VALUE/$energy/; s/PARTICLES/$num_parts/" myrun.mac > runs/run_$i.mac

    # capture program output directly into a variable
    OUTPUT=$(./exampleB1 runs/run_$i.mac)

    # extract values from the captured output
    total=$(echo "$OUTPUT" | awk '/total interactions/ {print $NF}')
    proton=$(echo "$OUTPUT" | awk '/proton hits/ {print $NF}')
    electron=$(echo "$OUTPUT" | awk '/electron hits/ {print $NF}')
    gamma=$(echo "$OUTPUT" | awk '/gamma hits/ {print $NF}')
    pion=$(echo "$OUTPUT" | awk '/pion hits/ {print $NF}')
    muon=$(echo "$OUTPUT" | awk '/muon hits/ {print $NF}')
    kaon=$(echo "$OUTPUT" | awk '/kaon hits/ {print $NF}')
    other=$(echo "$OUTPUT" | awk '/other hits/ {print $NF}')
    photons=$(echo "$OUTPUT" | awk '/optical photon hits/ {print $NF}')

    # default empty values to zero
    total=${total:-0}
    proton=${proton:-0}
    electron=${electron:-0}
    gamma=${gamma:-0}
    pion=${pion:-0}
    muon=${muon:-0}
    kaon=${kaon:-0}
    other=${other:-0}
    photons=${photons:-0}

    # append to CSV
    echo "$i,$total,$proton,$electron,$gamma,$pion,$muon,$kaon,$other,$photons" >> "$csvfile"

    # accumulate sums
    sum_total=$((sum_total + total))
    sum_proton=$((sum_proton + proton))
    sum_electron=$((sum_electron + electron))
    sum_gamma=$((sum_gamma + gamma))
    sum_pion=$((sum_pion + pion))
    sum_muon=$((sum_muon + muon))
    sum_kaon=$((sum_kaon + kaon))
    sum_other=$((sum_other + other))
    sum_photons=$((sum_photons + photons))
done

# compute averages
avg_total=$((sum_total / num_runs))
avg_proton=$((sum_proton / num_runs))
avg_electron=$((sum_electron / num_runs))
avg_gamma=$((sum_gamma / num_runs))
avg_pion=$((sum_pion / num_runs))
avg_muon=$((sum_muon / num_runs))
avg_kaon=$((sum_kaon / num_runs))
avg_other=$((sum_other / num_runs))
avg_photons=$((sum_photons / num_runs))

# normalize averages
norm_total=$((avg_total / num_parts))
norm_proton=$((avg_proton / num_parts))
norm_electron=$((avg_electron / num_parts))
norm_gamma=$((avg_gamma / num_parts))
norm_pion=$((avg_pion / num_parts))
norm_muon=$((avg_muon / num_parts))
norm_kaons=$((avg_kaon / num_parts))
norm_other=$((avg_other / num_parts))
norm_photons=$((avg_photons / num_parts))

# append totals and averages to CSV
echo "Total,$sum_total,$sum_proton,$sum_electron,$sum_gamma,$sum_pion,$sum_muon,$sum_kaon,$sum_other,$sum_photons" >> "$csvfile"
echo "Average,$avg_total,$avg_proton,$avg_electron,$avg_gamma,$avg_pion,$avg_muon,$avg_kaon,$avg_other,$avg_photons" >> "$csvfile"
echo "Per Proton,$norm_total,$norm_proton,$norm_electron,$norm_gamma,$norm_pion,$norm_muon,$norm_kaon,$norm_other,$norm_photons" >> "$csvfile"

# Output Header
echo
echo "===== Completed Batch $next ====="
echo "Displaying results from $csvfile:"
echo

# Output the .csv file
column -s, -t < "$csvfile" | less -S

