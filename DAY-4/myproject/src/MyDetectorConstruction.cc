#include "MyDetectorConstruction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

MyDetectorConstruction::MyDetectorConstruction() : 
G4VUserDetectorConstruction() {

    G4String matName = "G4_Si";

    fTargetMaterial = G4NistManager::Instance()->FindOrBuildMaterial(matName);
    if (!fTargetMaterial) {
        G4cerr << " **** Материал не найден! ****" << G4endl;
        exit(-1);
    }

    fTargetThickness = 5.6*CLHEP::um;

    fGunXPosition = -0.25*(1.1*fTargetThickness+fTargetThickness);
}

MyDetectorConstruction::~MyDetectorConstruction() {}

G4VPhysicalVolume* MyDetectorConstruction::Construct() {
    
    G4cout << " **** MyDetectorConstruction::Construct() вызвана! ****" << G4endl;

    // I
    G4Material* materialWorld = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");

    G4Material* materialTarget = fTargetMaterial;

    // II
    
    G4double targetXSize = fTargetThickness;
    G4double targetYZSize = 1.25*targetXSize;

    G4double worldXSize = 1.1*targetXSize;
    G4double worldYZSize = 1.1*targetYZSize;

    // World
    G4Box*              worldSolid   = new G4Box("world-solid",
                                            0.5*worldXSize,
                                            0.5*worldYZSize,
                                            0.5*worldYZSize);

    G4LogicalVolume*    worldLogical = new G4LogicalVolume(worldSolid,
                                                           materialWorld,
                                                           "logic-world");
    
    G4VPhysicalVolume*  worldPhysical = new G4PVPlacement(nullptr,
                                                         G4ThreeVector(0.,0.,0.),
                                                         worldLogical,
                                                         "physical-world",
                                                         nullptr,
                                                         false,
                                                         0);
    
    // Target                                                   
    G4Box*              targetSolid   = new G4Box("target-solid",
                                            0.5*targetXSize,
                                            0.5*targetYZSize,
                                            0.5*targetYZSize);

    G4LogicalVolume*    targetLogical = new G4LogicalVolume(targetSolid,
                                                           materialTarget,
                                                           "logic-target");
    
    G4VPhysicalVolume*  targetPhysical = new G4PVPlacement(nullptr,
                                                           G4ThreeVector(0.,0.,0.),
                                                           targetLogical,
                                                           "physical-target",
                                                           worldLogical,
                                                           false,
                                                           0);
    
    
    // III
    fTargetPhysicalVolume = targetPhysical;
    return worldPhysical;                                                       
}