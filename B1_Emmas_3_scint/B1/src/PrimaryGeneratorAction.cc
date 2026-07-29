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
//
// 3-scint Verify Run
// 
#include "PrimaryGeneratorAction.hh"

#include "G4Box.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

namespace B1
{

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
    G4cout << "PrimaryGeneratorAction: Messenger constructed" << G4endl;

    fParticleGun = new G4ParticleGun(1);

    fEnergy = 6.0 * GeV;
    fGunMessenger = new G4GenericMessenger(this, "/gun/", "Primary generator control");
    fGunMessenger->DeclarePropertyWithUnit("energy", "GeV", fEnergy)
                 .SetParameterName("energy", false)
                 .SetRange("energy > 0.");

    auto particleTable = G4ParticleTable::GetParticleTable();
    auto particle = particleTable->FindParticle("proton");

    fParticleGun->SetParticleDefinition(particle);
    fParticleGun->SetParticleEnergy(fEnergy);
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    // Use Scintillator1 as geometric reference
    const G4String targetName = "Scintillator1";

    auto targetLV = G4LogicalVolumeStore::GetInstance()->GetVolume(targetName);
    if (!targetLV) {
        G4ExceptionDescription msg;
        msg << "Target logical volume '" << targetName << "' not found.\n";
        G4Exception("PrimaryGeneratorAction::GeneratePrimaries()",
                    "MyCode0003", FatalException, msg);
        return;
    }

    auto targetBox = dynamic_cast<G4Box*>(targetLV->GetSolid());
    if (!targetBox) {
        G4ExceptionDescription msg;
        msg << "Target volume '" << targetName << "' is not a G4Box.\n";
        G4Exception("PrimaryGeneratorAction::GeneratePrimaries()",
                    "MyCode0004", FatalException, msg);
        return;
    }

    // Scintillator transverse size
    G4double hx = targetBox->GetXHalfLength();
    G4double hy = targetBox->GetYHalfLength();

    // We know from DetectorConstruction:
    // Scintillator1 is at z = -2 cm (firstZ)
    G4double scint1Z = -2.0 * cm;

    // Sample (x,y) uniformly across Scintillator1 face
    G4double x0 = (2.0 * G4UniformRand() - 1.0) * hx;
    G4double y0 = (2.0 * G4UniformRand() - 1.0) * hy;

    // Place gun upstream of Scintillator1 along -z
    G4double gunZ = scint1Z - 10.0 * cm;  // 5 cm upstream

    fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, gunZ));
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));

    fParticleGun->GeneratePrimaryVertex(event);
}

} // namespace B1
