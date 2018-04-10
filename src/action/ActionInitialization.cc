#include "action/ActionInitialization.hh"

#include "action/RunAction.hh"
#include "action/EventAction.hh"
#include "action/GeneratorAction.hh"

namespace MATHUSLA { namespace MU {

ActionInitialization::ActionInitialization(const G4String& generator)
    : G4VUserActionInitialization(), _generator(generator) {}

void ActionInitialization::BuildForMaster() const {
  SetUserAction(new RunAction);
}

void ActionInitialization::Build() const {
  SetUserAction(new RunAction);
  SetUserAction(new EventAction(100));
  SetUserAction(new GeneratorAction(_generator));
}

} } /* namespace MATHUSLA::MU */
