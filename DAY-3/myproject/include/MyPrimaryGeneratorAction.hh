
#ifndef MYPRIMARYGENERATORACTION_HH
#define MYPRIMARYGENERATORACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "MyDetectorConstruction.hh"
#include "globals.hh"
#include "G4ParticleGun.hh"

class MyDetectorConstruction;
class G4ParticleGun;
class G4Event;

class MyPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
		public:
			MyPrimaryGeneratorAction(MyDetectorConstruction* det);
			~MyPrimaryGeneratorAction() override;
			
			// Генерация начальных событий
			void GeneratePrimaries(G4Event*) override;
			
			// Кинематика источника частиц по-умолчанию
			void SetDefaultKinematics();

			// Обновление позиции источника (нужно для детектора)
			void UpdatePosition();

			const G4String& GetParticleName() { 
				return fParticleGun->GetParticleDefinition()->GetParticleName();
			}

			G4double GetParticleEnergy() {
				return fParticleGun->GetParticleEnergy();
			}
		private:
			MyDetectorConstruction* fMyDetector;
			G4ParticleGun* fParticleGun;	
};

#endif
