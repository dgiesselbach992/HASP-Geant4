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
/// \file RunAction.hh
/// \brief Definition of the B1::RunAction class

#ifndef B1RunAction_h
#define B1RunAction_h 1

#include "G4UserRunAction.hh"

#include "G4Accumulable.hh"
#include "G4AccumulableManager.hh"
#include "globals.hh"

#include "EventAction.hh"

class G4Run;

namespace B1
{

/// Run action class
///
/// In EndOfRunAction(), it calculates the dose in the selected volume
/// from the energy deposit accumulated via stepping and event actions.
/// The computed dose is then printed on the screen.

static const int NUM_DETECTORS = 3;

class RunAction : public G4UserRunAction
{
  public:
    RunAction();
    ~RunAction() override = default;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    void AddEdep(G4double edep);

    void SetEventAction(EventAction* ea) { fEventAction = ea; }

    // --- hit accumulation for global run summary ---
    void AddHit();
    
    // Per-particle hit counters
    void AddHit(int detID);

	void AddProtonHit(int detID);
	void AddElectronHit(int detID);
	void AddGammaHit(int detID);
	void AddPionHit(int detID);
	void AddMuonHit(int detID);
	void AddKaonHit(int detID);
	void AddOtherHit(int detID);

	void AddSiPMOpticalHit(int detID);

  private:
    G4Accumulable<G4double> fEdep  = 0.;
    G4Accumulable<G4double> fEdep2 = 0.;

    std::vector<G4Accumulable<G4int>> fTotalHits;
	std::vector<G4Accumulable<G4int>> fProtonHits;
	std::vector<G4Accumulable<G4int>> fElectronHits;
	std::vector<G4Accumulable<G4int>> fGammaHits;
	std::vector<G4Accumulable<G4int>> fPionHits;
	std::vector<G4Accumulable<G4int>> fMuonHits;
	std::vector<G4Accumulable<G4int>> fKaonHits;
	std::vector<G4Accumulable<G4int>> fOtherHits;
	
	std::vector<G4Accumulable<G4int>> fSiPMOpticalHits;

    EventAction*            fEventAction = nullptr;
};
}  // namespace B1
#endif
