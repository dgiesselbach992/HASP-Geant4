#!/bin/bash

mkdir -p basic_runs

# determine next CSV index
count=$(ls basic_runs/results_*.csv 2>/dev/null | wc -l)
next=$((count + 1))

# set output CSV name
csvfile="basic_runs/results_${next}.csv"

echo "run,total,proton,electron,gamma,pion,muon,other" > "$csvfile"

# initialize accumulators
sum_total=0
sum_proton=0
sum_electron=0
sum_gamma=0
sum_pion=0
sum_muon=0
sum_other=0

num_runs=10

energy=45

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
    total=$(echo "$OUTPUT" | grep "total interactions" | awk '{print $9}')
    proton=$(echo "$OUTPUT" | grep "proton hits" | awk '{print $5}')
    electron=$(echo "$OUTPUT" | grep "electron hits" | awk '{print $5}')
    gamma=$(echo "$OUTPUT" | grep "gamma hits" | awk '{print $5}')
    pion=$(echo "$OUTPUT" | grep "pion hits" | awk '{print $5}')
    muon=$(echo "$OUTPUT" | grep "muon hits" | awk '{print $5}')
    other=$(echo "$OUTPUT" | grep "other hits" | awk '{print $5}')

    # append to CSV
    echo "$i,$total,$proton,$electron,$gamma,$pion,$muon,$other" >> "$csvfile"

    # accumulate sums
    sum_total=$((sum_total + total))
    sum_proton=$((sum_proton + proton))
    sum_electron=$((sum_electron + electron))
    sum_gamma=$((sum_gamma + gamma))
    sum_pion=$((sum_pion + pion))
    sum_muon=$((sum_muon + muon))
    sum_other=$((sum_other + other))
done

# compute averages
avg_total=$((sum_total / num_runs))
avg_proton=$((sum_proton / num_runs))
avg_electron=$((sum_electron / num_runs))
avg_gamma=$((sum_gamma / num_runs))
avg_pion=$((sum_pion / num_runs))
avg_muon=$((sum_muon / num_runs))
avg_other=$((sum_other / num_runs))

# normalize averages
norm_total=$((avg_total / num_parts))
norm_proton=$((avg_proton / num_parts))
nrom_electron=$((avg_electron / num_parts))
norm_gamma=$((avg_gamma / num_parts))
norm_pion=$((avg_pion / num_parts))
norm_muon=$((avg_muon / num_parts))
norm_other=$((avg_other / num_parts))


# append totals and averages to CSV
echo "Total,$sum_total,$sum_proton,$sum_electron,$sum_gamma,$sum_pion,$sum_muon,$sum_other" >> "$csvfile"
echo "Average,$avg_total,$avg_proton,$avg_electron,$avg_gamma,$avg_pion,$avg_muon,$avg_other" >> "$csvfile"
echo "Per Proton,$norm_total,$norm_proton,$norm_electron,$norm_gamma,$norm_pion,$norm_muon,$norm_other" >> "$csvfile"

# Output Header
echo
echo "===== Completed Batch $next ====="
echo "Displaying results from $csvfile:"
echo

# Output the .csv file
column -s, -t < "$csvfile" | less -S

