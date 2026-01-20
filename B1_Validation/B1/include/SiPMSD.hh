#include "G4VSensitiveDetector.hh"
#include "globals.hh"

namespace B1 {

class SiPMSD : public G4VSensitiveDetector {
public:
    SiPMSD(const G4String& name)
        : G4VSensitiveDetector(name),
          fHitCount(0) {}

    virtual ~SiPMSD() {}

    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;

    G4int GetHitCount() const { return fHitCount; }

private:
    G4int fHitCount;
};

} // namespace B1
