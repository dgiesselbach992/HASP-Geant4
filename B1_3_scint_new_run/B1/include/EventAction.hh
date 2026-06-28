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
/// \file EventAction.hh
/// \brief Definition of the B1::EventAction class

#ifndef B1EventAction_h
#define B1EventAction_h

#include <map>
#include <set>

#include "G4UserEventAction.hh"
#include "globals.hh"

class G4Event;

namespace B1
{

// Updated struct with parentID included
struct ScintInfo {
    std::string particleName;
    G4int parentID;
    std::set<std::string> detectors;
};

class RunAction;

/// Event action class
class EventAction : public G4UserEventAction
{
  public:
    EventAction(RunAction* runAction);
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event* event) override;
    void EndOfEventAction(const G4Event* event) override;

    // FIXED: use ScintInfo instead of TrackScintInfo
    std::map<G4int, ScintInfo>& GetScintMap() { return fScintMap; }

    void AddEdep(G4double edep) { fEdep += edep; }
    
    RunAction* GetRunAction() { return fRunAction; }

  private:
    RunAction* fRunAction = nullptr;

    // FIXED: use ScintInfo instead of TrackScintInfo
    std::map<G4int, ScintInfo> fScintMap;

    G4double fEdep = 0.;
    G4int fHits = 0;
};

}  // namespace B1

#endif
