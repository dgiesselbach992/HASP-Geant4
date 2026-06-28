import subprocess
import re
import csv
from collections import Counter
import random
import os
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# ---------------- CONFIG ----------------
NUM_RUNS = 10          # how many runs
ENERGY = 5             # GeV
NUM_PARTICLES = 5000   # primaries per run
EXECUTABLE = "./exampleB1"
RESULTS_DIR = f"results_E{ENERGY}"
# ----------------------------------------

os.makedirs(RESULTS_DIR, exist_ok=True)

particles_cols = ["neutron","electron","positron","muon","pion","gamma","proton","other"]

def map_particle(name: str) -> str:
    if name == "neutron":
        return "neutron"
    if name == "e-":
        return "electron"
    if name == "e+":
        return "positron"
    if name.startswith("mu"):
        return "muon"
    if name.startswith("pi"):
        return "pion"
    if name == "gamma":
        return "gamma"
    if name == "proton":
        return "proton"
    return "other"

def extract_block(text: str, start_pattern: str) -> list[str]:
    """Grab lines after a header until the next --- / === / blank."""
    lines = text.splitlines()
    out = []
    in_block = False
    start_re = re.compile(start_pattern)
    for line in lines:
        if not in_block and start_re.search(line):
            in_block = True
            continue
        if in_block:
            if line.startswith("---") or line.startswith("===") or line.strip() == "":
                break
            out.append(line)
    return out

def parse_block(lines: list[str]) -> Counter:
    c = Counter()
    for line in lines:
        if ":" not in line:
            continue
        name, val = line.split(":", 1)
        name = name.strip()
        val = val.strip()
        if not val.isdigit():
            continue
        col = map_particle(name)
        c[col] += int(val)
    return c

def write_csv(path, rows):
    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["run"] + particles_cols)
        for r in rows:
            w.writerow(r)
        sums = ["sum"]
        for j in range(1, len(particles_cols)+1):
            sums.append(sum(r[j] for r in rows))
        w.writerow(sums)

rows_all = []
rows_12 = []
rows_123 = []
rows_tung = []

for run in range(1, NUM_RUNS+1):
    print(f"Run {run}/{NUM_RUNS}")
    seed1 = random.randint(1, 2**31-1)
    seed2 = random.randint(1, 2**31-1)

    # temporary macro for this run
    with open("tmp.mac", "w") as f:
        f.write(f"/random/setSeeds {seed1} {seed2}\n")
        f.write(f"/gun/energy {ENERGY} GeV\n")
        f.write(f"/run/beamOn {NUM_PARTICLES}\n")

    proc = subprocess.run(
        [EXECUTABLE, "tmp.mac"],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True
    )
    out = proc.stdout

    block_all  = extract_block(out, r"Particles that scintillated \(any detector\)")
    block_12   = extract_block(out, r"scintillated in Scintillators 1 & 2")
    block_123  = extract_block(out, r"scintillated in Scintillators 1, 2 & 3")
    block_tung = extract_block(out, r"Tungsten\(Target\)")

    c_all  = parse_block(block_all)
    c_12   = parse_block(block_12)
    c_123  = parse_block(block_123)
    c_tung = parse_block(block_tung)

    rows_all.append([run]  + [c_all[p]  for p in particles_cols])
    rows_12.append([run]   + [c_12[p]   for p in particles_cols])
    rows_123.append([run]  + [c_123[p]  for p in particles_cols])
    rows_tung.append([run] + [c_tung[p] for p in particles_cols])

write_csv(os.path.join(RESULTS_DIR, "scintAll.csv"),  rows_all)
write_csv(os.path.join(RESULTS_DIR, "scint12.csv"),   rows_12)
write_csv(os.path.join(RESULTS_DIR, "scint123.csv"),  rows_123)
write_csv(os.path.join(RESULTS_DIR, "tungsten.csv"),  rows_tung)

# --------- quick plot: produced vs scint in 3 detectors ---------
tung_df = pd.read_csv(os.path.join(RESULTS_DIR, "tungsten.csv"))
sc123_df = pd.read_csv(os.path.join(RESULTS_DIR, "scint123.csv"))

tung_sum = tung_df[tung_df["run"] == "sum"].iloc[0]
sc123_sum = sc123_df[sc123_df["run"] == "sum"].iloc[0]

produced = [tung_sum[p] for p in particles_cols]
scint123 = [sc123_sum[p] for p in particles_cols]

x = np.arange(len(particles_cols))
w = 0.35

plt.figure(figsize=(10,6))
plt.bar(x - w/2, produced, w, label="Produced in Tungsten")
plt.bar(x + w/2, scint123, w, label="Scintillated in 1,2,3")

plt.yscale("log")
plt.xticks(x, particles_cols, rotation=45)
plt.ylabel("Counts (log scale)")
plt.title(f"Production vs 3‑Detector Scintillation (E = {ENERGY} GeV)")
plt.legend()
plt.grid(axis="y", which="both", linestyle="--", alpha=0.4)
plt.tight_layout()
plt.show()
