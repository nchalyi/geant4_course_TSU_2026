# Практическое занятие 2

## 2. Создание приложения с нуля

### 2.0. Структура нашего проекта

1) Создадим папку для нашего проекта (например, myproject)
2) Внутри создадим файл `CMakeList.txt` с содержимым показаным выше
3) Создадим файл myproject.cc - основной управляющий нашим проектом код
4) Создадим папки `srс`, `include` и `build`
5) Так должна выглядеть получившаяся структура:
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
*Комментарий*: Проекты на Geant4 можно собирать любым удобным вам образом, в этом курсе мы следуем стандартной архитектуре, используемой в исходном коде и примерах Geant4.


### 2.1. Геометрия

Этот класс является обязательным для запуска полноценного Geant4 приложения. В нём описывается геометрия детектора/мишени.

Мы создадим брусок/коробку из заданного материала, помещённый в некоторый мировой объем.
`MyDetectorConstruction.hh`:
```cpp
// Это - include guard. Он необходим в .hh файлах, чтобы не было переобозначений при вызове этого файла из разных мест.
#ifndef MYDETECTORCONTRUCTION_HH 
#define MyDETECTORCONTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"

// Объявим этот класс заранее. Он является частью пакета Geant4, но в этом файле он нам нужен чтобы заранее определить объект типа G4Material.
class G4Material;

class MyDetectorConstruction : public G4VUserDetectorConstruction {

  // Объявление методов класса:
  public:

    // Конструктор и деструктор
    MyDetectorConstruction();
    ~MyDetectorConstruction() override;

    // Базовый класс содержит только один Construct() метод который вызывается G4RunManager в методе Initialize().
    // Метод Construct() должен возвращать G4VPhysicalVolume указатель на мировой объем.
    // Всё описание геометрии должно проходить именно в этом методе.
    G4VPhysicalVolume* Construct() override;

    // Реализацию другие методов, например для Sensitive Detector (SD), можно посмотреть в классе из Geant4 библиотеки G4VUserDetectorConstruction.
    
	// Метод GetGunXPosition() позволит нам возвращать правильную позицию источника частиц в зависимости от размеров детектора.
    G4double GetGunXPosition() { return fGunXPosition; }


  // Объявление данных
  private:

    // Материал мишени
    G4Material*            fTargetMaterial;

    // Толщина мишени
    G4double               fTargetThickness;
    
    // Позиция источника частиц
    G4double               fGunXPosition;

};

#endif   
```


Теперь перейдем к реализации описанных в `.hh` файле методов в `MyDetectorConstruction.cc`:
```cpp

#include "MyDetectorConstruction.hh"

// Для описания геометрии
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

// Для описания материалов
#include "G4Material.hh"
#include "G4NistManager.hh"

// Для необходимых физических констанкт и систем единиц
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"


// Конструктор
MyDetectorConstruction::MyDetectorConstruction()
:   G4VUserDetectorConstruction() {
    // Устанавливаем материал мишени по умолчанию
    G4String matName = "G4_Si";
	// Этот метод ищет наш материал в БД
    fTargetMaterial = G4NistManager::Instance()->FindOrBuildMaterial(matName);
    if (!fTargetMaterial) {
      G4cerr << " **** Ошибка в MyDetectorConstruction::MyDetectorConstruction(): Материал не найден! ****" << G4endl;
      exit(-1);      
    }
    // Толщина мишени по-умолчанию
    fTargetThickness = 1.0*CLHEP::cm; 
    // Установим позицию источника по умолчанию. Наш "мир" будет на 10% больше мишени, поэтому ставим gun немного сдвинуто в мире
    fGunXPosition    = -0.25*( 1.1*fTargetThickness + fTargetThickness );
}

// Деструктор
MyDetectorConstruction::~MyDetectorConstruction() {}

G4VPhysicalVolume* MyDetectorConstruction::Construct() {
    G4cout << " **** MyDetectorConstruction::Construct() вызван **** " << G4endl;
	// Шаг I. Материалы:
    // 1. Материал для мира: почти вакуум, водород низкой плотности
	G4Material* materialWorld = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
    // 2. Материал мишени: указатель хранится в fTargetMaterial 
    G4Material* materialTarget = fTargetMaterial;
    
    // Шаг II. Геометрия:
    // 1. Размеры мишени и мира
    G4double targetXSize  = fTargetThickness;
    G4double targetYZSize = 1.25*targetXSize;
	// Мир на 10% больше
    G4double worldXSize   = 1.1*targetXSize;
    G4double worldYZSize  = 1.1*targetYZSize;
    
    // 2. Размеры мишени и мира (оба это G4Box):

	// Геометрия в Geant4 делится на три части, вложенные друг в друга:
	// геометрическая, логическая и физическая.
	// Геометрическая задаёт размеры.
	// Логическая - материал, пересечения и пр. 
	// Физическая - положение, поворот, наследование от других объемов и пр.

    // a. Мир
    G4Box*              worldSolid   = new G4Box("solid-World",  // имя
    	                                       0.5*worldXSize,   // половина x-размера
    	                                       0.5*worldYZSize,  // половина y-размера 
    	                                       0.5*worldYZSize); // половина z-размера
    
	G4LogicalVolume*    worldLogical = new G4LogicalVolume(worldSolid,     // Ссылка на геометрическую часть
                                                           materialWorld,  // материал
                                                           "logic-World"); // имя

    G4VPhysicalVolume*  worldPhyscal = new G4PVPlacement(nullptr,                 // поворот
                                                         G4ThreeVector(0.,0.,0.), // трансляция
                                                         worldLogical,            // Ссылка на логику
                                                         "World",                 // Имя
                                                         nullptr,                 // Ссылка на материнский объем
                                                         false,                   // Не используется
                                                         0);                      // Номер копии
    
	// b. Мишень
    G4Box*              targetSolid   = new G4Box("solid-Target",   
    	                                          0.5*targetXSize, 
    	                                          0.5*targetYZSize,
    	                                          0.5*targetYZSize);
    G4LogicalVolume*    targetLogical = new G4LogicalVolume(targetSolid,   
                                                           materialTarget,  
                                                           "logic-Target"); 
    G4VPhysicalVolume*  targetPhyscal = new G4PVPlacement(nullptr,               
                                                          G4ThreeVector(0.,0.,0.),
                                                          targetLogical,          
                                                          "Target",               
                                                          worldLogical,            
                                                          false,                  
                                                          0);                                                      

    // Шаг III. В конце необходимо вернуть указатель на физическую часть мира:
    return worldPhyscal;
}

```


### 2.2. Пользовательские действия

Следующий на очереди - класс отвечающий за функционирование нашего приложение. Фактически это набор команд/действий, который приложение совершает во время своего выполнения.

Начнём в `MyActionInitialization.hh`:
```cpp

#ifndef MYACTIONINITIALIZATION_HH
#define MYACTIONINITIALIZATION_HH

#include "G4VUserActionInitialization.hh"
#include "MyDetectorConstruction.hh"

class MyDetectorConstruction;

class MyActionInitialization : public G4VUserActionInitialization {
	public:
		MyActionInitialization(MyDetectorConstruction* det);
		~MyActionInitialization() override;
		
		void Build() const override;
	private:
		MyDetectorConstruction* fMyDetector;

};

#endif

```

Теперь реализация в `MyActionInitialization.cc`:

```cpp 
#include "MyDetectorConstruction.hh"
#include "MyActionInitialization.hh"
#include "MyPrimaryGeneratorAction.hh"

// В конструкторе нам нужно передать в этот класс наш детектор
MyActionInitialization::MyActionInitialization(MyDetectorConstruction* det)
:	G4VUserActionInitialization(),
	fMyDetector(det) { }
	
MyActionInitialization::~MyActionInitialization() {	}

// Все действия нужно описать в функции Build()
void MyActionInitialization::Build() const {
	MyPrimaryGeneratorAction* primaryAction = new MyPrimaryGeneratorAction(fMyDetector);
	SetUserAction(primaryAction);
	// Фактически мы задали действие "определи детектор"
	// В будущем мы добавим сюда ещё действий для Step, Run, Event
}
```

### 2.3. Источник (генератор) частиц

Перейдем к созданию генератора частиц.
Именно в этом месте мы создадим пучок определённых частиц, укажем его размеры, энергию.

Начнём с include файла `MyPrimaryGeneratorAction.hh`:
```cpp

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

```

Теперь `MyPrimaryGeneratorAction.сс`:

```cpp

#include "MyPrimaryGeneratorAction.hh"
#include "MyDetectorConstruction.hh"

#include "G4ParticleGun.hh"
#include "G4Event.hh"
#include "G4ParticleTable.hh"

MyPrimaryGeneratorAction::MyPrimaryGeneratorAction(MyDetectorConstruction* det)
:	G4VUserPrimaryGeneratorAction(),
	fMyDetector(det),
	fParticleGun(nullptr) {

	// Создадим объект G4ParticleGun
	G4int nParticle = 1;
	fParticleGun = new G4ParticleGun (nParticle);
	SetDefaultKinematics();
}

MyPrimaryGeneratorAction::~MyPrimaryGeneratorAction() {
	delete fParticleGun;
}

// Генерация начальных частиц
void MyPrimaryGeneratorAction::GeneratePrimaries(G4Event* evt) {
	fParticleGun->GeneratePrimaryVertex(evt);
	// Метод GeneratePrimaryVertex этого объекта создает одну или несколько первичных частиц, используя параметры, которые мы предварительно задали в SetDefaultKinematics().
}

// Задание кинематики
void MyPrimaryGeneratorAction::SetDefaultKinematics() {
	G4ParticleDefinition* part = G4ParticleTable::GetParticleTable()->FindParticle("e-"); // Ищем электрон в БД
	fParticleGun->SetParticleDefinition( part ); // Передаем в Gun нашу налетающую частицу
	fParticleGun->SetParticleMomentumDirection ( G4ThreeVector(1.,0.,0.) ); // Выбираем направление пучка. В данном случае он будет двигаться по оси x
	fParticleGun->SetParticleEnergy( 30.*CLHEP::MeV ); // Задаем энергию пучка в МэВ
	UpdatePosition();
}

// Обновляем позицию генератора
void MyPrimaryGeneratorAction::UpdatePosition() {
	fParticleGun->SetParticlePosition ( G4ThreeVector ( fMyDetector->GetGunXPosition(),0.,0.) ); // Берём позицию из класса нашего детектора
}

```

### 2.4. Запуск приложения

1. Дополним наш main файл `myproject.cc` детектором и действиями, а также укажем в нём как запустить моделирование:

`myproject.cc`:
```cpp
#include "G4RunManagerFactory.hh"
#include "globals.hh"

#include "MyDetectorConstruction.hh"
#include "MyActionInitialization.hh"

#include "G4PhysListFactory.hh"

int main(){
	
	auto* runManager = G4RunManagerFactory::CreateRunManager(); // Сердце нашего приложения, управление всеми процессами
	
	MyDetectorConstruction* detector = new MyDetectorConstruction;
	runManager->SetUserInitialization( detector ); // Подключение геометрии нашего детектора
	
	G4PhysListFactory plFactory;
	G4VModularPhysicsList *pl = plFactory.GetReferencePhysList( "FTFP_BERT" );
	runManager->SetUserInitialization( pl ); // Подключение физики
	
	runManager->SetUserInitialization( new MyActionInitialization ( detector ) ); // Подключение модуля управления действиями, которому в аргумент указывается детектор
	
	runManager->Initialize(); // Запуск приложения
	
	runManager->BeamOn(10); // Запуск пучка частиц
	
	G4cout << " === END OF SIMULATION === " << G4endl;
	delete runManager;
	return 0;
}
```

2. Переходим в `build`, очищаем папку и делаем сборку заново:
```bash
cd build
cmake ../
make -j5
```

3. Запустим наше приложение
```bash
./myproject
```

4. Мы можем сохранить весь вывод в log файл и изучить его отдельно
```bash
./myproject > log.out
```
Назвать выходной файл можно как угодно.

5. Поздравляю! Мы написали простейшее Geant4 приложение. Оно уже что-то моделирует, но не вычисляет никаких интересных нам физических величин. К тому же, оно совершенно не гибкое и для изменения любых параметров требует изменять код напрямую. 

6. Нам предстоит добавить в этот пример визуализацию, изменение входных параметров через UI команды, а затем добавить вычисления поглощения энергии.

## 3. UI, визуализация и макросы

Давайте немного улучшим наше приложения. Добавим макросы для запуска, визуализации, а также UI мод.

1. Для начала опять дополним наш main файл `myproject.cc`:
```cpp
#include "G4RunManagerFactory.hh"
#include "globals.hh"

#include "MyDetectorConstruction.hh"
#include "MyActionInitialization.hh"

#include "G4PhysListFactory.hh"

// Добавим менеджеры UI и визуализации
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"

int main(){

	// Добавим обработку аргумента исполняемого файла
	if ( argc == 1 ) {
      ui = new G4UIExecutive(argc, argv);
    } else {
      macrofile = argv[1];
    }
	// Нет аргумента - запуск UI, есть - запуск в batch режиме
	
	auto* runManager = G4RunManagerFactory::CreateRunManager();
	
	MyDetectorConstruction* detector = new MyDetectorConstruction;
	runManager->SetUserInitialization( detector );
	
	G4PhysListFactory plFactory;
	G4VModularPhysicsList *pl = plFactory.GetReferencePhysList( "FTFP_BERT" );
	runManager->SetUserInitialization( pl ); 
	
	runManager->SetUserInitialization( new MyActionInitialization ( detector ) ); 

	// Ограничим чисто потоков, чтобы сделать выходной файл более читаемым
	runManager->SetNumberOfThreads(1);
	
	// Добавляем визуализацию:
    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

	// Указатель на менеджер UI
	G4UImanager* UImanager = G4UImanager::GetUIpointer();

	// Продолжение обработки аргумента
	if ( ! ui ) {
      // batch режим
      G4String command  = "/control/execute ";
      UImanager->ApplyCommand( command + macrofile );
      G4cout << " **** BATCH MODE **** " << G4endl;
    } else {
      // интерактивный режим
      ui->SessionStart();
      G4cout << " **** UI MODE **** " << G4endl;
      delete ui; // Не забудем удалить указатель UI!
    }
	
	G4cout << " === END OF SIMULATION === " << G4endl;
	delete runManager;
	delete visManager; // Не забудем удалить указатель визуализации!
	return 0;
}
```

2. Добавим в нашу корневую папку два макроса: для визуализации `vis.mac` и для настройки `run.mac`.

3. Начнём с `run.mac`. В этом макросе мы может указать некоторые параметры нашего приложения. Мы можем создавать свои UI команды для изменения других параметров (например, материала), интерфейс для этого доступен в Geant4.

```txt

#
# Устанавливаем вывод информации (verbose) о трекинге на каждом шаге симуляции в log файл. Доступно: 0 - ничего, 1 - вывод, 2 - более детальный вывод. 
/tracking/verbose 1
#
# Инициализация приложения (мы убрали эту команду из main() функции!)
/run/initialize
#
# Установка энергии и типа частицы в G4ParticleGun 
/gun/energy 4 MeV
/gun/particle e-
#
# Запуск симуляции с 10 событиями (мы убрали эту команду из main() функции!)
/run/beamOn 10

```

4. `vis.mac` помогает детально настроить визуализацию.
```txt

#
# Внимание! Перед запуском этого макроса используйте /run/initialize
#
# Используем драйвер OpenGL для просмотра:
/vis/open OGL
#
# Рисуем геометрию
/vis/drawVolume
#
# Делаем мировой объем невидимым
/vis/geometry/set/visibility logic-World 0  false
#
# Задаем цвет и стиль отображения мишени
/vis/geometry/set/colour logic-Target 0 0 0 255 0.3
/vis/viewer/set/style surface 
#
# Добавим оси координат
/vis/scene/add/axes 
/vis/scene/add/scale
#
# Добавим гладкие траектории 
/vis/scene/add/trajectories smooth
#
# Ограничим визуализацию 100 траекториями
/vis/scene/endOfEventAction accumulate 100
#
# Установим начальный угол просмотра
/vis/viewer/set/viewpointThetaPhi -40 -50

```

5. Теперь пересоберём приложение и запустим сначала в batch режиме (укажем в аргумент run.mac)
```bash
cd build
cmake ../
make -j6
./myproject run.mac > log.out
```

5. Теперь запустим в UI режиме
```bash
./myproject
```

6. В открывшемся окне запустим команду `/run/initialize`, а затем откроем `vis.mac`. После этого можно запустить пучок наших частиц через `/run/beamOn 10`.
Также можно сначала открыть через интерфейс `run.mac`, а затем `vis.mac`.
В качетстве альтернативы можно написать единый макрос с визуализацией и запуском.

7. Теперь наше приложение немного, но стало лучше!
