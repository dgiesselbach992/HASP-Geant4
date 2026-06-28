//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file RunAction.cc
/// \brief Implementation of the B1::RunAction class

#include "RunAction.hh"

#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4AccumulableManager.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

std::string ClassifyParticle(const std::string& name, G4int parentID)
{
    // --- Protons ---
    if (name == "proton") {
        return (parentID == 0) ? "primary_proton" : "secondary_proton";
    }

    // --- Neutrons ---
    if (name == "neutron") return name;

    // --- Electrons & Positrons ---
    if (name == "e-") return name;
    if (name == "e+") return name;

    // --- Muons ---
    if (name == "mu-") return name;
    if (name == "mu+") return name;

    // --- Pions ---
    if (name == "pi-") return name;
    if (name == "pi+") return name;

    // --- Gammas ---
    if (name == "gamma") return name;

    // --- Everything else ---
    return "other";
}

RunAction::RunAction()
{
  // add new units for dose
  //
  const G4double milligray = 1.e-3 * gray;
  const G4double microgray = 1.e-6 * gray;
  const G4double nanogray = 1.e-9 * gray;
  const G4double picogray = 1.e-12 * gray;

  new G4UnitDefinition("milligray", "milliGy", "Dose", milligray);
  new G4UnitDefinition("microgray", "microGy", "Dose", microgray);
  new G4UnitDefinition("nanogray", "nanoGy", "Dose", nanogray);
  new G4UnitDefinition("picogray", "picoGy", "Dose", picogray);

  // Register accumulable to the accumulable manager
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Register(fEdep);
  accumulableManager->Register(fEdep2);
  
  fTotalHits.resize(NUM_DETECTORS);
  fProtonHits.resize(NUM_DETECTORS);
  fElectronHits.resize(NUM_DETECTORS);
  fGammaHits.resize(NUM_DETECTORS);
  fPionHits.resize(NUM_DETECTORS);
  fMuonHits.resize(NUM_DETECTORS);
  fKaonHits.resize(NUM_DETECTORS);
  fOtherHits.resize(NUM_DETECTORS);

  fSiPMOpticalHits.resize(NUM_DETECTORS);

  for (int i = 0; i < NUM_DETECTORS; i++) {
    accumulableManager->Register(fTotalHits[i]);
    accumulableManager->Register(fProtonHits[i]);
    accumulableManager->Register(fElectronHits[i]);
    accumulableManager->Register(fGammaHits[i]);
    accumulableManager->Register(fPionHits[i]);
    accumulableManager->Register(fMuonHits[i]);
    accumulableManager->Register(fKaonHits[i]);
    accumulableManager->Register(fOtherHits[i]);

    accumulableManager->Register(fSiPMOpticalHits[i]);
  }
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::ScintCount(const std::string& name, G4int parentID) {
    std::string key = ClassifyParticle(name, parentID);
    fScintCounts[key]++;
}

void RunAction::EnterCount(const std::string& name, G4int parentID) {
    std::string key = ClassifyParticle(name, parentID);
    fEnterCounts[key]++;
}

void RunAction::ExitCount(const std::string& name, G4int parentID) {
    std::string key = ClassifyParticle(name, parentID);
    fEnterCounts[key]++;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run*)
{
  // inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);

  // reset accumulables to their initial values
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Reset();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* run)
{
  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) return;

  // Merge accumulables
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Merge();

  // Compute dose = total energy deposit in a run and its variance
  G4double edep  = fEdep.GetValue();
  G4double edep2 = fEdep2.GetValue();

  G4double rms = edep2 - edep * edep / nofEvents;
  if (rms > 0.)
    rms = std::sqrt(rms);
  else
    rms = 0.;

  const auto detConstruction = static_cast<const DetectorConstruction*>(
    G4RunManager::GetRunManager()->GetUserDetectorConstruction());
	G4double mass    = detConstruction->GetScoringVolume()->GetMass();
	G4double dose    = edep / mass;
	G4double rmsDose = rms / mass;

  // Run conditions
  const auto generatorAction = static_cast<const PrimaryGeneratorAction*>(
    G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());
  G4String runCondition;
  if (generatorAction) {
    const G4ParticleGun* particleGun = generatorAction->GetParticleGun();
    runCondition += particleGun->GetParticleDefinition()->GetParticleName();
    runCondition += " of ";
    G4double particleEnergy = particleGun->GetParticleEnergy();
    runCondition += G4BestUnit(particleEnergy, "Energy");
  }

  // Print
  if (IsMaster()) {
    G4cout << G4endl << "--------------------End of Global Run-----------------------";
  } else {
    G4cout << G4endl << "--------------------End of Local Run------------------------";
  }

  G4cout << G4endl
         << " The run is " << nofEvents << " " << runCondition << G4endl << G4endl;
  G4cout << "  --> cumulated edep per run in scoring volume = "
         << G4BestUnit(edep, "Energy")
         << " = " << edep/joule << " joule" << G4endl;
  G4cout << "  --> mass of scoring volume = "
         << G4BestUnit(mass, "Mass") << G4endl << G4endl;
  G4cout << " Absorbed dose per run in scoring volume = edep/mass = "
         << G4BestUnit(dose, "Dose")
         << "; rms = " << G4BestUnit(rmsDose, "Dose") << G4endl
         << "------------------------------------------------------------" << G4endl << G4endl;
  if (!IsMaster()) {
	for (int det = 0; det < NUM_DETECTORS; det++) {

        G4cout << G4endl;
        G4cout << "================ Detector " << det+1 << " ================" << G4endl;

        // Vacuum section
        G4cout << "=== Vacuum Detector (non-optical) ===" << G4endl;
        G4cout << "proton:   " << fProtonHits[det].GetValue()   << G4endl;
        G4cout << "electron: " << fElectronHits[det].GetValue() << G4endl;
        G4cout << "gamma:    " << fGammaHits[det].GetValue()    << G4endl;
        G4cout << "pion:     " << fPionHits[det].GetValue()     << G4endl;
        G4cout << "muon:     " << fMuonHits[det].GetValue()     << G4endl;
        G4cout << "kaon:     " << fKaonHits[det].GetValue()     << G4endl;
        G4cout << "other:    " << fOtherHits[det].GetValue()    << G4endl;
        G4cout << "total:    " << fTotalHits[det].GetValue()    << G4endl;

        G4cout << G4endl;

        // SiPM section
        G4cout << "=== SiPM Detector (optical photons) ===" << G4endl;
        G4cout << "optical_photons: "
               << fSiPMOpticalHits[det].GetValue()
               << G4endl;
               
        G4cout << "=== END DETECTOR " << det+1 << " ===" << G4endl;
    }
  }
	// only workers print scintillation + tungsten summaries
	if (IsMaster()) {
		G4cout << "\n================ Scintillation Summary ================\n";

		G4cout << "\n--- Particles that scintillated in the 2nd acintillator ---\n";
		for (const auto& kv : fScintCounts)
			G4cout << kv.first << ": " << kv.second << G4endl;

		G4cout << "\n--- Particles that entered the 2nd scintillator  ---\n";
		for (const auto& kv : fEnterCounts)
			G4cout << kv.first << ": " << kv.second << G4endl;

		G4cout << "\n--- Particles that exited the 2nd scintillator ---\n";
		for (const auto& kv : fExitCounts)
			G4cout << kv.first << ": " << kv.second << G4endl;
	}
	
	if (!IsMaster()) {
		// ============================================================
		// CSV OUTPUT SECTION
		// ============================================================

		G4int runID = run->GetRunID();

		// Fixed column order for CSV
		static const std::vector<std::string> cols = {
			"primary_proton",
			"secondary_proton",
			"neutron",
			"e-",
			"e+",
			"mu-",
			"mu+",
			"pi-",
			"pi+",
			"gamma",
			"other"
		};

		// ----------------------
		// Write entered.csv
		// ----------------------
		{
			std::ofstream ft("entered.csv", std::ios::app);

			if (runID == 0) {
				ft << "run";
				for (const auto& c : cols) ft << "," << c;
				ft << "\n";
			}

			ft << runID;
			for (const auto& c : cols) {
				auto it = fEnterCounts.find(c);
				int val = (it != fEnterCounts.end()) ? it->second : 0;
				ft << "," << val;
			}
			ft << "\n";
			}

		// ----------------------
		// Write scint.csv
		// ----------------------
		{
			std::ofstream fs("scint.csv", std::ios::app);

			if (runID == 0) {
				fs << "run";
				for (const auto& c : cols) fs << "," << c;
					fs << "\n";
				}

			fs << runID;
			for (const auto& c : cols) {
				auto it = fScintCounts.find(c);
				int val = (it != fScintCounts.end()) ? it->second : 0;
				fs << "," << val;
			}
			fs << "\n";
			}
			
		// ----------------------
		// Write exited.csv
		// ----------------------
		{
			std::ofstream fs("exited.csv", std::ios::app);

			if (runID == 0) {
				fs << "run";
				for (const auto& c : cols) fs << "," << c;
					fs << "\n";
				}

			fs << runID;
			for (const auto& c : cols) {
				auto it = fExitCounts.find(c);
				int val = (it != fExitCounts.end()) ? it->second : 0;
				fs << "," << val;
			}
			fs << "\n";
			}
			}
		}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::AddEdep(G4double edep)
{
  fEdep += edep;
  fEdep2 += edep * edep;
}

void RunAction::AddProtonHit(int detID)      { fProtonHits[detID] += 1; }
void RunAction::AddElectronHit(int detID)    { fElectronHits[detID] += 1; }
void RunAction::AddGammaHit(int detID)       { fGammaHits[detID] += 1; }
void RunAction::AddPionHit(int detID)        { fPionHits[detID] += 1; }
void RunAction::AddMuonHit(int detID)        { fMuonHits[detID] += 1; }
void RunAction::AddKaonHit(int detID)        { fKaonHits[detID] += 1; }
void RunAction::AddOtherHit(int detID)       { fOtherHits[detID] += 1; }

void RunAction::AddHit(int detID)            { fTotalHits[detID] += 1; }

void RunAction::AddSiPMOpticalHit(int detID) { fSiPMOpticalHits[detID] += 1; }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1
