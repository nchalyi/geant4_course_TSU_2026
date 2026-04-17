# Практическое занятие 3

## 4. Вычисление энергопотерь электрона в мишени

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

### 4.2. Действия на шаге (SteppingAction)

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

### 4.3. Действия на событии (Event)

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

### 4.4. Действия на всем сеансе (Run)

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

## 4.5. Наш уникальный Run класс

Нам нужно создать уникальный для нас Run класс, ведь функционала из `RunManager` нам недостаточно.

Традиционно, стратуем с `MyRun.hh`:
```cpp
#ifndef MYRUN_HH
#define MYRUN_HH

#include "G4Run.hh"
#include "Histo.hh"

class MyPrimaryGeneratorAction;
class MyDetectorConstruction;

class MyRun : public G4Run {
public:
	MyRun(MyDetectorConstruction* det, MyPrimaryGeneratorAction* prim);
	~MyRun() override;

	void Merge(const G4Run* run) override;  

	// Заполняем энергопотери в гистограмму
	void AddEnergyDeposit(G4double edep) {
		fHist->Fill(edep);
	}
	
	void EndOfRunSummary();

private:

	MyDetectorConstruction*      fDetector;
	MyPrimaryGeneratorAction*    fPrimary;

	Hist*                          fHist;

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
{
	// Создаем указатель из Histo класса
	fHist = new Hist("Hist_Edep.dat", 0.0, 10.0*CLHEP::keV, 100);
} 

MyRun::~MyRun() 
{
	delete fHist; // Не забудем очистить указатель!
}

// Метод для "склеивания" статистики, собранную разными рабочими потоками, 
// в один общий результат.
void MyRun::Merge(const G4Run* run) {
	// Поскольку наш Run немного другой, нужно опять посторить static_cast
	const MyRun* localRun = static_cast<const MyRun*>(run);

	// Добавляем гистограммы
	fHist->Add(localRun->fHist);

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

	// Записывам результаты в файл
	fHist->WriteToFile(true);
}
```

### 4.6. Класс для гистограммировния

Обычно в Geant4 для гистограммирования и записи в файлы используются встроенные классы для работы с пакетом ROOT CERN, который часто используется в анализе данных физики высоких энергий.

Мы не будем реализовывать такой подход, а вместо этого воспользуемся простым `Histo.cc` классом, который будет заполнять наши результаты в гистограмму и выдавать файл с ними.
Это простой класс, использующий только простейшие Geant4 функции.

Вот так будет выглядеть include файл `Histo.hh`:
```cpp
#ifndef HIST_HH
#define HIST_HH

#include "globals.hh"

#include <vector>

class Hist {

public:
  // Конструктор: мин/макс значение гистограммы и число бинов
  Hist(const G4String& filename, G4double min, G4double max, G4int numbin);
  // Конструктор (альтернативный): мин/макс значение гистограммы и ширина бина
  Hist(const G4String& filename, G4double min, G4double max, G4double delta);

  ~Hist() {}

  void Initialize();

  void ReSet(const G4String& filename, G4double min, G4double max, G4int numbins) {
    fFileName = filename;
    fMin      = min;
    fMax      = max;
    fNumBins  = numbins;
    fDelta    = (fMax - fMin) / fNumBins;
    fInvDelta = 1./fDelta;
    Initialize();
  }

  // Заполнить гистограмму величиной x
  void Fill(G4double x);

  // Заполнить гистограмму величиной x с весом w
  void Fill(G4double x, G4double w);

  G4int     GetNumBins() const { return fNumBins; }
  G4double  GetDelta()   const { return fDelta;   }
  G4double  GetMin()     const { return fMin;     }
  G4double  GetMax()     const { return fMax;     }
  G4double  GetSum()     const { return fSum;     }
  const std::vector<G4double>& GetX() const { return fx;       }
  const std::vector<G4double>& GetY() const { return fy;       }

  // Запись в файл
  void WriteToFile(G4bool isNorm=false);
  void WriteToFile(G4double norm);

  void Add(const Hist* hist);


// Данные
private:
  G4String              fFileName;
  std::vector<G4double> fx;
  std::vector<G4double> fy;
  G4double              fMin;
  G4double              fMax;
  G4double              fDelta;
  G4double              fInvDelta;
  G4double              fSum;
  G4int                 fNumBins;
};

#endif
```

Реализация `Histo.cc`:
```cpp
#include "Hist.hh"
#include  <cstdio>

Hist::Hist(const G4String& fname, G4double min, G4double max, G4int numbin) 
: fFileName(fname), 
  fMin(min), 
  fMax(max), 
  fDelta(0.), 
  fInvDelta(1.), 
  fSum(0.),
  fNumBins(numbin) {
  fDelta    = (fMax - fMin) / fNumBins;
  fInvDelta = 1./fDelta;
  Initialize();
}

Hist::Hist(const G4String& fname, G4double min, G4double max, G4double delta)
: fFileName(fname), 
  fMin(min), 
  fMax(max), 
  fDelta(delta), 
  fInvDelta(1.), 
  fSum(0.),
  fNumBins(0) {
  fInvDelta = 1./fDelta;  
  fNumBins = (G4int)((fMax - fMin) / (fDelta)) + 1.0;
  Initialize();
}


void Hist::Initialize() {
  fx.resize(fNumBins, 0.0);
  fy.resize(fNumBins, 0.0);
  for (G4int i = 0; i < fNumBins; ++i) {
    fx[i] = fMin + i * fDelta;
  }
  fSum = 0.0;  
}

void Hist::Fill(G4double x) {
  if (x<fMin) return;
  G4int indx = (x==fMax) ? fNumBins-1 : (G4int)((x - fMin) * fInvDelta);
   
  if (indx>-1 && indx<fNumBins) {  
    fy[indx] += 1.0;
    fSum     += 1.0;
  }
}


void Hist::Fill(G4double x, G4double w) {
  if (x<fMin) return;
  G4int indx = (x==fMax) ? fNumBins-1 : (G4int)((x - fMin) * fInvDelta);

  if (indx>-1 && indx<fNumBins) {  
    fy[indx] += w;
    fSum     += w;
  }
}

void Hist::WriteToFile(G4bool isNorm) {
  FILE* f = fopen(fFileName, "w");
  if (!f) {
    G4cerr << "\n ***** ERROR in Hist::WriteToFile  "
           << " cannot create the file = " << fFileName
           << G4endl; 
    exit(1);          
  }
  G4double norm = 1.0;
  if (isNorm) {
    norm  = 1. / (fSum*fDelta);
  }
  for (G4int i = 0; i < fNumBins; ++i) {
    fprintf(f, "%d\t%.8g\t%.8g\n", i, fx[i] + 0.5 * fDelta, fy[i] * norm);
  }
  fclose(f);
}


void Hist::WriteToFile(G4double norm) {
  FILE* f = fopen(fFileName, "w");
  if (!f) {
    G4cerr << "\n ***** ERROR in Hist::WriteToFile  "
           << " cannot create the file = " << fFileName 
           << G4endl; 
    exit(1);          
  }
  for (G4int i = 0; i < fNumBins; ++i) {
    fprintf(f, "%d\t%.8g\t%.8g\n", i, fx[i] + 0.5 * fDelta, fy[i] * norm);
  }
  fclose(f);
}

void Hist::Add(const Hist* hist) {
  if (fNumBins != hist->GetNumBins() || 
      fMin     != hist->GetMin() || 
      fMax     != hist->GetMax()) {
    G4cerr << "\n ***** ERROR in Hist::Add  "
           << " histograms have different dimensions ! "
           << G4endl;     
  }
  for (G4int i = 0; i < fNumBins; ++i) {
    fy[i] += hist->GetY()[i];
  } 
  fSum += hist->GetSum();
}
```

Теперь можем собрать наше приложение по стандартной схеме и посмотреть на выходной файл `Hist_edep.dat`!

В следующий раз мы сравним наши результаты с экспериментом, а также добавим доп. функционал в наше приложение.