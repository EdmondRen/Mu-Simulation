#include "detector/DetectorMessenger.hh"

#include "Geant4/G4UIdirectory.hh"
#include "Geant4/G4UIcmdWithAString.hh"
#include "Geant4/G4UIcmdWithADoubleAndUnit.hh"

namespace MATHUSLA { namespace MU {

DetectorMessenger::DetectorMessenger(DetectorConstruction* Det)
    : G4UImessenger(),
      fDetectorConstruction(Det) {

  fDirectory = new G4UIdirectory("/MATHUSLA/MU/");
  fDirectory->SetGuidance("UI commands for MATHUSLA Muon Detector");

  fDetDirectory = new G4UIdirectory("/MATHUSLA/MU/det/");
  fDetDirectory->SetGuidance("Detector construction control");

  fTargMatCmd = new G4UIcmdWithAString("/MATHUSLA/MU/det/setTargetMaterial", this);
  fTargMatCmd->SetGuidance("Select Material of the Target.");
  fTargMatCmd->SetParameterName("choice", false);
  fTargMatCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fChamMatCmd = new G4UIcmdWithAString("/MATHUSLA/MU/det/setChamberMaterial", this);
  fChamMatCmd->SetGuidance("Select Material of the Chamber.");
  fChamMatCmd->SetParameterName("choice", false);
  fChamMatCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fStepMaxCmd = new G4UIcmdWithADoubleAndUnit("/MATHUSLA/MU/det/stepMax", this);
  fStepMaxCmd->SetGuidance("Define a step max");
  fStepMaxCmd->SetParameterName("stepMax", false);
  fStepMaxCmd->SetUnitCategory("Length");
  fStepMaxCmd->AvailableForStates(G4State_Idle);
}

DetectorMessenger::~DetectorMessenger() {
  delete fTargMatCmd;
  delete fChamMatCmd;
  delete fStepMaxCmd;
  delete fDirectory;
  delete fDetDirectory;
}

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
  if (command == fTargMatCmd) {
    fDetectorConstruction->SetTargetMaterial(newValue);
  } else if (command == fChamMatCmd) {
    fDetectorConstruction->SetChamberMaterial(newValue);
  } else if (command == fStepMaxCmd) {
    fDetectorConstruction->SetMaxStep(fStepMaxCmd->GetNewDoubleValue(newValue));
  }
}

} } /* namespace MATHUSLA::MU */
