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