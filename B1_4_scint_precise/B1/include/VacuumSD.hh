#include "G4VSensitiveDetector.hh"
#include "globals.hh"

namespace B1 {

class VacuumSD : public G4VSensitiveDetector {
public:
    VacuumSD(const G4String& name, G4int detectorID)
        : G4VSensitiveDetector(name),
        fDetectorID(detectorID),
          fHitCount(0) {}

    virtual ~VacuumSD() {}

    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;

    G4int GetHitCount() const { return fHitCount; }
    G4int GetDetectorID() const { return fDetectorID; }

private:
    G4int fDetectorID;
    G4int fHitCount;
};

} // namespace B1

