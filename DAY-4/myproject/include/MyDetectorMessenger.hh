#ifndef MYDETECTORMESSENGER_HH
#define MYDETECTORMESSENGER_HH

#include "G4UImessenger.hh"

// forward declarations
class MyDetectorConstruction;
class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithAString;
class G4String;

class MyDetectorMessenger : public G4UImessenger {

  public:

    MyDetectorMessenger(MyDetectorConstruction* det);
    ~MyDetectorMessenger();

    // Функция для установки значения величины после вызова UI команды
    void SetNewValue(G4UIcommand*, G4String) override;

  private:

    // Объект нашего детектора
    MyDetectorConstruction*  fMyDetector; 

    // Объекты директории и команд UI
    G4UIdirectory*             fDirCMD;
    G4UIcmdWithADoubleAndUnit* fTargetThicknessCMD;
    G4UIcmdWithAString*        fTargetMaterialCMD;
};

#endif