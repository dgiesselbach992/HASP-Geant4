#include "SiPMSD.hh"
#include "RunAction.hh"
#include "G4RunManager.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4OpticalPhoton.hh"

namespace B1 {

G4bool SiPMSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    auto pre   = step->GetPreStepPoint();
    auto track = step->GetTrack();
    auto def   = track->GetDefinition();
    
    G4int id = fDetectorID;

    // Only count entries into the volume
    if (pre->GetStepStatus() != fGeomBoundary)
        return false;

    // Only optical photons
    if (def != G4OpticalPhoton::Definition()) {
        return false;
    }

    // Access RunAction
    auto* runAction =
        const_cast<B1::RunAction*>(
            static_cast<const B1::RunAction*>(
                G4RunManager::GetRunManager()->GetUserRunAction()
            )
        );

    // Count optical photon hit
    runAction->AddSiPMOpticalHit(fDetectorID);

    // Optionally stop photon after detection
    track->SetTrackStatus(fStopAndKill);

    return true;
}
} // namespace B1
