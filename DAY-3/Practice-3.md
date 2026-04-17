# Практическое занятие 3

## 4. Вычисление физических величин

Целью этой главы станет реализация вычисление величины потери энергии частицей (электрона)
в нашей мишени (кремний).

Для этого нам нужно построить Run, Step и EventAction классы и внести в них вычисления энергопотери. Фактически, мы будем вычислять её на Step, затем собирать по всем шагам в Event, а в конце собирать за все события в Run.

### 4.0. Структура нашего проекта

```text
myproject/──────────────────────────────────────────────────────────────────────
├── CMakeLists.txt          # 1. Файл для сборки проекта (CMake)
├── myproject.cc            # 2. Главный файл с функцией main()
├── run.mac, vis.mac        # 3. Макросы  
│
├── include/                # 4. Заголовочные файлы .hh (объявление)
│   ├── DetectorConstruction.hh     # Геометрия
│   ├── ActionInitialization.hh     # Инициализация действий
│   ├── PrimaryGeneratorAction.hh   # Генерация начальных частиц
│   ├── RunAction.hh                # Действия в начале и конце всего сеанса
│   ├── EventAction.hh              # Действия в начале и конце одного события
│   └── SteppingAction.hh           # Действия на каждом шаге трека
│
├── src/                    # 5. Исходные файлы .cc (реализация)
│   ├── DetectorConstruction.cc
│   ├── ActionInitialization.cc
│   ├── PrimaryGeneratorAction.cc
│   ├── RunAction.cc
│   ├── EventAction.cc
│   └── SteppingAction.cc
├── build/                  # 6. Файлы сборки
└───────────────────────────────────────────────────────────────────────────────
```

### 4.1. Инициализируем новые действия

Добавим несколько вещей в наш класс `MyActionInitialization`. Нам нужно объявить в нём действия на шаге, событии и всём прогоне (run-е) нашего приложения.

Файл include `MyActionInitialization.hh` нам менять не нужно, поэтому все обновлния будут в реализации `MyActionInitialization.cc`:

```cpp 
#include "MyDetectorConstruction.hh"
#include "MyActionInitialization.hh"
#include "MyPrimaryGeneratorAction.hh"

// Добавим новые классы, мы их реализуем далее
#include "MyEventAction.hh"
#include "MySteppingAction.hh"
#include "MyRunAction.hh"

MyActionInitialization::MyActionInitialization(MyDetectorConstruction* det)
:	G4VUserActionInitialization(),
	fMyDetector(det) { }
	
MyActionInitialization::~MyActionInitialization() {	}

// В прошлый раз вся сборка была Build(), 
// но теперь добавим действие для мастера, 
// реализация которого будет в MyRunAction
void MyActionInitialization::BuildForMaster() const {			
	SetUserAction( new MyRunAction(fMyDetector, nullptr));		
}		

void MyActionInitialization::Build() const {
	MyPrimaryGeneratorAction* primaryAction = new MyPrimaryGeneratorAction(fMyDetector);
	SetUserAction(primaryAction);

	// Объявим по очереди Run, Event и Stepping действия
	MyRunAction* runAction = new MyRunAction(fMyDetector, primaryAction);
	SetUserAction(runAction);   

	MyEventAction* eventAction = new MyEventAction();
	SetUserAction(eventAction);

	MySteppingAction* steppingAction = new MySteppingAction(fMyDetector, eventAction);
	SetUserAction(steppingAction);
}
```

### 4.3. Действия на шаге (SteppingAction)

Объект Step в Geant4 содержит информацию о том как изменилась энергия в начале и конце шага, что нам и нужно для вычисления энергопотерь.

Начнём с include файла `MySteppingAction.hh`:
```cpp
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

```

Теперь `MySteppingAction.сс`:

```cpp
#include "MySteppingAction.hh"

#include "MyDetectorConstruction.hh"
#include "MyEventAction.hh"

#include "G4Step.hh"

MySteppingAction::MySteppingAction(MyDetectorConstruction* det, MyEventAction* evta)
: G4UserSteppingAction(),
  fDetector(det),
  fEventAction(evta) {}

MySteppingAction::~MySteppingAction() {}

void MySteppingAction::UserSteppingAction(const G4Step* step) {

	// Опредеяем, действиельно ли наш шаг лежит в объеме мишени
	if( step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()  
		!= fDetector->GetTargetPhysicalVolume() ) return;
	
	// Вычисляем энергопотери
	G4double edep = step->GetTotalEnergyDeposit();
	// Если потери не нулевые, передаем величину в Event
	if (edep>0.0) {
		fEventAction->AddEnergyDeposit(edep);
	}

}

```

### 4.4. Действия на событии (Event)

Реализуем `EventAction` класс.

Объявления `MyEventAction.hh`:
```cpp
#ifndef MYEVENTACATION_HH
#define MYEVENTACATION_HH

#include "G4UserEventAction.hh"

#include "globals.hh"

class MyEventAction : public G4UserEventAction {
public:

	MyEventAction();
	~MyEventAction() override;

	void BeginOfEventAction(const G4Event* evt) override;
	void EndOfEventAction(const G4Event* evt) override;

	// Реализуем небольшую функцию для последовательного суммирования edep из Step
	void AddEnergyDeposit(G4double edep) {
		fEdepPerEvent += edep;
  	}

private:

	G4double fEdepPerEvent;

};

#endif
```

Реализуем `MyEventAction.cc`:
```cpp
#include "MyEventAction.hh"

#include "G4RunManager.hh"
#include "MyRun.hh"

MyEventAction::MyEventAction() 
: G4UserEventAction(),
  fEdepPerEvent(0.0)
{}

MyEventAction::~MyEventAction()
{} 

// В начале нашего события энергопотери нужно обращать в ноль
void MyEventAction::BeginOfEventAction(const G4Event*) {
	fEdepPerEvent = 0.0;
}

// А в конце события передать всё в Run
void MyEventAction::EndOfEventAction(const G4Event*) {
	// Мы создадим свой класс MyRun с уникальными для нас методами, 
	// поэтому нам нужно привести стандартный класс Run из RunManager к нашему типу MyRun
	MyRun* run = static_cast<MyRun*>(G4RunManager::GetRunManager()->GetNonConstCurrentRun());
	run->AddEnergyDeposit(fEdepPerEvent); // Уникальный метод. В RunManager его нет!
}
```

### 4.5. Действия на всем сеансе (Run)

Реализуем `RunAction` класс.

Объявления `MyRunAction.hh`:
```cpp
#ifndef MYRUNACTION_HH
#define MYRUNACTION_HH


#include "G4UserRunAction.hh"

class MyRun;
class MyPrimaryGeneratorAction;
class MyDetectorConstruction;


class MyRunAction : public G4UserRunAction {
public:

	MyRunAction(MyDetectorConstruction* det, MyPrimaryGeneratorAction* prim);
	~MyRunAction() override;
	
	G4Run* GenerateRun() override;

	void BeginOfRunAction(const G4Run*) override;
	void EndOfRunAction(const G4Run*) override;


private:

	MyDetectorConstruction*      fDetector;
	MyPrimaryGeneratorAction*    fPrimary;

	MyRun*  fRun;
  
};

#endif 
```

Реализуем `MyEventAction.cc`:
```cpp

#include "MyRunAction.hh"

#include "MyRun.hh"
#include "MyPrimaryGeneratorAction.hh"
#include "MyDetectorConstruction.hh"

#include "G4Run.hh"

MyRunAction::MyRunAction(MyDetectorConstruction* det, MyPrimaryGeneratorAction* prim)
: G4UserRunAction(),
  fDetector(det),
  fPrimary(prim),
  fRun(nullptr)
{}


MyRunAction::~MyRunAction()
{}

// Создаем указатель из класса MyRun, который реализуем далее
G4Run* MyRunAction::GenerateRun()  {
	fRun = new MyRun(fDetector, fPrimary);
	return fRun;
}

// В начале сеанса, обновим позицию источника частиц
void MyRunAction::BeginOfRunAction(const G4Run*) {
	if (fPrimary != nullptr) {
		fPrimary->UpdatePosition();
	}
}

// В конце сеанса, выведем итоги рана в логи
void MyRunAction::EndOfRunAction(const G4Run*) {
	if (IsMaster()) {
		fRun->EndOfRunSummary();
	}
}

```

## 4.6. Наш уникальный Run класс

Нам нужно создать уникальный для нас Run класс, ведь функционала из `RunManager` нам недостаточно.

Традиционно, стратуем с `MyRun.hh`:
```cpp
#ifndef MYRUN_HH
#define MYRUN_HH

#include "G4Run.hh"

class MyPrimaryGeneratorAction;
class MyDetectorConstruction;

class MyRun : public G4Run {
public:
	MyRun(MyDetectorConstruction* det, MyPrimaryGeneratorAction* prim);
	~MyRun() override;

	void Merge(const G4Run* run) override;  

	void AddEnergyDeposit(G4double edep);
	
	void EndOfRunSummary();

private:

	MyDetectorConstruction*      fDetector;
	MyPrimaryGeneratorAction*    fPrimary;

};

#endif
```

Реализуем `MyRun.cc`:
```cpp
#include "MyRun.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "MyPrimaryGeneratorAction.hh"
#include "MyDetectorConstruction.hh"

MyRun::MyRun(MyDetectorConstruction* det, MyPrimaryGeneratorAction* prim) 
: G4Run(), 
  fDetector(det),
  fPrimary(prim) 
{} 

MyRun::~MyRun() 
{}

// Метод для "склеивания" статистики, собранную разными рабочими потоками, 
// в один общий результат.
void MyRun::Merge(const G4Run* run) {
	// Поскольку наш Run немного другой, нужно опять посторить static_cast
	const MyRun* localRun = static_cast<const MyRun*>(run);

	fPrimary = localRun->fPrimary;
	// В конце вызываем метод Merge базового класса, чтобы объединить члены базового класса
	G4Run::Merge(run);
}

// Вывод информации в конце сеанса
void MyRun::EndOfRunSummary() {
	G4int nEvents  = GetNumberOfEvent(); // Получим число событий
	if (nEvents==0) return; // Если событий нет, пропускаем

	G4cout << " #Events = " << nEvents << G4endl;
	G4cout << " Particle type: " << fPrimary->GetParticleName() 
			<< " Energy = " <<  fPrimary->GetParticleEnergy() 
			<< G4endl;
	G4cout << " Target thickness = " << fDetector->GetTargetThickness() << G4endl;
}
```

На следующем занятии мы добавим в Run заполнение гистограмм и сравним наше моделирование с экспериментом!
