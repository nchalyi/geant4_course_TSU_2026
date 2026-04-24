#ifndef MYSTEPPINGACTION_HH
#define MYSTEPPINGACTION_HH

#include "G4UserSteppingAction.hh"

class MyDetectorConstruction;
class MyEventAction;

// Наследуемся от стандартного Geant4 интерфейса
class MySteppingAction : public G4UserSteppingAction {

public:
 
	MySteppingAction(MyDetectorConstruction* det,
                     MyEventAction*  eventact);
	~MySteppingAction() override;


	void UserSteppingAction(const G4Step* step) override;

private:

	MyDetectorConstruction* fDetector;
	MyEventAction*          fEventAction;

};

#endif 