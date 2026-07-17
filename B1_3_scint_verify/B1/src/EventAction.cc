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
/// \brief Implementation of the B1::EventAction class

#include "EventAction.hh"
#include "RunAction.hh"
#include "G4Event.hh"

namespace B1
{

EventAction::EventAction(RunAction* runAction)
    : fRunAction(runAction)
{
}

void EventAction::BeginOfEventAction(const G4Event*)
{
    fTrackMap.clear();
    fEdep = 0.;
    fHits = 0;
}

void EventAction::EndOfEventAction(const G4Event* event)
{
    // accumulate statistics in run action (optional)
    fRunAction->AddEdep(fEdep);

    G4int eventID = event->GetEventID();

    // Loop over all particles that touched any air detector
    for (const auto& entry : fTrackMap)
    {
        int trackID = entry.first;
        const auto& info = entry.second;

        const std::string& particleName = info.particleName;

        // Build 8 binary flags
        int tungsten_entry = info.detectors.count("tungsten_entry") ? 1 : 0;
        int tungsten_exit  = info.detectors.count("tungsten_exit")  ? 1 : 0;

        int sc1_entry = info.detectors.count("sc1_entry") ? 1 : 0;
        int sc1_exit  = info.detectors.count("sc1_exit")  ? 1 : 0;

        int sc2_entry = info.detectors.count("sc2_entry") ? 1 : 0;
        int sc2_exit  = info.detectors.count("sc2_exit")  ? 1 : 0;

        int sc3_entry = info.detectors.count("sc3_entry") ? 1 : 0;
        int sc3_exit  = info.detectors.count("sc3_exit")  ? 1 : 0;

        // Print CSV row (one per particle)
        G4cout
            << eventID << ","
            << trackID << ","
            << particleName << ","
            << tungsten_entry << ","
            << tungsten_exit << ","
            << sc1_entry << ","
            << sc1_exit << ","
            << sc2_entry << ","
            << sc2_exit << ","
            << sc3_entry << ","
            << sc3_exit
            << "\n";
    }
}

} // namespace B1
