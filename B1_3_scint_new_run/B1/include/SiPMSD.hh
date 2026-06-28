#include "G4VSensitiveDetector.hh"
#include "globals.hh"

namespace B1 {

class SiPMSD : public G4VSensitiveDetector {
public:
    SiPMSD(const G4String& name, G4int detectorID)
        : G4VSensitiveDetector(name),
          fDetectorID(detectorID),
          fHitCount(0) {}

    virtual ~SiPMSD() {}

    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;

    G4int GetHitCount() const { return fHitCount; }
    G4int GetDetectorID() const { return fDetectorID; }

private:
    G4int fDetectorID;
    G4int fHitCount;
};

} // namespace B1
