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
  std::string name_, nocut_name_;
  std::unique_ptr<TH1F> h_cutflow_;
  std::map<std::string,std::pair<std::size_t,std::unique_ptr<TH1F>>> cuts_;
  bool keep_;
  std::vector<std::optional<bool>> cut_desc_;
 public:
  Cutflow(const std::string& name, const std::string& nocut_name)
    : name_{name}, nocut_name_{nocut_name} {}
  void add(const std::string& name, int nbins, float min, float max) {
    if (h_cutflow_) {
      throw std::runtime_error("Cannot add more cuts after init");
    }
    auto h = std::make_unique<TH1F>(
                ("nm1_"+name+"_h").c_str(),
                (name+" N-1").c_str(),
                nbins, min, max
            );
    cuts_.emplace(
        name,
        std::make_pair<std::size_t, std::unique_ptr<TH1F>>(
          cuts_.size(),
          std::move(h)
        )
    );
  }
  void init() {
    if (h_cutflow_) {
      throw std::runtime_error("Cannot init twice.");
    }
    h_cutflow_ = std::make_unique<TH1F>(
        (name_+"_cutflow_h").c_str(),
        (name_+"Cutflow").c_str(),
        cuts_.size()+1, -1.5, cuts_.size()-0.5
    );
    cut_desc_.resize(cuts_.size());
  }
  void begin_event() {
    h_cutflow_->Fill(nocut_name_.c_str(), 1.);
    keep_ = true;
    for (auto& d : cut_desc_) d.reset();
  }
  void apply(const std::string& name, bool descision) {
    if (cuts_.find(name) == cuts_.end()) {
      throw std::runtime_error("Cut "+name+" not `add`ed to cutflow.");
    }
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
  bool isSignal_{false};
  double calTimeOffset_{0.0};

  Cutflow vertex_cf_{"vertex","reconstructed"};
  Cutflow event_cf_{"event","readout"};
  std::unique_ptr<TH2F> n_vertices_h_;
  std::shared_ptr<TrackSmearingTool> smearingTool_;
  std::shared_ptr<AnaHelpers> _ah; //!< description

  /**
   * Map of beamspot by run, currently not used but available I guess??
   */
  std::map<int,std::vector<double>> beamspot_corrections_;

  /// config for projecting vertex candidates back to target
  json v0proj_fits_;

  /**
   * Map of correction name to correction value for analysis-level
   * corrections on tracks.
   */
  std::map<std::string,double> track_corrections_;
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
    auto beamspot_corrections_json = json_load(beamPosCfg);
    for (const auto& [run, entry]: beamspot_corrections_json.items()) {
      beamspot_corrections_[std::stoi(run)] = {
        entry["beamspot_x"],
        entry["beamspot_y"],
        entry["beamspot_z"]
      };
      /**
       * Alic has updated to be what is shown below,
       * but in the same update, stopped using the beamspot corrections
       * anywhere in the analyzer
      beamspot_corrections_[std::stoi(run)] = {
        entry["unrotated_mean_x"],
        entry["unrotated_mean_y"]
      };
       */
    }
  }

  auto v0ProjectionFitsCfg = parameters.getString("v0ProjectionFitsCfg");
  if (not v0ProjectionFitsCfg.empty()) {
    std::cout << "Loading projection fits from " << v0ProjectionFitsCfg << std::endl;
    v0proj_fits_ = json_load(v0ProjectionFitsCfg);
  }

  auto trackBiasCfg = parameters.getString("trackBiasCfg");
  if (not trackBiasCfg.empty()) {
    std::cout << "Loading track corrections from " << trackBiasCfg << std::endl;
    auto track_corr = json_load(trackBiasCfg);
    std::cout << track_corr << std::endl;
    for (const auto& [name, corr]: track_corr.items()) {
      // validate names of corrections, Track::applyCorrection silently ignores
      // names that don't exactly match the names within it, so we check for
      // misspellings once here
      if (name != "track_z0" and name != "track_time") {
        std::cerr << "WARNING: Unrecognized track correction "
          << name << " : " << corr
          << " will be skipped."
          << std::endl;
        continue;
      }
      track_corrections_[name] = corr;
    }
  }

  calTimeOffset_ = parameters.getDouble("calTimeOffset");

  isData_ = parameters.getInteger("isData") != 0;
  isSignal_ = parameters.getInteger("isSignal") != 0;
}

void PreSelectAndCategorize::initialize(TTree* tree) {
  _ah =  std::make_shared<AnaHelpers>();
  //init Reading Tree
  bus_.board_input<EventHeader>(tree, "EventHeader");
  bus_.board_input<std::vector<Vertex*>>(tree, vtxColl_);
  if(not isData_ and not mcColl_.empty())
    bus_.board_input<std::vector<MCParticle*>>(tree, mcColl_);
  /* pre-selection on vertices */
  vertex_cf_.add("abs_ele_track_before_6ns", 120, 0.0, 24.0);
  vertex_cf_.add("abs_pos_track_before_6ns", 120, 0.0, 24.0);
  vertex_cf_.add("clusters_within_1.45ns", 100,0.0,10.0);
  vertex_cf_.add("ele_track_cluster_within_4ns", 80, 0.0, 16.0);
  vertex_cf_.add("pos_track_cluster_within_4ns", 80, 0.0, 16.0);
  vertex_cf_.add("ele_track_chi2ndf", 100, 0.0, 20.0);
  vertex_cf_.add("pos_track_chi2ndf", 100, 0.0, 20.0);
  vertex_cf_.add("electron_below_1.75GeV", 230,0.0,2.3);
  vertex_cf_.add("ele_min_8_hits", 14, 0, 14);
  vertex_cf_.add("pos_min_8_hits", 14, 0, 14);
  vertex_cf_.add("vertex_chi2", 100,0.0,50.0);
  vertex_cf_.add("psum_lt_2.4GeV", 300, 0.0, 3.0);
  vertex_cf_.init();
  
  /* event selection after vertex selection */
  event_cf_.add("pair1trigger", 2, -0.5, 1.5);
  event_cf_.add("at_least_one_vertex", 10, 0.0, 10.0);
  event_cf_.add("no_extra_vertices", 10, 0.0, 10.0);
  if (isSignal_) {
    event_cf_.add("at_least_one_true_vd", 3, 0.0, 2.0);
    event_cf_.add("no_extra_true_vd", 3, 0.0, 2.0);
  }
  event_cf_.init();

  n_vertices_h_ = std::make_unique<TH2F>(
      "n_vertices_h",
      "N Vertices in Event (readout and preselected)",
      10,-0.5,9.5,
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
  if (bus_.has(mcColl_) and isSignal_) {
    bus_.board_output<MCParticle>(output_tree_.get(), "true_vd");
    bus_.board_output<bool>(output_tree_.get(), "isRadEle");
  }
}

bool PreSelectAndCategorize::process(IEvent*) {
  const auto& eh{bus_.get<EventHeader>("EventHeader")};
  int run_number = eh.getRunNumber();

  event_cf_.begin_event();

  // re-apply trigger desicion to data sample since it contains
  // other triggers within it
  // MC is created with only this trigger AND the event header
  // is not updated so we need to skip this check for MC
  event_cf_.apply("pair1trigger", (not isData_ or eh.isPair1Trigger()));
  if (not event_cf_.keep()) {
    // we leave BEFORE filling the vertex counting histogram
    // so that the vertex count histogram is relative to this trigger
    // and does not include other triggers
    return true;
  }

  const auto& vtxs{bus_.get<std::vector<Vertex*>>(vtxColl_)};
  /**
   * pre-selection on vertices defining "quality" vertices
   *
   * In this process of pre-selecting vertices, we also apply corrections
   * and momentum smearing to the underlying particle tracks and clusters.
   * These are then stored in our new copy of these objects in the resulting
   * preselected_vtx collection (vtx, ele, pos).
   *
   * We need to keep the particles separately since the particles "stored"
   * within Vertex are references to the collections. We make a copy here
   * to apply corrections and do not modify the collections referenced
   * elsewhere in memory.
   */
  std::vector<std::tuple<Vertex,Particle,Particle>> preselected_vtx;
  for (Vertex* vtx : vtxs) {
    // access the indiviual Vertex, electron, and positron
    // and add corrections to them before applying pre-selection
    int i_ele{-1}, i_pos{-1};
    for (int ipart = 0; ipart < vtx->getParticles().GetEntries(); ++ipart) {
      int pdg_id = ((Particle*)vtx->getParticles().At(ipart))->getPDG();
      if (pdg_id == 11) {
        i_ele = ipart;
      } else if (pdg_id == -11) {
        i_pos = ipart;
      }
    }
    if (i_ele < 0 or i_pos < 0) {
      throw std::runtime_error("Vertex formed without either an electron or positron!");
    }

    Particle ele = *dynamic_cast<Particle*>(vtx->getParticles().At(i_ele));
    Particle pos = *dynamic_cast<Particle*>(vtx->getParticles().At(i_pos));

    // for both particles, get cluster, update, re-set cluster
    CalCluster ele_clu = ele.getCluster();
    CalCluster pos_clu = pos.getCluster();

    ele_clu.setTime(ele_clu.getTime() - calTimeOffset_);
    pos_clu.setTime(pos_clu.getTime() - calTimeOffset_);
  
    ele.setCluster(&ele_clu);
    pos.setCluster(&pos_clu);
  
    Track ele_trk = ele.getTrack();
    Track pos_trk = pos.getTrack();

    // apply track_z0 and track_time corrections loaded from JSON
    for (const auto& [name, corr]: track_corrections_) {
      ele_trk.applyCorrection(name, corr);
      pos_trk.applyCorrection(name, corr);
    }
  
    // smear track momentum
    if (smearingTool_) {
      double ele_smear = smearingTool_->updateWithSmearP(ele_trk);
      double pos_smear = smearingTool_->updateWithSmearP(pos_trk);
      smearingTool_->updateVertexWithSmearP(vtx, ele_smear, pos_smear);
    }
  
    // put tracks back into their particles
    // with their new data
    ele.setTrack(&ele_trk);
    pos.setTrack(&pos_trk);

    // corrections have been applied, now onto pre-selecting

    double cluster_tdiff{abs(ele.getCluster().getTime()-pos.getCluster().getTime())};
    int ele_nhits = ele.getTrack().getTrackerHitCount();
    if (not ele.getTrack().isKalmanTrack()) ele_nhits*=2;
    int pos_nhits = pos.getTrack().getTrackerHitCount();
    if (not pos.getTrack().isKalmanTrack()) pos_nhits*=2;
    double ele_track_cluster_tdiff{
          abs(ele.getTrack().getTrackTime()-ele.getCluster().getTime())
    };
    double pos_track_cluster_tdiff{
          abs(pos.getTrack().getTrackTime()-pos.getCluster().getTime())
    };

    vertex_cf_.begin_event();
    vertex_cf_.apply("abs_ele_track_before_6ns", abs(ele.getTrack().getTrackTime()) < 6.0);
    vertex_cf_.apply("abs_pos_track_before_6ns", abs(pos.getTrack().getTrackTime()) < 6.0);
    vertex_cf_.apply("clusters_within_1.45ns", cluster_tdiff < 1.45);
    vertex_cf_.apply("ele_track_cluster_within_4ns", ele_track_cluster_tdiff < 4.0);
    vertex_cf_.apply("pos_track_cluster_within_4ns", pos_track_cluster_tdiff < 4.0);
    vertex_cf_.apply("ele_track_chi2ndf", ele.getTrack().getChi2Ndf() < 20.0);
    vertex_cf_.apply("pos_track_chi2ndf", pos.getTrack().getChi2Ndf() < 20.0);
    vertex_cf_.apply("electron_below_1.75GeV", ele.getTrack().getP() < 1.75);
    vertex_cf_.apply("ele_min_8_hits", ele_nhits > 7);
    vertex_cf_.apply("pos_min_8_hits", pos_nhits > 7);
    vertex_cf_.apply("vertex_chi2", vtx->getChi2() < 20.0);
    vertex_cf_.apply("psum_lt_2.4GeV", ele.getTrack().getP()+pos.getTrack().getP() < 2.4);
  
    vertex_cf_.fill_nm1("abs_ele_track_before_6ns", abs(ele.getTrack().getTrackTime()));
    vertex_cf_.fill_nm1("abs_pos_track_before_6ns", abs(pos.getTrack().getTrackTime()));
    vertex_cf_.fill_nm1("clusters_within_1.45ns", cluster_tdiff);
    vertex_cf_.fill_nm1("ele_track_cluster_within_4ns", ele_track_cluster_tdiff);
    vertex_cf_.fill_nm1("pos_track_cluster_within_4ns", pos_track_cluster_tdiff);
    vertex_cf_.fill_nm1("ele_track_chi2ndf", ele.getTrack().getChi2Ndf());
    vertex_cf_.fill_nm1("pos_track_chi2ndf", pos.getTrack().getChi2Ndf());
    vertex_cf_.fill_nm1("electron_below_1.75GeV", ele.getTrack().getP());
    vertex_cf_.fill_nm1("ele_min_8_hits", ele_nhits);
    vertex_cf_.fill_nm1("pos_min_8_hits", pos_nhits);
    vertex_cf_.fill_nm1("vertex_chi2", vtx->getChi2());
    vertex_cf_.fill_nm1("psum_lt_2.4GeV", ele.getTrack().getP()+pos.getTrack().getP());
  
    if (vertex_cf_.keep()) {
      preselected_vtx.emplace_back(*vtx, ele, pos);
    }
  }

  n_vertices_h_->Fill(vtxs.size(), preselected_vtx.size());
  event_cf_.apply("at_least_one_vertex", preselected_vtx.size() > 0);
  event_cf_.apply("no_extra_vertices", preselected_vtx.size() < 2);
  if (not event_cf_.keep()) {
    return true;
  }
  
  // correct number of vertices (i.e. only one)
  // unpack the vector of vertices into the single elements
  auto [ vtx, ele, pos ] = preselected_vtx.at(0);

  // earliest layer hit categories
  bool eleL1{false}, eleL2{false},
       posL1{false}, posL2{false};
  Track ele_trk{ele.getTrack()}, pos_trk{pos.getTrack()};
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
  bus_.set("ele", ele);
  bus_.set("pos", pos);

  /**
   * This is where the output TTree is filled,
   * if we leave before this point, then the event will not
   * be kept as part of pre-selection.
   *
   * We wait until here to copy over the MCParticles to avoid
   * unnecessary copying if the event is not going to be kept
   * anyways.
   */
  if (bus_.has(mcColl_) and isSignal_) {
    /**
     * Before we loop through the MCParticles we go through the
     * the hits on the electron track in this vertex and find out
     * which MCParticle has the most hits on the track.
     */
    std::map<int, int> count_per_particle_id;
    for (const auto& [layer_id, particle_id] : ele_trk.getMcpHits()) {
      if (count_per_particle_id.find(particle_id) == count_per_particle_id.end()) {
        count_per_particle_id[particle_id] = 0;
      }
      count_per_particle_id[particle_id]++;
    }
    int truth_ele_id{-1}, max_nhits{0};
    for (const auto& [particle_id, count] : count_per_particle_id) {
      if (count > max_nhits) {
        truth_ele_id = particle_id;
        max_nhits = count;
      }
    }
    /**
     * The implementation of the beam-overlay mechanism very rarely causes two
     * signal events to occur within the same software event (or no signal event
     * to happen at all). The complexity of handling these cases is too high and
     * the rarity (both in simulation and expected due to small cross section)
     * motivates just removing these events from the sample.
     *
     * The warnings are still printed so an estimate on the number of events being
     * dropped can be retrieved from the logs.
     */
    const auto& mc_ptr{bus_.get<std::vector<MCParticle*>>(mcColl_)};
    MCParticle* vd{nullptr};
    int n_vd{0};
    bool ele_is_rad_ele{false};
    for (MCParticle* ptr : mc_ptr) {
      if (ptr->getPDG() == 625) {
        n_vd++;
        vd = ptr;
      } else if (ptr->getID() == truth_ele_id) {
        ele_is_rad_ele = (ptr->getMomPDG() == 625);
      }
    }
    event_cf_.apply("at_least_one_true_vd", n_vd > 0);
    event_cf_.apply("no_extra_true_vd", n_vd < 2);
    if (not event_cf_.keep()) {
      return true;
    }
    if (not vd) {
      throw std::runtime_error(
          "ERROR: Logic error: checked for VD earlier but there isn't one."
      );
    }
    bus_.set("true_vd", *vd);
    bus_.set("isRadEle", ele_is_rad_ele);
  }
  output_tree_->Fill();
  return true;
}

void PreSelectAndCategorize::finalize() {
  outF_->cd();
  output_tree_->Write();
  n_vertices_h_->Write();
  vertex_cf_.save();
  event_cf_.save();
}

DECLARE_PROCESSOR(PreSelectAndCategorize);
