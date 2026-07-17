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
/// \brief Implementation of the B1::DetectorConstruction class

#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"

#include "VacuumSD.hh"
#include "EventAction.hh"

namespace B1
{

DetectorConstruction::DetectorConstruction()
{
    // Names for the 8 air detectors
    fAirDetectorNames = {
        "tungsten_entry",
        "tungsten_exit",
        "sc1_entry",
        "sc1_exit",
        "sc2_entry",
        "sc2_exit",
        "sc3_entry",
        "sc3_exit"
    };

    fAirDetectors.fill(nullptr);
    fScintillators.fill(nullptr);
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    auto nist = G4NistManager::Instance();

    // -------------------------
    // World
    // -------------------------
    auto world_mat = nist->FindOrBuildMaterial("G4_Galactic");
    auto solidWorld = new G4Box("World", 50*cm, 50*cm, 50*cm);
    auto logicWorld = new G4LogicalVolume(solidWorld, world_mat, "World");
    auto physWorld  = new G4PVPlacement(nullptr, {}, logicWorld, "World", nullptr, false, 0);

    // -------------------------
    // Dimensions
    // -------------------------
    const G4double detXY = 3.81*cm;       // width/height for scintillators & air detectors
    const G4double tungstenXY = 3.81*cm;
    const G4double tungstenZ = 15.24*cm;

    const G4double scintZ = 1.0*cm;
    const G4double airZ = 1.0*mm;

    const G4double gapZ = 1.0*cm;

    // -------------------------
    // Tungsten block
    // -------------------------
    auto tungsten_mat = nist->FindOrBuildMaterial("G4_AIR");
    auto solidTungsten = new G4Box("Tungsten", 0.5*tungstenXY, 0.5*tungstenXY, 0.5*tungstenZ);
    fTungsten = new G4LogicalVolume(solidTungsten, tungsten_mat, "Tungsten");

    G4double z0 = -20*cm;  // starting reference point
    new G4PVPlacement(nullptr, G4ThreeVector(0,0,z0), fTungsten, "Tungsten", logicWorld, false, 0);

    // -------------------------
    // Helper to create air detectors
    // -------------------------
    auto makeAirDetector =
        [&](const std::string& name, const G4ThreeVector& pos, int index)
    {
        auto air_mat = nist->FindOrBuildMaterial("G4_AIR");
        auto solid = new G4Box(name, 0.5*detXY, 0.5*detXY, 0.5*airZ);
        auto logic = new G4LogicalVolume(solid, air_mat, name);

        new G4PVPlacement(nullptr, pos, logic, name, logicWorld, false, 0);
        fAirDetectors[index] = logic;
    };

    // -------------------------
    // Tungsten entry/exit detectors
    // -------------------------
    makeAirDetector("tungsten_entry",
                    G4ThreeVector(0,0,z0 - 0.5*tungstenZ - 0.5*airZ),
                    0);

    makeAirDetector("tungsten_exit",
                    G4ThreeVector(0,0,z0 + 0.5*tungstenZ + 0.5*airZ),
                    1);

    // -------------------------
    // Scintillators + air detectors
    // -------------------------
    auto scint_mat = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");

    auto makeScint =
        [&](const std::string& name, const G4ThreeVector& pos, int index)
    {
        auto solid = new G4Box(name, 0.5*detXY, 0.5*detXY, 0.5*scintZ);
        auto logic = new G4LogicalVolume(solid, scint_mat, name);

        new G4PVPlacement(nullptr, pos, logic, name, logicWorld, false, 0);
        fScintillators[index] = logic;
    };

    // -------------------------
    // Scintillator stack layout
    // -------------------------
    G4double z = z0 + 0.5*tungstenZ + airZ + gapZ;

    // Scintillator 1
    makeAirDetector("sc1_entry", G4ThreeVector(0,0,z - 0.5*scintZ - 0.5*airZ), 2);
    makeScint("Scintillator1",   G4ThreeVector(0,0,z), 0);
    makeAirDetector("sc1_exit",  G4ThreeVector(0,0,z + 0.5*scintZ + 0.5*airZ), 3);

    z += scintZ + airZ + gapZ;

    // Scintillator 2
    makeAirDetector("sc2_entry", G4ThreeVector(0,0,z - 0.5*scintZ - 0.5*airZ), 4);
    makeScint("Scintillator2",   G4ThreeVector(0,0,z), 1);
    makeAirDetector("sc2_exit",  G4ThreeVector(0,0,z + 0.5*scintZ + 0.5*airZ), 5);

    z += scintZ + airZ + gapZ;

    // Scintillator 3
    makeAirDetector("sc3_entry", G4ThreeVector(0,0,z - 0.5*scintZ - 0.5*airZ), 6);
    makeScint("Scintillator3",   G4ThreeVector(0,0,z), 2);
    makeAirDetector("sc3_exit",  G4ThreeVector(0,0,z + 0.5*scintZ + 0.5*airZ), 7);

    return physWorld;
}

void DetectorConstruction::ConstructSDandField()
{
    auto sdManager = G4SDManager::GetSDMpointer();

    // All 8 air detectors use VacuumSD
    for (int i = 0; i < 8; i++)
    {
        auto sd = new B1::VacuumSD("AirSD_" + std::to_string(i), i);
        sdManager->AddNewDetector(sd);

        if (fAirDetectors[i])
            fAirDetectors[i]->SetSensitiveDetector(sd);
    }
}

} // namespace B1
