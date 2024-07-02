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

class EventBus {
  framework::Bus bus_;
 public:
  bool has(const std::string& name) {
    return bus_.isOnBoard(name);
  }
  template<typename BaggageType>
  void board_input(TTree* tree, const std::string& name) {
    if (bus_.isOnBoard(name)) {
      throw std::runtime_error(name+" is already on board the bus.");
    }
    bus_.board<BaggageType>(name);
    if (bus_.attach(tree, name, false /* create if missing */) == 0) {
      throw std::runtime_error("Unable to find branch "+name+" in input TTree.");
    }
  }
  template<typename BaggageType>
  void board_output(TTree* tree, const std::string& name) {
    if (bus_.isOnBoard(name)) {
      throw std::runtime_error(name+" is already on board the bus.");
    }
    bus_.board<BaggageType>(name);
    if (bus_.attach(tree, name, true /* create if missing */) == 0) {
      throw std::runtime_error("Unable to create branch "+name+" in output TTree.");
    }
  }
  template<typename BaggageType>
  const BaggageType& get(const std::string& name) {
    return bus_.get<BaggageType>(name);
  }
  template<typename BaggageType>
  void set(const std::string& name, const BaggageType& obj) {
    if (bus_.isOnBoard(name)) {
      bus_.update(name, obj);
      return;
    }
    throw std::runtime_error("No branch "+name+" boarded on the bus.");
  }
};

class TrueSimZDistribution : public Processor {
 public:
  TrueSimZDistribution(const std::string& name, Process& process):
    Processor(name,process) {}
  ~TrueSimZDistribution() = default;
  virtual void configure(const ParameterSet& parameters) final {
    subdir_ = parameters.getString("subdir", "");
  }
  virtual void initialize(TTree* tree) final;
  virtual bool process(IEvent* ievent) final;
  virtual void finalize() final;
 private:
  EventBus bus_;
  std::string subdir_;

  std::string mcColl_{"MCParticle"}; //!< description
  std::unique_ptr<TH1F> true_z_h_;
};

void TrueSimZDistribution::initialize(TTree* tree) {
  bus_.board_input<std::vector<MCParticle*>>(tree, mcColl_);
  true_z_h_ = std::make_unique<TH1F>(
      "true_z_h",
      "True Z Distribution of Simulation",
      250, -4.3, 245.7
  );
}

bool TrueSimZDistribution::process(IEvent*) {
  const auto& mc_ptr{bus_.get<std::vector<MCParticle*>>(mcColl_)};
  for (MCParticle* ptr : mc_ptr) {
    if (ptr->getPDG() == 625) {
      true_z_h_->Fill(ptr->getVertexPosition().at(2));
    }
  }
  return true;
}

void TrueSimZDistribution::finalize() {
  outF_->cd();
  if (not subdir_.empty()) {
    outF_->mkdir(subdir_.c_str())->cd();
  }
  true_z_h_->Write();
}

DECLARE_PROCESSOR(TrueSimZDistribution);
