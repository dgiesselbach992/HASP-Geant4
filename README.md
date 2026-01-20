# HASP-Geant4
This is best read in code format.

G4 VM uploaded 20Jan2026
Link included in allows you to download a VM which has Geant4 already downloaded(you will need a VM software preferable VMWare Workstation or Fusion)

Linux Help Sheet uploaded 20Jan2026
Linux Help Sheet contains helpful tips for navigating Geant4 VM, linux, and Geant4 code.

example_B1.zip uploaded 20Jan2026
example_B1 contains 4 sub-folders:
-B1_3_objects: a sample simulation with 3 bodies(lead target, plastic scintillator, and silicon "detector"); not updated
-B1_lead_only: test simulation with 2 bodies(tungsten target(I know, I know) and silicon "detector"); most up to date; produced all recent outputs(see G4 Outputs)
-B1_Originals: all original B1 examples
-B1_Validation: clone of B1_lead_only; 2-body system with lead or tungsten target and silicon detector; produced recent validation results(see G4 Validation Results)

Instructions for example_B1:
Once unzipped, each one of the folders within can be independently run when placed in the G4 VM
To quickly run a simulation(these steps must be carried out using Bash): select one of the 4 folders, find the B1/ folder, cd into the B1/ folder, enter "cmake ..", enter "cd ..", enter "make -j2"(or "make -j4" depending on how many cores you have available), run the simulation using "./example_B1"
To run one of the pre-coded files(assuming you have already prepared the files as previously discussed; cannot be done in B1_originals): while in the desired folder(one of the 4) enter "make -j2"(or "make -j4"), enter "chmod +x file_name.sh"(a safe bet is the myrun.sh file; it's a simple simulation that exists in all of the folders(except B1_originals)), enter "./file_name.sh"(must be the same file name as used in "chmod +x" command), expected run time is roughly a minute(you should see live updates on screen; myrun.sh executes 10 total runs)
***WARNING*** The precoded files are currently configured to run on 4 cores. If you are running less than 4 cores, the script run time will increase. The number of cores being used can be changed in the .sh file: see Linux Help File for further instructions on how to do this. 

I will make updates as I make further updates or come across issues. 
DM me on Slack if you have any questons.
