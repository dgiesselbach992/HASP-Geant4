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
/// \file DetectorConstruction.hh
/// \brief Definition of the B1::DetectorConstruction class

#ifndef B1DetectorConstruction_h
#define B1DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include <array>
#include <string>

class G4VPhysicalVolume;
class G4LogicalVolume;
class EventAction;

namespace B1
{

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction();
    ~DetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

    void SetEventAction(EventAction* ea) { fEventAction = ea; }

    const std::array<G4LogicalVolume*, 8>& GetAirDetectors() const { return fAirDetectors; }
    const std::array<std::string, 8>& GetAirDetectorNames() const { return fAirDetectorNames; }

    const std::array<G4LogicalVolume*, 3>& GetScintillators() const { return fScintillators; }

private:
    EventAction* fEventAction = nullptr;

    // 8 air detectors (entry/exit for tungsten + 3 scintillators)
    std::array<G4LogicalVolume*, 8> fAirDetectors;
    std::array<std::string, 8> fAirDetectorNames;

    // 3 scintillators (placeholders)
    std::array<G4LogicalVolume*, 3> fScintillators;

    // Tungsten block
    G4LogicalVolume* fTungsten = nullptr;
};

}  // namespace B1

#endif
