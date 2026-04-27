#include "MyDetectorMessenger.hh"

#include "MyDetectorConstruction.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4String.hh"

MyDetectorMessenger::MyDetectorMessenger(MyDetectorConstruction* det)
: G4UImessenger(),
  fMyDetector(det),
  fDirCMD(nullptr),
  fTargetThicknessCMD(nullptr),
  fTargetMaterialCMD(nullptr) {

    fDirCMD = new G4UIdirectory("/myproject/detector/");
    fDirCMD->SetGuidance("UI commands for my detector");

    fTargetThicknessCMD = new G4UIcmdWithADoubleAndUnit("/myproject/detector/setTargetThickness", this);
    fTargetThicknessCMD->SetGuidance("Sets target thickness");
    fTargetThicknessCMD->SetParameterName("TargetSizeX", false);
    fTargetThicknessCMD->SetRange("TargetSizeX>0.");
    fTargetThicknessCMD->SetUnitCategory("Length");
    fTargetThicknessCMD->AvailableForStates(G4State_PreInit, G4State_Idle);

    fTargetMaterialCMD = new G4UIcmdWithAString("/myproject/detector/setTargetMaterial", this);
    fTargetMaterialCMD->SetGuidance("Sets target material");
    fTargetMaterialCMD->SetParameterName("TargetMaterial", false);
    fTargetMaterialCMD->AvailableForStates(G4State_PreInit, G4State_Idle);
}

MyDetectorMessenger::~MyDetectorMessenger() {
    delete fDirCMD;
    delete fTargetThicknessCMD;
    delete fTargetMaterialCMD;
}

void MyDetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    if (command == fTargetThicknessCMD) {
        G4double thickness = fTargetThicknessCMD->GetNewDoubleValue(newValue);
        fMyDetector->SetTargetThickness(thickness);
    }
    if (command == fTargetMaterialCMD) {
        fMyDetector->SetTargetMaterial(newValue);
    }
}