# Практическое занятие 4

## 5. Создание новых UI комманд

Целью этой главы станет реализация дополнительных UI команд для управления приложением через макросы.
Мы реализуем функции для смены толщины и материала мишени.

Для этого нам нужно построить Run, Step и EventAction классы и внести в них вычисления энергопотери. Фактически, мы будем вычислять её на Step, затем собирать по всем шагам в Event, а в конце собирать за все события в Run.

### 5.0. Структура нашего проекта

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
│   ├── Run.hh                      # Наша изменённая копия G4Run с новым функционалом
│   ├── Hist.hh                     # Простой класс для гистограммирования
│   ├── EventAction.hh              # Действия в начале и конце одного события
│   └── SteppingAction.hh           # Действия на каждом шаге трека
│
├── src/                    # 5. Исходные файлы .cc (реализация)
│   ├── DetectorConstruction.cc
│   ├── ActionInitialization.cc
│   ├── PrimaryGeneratorAction.cc
│   ├── RunAction.cc
│   ├── Run.cc
│   ├── Hist.cc
│   ├── EventAction.cc
│   └── SteppingAction.cc
├── build/                  # 6. Файлы сборки
└───────────────────────────────────────────────────────────────────────────────
```

### 5.1. Модификация MyDetectorConstruction

Чтобы реализовать наши новые UI команды, нам нужно немного изменить `MyDetectorConstruction` класс.
Для этого добавим некоторые новые функции, которые позволят нам менять толщину и материал мишени из вне.

Также далее мы напишем так называемый `messenger` класс, в котором напишем наши UI команды. Поэтому нам нужно добавить хедер `.hh` этого нового класса, а также объявить указатель на него.

Изменим сначала include `MyDetectorConstruction.hh`:
```cpp 
#ifndef MYDETECTORCONSTRUCTION_HH
#define MYDETECTORCONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"

class G4Material;

class MyDetectorConstruction : public G4VUserDetectorConstruction {

    public:

        MyDetectorConstruction();
        ~MyDetectorConstruction() override;

        // Новая inline фунция для изменения толщины мишени
        void SetTargetThickness(const G4double thickness) { 
            fTargetThickness = thickness; 
        }
        G4double GetTargetThickness() const {
            return fTargetThickness;
        }

        // Новые функции для установки и получения материала мишени
        void SetTargetMaterial(const G4String& matName); // Реализуем в .cc!
        const G4Material* GetTargetMaterial() const  { 
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

        MyDetectorMessenger* fDetectorMessenger; // Новый указатель на messenger класс
};

#endif
```

Перейдём к реализации `MyDetectorConstruction.hh`:
```cpp 
#include "MyDetectorConstruction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

MyDetectorConstruction::MyDetectorConstruction() 
:  G4VUserDetectorConstruction()
   fTargetMaterial(nullptr) // Укажем nullptr для материала перед вызовом конструктора
{
    // Теперь часть с материалом мишени в отдельном методе.
    // В констукторе просто объявим значение по-умолчанию
    SetTargetMaterial("G4_Si"); 

    fTargetThickness = 1.0*CLHEP::cm;

    fGunXPosition = -0.25*(1.1*fTargetThickness+fTargetThickness);

    // Создаем объект messenger класса (для собственных UI комманд)
    fDetMessenger = new YourDetectorMessenger(this);
}

MyDetectorConstruction::~MyDetectorConstruction() {
    // В деструкторе удалим объект fDetMessenger
    delete fDetMessenger;
}

// Переносим выбор материала из конструктора в отдельный метод
void MyDetectorConstruction::SetTargetMaterial(const G4String& matName) {
    fTargetMaterial = G4NistManager::Instance()->FindOrBuildMaterial(matName);
    if (!fTargetMaterial) {
        G4cerr << " **** Материал не найден! ****" << G4endl;
        exit(-1);
    }
}

G4VPhysicalVolume* MyDetectorConstruction::Construct() {
    // Наша реализация Construct. Её не трогаем                                                      
}
```

### 5.2. UI команды. MyDetectorMessenger

Настало время создать класс messenger, в котором создадим наши собственные UI команды.

Начнём с include файла `MyDetectorMessenger.hh`:
```cpp
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

    YourDetectorMessenger(YourDetectorConstruction* det);
   ~YourDetectorMessenger();

    // Функция для установки значения величины после вызова UI команды
    void SetNewValue(G4UIcommand* command, G4String newValue) override;

  private:

    // Объект нашего детектора
    YourDetectorConstruction*  fYourDetector; 

    // Объекты директории и команд UI
    G4UIdirectory*             fDirCMD;
    G4UIcmdWithADoubleAndUnit* fTargetThicknessCMD;
    G4UIcmdWithAString*        fTargetMaterialCMD;
};

#endif
```

Теперь `MyDetectorMessenger.сс`:

```cpp
#include "YourDetectorMessenger.hh"

#include "YourDetectorConstruction.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4String.hh"


YourDetectorMessenger::YourDetectorMessenger(YourDetectorConstruction* det)
:   G4UImessenger(),
    fYourDetector(det),
    fDirCMD(nullptr),
    fTargetThicknessCMD(nullptr),
    fTargetMaterialCMD(nullptr) {

    // Создадим директорию детектор
    fDirCMD = new G4UIdirectory("/myproject/detector");
    // Опционально можем оставить комментарий что это за директория
    fDirCMD->SetGuidance("UI commands specific to the detector in this project");

    // UI команда для изменения толщины мишени
    fTargetThicknessCMD = new G4UIcmdWithADoubleAndUnit("/myproject/detector/setTargetThickness",this);
    // Описание
    fTargetThicknessCMD->SetGuidance("Sets target thickness");
    // Имя параметра, второй параметр omittable=false означает что пользователю нужно указать значение
    fTargetThicknessCMD->SetParameterName("TargetSizeX",false);
    // Указываем допустимые величины параметра.
    fTargetThicknessCMD->SetRange("TargetSizeX>0.");
    // Выбираем систему единиц как длинну
    fTargetThicknessCMD->SetUnitCategory("Length");
    // Указываем, что параметр можно менять PreInit и Idle состояния приложения (см. лекции)
    fTargetThicknessCMD->AvailableForStates(G4State_PreInit, G4State_Idle);


    // UI команда для изменения материала мишени
    fTargetMaterialCMD = new G4UIcmdWithAString("/myproject/detector/setTargetMaterial",this);
    fTargetMaterialCMD->SetGuidance("Sets target material");
    fTargetMaterialCMD->SetParameterName("TargetMaterial",false);
    fTargetMaterialCMD->AvailableForStates(G4State_PreInit, G4State_Idle);
}


YourDetectorMessenger::~YourDetectorMessenger() {
  // Удаляем все new объекты
  delete fTargetThicknessCMD;
  delete fTargetMaterialCMD;
  delete fDirCMD;
}


void YourDetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    // Установка толщины мишени
    if (command == fTargetThicknessCMD) {
        G4double thickness = fTargetThicknessCMD->GetNewDoubleValue(newValue);
        fYourDetector->SetTargetThickness(thickness);
    }
    // Установка материала мишени
    if (command == fTargetMaterialCMD) {
        fYourDetector->SetTargetMaterial(newValue);
   }
}
```
### 5.3. Компиляция и сравнение с экспериментом.

Теперь наше приложение полностью готово!

Нужно только собрать проект в build, решить ошибки компиляции (если таковые будут) и запустить тестовый макрос `run.mac`.

Нам осталось только проверить Geant4 моделирование в нашем приложении с реальным экспериментом.
Мы будем использовать материал из [этой статьи](https://iopscience.iop.org/article/10.1088/1748-0221/6/06/P06013). [PDF публикации](/experiment/Meroli2011_paper.pdf).

1. Для этого создадим новый макрос `experiment.mac`, в котором укажем параметры для моделирования как в эксперименте:
```text
#
# Устанавливаем вывод информации (verbose)
/tracking/verbose 1
/run/verbose 1
/control/verbose 1
/process/verbose 0
#
# Можно установить число потоков
#/run/numberOfThreads 4
#
# Установим наши материалы и толщину мишени через новые UI команды
/myproject/detector/setTargetThickness 5.6 um
/myproject/detector/setTargetMaterial G4_Si
# Инициализация приложения
/run/initialize
# Установка глобального порог пробега частиц
# Мы просим Geant4 не генерировать вторичные частицы (например, гамма-кванты, электроны или позитроны),
# если их расчетный пробег в данном материале меньше 1 микрона
/run/setCut 1.0 um
#
# Установка энергии и типа частицы
/gun/energy 100 MeV
/gun/particle e-
#
# Выписывать информацию и о симуляции каждое 100-тысячное событие
/run/printProgress 100000
# Запуск симуляции с 1 000 000 событиями
/run/beamOn 1000000
```

2. Запустим наше приложение в batch режиме
```bash
./myproject experiment.mac
```

3. На выходе получим файл `Hist_edep.dat`.

4. Возьмем файл с данными [здесь](/experiment/exp_Meroli_100MeV_electron_5p6um_Si.dat).

5. Теперь нужно сравнить. Мы подготовили простой python скрипт для построения графиков `plot.py` который находится в директории `/experiment`. Вы можете построить график по-своему.

6. Из-за того что экспериментальные данные указаны в keV, а мы моделируем в MeV, а величина измерения немного отличается от того что выдает Geant4 нам нужно ввести скейлинг для наших данных:`x*1000, y*6.4`.

7. Посмотрим на получившееся изображение.

Готово! На этом наш курс завершён!

### Литература

Soon