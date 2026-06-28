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
// Precise 3-Scint Test
// 
/// \file EventAction.cc
/// \brief Implementation of the B1::EventAction class

#include "EventAction.hh"
#include "RunAction.hh"
#include "G4Event.hh"

namespace B1
{

EventAction::EventAction(RunAction* runAction) : fRunAction(runAction) {}

void EventAction::BeginOfEventAction(const G4Event*)
{
  fScintMap.clear();
  fEdep = 0.;
  fHits = 0;
}

void EventAction::EndOfEventAction(const G4Event* event)
{
    // accumulate statistics in run action
    fRunAction->AddEdep(fEdep);

    // ------------------------------------------------------------
    // Human-readable debug output
    // ------------------------------------------------------------
    /* if (!fScintMap.empty()) {
        G4cout << "SCINTILLATION REPORT:\n";

        for (const auto& entry : fScintMap) {
            G4int trackID = entry.first;
            const auto& info = entry.second;

            G4cout << "track " << trackID << ": "
                   << info.particleName << " -> ";

            bool first = true;
            for (const auto& det : info.detectors) {
                if (!first) G4cout << ",";
                G4cout << det;
                first = false;
            }
            G4cout << "\n";
        }
    } else {
        G4cout << "SCINTILLATION REPORT:\n";
        G4cout << "none\n";
    } */
    // ------------------------------------------------------------
    // Machine-readable CSV output
    // ------------------------------------------------------------
    G4int eventID = event->GetEventID();

    int d1 = 0, d2 = 0, d3 = 0;

    for (const auto& entry : fScintMap) {
        const auto& info = entry.second;

        if (info.detectors.count("Scintillator 1")) d1 = 1;
        if (info.detectors.count("Scintillator 2")) d2 = 1;
        if (info.detectors.count("Scintillator 3")) d3 = 1;
    }

    // Default: no scintillation
    int trackID = 0;
    G4String particleType = "null";

    // If scintillation occurred, classify the particle
    if (!fScintMap.empty()) {
        const auto& first = *fScintMap.begin();
        trackID = first.first;

        const auto& info = first.second;

        // Proton classification
        if (info.particleName == "proton") {
            if (info.parentID == 0) {
                particleType = "primary_proton";
            } else {
                particleType = "secondary_proton";
            }
        } else {
            // Non-proton particles keep their actual name
            particleType = info.particleName;
        }
    }

    // Print CSV row
    G4cerr << eventID << ","
           << trackID << ","
           << particleType << ","
           << d1 << ","
           << d2 << ","
           << d3 << "\n";
}

}  // namespace B1
