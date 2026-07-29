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
/// \file RunAction.cc
/// \brief Implementation of the B1::RunAction class

#include "RunAction.hh"
#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

namespace B1
{

RunAction::RunAction()
{
    // Optional: define custom units (kept for compatibility)
    const G4double milligray = 1.e-3 * gray;
    const G4double microgray = 1.e-6 * gray;
    const G4double nanogray  = 1.e-9 * gray;
    const G4double picogray  = 1.e-12 * gray;

    new G4UnitDefinition("milligray", "milliGy", "Dose", milligray);
    new G4UnitDefinition("microgray", "microGy", "Dose", microgray);
    new G4UnitDefinition("nanogray",  "nanoGy",  "Dose", nanogray);
    new G4UnitDefinition("picogray",  "picoGy",  "Dose", picogray);
}

void RunAction::BeginOfRunAction(const G4Run*)
{
    // Optional: disable random number storage
    G4RunManager::GetRunManager()->SetRandomNumberStore(false);

    // Reset energy deposition
    fEdep = 0.;
    fEdep2 = 0.;

    // Print CSV header (optional)
    G4cout
        << "eventID,trackID,particle,"
        << "tungsten_entry,tungsten_exit,"
        << "sc1_entry,sc1_exit,"
        << "sc2_entry,sc2_exit,"
        << "sc3_entry,sc3_exit"
        << "\n";
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    G4int nofEvents = run->GetNumberOfEvent();

    if (IsMaster())
    {
        G4cout << "\n==================== End of Run ====================\n";
        G4cout << "Number of events: " << nofEvents << "\n";
        G4cout << "Total energy deposition: "
               << G4BestUnit(fEdep.GetValue(), "Energy") << "\n";
        G4cout << "====================================================\n\n";
    }
}

void RunAction::AddEdep(G4double edep)
{
    fEdep  += edep;
    fEdep2 += edep * edep;
}

} // namespace B1
