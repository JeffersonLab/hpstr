// HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "EventHeader.h"
#include "TSData.h"
#include "Vertex.h"
#include "Track.h"
#include "TrackerHit.h"
#include "MCParticle.h"
#include "Particle.h"
#include "Processor.h"
#include "BaseSelector.h"
#include "TrackHistos.h"
#include "MCAnaHistos.h"
#include "utilities.h"

#include "TrackSmearingTool.h"
#include "FlatTupleMaker.h"
#include "AnaHelpers.h"

#include "Bus.h"

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"

// C++ 
#include <memory>
#include <iostream>
#include <fstream>
#include <map>


class CutflowHist {
  std::unique_ptr<TH1F> h_cutflow_;
  std::map<std::string,int> cuts_;
 public:
  CutflowHist(int max_cuts) {
    h_cutflow_ = std::make_unique<TH1F>(
        "cutflow_h",
        "Cutflow Histogram",
        max_cuts+1,0,max_cuts+1
    );
    h_cutflow_->Sumw2();
    cuts_["no cuts"] = 1;
  }
  void save() {
    for (const auto& [name, ibin]: cuts_) {
      // shift bin labels by one since the lowest bin label index
      // is underflow and not ibin==0
      h_cutflow_->GetXaxis()->SetBinLabel(ibin+1, name.c_str());
    }
    h_cutflow_->Write();
  }
  bool should_cut(const std::string& name, bool descision) {
    if (cuts_.find(name) == cuts_.end()) {
      auto ibin = cuts_.size()+1;
      if (ibin == h_cutflow_->GetNbinsX()) {
        throw std::runtime_error("More cuts that allowed in CutflowHist");
      }
      cuts_[name] = ibin;
    }
    if (not descision) h_cutflow_->Fill(cuts_.at(name));
    return descision;
  }
};

class PreSelectAndCategorize : public Processor {
 public:
  PreSelectAndCategorize(const std::string& name, Process& process):
    Processor(name,process) {}
  ~PreSelectAndCategorize() = default;
  virtual void configure(const ParameterSet& parameters) final;
  virtual void initialize(TTree* tree) final;
  virtual void setFile(TFile* out_file) final;
  virtual bool process(IEvent* ievent) final;
  virtual void finalize() final;
 private:
  template<typename BaggageType>
  void board(TTree* tree, const std::string& name, bool create_if_missing) {
    if (bus_.isOnBoard(name)) {
      throw std::runtime_error(name+" is already on board the bus.");
    }
    bus_.board<BaggageType>(name);
    if (bus_.attach(tree, name, create_if_missing) == 0) {
      throw std::runtime_error("Unable to attach "+name+" to TTree.");
    }
  }
  framework::Bus bus_;

  std::string vtxColl_{"UnconstrainedV0Vertices_KF"};
  std::string mcColl_{"MCParticle"}; //!< description
  int isRadPDG_{622}; //!< description
  std::string pSmearingFile_{""};

  std::unique_ptr<TTree> output_tree_;

  std::shared_ptr<TrackSmearingTool> smearingTool_;

  std::shared_ptr<TrackHistos> _vtx_histos; //!< description
  std::shared_ptr<MCAnaHistos> _mc_vtx_histos; //!< description

  std::string histoCfg_{""}; //!< description
  std::string mcHistoCfg_{""}; //!< description
  double timeOffset_{-999}; //!< description
  double beamE_{2.3}; //!< In GeV. Default is 2016 value;
  int isData_{0}; //!< description
  std::shared_ptr<AnaHelpers> _ah; //!< description

  int debug_{0}; //!< Debug level
  std::string beamPosCfg_{""}; //!< json containing run dep beamspot positions
  json bpc_configs_; //!< json object
  std::vector<double> beamPosCorrections_ = {0.0,0.0,0.0}; //!< holds beam position corrections
  std::string v0ProjectionFitsCfg_{""};//!< json file w run dependent v0 projection fits
  json v0proj_fits_;//!< json object v0proj
  double eleTrackTimeBias_ = 0.0;
  double posTrackTimeBias_ = 0.0;
  int current_run_number_{-999}; //!< track current run number
  CutflowHist h_cf_{5};
};

void PreSelectAndCategorize::configure(const ParameterSet& parameters) {
  if (not pSmearingFile_.empty()) {
    // just using the same seed=42 for now
    smearingTool_    =   std::make_shared<TrackSmearingTool>(pSmearingFile_,true);
  }
}

void PreSelectAndCategorize::initialize(TTree* tree) {
  //init Reading Tree
  board<EventHeader>(tree, "EventHeader", false);
  board<std::vector<Vertex*>>(tree, vtxColl_, false);
  if(!isData_ && !mcColl_.empty())
    board<std::vector<MCParticle*>>(tree, mcColl_, false);
}

void PreSelectAndCategorize::setFile(TFile* out_file) {
  Processor::setFile(out_file);
  // create output TTree in output file
  output_tree_ = std::make_unique<TTree>("preselection","PreSelected and Categorized Vertices",99,out_file);
  board<double>(output_tree_.get(), "weight", true);
  board<Vertex>(output_tree_.get(), "vertex", true);
  board<Particle>(output_tree_.get(), "ele", true);
  board<Particle>(output_tree_.get(), "pos", true);
}

bool PreSelectAndCategorize::process(IEvent* ievent) {
  const auto& eh{bus_.get<EventHeader>("EventHeader")};
  if(debug_) {
   std:: cout << "----------------- Event " << eh.getEventNumber() << " -----------------" << std::endl;
  }
  bus_.update("weight", 1.);
  int run_number = eh.getRunNumber();
  h_cf_.should_cut("no cuts", false);

  const auto& vtxs{bus_.get<std::vector<Vertex*>>(vtxColl_)};
  if (h_cf_.should_cut("no vertex", vtxs.size() < 1)) {
    return true;
  }
  
  if (h_cf_.should_cut("no extra vertices", vtxs.size() > 1)) {
    return true;
  }
  
  // just right
  Vertex vtx = *vtxs.at(0);
  Particle ele = *dynamic_cast<Particle*>(vtx.getParticles().At(0));
  Particle pos = *dynamic_cast<Particle*>(vtx.getParticles().At(1));

  bus_.update("vertex", vtx);
  bus_.update("ele", ele);
  bus_.update("pos", pos);
  output_tree_->Fill();
  return true;
}

void PreSelectAndCategorize::finalize() {
  //TODO clean this up a little.
  outF_->cd();
  output_tree_->Write();
  h_cf_.save();
  //_vtx_histos->saveHistos(outF_,_vtx_histos->getName());
  //outF_->cd(_vtx_histos->getName().c_str());
  //vtxSelector->getCutFlowHisto()->Write();
}

DECLARE_PROCESSOR(PreSelectAndCategorize);
