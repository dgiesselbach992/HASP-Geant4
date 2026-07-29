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
/// \file EventAction.cc
/// \brief Implementation of the EventAction class

#include "EventAction.hh"
#include "RunAction.hh"
#include "G4Event.hh"
#include "G4SDManager.hh"
#include "ScintillatorSD.hh"


namespace B1 {

EventAction::EventAction(RunAction* runAction)
: fRunAction(runAction)
{
}

void EventAction::BeginOfEventAction(const G4Event*)
{
    // Nothing to clear here — SD handles per-event resets internally
}

void EventAction::EndOfEventAction(const G4Event* event)
{
    G4int eventID = event->GetEventID();

    // Retrieve the ScintillatorSD
    auto sdManager = G4SDManager::GetSDMpointer();
    auto scintSD = dynamic_cast<ScintillatorSD*>(sdManager->FindSensitiveDetector("ScintSD"));

    if (!scintSD) {
        G4cerr << "Error: ScintillatorSD not found!" << G4endl;
        return;
    }

    const auto& edepMap     = scintSD->GetTrackEdep();
    const auto& particleMap = scintSD->GetTrackParticle();

    // Loop over all tracks that deposited energy
    for (const auto& entry : edepMap)
    {
        G4int trackID = entry.first;
        const auto& edeps = entry.second;

        G4String particleName = "unknown";
        auto it = particleMap.find(trackID);
        if (it != particleMap.end()) {
            particleName = it->second;
        }

        // Print CSV row
        G4cout
            << eventID << ","
            << trackID << ","
            << particleName << ","
            << edeps[0] << ","
            << edeps[1] << ","
            << edeps[2]
            << "\n";
    }
}
} // namespace B1
