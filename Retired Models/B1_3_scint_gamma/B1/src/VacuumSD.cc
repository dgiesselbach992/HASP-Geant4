#include "VacuumSD.hh"
#include "RunAction.hh"
#include "G4RunManager.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4OpticalPhoton.hh"

namespace B1 {

G4bool VacuumSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    auto pre   = step->GetPreStepPoint();
    auto track = step->GetTrack();
    auto def   = track->GetDefinition();
    
    G4int id = fDetectorID;

    // Only count entries into the volume
    if (pre->GetStepStatus() != fGeomBoundary)
		return false;

    // Ignore optical photons here – we want charged/neutral particles etc.
    if (def == G4OpticalPhoton::Definition()) {
        return false;
    }

    // Access RunAction
    auto* runAction =
        const_cast<B1::RunAction*>(
            static_cast<const B1::RunAction*>(
                G4RunManager::GetRunManager()->GetUserRunAction()
            )
        );

    G4String name = def->GetParticleName();

    if (name == "proton") {
        runAction->AddProtonHit(fDetectorID);
    }
    else if (name == "e-") {
        runAction->AddElectronHit(fDetectorID);
    }
    else if (name == "gamma") {
        runAction->AddGammaHit(fDetectorID);
    }
    else if (name == "pi+" || name == "pi-" || name == "pi0") {
        runAction->AddPionHit(fDetectorID);
    }
    else if (name == "mu+" || name == "mu-") {
        runAction->AddMuonHit(fDetectorID);
    }
    else if (name == "kaon+" || name == "kaon-" ||
             name == "kaon0L" || name == "kaon0S") {
        runAction->AddKaonHit(fDetectorID);
    }
    else {
        runAction->AddOtherHit(fDetectorID);
    }

    // Total particle hits (non‑optical)
    runAction->AddHit(fDetectorID);

    return true;
}
} // namespace B1
