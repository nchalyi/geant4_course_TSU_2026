#include "G4RunManagerFactory.hh"
#include "globals.hh"

#include "MyDetectorConstruction.hh"
#include "G4PhysListFactory.hh"
#include "MyActionInitialization.hh"

#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"

int main(int argc, char** argv) {

    G4String macrofile = "";
    G4UIExecutive* ui = nullptr;
    if (argc == 1) {
        ui = new G4UIExecutive (argc, argv);
    } else {
        macrofile = argv[1];
    }

    auto* runManager = G4RunManagerFactory::CreateRunManager(); // hearth of our programm

    MyDetectorConstruction* detector = new MyDetectorConstruction;
    runManager->SetUserInitialization( detector );

    G4PhysListFactory plFactory;
    G4VModularPhysicsList *pl = plFactory.GetReferencePhysList( "FTFP_BERT" );
    runManager->SetUserInitialization( pl ); // Physics

    runManager->SetUserInitialization( new MyActionInitialization (detector) );

    runManager->SetNumberOfThreads(1);

    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    if (!ui) {
        G4String command = "/control/execute ";
        UImanager->ApplyCommand (command + macrofile );
        G4cout << " **** BATCH MODE **** " << G4endl;
    } else {
        ui->SessionStart();
        G4cout << " **** UI MODE **** " << G4endl;
        delete ui;
    }

    G4cout << " === END OF SIMULATION === " << G4endl;
    delete visManager;
    delete runManager;
    return 0;
}