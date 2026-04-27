#ifndef MYDETECTORCONSTRUCTION_HH
#define MYDETECTORCONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"

class MyDetectorMessenger;

class G4Material;

class MyDetectorConstruction : public G4VUserDetectorConstruction {

    public:

        MyDetectorConstruction();
        ~MyDetectorConstruction() override;

        void SetTargetThickness(const G4double thickness) {
            fTargetThickness = thickness;
        }
        G4double GetTargetThickness() const {
            return fTargetThickness;
        }

        void SetTargetMaterial(const G4String& matName);
        const G4Material* GetTargetMaterial() {
            return fTargetMaterial;
        }

        const G4VPhysicalVolume* GetTargetPhysicalVolume() const {
            return fTargetPhysicalVolume;
        }

        G4VPhysicalVolume* Construct() override;

        G4double GetGunXPostinion() { return fGunXPosition; }

    private:

        G4Material* fTargetMaterial;

        G4double fTargetThickness;

        G4double fGunXPosition;

        G4VPhysicalVolume* fTargetPhysicalVolume;

        MyDetectorMessenger* fDetMessenger;
};

#endif
