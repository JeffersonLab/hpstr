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


class Cutflow {
  std::unique_ptr<TH1F> h_cutflow_;
  std::map<std::string,std::pair<std::size_t,std::unique_ptr<TH1F>>> cuts_;
  bool keep_;
  std::vector<std::optional<bool>> cut_desc_;
 public:
  void add(const std::string& name, int nbins, float min, float max) {
    if (h_cutflow_) {
      throw std::runtime_error("Cannot add more cuts after init");
    }
    auto h = std::make_unique<TH1F>(
                ("nm1_"+name+"_h").c_str(),
                (name+" N-1").c_str(),
                nbins, min, max
            );
    cuts_.emplace(name, std::make_pair<std::size_t, std::unique_ptr<TH1F>>(cuts_.size(), std::move(h)));
  }
  void init() {
    if (h_cutflow_) {
      throw std::runtime_error("Cannot init twice.");
    }
    h_cutflow_ = std::make_unique<TH1F>(
        "cutflow_h",
        "Cutflow",
        cuts_.size()+1, -1.5, cuts_.size()-0.5
    );
    cut_desc_.resize(cuts_.size());
  }
  void begin_event() {
    h_cutflow_->Fill("readout", 1.);
    keep_ = true;
    for (auto& d : cut_desc_) d.reset();
  }
  void apply(const std::string& name, bool descision) {
    keep_ = (keep_ and descision);
    if (keep_) h_cutflow_->Fill(name.c_str(), 1.);
    int i_cut = cuts_.at(name).first;
    cut_desc_[i_cut] = descision;
  }
  template<typename T>
  void fill_nm1(const std::string& name, T value) {
    bool should_fill{true};
    for (std::size_t i_cut{0}; i_cut < cuts_.size(); i_cut++) {
      if (i_cut != cuts_[name].first and cut_desc_[i_cut].has_value()) {
        should_fill = (should_fill and cut_desc_[i_cut].value());
      }
    }
    if (should_fill) {
      cuts_.at(name).second->Fill(value);
    }
  }
  bool keep() const {
    return keep_;
  }
  void save() {
    h_cutflow_->Write();
    for (const auto& [_name, entry] : cuts_) entry.second->Write();
  }
};

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
  bool isData_{false};
  double eleTrackTimeBias_{0.0};
  double posTrackTimeBias_{0.0};
  double calTimeOffset_{0.0};

  Cutflow cf_;
  std::unique_ptr<TH1F> n_vertices_h_;
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
  auto pSmearingFile = parameters.getString("pSmearingFile");
  if (not pSmearingFile.empty()) {
    // just using the same seed=42 for now
    std::cout << "Loading momentum smearing from " << pSmearingFile << std::endl;
    smearingTool_ = std::make_shared<TrackSmearingTool>(pSmearingFile,true);
  }

  auto beamPosCfg = parameters.getString("beamPosCfg");
  if (not beamPosCfg.empty()) {
    std::cout << "Loading beamspot corrections from " << beamPosCfg << std::endl;
    bpc_configs_ = json_load(beamPosCfg);
  }

  auto v0ProjectionFitsCfg = parameters.getString("v0ProjectionFitsCfg");
  if (not v0ProjectionFitsCfg.empty()) {
    std::cout << "Loading projection fits from " << v0ProjectionFitsCfg << std::endl;
    v0proj_fits_ = json_load(v0ProjectionFitsCfg);
  }

  eleTrackTimeBias_ = parameters.getDouble("eleTrackTimeBias");
  posTrackTimeBias_ = parameters.getDouble("posTrackTimeBias");
  calTimeOffset_ = parameters.getDouble("calTimeOffset");

  isData_ = parameters.getInteger("isData") != 0;
}

void PreSelectAndCategorize::initialize(TTree* tree) {
  _ah =  std::make_shared<AnaHelpers>();
  //init Reading Tree
  bus_.board_input<EventHeader>(tree, "EventHeader");
  bus_.board_input<std::vector<Vertex*>>(tree, vtxColl_);
  if(not isData_ and not mcColl_.empty())
    bus_.board_input<std::vector<MCParticle*>>(tree, mcColl_);
  
  cf_.add("at_least_one_vertex", 10, 0.0, 10.0);
  cf_.add("no_extra_vertices", 10, 0.0, 10.0);
  /* further selection on vertices
  cf_.add("clusters_within_1.45ns", 100,0.0,10.0);
  cf_.add("electron_below_1.75GeV", 230,0.0,2.3);
  cf_.add("vertex_chi2", 100,0.0,50.0);
  cf_.add("ele_track_chi2ndf", 100, 0.0, 20.0);
  cf_.add("pos_track_chi2ndf", 100, 0.0, 20.0);
  cf_.add("ele_min_8_hits", 14, 0, 14);
  cf_.add("pos_min_8_hits", 14, 0, 14);
  cf_.add("ele_track_before_6ns", 120, 0.0, 24.0);
  cf_.add("pos_track_before_6ns", 120, 0.0, 24.0);
  cf_.add("ele_track_cluster_within_4ns", 80, 0.0, 16.0);
  cf_.add("pos_track_cluster_within_4ns", 80, 0.0, 16.0);
  */
  cf_.init();

  n_vertices_h_ = std::make_unique<TH1F>(
      "n_vertices_h",
      "N Vertices in Event (no selection)",
      10,-0.5,9.5
  );
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
  if (bus_.has(mcColl_)) {
    bus_.board_output<std::vector<MCParticle>>(output_tree_.get(), "mc");
  }
}

bool PreSelectAndCategorize::process(IEvent*) {
  const auto& eh{bus_.get<EventHeader>("EventHeader")};
  int run_number = eh.getRunNumber();
  cf_.begin_event();

  const auto& vtxs{bus_.get<std::vector<Vertex*>>(vtxColl_)};
  n_vertices_h_->Fill(vtxs.size());
  cf_.apply("at_least_one_vertex", vtxs.size() > 0);
  cf_.apply("no_extra_vertices", vtxs.size() < 2);
  if (not cf_.keep()) {
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
  double cluster_tdiff{abs(ele.getCluster().getTime()-pos.getCluster().getTime())};
  int ele_nhits = ele.getTrack().getTrackerHitCount();
  if (not ele.getTrack().isKalmanTrack()) ele_nhits*=2;
  int pos_nhits = pos.getTrack().getTrackerHitCount();
  if (not pos.getTrack().isKalmanTrack()) pos_nhits*=2;
  cf_.apply("clusters_within_1.45ns", cluster_tdiff < 1.45);
  cf_.apply("electron_below_1.75GeV", ele.getTrack().getP() < 1.75);
  cf_.apply("vertex_chi2", vtx.getChi2() < 20.0);
  cf_.apply("ele_track_chi2ndf", ele.getTrack().getChi2Ndf() < 20.0);
  cf_.apply("pos_track_chi2ndf", pos.getTrack().getChi2Ndf() < 20.0);
  cf_.apply("ele_min_8_hits", ele_nhits > 7);
  cf_.apply("pos_min_8_hits", pos_nhits > 7);
  // left timing until last since it is having the biggest effect besides
  // requiring a vertex
  double ele_track_cluster_tdiff{
        abs(ele.getTrack().getTrackTime()-ele.getCluster().getTime())
  };
  double pos_track_cluster_tdiff{
        abs(pos.getTrack().getTrackTime()-pos.getCluster().getTime())
  };
  cf_.apply("ele_track_before_6ns", ele.getTrack().getTrackTime() < 6.0);
  cf_.apply("pos_track_before_6ns", pos.getTrack().getTrackTime() < 6.0);
  cf_.apply("ele_track_cluster_within_4ns", ele_track_cluster_tdiff < 4.0);
  cf_.apply("pos_track_cluster_within_4ns", pos_track_cluster_tdiff < 4.0);

  cf_.fill_nm1("clusters_within_1.45ns", cluster_tdiff);
  cf_.fill_nm1("electron_below_1.75GeV", ele.getTrack().getP());
  cf_.fill_nm1("vertex_chi2", vtx.getChi2());
  cf_.fill_nm1("ele_track_chi2ndf", ele.getTrack().getChi2Ndf());
  cf_.fill_nm1("pos_track_chi2ndf", pos.getTrack().getChi2Ndf());
  cf_.fill_nm1("ele_min_8_hits", ele_nhits);
  cf_.fill_nm1("pos_min_8_hits", pos_nhits);
  cf_.fill_nm1("ele_track_before_6ns", ele.getTrack().getTrackTime());
  cf_.fill_nm1("pos_track_before_6ns", pos.getTrack().getTrackTime());
  cf_.fill_nm1("ele_track_cluster_within_4ns", ele_track_cluster_tdiff);
  cf_.fill_nm1("pos_track_cluster_within_4ns", pos_track_cluster_tdiff);

  if (not cf_.keep()) {
    return true;
  }
   */

  /**
   * This is where the output TTree is filled,
   * if we leave before this point, then the event will not
   * be kept as part of pre-selection.
   *
   * We wait until here to copy over the MCParticles to avoid
   * unnecessary copying if the event is not going to be kept
   * anyways.
   */
  if (bus_.has(mcColl_)) {
    const auto& mc_ptr{bus_.get<std::vector<MCParticle*>>(mcColl_)};
    std::vector<MCParticle> mc;
    mc.reserve(mc_ptr.size());
    for (MCParticle* ptr : mc_ptr) {
      mc.push_back(*ptr);
    }
    bus_.set("mc", mc);
  }
  output_tree_->Fill();
  return true;
}

void PreSelectAndCategorize::finalize() {
  outF_->cd();
  output_tree_->Write();
  n_vertices_h_->Write();
  cf_.save();
}

DECLARE_PROCESSOR(PreSelectAndCategorize);
