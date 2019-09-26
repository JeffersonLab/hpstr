#ifndef CLUSTERHISTOS_H
#define CLUSTERHISTOS_H

#include "TFile.h"
#include "HistoManager.h"
#include "TGraphErrors.h"
#include "TKey.h"
#include "TList.h"

#include "TrackerHit.h"
#include "RawSvtHit.h"

#include "ModuleMapper.h"

#include <string>


class ClusterHistos : public HistoManager{
  
 public:
  ClusterHistos(const std::string& inputName);
  ~ClusterHistos();

  virtual void Define3DHistos(){};
  virtual void Define2DHistos();
  virtual void Define1DHistos();

  //virtual void GetHistosFromFile(TFile* inFile, const std::string& name,const std::string& folder="");


  void FillHistograms(TrackerHit* hit,float weight = 1.);
  //void BuildAxesMap();
  
  void setBaselineFitsDir(const std::string& baselineFits) {baselineFits_ = baselineFits;};
  bool LoadBaselineHistos(const std::string& baselineRun);
  
  //void setBaselineFits(const std::string& baselineFits){baselineFits_ = baselineFits;};
  //std::string getBaselineFits const () {return baselineFits_;};
  

 private:
  
  std::vector<std::string> variables{"charge","cluSize"};
  
  std::vector<std::string> half_module_names{};


  std::map<std::string, int>    cluSizeMap;
  std::map<std::string, double> chargeMap;
  std::map<std::string, double> chargeCorrectedMap;
  std::map<std::string, double> cluPositionMap;
  
  std::string baselineFits_{"/nfs/hps3/svtTests/jlabSystem/baselines/fits/"};
  std::string baselineRun_{""};

  std::map<std::string, TGraphErrors*> baselineGraphs;

  //TODO clean this
  ModuleMapper *mmapper_{nullptr};
};


#endif
