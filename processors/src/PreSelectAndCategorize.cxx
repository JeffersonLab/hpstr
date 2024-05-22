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
        max_cuts,-0.5,max_cuts-0.5
    );
    h_cutflow_->Sumw2();
    cuts_["readout"] = 0;
  }
  void save() {
    std::vector<std::string> labels(h_cutflow_->GetNbinsX(),"");
    for (const auto& [name, ibin]: cuts_) {
      labels[ibin] = name;
    }
    for (int ilabel{0}; ilabel < labels.size(); ilabel++) {
      h_cutflow_->GetXaxis()->SetBinLabel(ilabel+1, labels[ilabel].c_str());
    }
    h_cutflow_->Write();
  }
  bool should_cut(const std::string& name, bool descision) {
    if (cuts_.find(name) == cuts_.end()) {
      auto ibin = cuts_.size();
      if (ibin == h_cutflow_->GetNbinsX()) {
        throw std::runtime_error("More cuts that allowed in CutflowHist");
      }
      cuts_[name] = ibin;
    }
    if (not descision) h_cutflow_->Fill(cuts_.at(name));
    return descision;
  }
};

class EventBus {
  framework::Bus bus_;
 public:
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
  EventBus bus_;
  std::unique_ptr<TTree> output_tree_;

  std::string vtxColl_{"UnconstrainedV0Vertices_KF"};
  std::string mcColl_{"MCParticle"}; //!< description
  int isRadPDG_{622}; //!< description
  double beamE_{2.3}; //!< In GeV. Default is 2016 value;
  int isData_{0}; //!< description
  double eleTrackTimeBias_{0.0};
  double posTrackTimeBias_{0.0};
  double calTimeOffset_{0.0};

  CutflowHist h_cf_{20};
  std::shared_ptr<TrackSmearingTool> smearingTool_;
  std::shared_ptr<AnaHelpers> _ah; //!< description

  json bpc_configs_; //!< json object
  json v0proj_fits_;//!< json object v0proj
};


json json_load(const std::string& filepath) {
  json obj;
  std::ifstream file{filepath};
  file >> obj;
  file.close();
  return obj;
}

void PreSelectAndCategorize::configure(const ParameterSet& parameters) {
  auto pSmearingFile = parameters.getString("pSmearingFile","");
  if (not pSmearingFile.empty()) {
    // just using the same seed=42 for now
    smearingTool_ = std::make_shared<TrackSmearingTool>(pSmearingFile,true);
  }

  auto beamPosCfg = parameters.getString("beamPosCfg", "");
  if (not beamPosCfg.empty()) {
    bpc_configs_ = json_load(beamPosCfg);
  }

  auto v0ProjectionFitsCfg = parameters.getString("v0ProjectionFitsCfg", "");
  if (not v0ProjectionFitsCfg.empty()) {
    v0proj_fits_ = json_load(v0ProjectionFitsCfg);
  }

  eleTrackTimeBias_ = parameters.getDouble("eleTrackTimeBias");
  posTrackTimeBias_ = parameters.getDouble("posTrackTimeBias");
  calTimeOffset_ = parameters.getDouble("calTimeOffset");
}

void PreSelectAndCategorize::initialize(TTree* tree) {
  _ah =  std::make_shared<AnaHelpers>();
  //init Reading Tree
  bus_.board_input<EventHeader>(tree, "EventHeader");
  bus_.board_input<std::vector<Vertex*>>(tree, vtxColl_);
  if(!isData_ && !mcColl_.empty())
    bus_.board_input<std::vector<MCParticle*>>(tree, mcColl_);
}

void PreSelectAndCategorize::setFile(TFile* out_file) {
  Processor::setFile(out_file);
  // create output TTree in output file
  output_tree_ = std::make_unique<TTree>(
      "preselection",
      "PreSelected and Categorized Vertices",
      99,
      out_file
  );
  bus_.board_output<double>(output_tree_.get(), "weight");
  bus_.board_output<Vertex>(output_tree_.get(), "vertex");
  bus_.board_output<double>(output_tree_.get(), "vertex_invm_smear");
  bus_.board_output<Particle>(output_tree_.get(), "ele");
  bus_.board_output<Particle>(output_tree_.get(), "pos");
  for (const auto& name : {"eleL1","eleL2","posL1","posL2"}) {
    bus_.board_output<bool>(output_tree_.get(), name);
  }
  if (not v0proj_fits_.empty()) {
    for (const auto& name : {
        "vtx_proj_sig",
        "vtx_proj_x", "vtx_proj_x_sig",
        "vtx_proj_y", "vtx_proj_y_sig" 
    }) {
      bus_.board_output<double>(output_tree_.get(), name);
    }
  }
}

bool PreSelectAndCategorize::process(IEvent*) {
  const auto& eh{bus_.get<EventHeader>("EventHeader")};
  int run_number = eh.getRunNumber();
  h_cf_.should_cut("readout", false);

  const auto& vtxs{bus_.get<std::vector<Vertex*>>(vtxColl_)};
  if (h_cf_.should_cut("at least one vertex", vtxs.size() < 1)) {
    return true;
  }
  
  if (h_cf_.should_cut("no extra vertices", vtxs.size() > 1)) {
    return true;
  }
  
  // correct number of vertices (i.e. only one)
  // access the indiviual Vertex, electron, and positron
  // and add corrections to them before serialization
  Vertex vtx = *vtxs.at(0);
  int i_ele{-1}, i_pos{-1};
  for (int ipart = 0; ipart < vtx.getParticles().GetEntries(); ++ipart) {
    int pdg_id = ((Particle*)vtx.getParticles().At(ipart))->getPDG();
    if (pdg_id == 11) {
      i_ele = ipart;
    } else if (pdg_id == -11) {
      i_pos = ipart;
    }
  }
  if (i_ele < 0 or i_pos < 0) {
    throw std::runtime_error("Vertex formed without either an electron or positron!");
  }

  Particle ele = *dynamic_cast<Particle*>(vtx.getParticles().At(i_ele));
  Particle pos = *dynamic_cast<Particle*>(vtx.getParticles().At(i_pos));

  // for both particles, get cluster, update, re-set cluster
  CalCluster ele_clu = ele.getCluster();
  CalCluster pos_clu = pos.getCluster();

  ele_clu.setTime(ele_clu.getTime() - calTimeOffset_);
  pos_clu.setTime(pos_clu.getTime() - calTimeOffset_);

  ele.setCluster(&ele_clu);
  pos.setCluster(&pos_clu);

  Track ele_trk = ele.getTrack();
  Track pos_trk = pos.getTrack();

  // deduce beam position corrections if config was passed
  std::vector<double> beamPosCorrections = {0., 0., 0.};
  if (not bpc_configs_.empty()) {
    for (const auto& [run, entry] : bpc_configs_.items()) {
      if (std::stoi(run) > eh.getRunNumber()) {
        break;
      }
      beamPosCorrections = {
        entry["beamspot_x"],
        entry["beamspot_y"],
        entry["beamspot_z"]
      };
    }
  }
  ele_trk.applyCorrection("z0", beamPosCorrections.at(1));
  pos_trk.applyCorrection("z0", beamPosCorrections.at(1));

  // correct track timing bias
  ele_trk.applyCorrection("track_time", eleTrackTimeBias_);
  pos_trk.applyCorrection("track_time", posTrackTimeBias_);

  // smear track momentum
  double invm_smear = 1.;
  if (smearingTool_) {
    double unsmeared_prod = ele_trk.getP()*pos_trk.getP();
    smearingTool_->updateWithSmearP(ele_trk);
    smearingTool_->updateWithSmearP(pos_trk);
    double smeared_prod = ele_trk.getP()*pos_trk.getP();
    invm_smear = sqrt(smeared_prod/unsmeared_prod);
  }

  // put tracks back into their particles
  // with their new data
  ele.setTrack(&ele_trk);
  pos.setTrack(&pos_trk);

  // earliest layer hit categories
  bool eleL1{false}, eleL2{false},
       posL1{false}, posL2{false};
  _ah->InnermostLayerCheck(&ele_trk, eleL1, eleL2);
  _ah->InnermostLayerCheck(&pos_trk, posL1, posL2);
  bus_.set("eleL1", eleL1);
  bus_.set("eleL2", eleL2);
  bus_.set("posL1", posL1);
  bus_.set("posL2", posL2);

  // calculate target projection and its significance
  if (not v0proj_fits_.empty()) {
    double vtx_proj_x{-1.0}, vtx_proj_y{-1.0},
           vtx_proj_x_sig{-1.0}, vtx_proj_y_sig{-1.0},
           vtx_proj_sig{-1.0};
    vtx_proj_sig = utils::v0_projection_to_target_significance(
        v0proj_fits_, eh.getRunNumber(),
        vtx_proj_x, vtx_proj_y, vtx_proj_x_sig, vtx_proj_y_sig,
        vtx.getX(), vtx.getY(), vtx.getZ(),
        vtx.getP().X(), vtx.getP().Y(), vtx.getP().Z()
    );
    bus_.set("vtx_proj_sig", vtx_proj_sig);
    bus_.set("vtx_proj_x", vtx_proj_x);
    bus_.set("vtx_proj_x_sig", vtx_proj_x_sig);
    bus_.set("vtx_proj_y", vtx_proj_y);
    bus_.set("vtx_proj_y_sig", vtx_proj_y_sig);
  }

  bus_.set("weight", 1.);
  bus_.set("vertex", vtx);
  bus_.set("vertex_invm_smear", vtx.getInvMass()*invm_smear);
  bus_.set("ele", ele);
  bus_.set("pos", pos);

  /**
   * Further pre-selection on vertices.
   */

  if (h_cf_.should_cut(
        "clusters within 1.45ns",
        abs(ele.getCluster().getTime()-pos.getCluster().getTime()) > 1.45
  )) {
    return true;
  }

  if (h_cf_.should_cut(
        "electron below 1.75GeV",
        ele.getTrack().getP() > 1.75
  )) {
    return true;
  }

  if (h_cf_.should_cut(
        "vertex $\\chi^2 < 20$",
        vtx.getChi2() > 20.0
  )) {
    return true;
  }

  if (h_cf_.should_cut(
        "ele track $\\chi^2/ndf < 20$",
        ele.getTrack().getChi2Ndf() > 20.0
  )) {
    return true;
  }

  if (h_cf_.should_cut(
        "pos track $\\chi^2/ndf < 20$",
        pos.getTrack().getChi2Ndf() > 20.0
  )) {
    return true;
  }

  int ele_nhits = ele.getTrack().getTrackerHitCount();
  if (not ele.getTrack().isKalmanTrack()) ele_nhits*=2;
  if (h_cf_.should_cut(
        "ele track has at least 7 2D hits",
        ele_nhits < 8
  )) {
    return true;
  }

  int pos_nhits = pos.getTrack().getTrackerHitCount();
  if (not pos.getTrack().isKalmanTrack()) pos_nhits*=2;
  if (h_cf_.should_cut(
        "pos track has at least 7 2D hits",
        pos_nhits < 8
  )) {
    return true;
  }

  // left timing until last since it is having the biggest effect besides
  // requiring a vertex
  if (h_cf_.should_cut(
        "ele track earlier than 6ns",
        ele.getTrack().getTrackTime() > 6.0
  )) {
    return true;
  }

  if (h_cf_.should_cut(
        "pos track earlier than 6ns",
        pos.getTrack().getTrackTime() > 6.0
  )) {
    return true;
  }

  if (h_cf_.should_cut(
        "ele track and cluster within 4ns",
        abs(ele.getTrack().getTrackTime()-ele.getCluster().getTime()) > 4.0
  )) {
    return true;
  }

  if (h_cf_.should_cut(
        "pos track and cluster within 4ns",
        abs(pos.getTrack().getTrackTime()-pos.getCluster().getTime()) > 4.0
  )) {
    return true;
  }
  

  /**
   * This is where the output TTree is filled,
   * if we leave before this point, then the event will not
   * be kept as part of pre-selection.
   */
  output_tree_->Fill();
  return true;
}

void PreSelectAndCategorize::finalize() {
  outF_->cd();
  output_tree_->Write();
  h_cf_.save();
}

DECLARE_PROCESSOR(PreSelectAndCategorize);
