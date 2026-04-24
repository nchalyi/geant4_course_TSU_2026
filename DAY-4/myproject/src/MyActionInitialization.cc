#include "MyDetectorConstruction.hh"
#include "MyActionInitialization.hh"
#include "MyPrimaryGeneratorAction.hh"

#include "MyRunAction.hh"
#include "MyEventAction.hh"
#include "MySteppingAction.hh"

MyActionInitialization::MyActionInitialization(MyDetectorConstruction* det)
: G4VUserActionInitialization(),
  fMyDetector(det) {}

MyActionInitialization::~MyActionInitialization() {}

void MyActionInitialization::BuildForMaster() const {
    SetUserAction( new MyRunAction(fMyDetector, nullptr));
}

void MyActionInitialization::Build() const {
    MyPrimaryGeneratorAction* primaryAction = new MyPrimaryGeneratorAction(fMyDetector);
    SetUserAction(primaryAction);

    MyRunAction* runAction = new MyRunAction(fMyDetector, primaryAction);
    SetUserAction(runAction);

    MyEventAction* eventAction = new MyEventAction();
    SetUserAction(eventAction);

    MySteppingAction* steppingAction = new MySteppingAction(fMyDetector, eventAction);
    SetUserAction(steppingAction);
}