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
/// \file DetectorConstruction.cc
/// \brief Implementation of the B1::DetectorConstruction class

#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Trd.hh"
#include "G4UserLimits.hh"

#include "G4SDManager.hh"
#include "SiPMSD.hh"
#include "EventAction.hh"

#include "G4LogicalVolumeStore.hh"

#include "G4MaterialPropertiesTable.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4OpticalPhoton.hh"
#include "G4LogicalSkinSurface.hh"

#include "VacuumSD.hh"

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();
  
  // --- Scintillator material and optical properties ---

  G4Material* scint_mat = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");

  // Photon energy range
  const G4int nEntries = 3;
  G4double photonEnergy[nEntries] = {
    2.0 * eV,
    2.5 * eV,
    3.0 * eV
  };

  // Refractive index
  G4double rindexScint[nEntries] = {1.58, 1.58, 1.58};

  // Emission spectrum
  G4double scintFast[nEntries] = {0.1, 1.0, 0.3};

  auto scintMPT = new G4MaterialPropertiesTable();
  scintMPT->AddProperty("RINDEX", photonEnergy, rindexScint, nEntries, true);
  scintMPT->AddProperty("FASTCOMPONENT", photonEnergy, scintFast, nEntries, true);

  scintMPT->AddConstProperty("SCINTILLATIONYIELD", 10000. / MeV, true); // real value is about 10,000 per MeV
  scintMPT->AddConstProperty("RESOLUTIONSCALE", 1.0, true);
  scintMPT->AddConstProperty("FASTTIMECONSTANT", 2.0 * ns, true);
  scintMPT->AddConstProperty("YIELDRATIO", 1.0, true);

  scint_mat->SetMaterialPropertiesTable(scintMPT);

  // Envelope parameters
  //
  G4double env_sizeXY = 24 * cm, env_sizeZ = 35 * cm;
  G4Material* env_mat = nist->FindOrBuildMaterial("G4_Galactic");

  // Option to switch on/off checking of volumes overlaps
  //
  G4bool checkOverlaps = true;

  //
  // World
  //
  G4double world_sizeXY = 1.2 * env_sizeXY;
  G4double world_sizeZ = 1.2 * env_sizeZ;
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_Galactic");

  auto solidWorld =
    new G4Box("World",  // its name
              0.5 * world_sizeXY, 0.5 * world_sizeXY, 0.5 * world_sizeZ);  // its size

  auto logicWorld = new G4LogicalVolume(solidWorld,  // its solid
                                        world_mat,  // its material
                                        "World");  // its name

  auto physWorld = new G4PVPlacement(nullptr,  // no rotation
                                     G4ThreeVector(),  // at (0,0,0)
                                     logicWorld,  // its logical volume
                                     "World",  // its name
                                     nullptr,  // its mother  volume
                                     false,  // no boolean operation
                                     0,  // copy number
                                     checkOverlaps);  // overlaps checking

  //
  // Envelope
  //
  auto solidEnv = new G4Box("Envelope",  // its name
                            0.5 * env_sizeXY, 0.5 * env_sizeXY, 0.5 * env_sizeZ);  // its size
                            
  // Refractive index for world and envelope (vacuum-like)
  G4double rindexWorld[nEntries] = {1.0, 1.0, 1.0};

  auto worldMPT = new G4MaterialPropertiesTable();
  worldMPT->AddProperty("RINDEX", photonEnergy, rindexWorld, nEntries);
  world_mat->SetMaterialPropertiesTable(worldMPT);

  auto envMPT = new G4MaterialPropertiesTable();
  envMPT->AddProperty("RINDEX", photonEnergy, rindexWorld, nEntries);
  env_mat->SetMaterialPropertiesTable(envMPT);


  auto logicEnv = new G4LogicalVolume(solidEnv,  // its solid
                                      env_mat,  // its material
                                      "Envelope");  // its name

  new G4PVPlacement(nullptr,  // no rotation
                    G4ThreeVector(),  // at (0,0,0)
                    logicEnv,  // its logical volume
                    "Envelope",  // its name
                    logicWorld,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    checkOverlaps);  // overlaps checking

  //
  // Shape 1
  //
  G4String shape1_name = "Tungsten(Target)";
  G4Material* shape1_mat = nist->FindOrBuildMaterial("G4_W");
  G4ThreeVector pos1 = G4ThreeVector(4 * cm, 4 * cm, -4.75 * cm);

  // Rectangluar shape
  G4double shape1_x = 5 * cm;
  G4double shape1_y = 5 * cm;
  G4double shape1_z = 10 * cm;
  auto solidShape1 =
	new G4Box(shape1_name,  // its name
              0.5 * shape1_x, 0.5 * shape1_y,
			  0.5 * shape1_z);  // its size

  auto logicShape1 = new G4LogicalVolume(solidShape1,  // its solid
                                         shape1_mat,  // its material
                                         shape1_name);  // its name

  new G4PVPlacement(nullptr,  // no rotation
                    pos1,  // at position
                    logicShape1,  // its logical volume
                    shape1_name,  // its name
                    logicEnv,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    checkOverlaps);  // overlaps checking
  
  //
  // Shape 2
  //            
  G4String shape2_name = "Detector 1";
  G4Material* shape2_mat = nist->FindOrBuildMaterial("G4_Galactic");
  G4ThreeVector pos2 = G4ThreeVector(4 * cm, 4 * cm, 6.75 * cm);

  G4double shape2_x = 5 * cm;
  G4double shape2_y = 5 * cm;
  G4double shape2_z = 1.0 * cm;

  auto solidShape2 =
    new G4Box(shape2_name,
            0.5 * shape2_x, 0.5 * shape2_y,
            0.5 * shape2_z);

  auto logicShape2 = new G4LogicalVolume(solidShape2,
                                     shape2_mat,
                                     shape2_name);
                                     
  new G4PVPlacement(nullptr,  // no rotation
                    pos2,  // at position
                    logicShape2,  // its logical volume
                    shape2_name,  // its name
                    logicEnv,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    checkOverlaps);  // overlaps checking
          
  //
  // Shape 3
  //            
  G4String shape3_name = "Scintillator 2";
  G4Material* shape3_mat = scint_mat;   // the scintillator material you defined earlier
  G4ThreeVector pos3 = G4ThreeVector(4 * cm, 4 * cm, 8.75 * cm);

  G4double shape3_x = 5 * cm;
  G4double shape3_y = 5 * cm;
  G4double shape3_z = 1.0 * cm;   // thickness of scintillator

  auto solidShape3 =
    new G4Box(shape3_name,
            0.5 * shape3_x, 0.5 * shape3_y,
            0.5 * shape3_z);

  auto logicShape3 = new G4LogicalVolume(solidShape3,
                                     shape3_mat,
                                     shape3_name);
                                     
  // Set Shape3 as scoring volume
  fScoringVolume = logicShape3;

  G4double reflectivity[3] = {0.95, 0.95, 0.95};  // 95% reflective
  G4double efficiency[3]   = {0.0, 0.0, 0.0};     // no detection here

  auto wrapSurface = new G4OpticalSurface("ScintWrap");
  wrapSurface->SetType(dielectric_metal);
  wrapSurface->SetFinish(groundfrontpainted);  // or polishedfrontpainted
  wrapSurface->SetModel(unified);

  auto wrapMPT = new G4MaterialPropertiesTable();
  wrapMPT->AddProperty("REFLECTIVITY", photonEnergy, reflectivity, nEntries);
  wrapMPT->AddProperty("EFFICIENCY",   photonEnergy, efficiency,   nEntries);
  wrapSurface->SetMaterialPropertiesTable(wrapMPT);

  // Apply as a skin surface to the scintillator
  new G4LogicalSkinSurface("ScintWrapSurface", logicShape3, wrapSurface);

  //
  // Shape 4
  //
  G4String shape4_name = "SiPM 1";
  G4Material* shape4_mat = nist->FindOrBuildMaterial("G4_Si");
  G4ThreeVector pos4 = G4ThreeVector(4 * cm, 4 * cm, 9.5 * cm);
  
  G4double rindexSiPM[nEntries] = {3.5, 3.5, 3.5};

  auto sipmMPT = new G4MaterialPropertiesTable();
  sipmMPT->AddProperty("RINDEX", photonEnergy, rindexSiPM, nEntries);
  shape4_mat->SetMaterialPropertiesTable(sipmMPT);

  // Rectangular shape
  G4double shape4_x = 5 * mm;
  G4double shape4_y = 5 * mm;
  G4double shape4_z = 0.5 * cm;
  auto solidShape4 =
    new G4Box(shape4_name,
            0.5 * shape4_x, 0.5 * shape4_y,
            0.5 * shape4_z);

  auto logicShape4 = new G4LogicalVolume(solidShape4,
                                       shape4_mat,
                                       shape4_name);

  // Optical surface between scintillator and SiPM
  auto scintSiPMSurface = new G4OpticalSurface("ScintSiPMSurface");
  scintSiPMSurface->SetType(dielectric_dielectric);
  scintSiPMSurface->SetModel(unified);
  scintSiPMSurface->SetFinish(polished);
  
  auto physScint1 = new G4PVPlacement(nullptr,
                                   pos3,
                                   logicShape3,
                                   shape3_name,
                                   logicEnv,
                                   false,
                                   0,
                                   checkOverlaps);

  auto physSiPM1  = new G4PVPlacement(nullptr,
                                   pos4,
                                   logicShape4,
                                   shape4_name,
                                   logicEnv,
                                   false,
                                   0,
                                   checkOverlaps);

  new G4LogicalBorderSurface("ScintToSiPM1",
                           physScint1,
                           physSiPM1,
                           scintSiPMSurface);
  //
  // Shape 5
  //            
  G4String shape5_name = "Detector 2";
  G4Material* shape5_mat = nist->FindOrBuildMaterial("G4_Galactic");
  G4ThreeVector pos5 = G4ThreeVector(4 * cm, -4 * cm, 6.75 * cm);

  G4double shape5_x = 5 * cm;
  G4double shape5_y = 5 * cm;
  G4double shape5_z = 1.0 * cm;

  auto solidShape5 =
    new G4Box(shape5_name,
            0.5 * shape5_x, 0.5 * shape5_y,
            0.5 * shape5_z);

  auto logicShape5 = new G4LogicalVolume(solidShape5,
                                     shape5_mat,
                                     shape5_name);
                                     
  new G4PVPlacement(nullptr,  // no rotation
                    pos5,  // at position
                    logicShape5,  // its logical volume
                    shape5_name,  // its name
                    logicEnv,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    checkOverlaps);  // overlaps checking
          
  //
  // Shape 6
  //            
  G4String shape6_name = "Scintillator 2";
  G4Material* shape6_mat = scint_mat;   // the scintillator material you defined earlier
  G4ThreeVector pos6 = G4ThreeVector(4 * cm, -4 * cm, 8.75 * cm);

  G4double shape6_x = 5 * cm;
  G4double shape6_y = 5 * cm;
  G4double shape6_z = 1.0 * cm;   // thickness of scintillator

  auto solidShape6 =
    new G4Box(shape6_name,
            0.5 * shape6_x, 0.5 * shape6_y,
            0.5 * shape6_z);

  auto logicShape6 = new G4LogicalVolume(solidShape6,
                                     shape6_mat,
                                     shape6_name);
                                     
  // Set Shape6 as scoring volume
  fScoringVolume = logicShape6;

  // Already declared
  // G4double reflectivity[3] = {0.95, 0.95, 0.95};  // 95% reflective
  // G4double efficiency[3]   = {0.0, 0.0, 0.0};     // no detection here

  // I don't think this code is needed
  // auto wrapSurface = new G4OpticalSurface("ScintWrap");
  // wrapSurface->SetType(dielectric_metal);
  // wrapSurface->SetFinish(groundfrontpainted);  // or polishedfrontpainted
  // wrapSurface->SetModel(unified);

  // auto wrapMPT = new G4MaterialPropertiesTable();
  // wrapMPT->AddProperty("REFLECTIVITY", photonEnergy, reflectivity, nEntries);
  // wrapMPT->AddProperty("EFFICIENCY",   photonEnergy, efficiency,   nEntries);
  // wrapSurface->SetMaterialPropertiesTable(wrapMPT);

  // Apply as a skin surface to the scintillator
  new G4LogicalSkinSurface("ScintWrapSurface", logicShape6, wrapSurface);

  //
  // Shape 7
  //
  G4String shape7_name = "SiPM 2";
  G4Material* shape7_mat = nist->FindOrBuildMaterial("G4_Si");
  G4ThreeVector pos7 = G4ThreeVector(4 * cm, -4 * cm, 9.5 * cm);
  
  // Already declared
  // G4double rindexSiPM[nEntries] = {3.5, 3.5, 3.5};

  // auto sipmMPT = new G4MaterialPropertiesTable();
  // sipmMPT->AddProperty("RINDEX", photonEnergy, rindexSiPM, nEntries);
  shape7_mat->SetMaterialPropertiesTable(sipmMPT);

  // Rectangular shape
  G4double shape7_x = 5 * mm;
  G4double shape7_y = 5 * mm;
  G4double shape7_z = 0.5 * cm;
  auto solidShape7 =
    new G4Box(shape7_name,
            0.5 * shape7_x, 0.5 * shape7_y,
            0.5 * shape7_z);

  auto logicShape7 = new G4LogicalVolume(solidShape7,
                                       shape7_mat,
                                       shape7_name);

  // Also not needed twice I think
  // Optical surface between scintillator and SiPM
  // auto scintSiPMSurface = new G4OpticalSurface("ScintSiPMSurface");
  // scintSiPMSurface->SetType(dielectric_dielectric);
  // scintSiPMSurface->SetModel(unified);
  // scintSiPMSurface->SetFinish(polished);
  
  auto physScint2 = new G4PVPlacement(nullptr,
                                   pos6,
                                   logicShape6,
                                   shape6_name,
                                   logicEnv,
                                   false,
                                   0,
                                   checkOverlaps);

  auto physSiPM2  = new G4PVPlacement(nullptr,
                                   pos7,
                                   logicShape7,
                                   shape7_name,
                                   logicEnv,
                                   false,
                                   0,
                                   checkOverlaps);

  new G4LogicalBorderSurface("ScintToSiPM2",
                           physScint2,
                           physSiPM2,
                           scintSiPMSurface);
  //
  // Shape 8
  //            
  G4String shape8_name = "Detector 3";
  G4Material* shape8_mat = nist->FindOrBuildMaterial("G4_Galactic");
  G4ThreeVector pos8 = G4ThreeVector(-4 * cm, -4 * cm, 6.75 * cm);

  G4double shape8_x = 5 * cm;
  G4double shape8_y = 5 * cm;
  G4double shape8_z = 1.0 * cm;

  auto solidShape8 =
    new G4Box(shape8_name,
            0.5 * shape8_x, 0.5 * shape8_y,
            0.5 * shape8_z);

  auto logicShape8 = new G4LogicalVolume(solidShape8,
                                     shape8_mat,
                                     shape8_name);
                                     
  new G4PVPlacement(nullptr,  // no rotation
                    pos8,  // at position
                    logicShape8,  // its logical volume
                    shape8_name,  // its name
                    logicEnv,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    checkOverlaps);  // overlaps checking
          
  //
  // Shape 9
  //            
  G4String shape9_name = "Scintillator 3";
  G4Material* shape9_mat = scint_mat;   // the scintillator material you defined earlier
  G4ThreeVector pos9 = G4ThreeVector(-4 * cm, -4 * cm, 8.75 * cm);

  G4double shape9_x = 5 * cm;
  G4double shape9_y = 5 * cm;
  G4double shape9_z = 1.0 * cm;   // thickness of scintillator

  auto solidShape9 =
    new G4Box(shape9_name,
            0.5 * shape9_x, 0.5 * shape9_y,
            0.5 * shape9_z);

  auto logicShape9 = new G4LogicalVolume(solidShape9,
                                     shape9_mat,
                                     shape9_name);
                                     
  // Set Shape6 as scoring volume
  fScoringVolume = logicShape9;

  // Already declared
  // G4double reflectivity[3] = {0.95, 0.95, 0.95};  // 95% reflective
  // G4double efficiency[3]   = {0.0, 0.0, 0.0};     // no detection here

  // I don't think this code is needed
  // auto wrapSurface = new G4OpticalSurface("ScintWrap");
  // wrapSurface->SetType(dielectric_metal);
  // wrapSurface->SetFinish(groundfrontpainted);  // or polishedfrontpainted
  // wrapSurface->SetModel(unified);

  // auto wrapMPT = new G4MaterialPropertiesTable();
  // wrapMPT->AddProperty("REFLECTIVITY", photonEnergy, reflectivity, nEntries);
  // wrapMPT->AddProperty("EFFICIENCY",   photonEnergy, efficiency,   nEntries);
  // wrapSurface->SetMaterialPropertiesTable(wrapMPT);

  // Apply as a skin surface to the scintillator
  new G4LogicalSkinSurface("ScintWrapSurface", logicShape9, wrapSurface);

  //
  // Shape 10
  //
  G4String shape10_name = "SiPM 3";
  G4Material* shape10_mat = nist->FindOrBuildMaterial("G4_Si");
  G4ThreeVector pos10 = G4ThreeVector(-4 * cm, -4 * cm, 9.5 * cm);
  
  // Already declared
  // G4double rindexSiPM[nEntries] = {3.5, 3.5, 3.5};

  // auto sipmMPT = new G4MaterialPropertiesTable();
  // sipmMPT->AddProperty("RINDEX", photonEnergy, rindexSiPM, nEntries);
  shape10_mat->SetMaterialPropertiesTable(sipmMPT);

  // Rectangular shape
  G4double shape10_x = 5 * mm;
  G4double shape10_y = 5 * mm;
  G4double shape10_z = 0.5 * cm;
  auto solidShape10 =
    new G4Box(shape10_name,
            0.5 * shape10_x, 0.5 * shape10_y,
            0.5 * shape10_z);

  auto logicShape10 = new G4LogicalVolume(solidShape10,
                                       shape10_mat,
                                       shape10_name);

  // Also not needed twice I think
  // Optical surface between scintillator and SiPM
  // auto scintSiPMSurface = new G4OpticalSurface("ScintSiPMSurface");
  // scintSiPMSurface->SetType(dielectric_dielectric);
  // scintSiPMSurface->SetModel(unified);
  // scintSiPMSurface->SetFinish(polished);
  
  auto physScint3 = new G4PVPlacement(nullptr,
                                   pos9,
                                   logicShape9,
                                   shape9_name,
                                   logicEnv,
                                   false,
                                   0,
                                   checkOverlaps);

  auto physSiPM3  = new G4PVPlacement(nullptr,
                                   pos10,
                                   logicShape10,
                                   shape10_name,
                                   logicEnv,
                                   false,
                                   0,
                                   checkOverlaps);

  new G4LogicalBorderSurface("ScintToSiPM3",
                           physScint3,
                           physSiPM3,
                           scintSiPMSurface);
                     
  //
  // Shape 11
  //            
  G4String shape11_name = "Detector 4";
  G4Material* shape11_mat = nist->FindOrBuildMaterial("G4_Galactic");
  G4ThreeVector pos11 = G4ThreeVector(-4 * cm, 4 * cm, 6.75 * cm);

  G4double shape11_x = 5 * cm;
  G4double shape11_y = 5 * cm;
  G4double shape11_z = 1.0 * cm;

  auto solidShape11 =
    new G4Box(shape11_name,
            0.5 * shape11_x, 0.5 * shape11_y,
            0.5 * shape11_z);

  auto logicShape11 = new G4LogicalVolume(solidShape11,
                                     shape11_mat,
                                     shape11_name);
                                     
  new G4PVPlacement(nullptr,  // no rotation
                    pos11,  // at position
                    logicShape11,  // its logical volume
                    shape11_name,  // its name
                    logicEnv,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    checkOverlaps);  // overlaps checking
          
  //
  // Shape 12
  //            
  G4String shape12_name = "Scintillator 4";
  G4Material* shape12_mat = scint_mat;   // the scintillator material you defined earlier
  G4ThreeVector pos12 = G4ThreeVector(-4 * cm, 4 * cm, 8.75 * cm);

  G4double shape12_x = 5 * cm;
  G4double shape12_y = 5 * cm;
  G4double shape12_z = 1.0 * cm;   // thickness of scintillator

  auto solidShape12 =
    new G4Box(shape12_name,
            0.5 * shape12_x, 0.5 * shape12_y,
            0.5 * shape12_z);

  auto logicShape12 = new G4LogicalVolume(solidShape12,
                                     shape12_mat,
                                     shape12_name);
                                     
  // Set Shape6 as scoring volume
  fScoringVolume = logicShape12;

  // Already declared
  // G4double reflectivity[3] = {0.95, 0.95, 0.95};  // 95% reflective
  // G4double efficiency[3]   = {0.0, 0.0, 0.0};     // no detection here

  // I don't think this code is needed
  // auto wrapSurface = new G4OpticalSurface("ScintWrap");
  // wrapSurface->SetType(dielectric_metal);
  // wrapSurface->SetFinish(groundfrontpainted);  // or polishedfrontpainted
  // wrapSurface->SetModel(unified);

  // auto wrapMPT = new G4MaterialPropertiesTable();
  // wrapMPT->AddProperty("REFLECTIVITY", photonEnergy, reflectivity, nEntries);
  // wrapMPT->AddProperty("EFFICIENCY",   photonEnergy, efficiency,   nEntries);
  // wrapSurface->SetMaterialPropertiesTable(wrapMPT);

  // Apply as a skin surface to the scintillator
  new G4LogicalSkinSurface("ScintWrapSurface", logicShape12, wrapSurface);

  //
  // Shape 13
  //
  G4String shape13_name = "SiPM 4";
  G4Material* shape13_mat = nist->FindOrBuildMaterial("G4_Si");
  G4ThreeVector pos13 = G4ThreeVector(-4 * cm, 4 * cm, 9.5 * cm);
  
  // Already declared
  // G4double rindexSiPM[nEntries] = {3.5, 3.5, 3.5};

  // auto sipmMPT = new G4MaterialPropertiesTable();
  // sipmMPT->AddProperty("RINDEX", photonEnergy, rindexSiPM, nEntries);
  shape13_mat->SetMaterialPropertiesTable(sipmMPT);

  // Rectangular shape
  G4double shape13_x = 5 * mm;
  G4double shape13_y = 5 * mm;
  G4double shape13_z = 0.5 * cm;
  auto solidShape13 =
    new G4Box(shape13_name,
            0.5 * shape13_x, 0.5 * shape13_y,
            0.5 * shape13_z);

  auto logicShape13 = new G4LogicalVolume(solidShape13,
                                       shape13_mat,
                                       shape13_name);

  // Also not needed twice I think
  // Optical surface between scintillator and SiPM
  // auto scintSiPMSurface = new G4OpticalSurface("ScintSiPMSurface");
  // scintSiPMSurface->SetType(dielectric_dielectric);
  // scintSiPMSurface->SetModel(unified);
  // scintSiPMSurface->SetFinish(polished);
  
  auto physScint4 = new G4PVPlacement(nullptr,
                                   pos12,
                                   logicShape12,
                                   shape12_name,
                                   logicEnv,
                                   false,
                                   0,
                                   checkOverlaps);

  auto physSiPM4  = new G4PVPlacement(nullptr,
                                   pos13,
                                   logicShape13,
                                   shape13_name,
                                   logicEnv,
                                   false,
                                   0,
                                   checkOverlaps);

  new G4LogicalBorderSurface("ScintToSiPM4",
                           physScint4,
                           physSiPM4,
                           scintSiPMSurface);
  //
  // Shape 14
  //
  G4String shape14_name = "Tungsten(Target)";
  G4Material* shape14_mat = nist->FindOrBuildMaterial("G4_W");
  G4ThreeVector pos14 = G4ThreeVector(-4 * cm, -4 * cm, -4.75 * cm);

  // Rectangluar shape
  G4double shape14_x = 5 * cm;
  G4double shape14_y = 5 * cm;
  G4double shape14_z = 10 * cm;
  auto solidShape14 =
	new G4Box(shape14_name,  // its name
              0.5 * shape14_x, 0.5 * shape14_y,
			  0.5 * shape14_z);  // its size

  auto logicShape14 = new G4LogicalVolume(solidShape14,  // its solid
                                         shape14_mat,  // its material
                                         shape14_name);  // its name

  new G4PVPlacement(nullptr,  // no rotation
                    pos14,  // at position
                    logicShape14,  // its logical volume
                    shape14_name,  // its name
                    logicEnv,  // its mother  volume
                    false,  // no boolean operation
                    0,  // copy number
                    checkOverlaps);  // overlaps checking

  //
  // always return the physical World
  //
  return physWorld;
}



void DetectorConstruction::ConstructSDandField()
{
    auto sdManager = G4SDManager::GetSDMpointer();
    auto lvStore   = G4LogicalVolumeStore::GetInstance();

    // ------------------------------------------------------------
    // 1. SiPM SD 1 (optical photons only)
    // ------------------------------------------------------------
    auto sipmSD1 = new B1::SiPMSD("SiPMSD1", 0);
    sdManager->AddNewDetector(sipmSD1);

    auto logicSiPM1 = lvStore->GetVolume("SiPM 1");
    if (logicSiPM1) {
        logicSiPM1->SetSensitiveDetector(sipmSD1);
    }

    // ------------------------------------------------------------
    // 2. Vacuum SD 1 (non-optical particle types)
    // ------------------------------------------------------------
    auto vacuumSD1 = new B1::VacuumSD("VacuumSD1", 0);
    sdManager->AddNewDetector(vacuumSD1);

    auto logicVacuum1 = lvStore->GetVolume("Detector 1");
    if (logicVacuum1) {
        logicVacuum1->SetSensitiveDetector(vacuumSD1);
    }
    
    // ------------------------------------------------------------
    // 3. SiPM SD 2 (optical photons only)
    // ------------------------------------------------------------
    auto sipmSD2 = new B1::SiPMSD("SiPMSD2", 1);
    sdManager->AddNewDetector(sipmSD2);

    auto logicSiPM2 = lvStore->GetVolume("SiPM 2");
    if (logicSiPM2) {
        logicSiPM2->SetSensitiveDetector(sipmSD2);
    }

    // ------------------------------------------------------------
    // 4. Vacuum SD 2 (non-optical particle types)
    // ------------------------------------------------------------
    auto vacuumSD2 = new B1::VacuumSD("VacuumSD2", 1);
    sdManager->AddNewDetector(vacuumSD2);

    auto logicVacuum2 = lvStore->GetVolume("Detector 2");
    if (logicVacuum2) {
        logicVacuum2->SetSensitiveDetector(vacuumSD2);
    }
    
    // ------------------------------------------------------------
    // 5. SiPM SD 3 (optical photons only)
    // ------------------------------------------------------------
    auto sipmSD3 = new B1::SiPMSD("SiPMSD3", 2);
    sdManager->AddNewDetector(sipmSD3);

    auto logicSiPM3 = lvStore->GetVolume("SiPM 3");
    if (logicSiPM3) {
        logicSiPM3->SetSensitiveDetector(sipmSD3);
    }

    // ------------------------------------------------------------
    // 6. Vacuum SD 3 (non-optical particle types)
    // ------------------------------------------------------------
    auto vacuumSD3 = new B1::VacuumSD("VacuumSD3", 2);
    sdManager->AddNewDetector(vacuumSD2);

    auto logicVacuum3 = lvStore->GetVolume("Detector 3");
    if (logicVacuum3) {
        logicVacuum3->SetSensitiveDetector(vacuumSD3);
    }
    
    // ------------------------------------------------------------
    // 7. SiPM SD 4 (optical photons only)
    // ------------------------------------------------------------
    auto sipmSD4 = new B1::SiPMSD("SiPMSD4", 3);
    sdManager->AddNewDetector(sipmSD4);

    auto logicSiPM4 = lvStore->GetVolume("SiPM 4");
    if (logicSiPM4) {
        logicSiPM4->SetSensitiveDetector(sipmSD4);
    }
    
    // ------------------------------------------------------------
    // 8. Vacuum SD 4 (non-optical particle types)
    // ------------------------------------------------------------
    auto vacuumSD4 = new B1::VacuumSD("VacuumSD4", 3);
    sdManager->AddNewDetector(vacuumSD4);

    auto logicVacuum4 = lvStore->GetVolume("Detector 4");
    if (logicVacuum4) {
        logicVacuum4->SetSensitiveDetector(vacuumSD4);
    }
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1
