#include "SiPMSD.hh"
#include "RunAction.hh"
#include "G4RunManager.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

namespace B1 {

G4bool B1::SiPMSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    auto pre = step->GetPreStepPoint();

    // Count only entries into the volume
    if (pre->GetStepStatus() == fGeomBoundary) {

        auto track = step->GetTrack();
        auto name  = track->GetDefinition()->GetParticleName();
        
        // Only needed for debugging
        // fHitCount++;
        // G4cout << "SiPMSD HIT " << fHitCount
               // << " : " << name << " entered SiPM" << G4endl;

        // Get RunAction and increment global hit counter
        auto* runAction =
            const_cast<B1::RunAction*>(
                static_cast<const B1::RunAction*>(
                    G4RunManager::GetRunManager()->GetUserRunAction()
                )
            );
        // Increment per-particle counters
        if (name == "proton") {
			runAction->AddProtonHit();
		} else if (name == "e-") {
			runAction->AddElectronHit();
		} else if (name == "gamma") {
			runAction->AddGammaHit();
		} else if (name == "pi+" || name == "pi-" || name == "pi0") {
			runAction->AddPionHit();
		} else if (name == "mu+" || name == "mu-") {
			runAction->AddMuonHit();
		} else {
		runAction->AddOtherHit();
		}
		// Increment total hits
        runAction->AddHit();
    }
    return true;
}
} // namespace B1
