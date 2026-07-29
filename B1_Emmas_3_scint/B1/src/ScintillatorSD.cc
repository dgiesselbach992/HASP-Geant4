#include "ScintillatorSD.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4TouchableHistory.hh"
#include "G4RunManager.hh"
#include "G4ParticleDefinition.hh"

namespace B1 {

ScintillatorSD::ScintillatorSD(const G4String& name)
: G4VSensitiveDetector(name)
{
    trackEdep.clear();
    trackParticle.clear();
}

G4bool ScintillatorSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    G4double edep = step->GetTotalEnergyDeposit();
    if (edep <= 0.0) return false;

    auto track = step->GetTrack();
    G4int trackID = track->GetTrackID();
    G4String particle = track->GetDefinition()->GetParticleName();

    // *** FIXED: use POST step point to get the actual scintillator volume ***
    auto postTouchable = step->GetPostStepPoint()->GetTouchable();
    auto postVolume = postTouchable->GetVolume();

    if (!postVolume) return false;  // safety check

    G4int copyNo = postVolume->GetCopyNo();   // 0,1,2 for your scintillators

    // Initialize per-track storage if needed
    if (trackEdep.find(trackID) == trackEdep.end()) {
        trackEdep[trackID] = {0.0, 0.0, 0.0};
        trackParticle[trackID] = particle;
    }

    // Accumulate energy deposition in the correct scintillator layer
    trackEdep[trackID][copyNo] += edep;

    return true;
}

void ScintillatorSD::EndOfEvent(G4HCofThisEvent*)
{
    // EventAction will pull data from this SD
}

} // namespace B1
