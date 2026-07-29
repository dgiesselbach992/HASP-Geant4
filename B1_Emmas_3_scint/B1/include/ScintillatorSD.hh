#ifndef ScintillatorSD_hh
#define ScintillatorSD_hh

#include "G4VSensitiveDetector.hh"
#include "globals.hh"
#include <map>
#include <array>

namespace B1 {

class ScintillatorSD : public G4VSensitiveDetector {
public:
    ScintillatorSD(const G4String& name);
    virtual ~ScintillatorSD() {}

    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;
    virtual void EndOfEvent(G4HCofThisEvent*) override;

    const std::map<G4int, std::array<G4double,3>>& GetTrackEdep() const {
        return trackEdep;
    }

    const std::map<G4int, G4String>& GetTrackParticle() const {
        return trackParticle;
    }

private:
    std::map<G4int, std::array<G4double,3>> trackEdep;
    std::map<G4int, G4String> trackParticle;
};

} // namespace B1

#endif
