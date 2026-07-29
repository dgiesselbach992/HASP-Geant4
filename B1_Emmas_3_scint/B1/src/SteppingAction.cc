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
/// \file SteppingAction.cc
/// \brief Implementation of the B1::SteppingAction class

#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"

#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4LogicalVolume.hh"

namespace B1
{

SteppingAction::SteppingAction(EventAction* eventAction)
    : fEventAction(eventAction)
{
}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    // Get detector construction
    const auto detConstruction =
        static_cast<const DetectorConstruction*>(
            G4RunManager::GetRunManager()->GetUserDetectorConstruction());

    const auto& airLVs    = detConstruction->GetAirDetectors();
    const auto& airNames  = detConstruction->GetAirDetectorNames();

    // Get current logical volume
    auto prePoint = step->GetPreStepPoint();
    auto touchable = prePoint->GetTouchableHandle();
    if (!touchable || !touchable->GetVolume()) return;

    auto preLV = touchable->GetVolume()->GetLogicalVolume();

    // Check if particle is inside any air detector
    for (int i = 0; i < 8; i++)
    {
        if (preLV == airLVs[i])
        {
            auto track = step->GetTrack();
            int trackID = track->GetTrackID();
            std::string particleName = track->GetDefinition()->GetParticleName();

            // Update event map
            auto& trackMap = fEventAction->GetTrackMap();
            auto& info = trackMap[trackID];

            info.particleName = particleName;
            info.detectors.insert(airNames[i]);

            return; // done for this step
        }
    }
}

} // namespace B1
