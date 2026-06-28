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
/// \file PrimaryGeneratorAction.cc
/// \brief Implementation of the B1::PrimaryGeneratorAction class

#include "PrimaryGeneratorAction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  G4cout << "PrimaryGeneratorAction: Messenger constructed" << G4endl;	
  
  G4int n_particle = 1; //Flux at 36 km --> 0.5-3 particles* cm^-2 * s^-1 --> roughly 500 particles per second over a 20 cm x 20 cm area
  fParticleGun = new G4ParticleGun(n_particle);
  fEnergy = 6.0 * GeV;   // default value, anything you want
  fGunMessenger = new G4GenericMessenger(this, "/gun/", "Primary generator control");
  fGunMessenger->DeclarePropertyWithUnit("energy", "GeV", fEnergy).SetParameterName("energy", false).SetRange("energy > 0.");

  // default particle kinematic
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4String particleName;
  G4ParticleDefinition* particle = particleTable->FindParticle(particleName = "proton");
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
  fParticleGun->SetParticleEnergy(fEnergy);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  // --- Get envelope dimensions (unchanged) ---
  G4double envSizeXY = 0.;
  G4double envSizeZ  = 0.;

  if (!fEnvelopeBox) {
    G4LogicalVolume* envLV =
      G4LogicalVolumeStore::GetInstance()->GetVolume("Envelope");
    if (envLV) fEnvelopeBox = dynamic_cast<G4Box*>(envLV->GetSolid());
  }

  if (fEnvelopeBox) {
    envSizeXY = fEnvelopeBox->GetXHalfLength() * 2.;
    envSizeZ  = fEnvelopeBox->GetZHalfLength() * 2.;
  } else {
    G4ExceptionDescription msg;
    msg << "Envelope volume not found. Gun placed at origin.";
    G4Exception("PrimaryGeneratorAction::GeneratePrimaries()",
                "MyCode0002", JustWarning, msg);
    fParticleGun->SetParticlePosition(G4ThreeVector());
    fParticleGun->GeneratePrimaryVertex(event);
    return;
  }

  // --- NEW: retrieve Shape 1 ("Tungsten(Target)") ---
  static G4Box* targetBox = nullptr;

  if (!targetBox) {
    G4LogicalVolume* targetLV =
      G4LogicalVolumeStore::GetInstance()->GetVolume("Tungsten(Target)");

    if (targetLV) {
      targetBox = dynamic_cast<G4Box*>(targetLV->GetSolid());
    }

    if (!targetBox) {
      G4ExceptionDescription msg;
      msg << "Could not find G4Box for Tungsten(Target).";
      G4Exception("PrimaryGeneratorAction::GeneratePrimaries()",
                  "MyCode0003", JustWarning, msg);
    }
  }

  // --- Sample position ---
  G4double x0 = 0.;
  G4double y0 = 0.;
  G4double z0 = -0.5 * envSizeZ;   // gun at front of envelope

  if (targetBox) {
    // Use tungsten target dimensions for XY sampling
    G4double sizeX = targetBox->GetXHalfLength() * 2.;
    G4double sizeY = targetBox->GetYHalfLength() * 2.;

    x0 = (G4UniformRand() - 0.5) * sizeX;
    y0 = (G4UniformRand() - 0.5) * sizeY;
  } else {
    // fallback: original envelope-based sampling
    G4double size = 0.8;
    x0 = size * envSizeXY * (G4UniformRand() - 0.5);
    y0 = size * envSizeXY * (G4UniformRand() - 0.5);
  }

  fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, z0));
  fParticleGun->GeneratePrimaryVertex(event);
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
}  // namespace B1
