
#ifndef MYPRIMARYGENERATORACTION_HH
#define MYPRIMARYGENERATORACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "MyDetectorConstruction.hh"
#include "globals.hh"

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
		private:
			MyDetectorConstruction* fMyDetector;
			G4ParticleGun* fParticleGun;	
};

#endif
