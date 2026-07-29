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
// 3-scint Verify Run
// 
/// \file DetectorConstruction.cc
/// \brief Implementation of the DetectorConstruction class

#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"

#include "ScintillatorSD.hh"

namespace B1 {

DetectorConstruction::DetectorConstruction()
{
    fScintillators.fill(nullptr);
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    auto nist = G4NistManager::Instance();

    // -------------------------
    // Dimensions
    // -------------------------
    const G4double detXY  = 3.81*cm;
    const G4double scintZ = 1.0*cm;
    const G4double gapZ   = 1.0*cm;

    // Total Z extent: 3 scintillators + 2 gaps + margin
    const G4double totalZ =
        3.0*scintZ + 2.0*gapZ + 2.0*cm; // small margin

    // -------------------------
    // World (tight around stack)
    // -------------------------
    auto world_mat   = nist->FindOrBuildMaterial("G4_Galactic");
    auto solidWorld  = new G4Box("World", 0.5*detXY + 1.0*cm,
                                 0.5*detXY + 1.0*cm,
                                 0.5*totalZ);
    auto logicWorld  = new G4LogicalVolume(solidWorld, world_mat, "World");
    auto physWorld   = new G4PVPlacement(nullptr, {}, logicWorld,
                                         "World", nullptr, false, 0);

    // -------------------------
    // Scintillator material
    // -------------------------
    auto scint_mat = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");

    auto makeScint =
        [&](const std::string& name, const G4ThreeVector& pos, int index)
    {
        auto solid = new G4Box(name, 0.5*detXY, 0.5*detXY, 0.5*scintZ);
        auto logic = new G4LogicalVolume(solid, scint_mat, name);

        // copyNo = index (0,1,2)
        new G4PVPlacement(nullptr, pos, logic, name,
                          logicWorld, false, index);

        fScintillators[index] = logic;
    };

    // -------------------------
    // Scintillator stack layout
    // -------------------------
    G4double zCenter = 0.0;
    G4double firstZ  = zCenter - (scintZ + gapZ); // Scint 1
    G4double secondZ = zCenter;                   // Scint 2
    G4double thirdZ  = zCenter + (scintZ + gapZ); // Scint 3

    makeScint("Scintillator1", G4ThreeVector(0,0,firstZ), 0);
    makeScint("Scintillator2", G4ThreeVector(0,0,secondZ), 1);
    makeScint("Scintillator3", G4ThreeVector(0,0,thirdZ), 2);

    return physWorld;
}

void DetectorConstruction::ConstructSDandField()
{
    auto sdManager = G4SDManager::GetSDMpointer();

    // One ScintillatorSD for all 3 scintillators
    auto scintSD = new ScintillatorSD("ScintSD");
    sdManager->AddNewDetector(scintSD);

    for (int i = 0; i < 3; ++i)
    {
        if (fScintillators[i])
            fScintillators[i]->SetSensitiveDetector(scintSD);
    }
}
} // namespace B1
