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
    // Retrieve the tungsten target logical volume
    G4LogicalVolume* targetLV =
        G4LogicalVolumeStore::GetInstance()->GetVolume("Tungsten(Target)");

    if (!targetLV) {
        G4Exception("PrimaryGeneratorAction::GeneratePrimaries",
                    "MyCode0003", FatalException,
                    "Logical volume 'Tungsten(Target)' not found!");
    }

    // Cast to G4Box (your tungsten target is a box)
    G4Box* box = dynamic_cast<G4Box*>(targetLV->GetSolid());
    if (!box) {
        G4Exception("PrimaryGeneratorAction::GeneratePrimaries",
                    "MyCode0004", FatalException,
                    "'Tungsten(Target)' is not a G4Box!");
    }

    // Half-lengths
    G4double hx = box->GetXHalfLength();
    G4double hy = box->GetYHalfLength();
    G4double hz = box->GetZHalfLength();

    // Sample uniformly inside the box (local == global coordinates)
    G4double x0 = (2*G4UniformRand() - 1) * hx;
    G4double y0 = (2*G4UniformRand() - 1) * hy;
    G4double z0 = (2*G4UniformRand() - 1) * hz;

    fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, z0));

    // Fire
    fParticleGun->GeneratePrimaryVertex(event);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1
