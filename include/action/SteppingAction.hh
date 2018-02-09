#ifndef STEPPINGACTION_H
#define STEPPINGACTION_H
#pragma once

#include "Geant4/G4UserSteppingAction.hh"

namespace MATHUSLA { namespace MU {

class SteppingAction : public G4UserSteppingAction {
 public:
  SteppingAction();
  virtual ~SteppingAction();

  virtual void UserSteppingAction(const G4Step*);

 private:
  G4SteppingManager* fpSteppingManager;
};

} } /* namespace MATHUSLA::MU */

#endif /* STEPPINGACTION_H */
