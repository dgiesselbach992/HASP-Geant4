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
/// \file SteppingAction.cc
/// \brief Implementation of the B1::SteppingAction class

#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"

#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"

#include "G4OpticalPhoton.hh"

#include "RunAction.hh"

static bool fullTracking = true; // flip manually

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(EventAction* eventAction) : fEventAction(eventAction) {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    if (!fullTracking) return;

    // ------------------------------------------------------------
    // Ensure scoring volume pointer is initialized
    // ------------------------------------------------------------
    if (!fScoringVolume) {
        const auto detConstruction =
            static_cast<const DetectorConstruction*>(
                G4RunManager::GetRunManager()->GetUserDetectorConstruction());
        fScoringVolume = detConstruction->GetScoringVolume();
    }

    // ------------------------------------------------------------
    // Safe extraction of pre/post volumes
    // ------------------------------------------------------------
    auto prePoint  = step->GetPreStepPoint();
    auto postPoint = step->GetPostStepPoint();

    auto preTouchable  = prePoint->GetTouchableHandle();
    auto postTouchable = postPoint->GetTouchableHandle();

    G4LogicalVolume* preVol  = nullptr;
    G4LogicalVolume* postVol = nullptr;

    if (preTouchable && preTouchable->GetVolume()) {
        preVol = preTouchable->GetVolume()->GetLogicalVolume();
    }
    if (postTouchable && postTouchable->GetVolume()) {
        postVol = postTouchable->GetVolume()->GetLogicalVolume();
    }
    // ------------------------------------------------------------
    // Energy deposition inside scoring volume
    // ------------------------------------------------------------
    if (preVol == fScoringVolume) {
        fEventAction->AddEdep(step->GetTotalEnergyDeposit());
    }
    // ------------------------------------------------------------
    // Scintillation tracking inside scintillators
    // ------------------------------------------------------------
    // Identify the volume where this step occurs
    auto volume = preTouchable->GetVolume();
    if (!volume) return;

    G4String volName = volume->GetName();

    // Only care about steps inside the three scintillators
    if (volName == "Scintillator 1" ||
        volName == "Scintillator 2" ||
        volName == "Scintillator 3") {

        // Look at secondaries produced in this step
        const auto secondaries = step->GetSecondaryInCurrentStep();
        bool producedOpticalPhoton = false;

        for (const auto* sec : *secondaries) {
            if (sec->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
                producedOpticalPhoton = true;
                break;
            }
        }

        // If no optical photons were produced, no scintillation to record
        if (!producedOpticalPhoton) {
            return;
        }

        // The primary track that caused scintillation
        auto track    = step->GetTrack();
        auto particle = track->GetDefinition();

        G4int trackID = track->GetTrackID();
        G4String particleName = particle->GetParticleName();

        // Update the event's scintillation map
        auto& scintMap = fEventAction->GetScintMap();
		auto* runAction = fEventAction->GetRunAction();
		auto it = scintMap.find(trackID);

		if (it == scintMap.end()) {
			// First time this track scintillates → create a fresh struct
			ScintInfo info;
			info.particleName = particleName;              // now std::string
			info.parentID = track->GetParentID();
			info.detectors.insert(volName);
			scintMap[trackID] = info;
			runAction->AddScintAll(particleName, track->GetParentID());
		} else {
			// Track already exists → only update detectors
			it->second.detectors.insert(volName);
		}
    }
    // ------------------------------------------------------------
	// Tungsten EXIT tracking (correct behavior)
	// ------------------------------------------------------------
	auto prePV  = preTouchable->GetVolume();
	auto postPV = postTouchable->GetVolume();

	if (prePV && prePV->GetName() == "Tungsten(Target)" &&
		postPV && postPV->GetName() != "Tungsten(Target)" &&
		postPoint->GetStepStatus() == fGeomBoundary) {
		auto* track = step->GetTrack();
		std::string name = track->GetDefinition()->GetParticleName();
		fEventAction->GetRunAction()->AddTungstenExit(name, track->GetParentID());
	}
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
}  // namespace B1
