#include "G4RunManagerFactory.hh"
#include "globals.hh"

#include "G4PhysListFactory.hh"

int main() {

    auto* runManager = G4RunManagerFactory::CreateRunManager(); // hearth of our programm

    G4PhysListFactory plFactory;
    G4VModularPhysicsList *pl = plFactory.GetReferencePhysList( "FTFP_BERT" );
    runManager->SetUserInitialization( pl ); // Physics

    G4cout << " === END OF SIMULATION === " << G4endl;
    delete runManager;
    return 0;
}