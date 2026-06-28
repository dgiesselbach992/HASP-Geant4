#!/bin/bash

mkdir -p energy_runs

# determine next CSV index
count=$(ls energy_runs/results_*.csv 2>/dev/null | wc -l)
next=$((count + 1))

# set output CSV name
csvfile="energy_runs/results_${next}.csv"
echo "run,total,proton,electron,gamma,pion,muon,other" > "$csvfile"

# global accumulators (for Total/Average rows)
sum_total=0
sum_proton=0
sum_electron=0
sum_gamma=0
sum_pion=0
sum_muon=0
sum_other=0

num_runs=10   # number of big runs

for big in {1..10}
do
    echo "=== Starting big run $big ==="

    # reset big-run accumulators
    big_total=0
    big_proton=0
    big_electron=0
    big_gamma=0
    big_pion=0
    big_muon=0
    big_other=0

    # 100 mini-runs of 10 events each
    for mini in {1..100}
    do
        # generate high‑quality random seeds
        SEED1=$(od -An -N4 -tu4 < /dev/urandom)
        SEED2=$(od -An -N4 -tu4 < /dev/urandom)

        # ENERGY PLACEHOLDER (replace with CDF later)
        ENERGY=$(python3 sample_energy.py)

        # create a per‑mini‑run macro with unique seeds + energy
        sed "s/SEED1/$SEED1/; s/SEED2/$SEED2/; s/ENERGY_VALUE/$ENERGY/" smaller_run.mac > runs/run_${big}_${mini}.mac

        # run Geant4 and capture output
        OUTPUT=$(./exampleB1 runs/run_${big}_${mini}.mac)

        # extract values
        total=$(echo "$OUTPUT" | grep "total interactions" | awk '{print $9}')
        proton=$(echo "$OUTPUT" | grep "proton hits" | awk '{print $5}')
        electron=$(echo "$OUTPUT" | grep "electron hits" | awk '{print $5}')
        gamma=$(echo "$OUTPUT" | grep "gamma hits" | awk '{print $5}')
        pion=$(echo "$OUTPUT" | grep "pion hits" | awk '{print $5}')
        muon=$(echo "$OUTPUT" | grep "muon hits" | awk '{print $5}')
        other=$(echo "$OUTPUT" | grep "other hits" | awk '{print $5}')

        # accumulate into big-run totals
        big_total=$((big_total + total))
        big_proton=$((big_proton + proton))
        big_electron=$((big_electron + electron))
        big_gamma=$((big_gamma + gamma))
        big_pion=$((big_pion + pion))
        big_muon=$((big_muon + muon))
        big_other=$((big_other + other))
    done

    # append ONE line per big run to CSV
    echo "$big,$big_total,$big_proton,$big_electron,$big_gamma,$big_pion,$big_muon,$big_other" >> "$csvfile"

    # accumulate into global totals
    sum_total=$((sum_total + big_total))
    sum_proton=$((sum_proton + big_proton))
    sum_electron=$((sum_electron + big_electron))
    sum_gamma=$((sum_gamma + big_gamma))
    sum_pion=$((sum_pion + big_pion))
    sum_muon=$((sum_muon + big_muon))
    sum_other=$((sum_other + big_other))

done

# compute averages
avg_total=$((sum_total / num_runs))
avg_proton=$((sum_proton / num_runs))
avg_electron=$((sum_electron / num_runs))
avg_gamma=$((sum_gamma / num_runs))
avg_pion=$((sum_pion / num_runs))
avg_muon=$((sum_muon / num_runs))
avg_other=$((sum_other / num_runs))

# append totals and averages to CSV
echo "Total,$sum_total,$sum_proton,$sum_electron,$sum_gamma,$sum_pion,$sum_muon,$sum_other" >> "$csvfile"
echo "Average,$avg_total,$avg_proton,$avg_electron,$avg_gamma,$avg_pion,$avg_muon,$avg_other" >> "$csvfile"

# Output Header
echo
echo "===== Completed Batch $next ====="
echo "Displaying results from $csvfile:"
echo

# Output the .csv file
column -s, -t < "$csvfile" | less -S
