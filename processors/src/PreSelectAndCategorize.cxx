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
                (name_+"_nm1_"+name+"_h").c_str(),
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
  void board_output(std::vector<TTree*> trees, const std::string& name) {
    if (bus_.isOnBoard(name)) {
      throw std::runtime_error(name+" is already on board the bus.");
    }
    bus_.board<BaggageType>(name);
    for (TTree* tree: trees) {
      if (bus_.attach(tree, name, true) == 0) {
        throw std::runtime_error("Unable to create branch "+name+" in output TTree.");
      }
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
  std::vector<std::string> rec_cat_names_{"l1l1","l1l2"};
  std::map<std::string, std::unique_ptr<TTree>> output_trees_;

  std::string vtxColl_{"UnconstrainedV0Vertices_KF"};
  std::string mcColl_{"MCParticle"}; //!< description
  bool isData_{false};
  bool isSignal_{false};
  double calTimeOffset_{0.0};

  Cutflow vertex_cf_{"vertex","reconstructed"};
  std::map<std::string, Cutflow> event_cf_;
  std::map<std::string, std::unique_ptr<TH1F>> n_vertices_h_;
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
  vertex_cf_.add("vtx_max_p_2.4GeV", 300, 0.0, 3.0);
  vertex_cf_.init();
  
  /* event selection after vertex selection */
  for (const auto& rcname: rec_cat_names_) {
    auto [it, inserted] = event_cf_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(rcname),
        std::forward_as_tuple(rcname+"_event","readout")
    );
    event_cf_.at(rcname).add("pair1trigger", 2, -0.5, 1.5);
    event_cf_.at(rcname).add("at_least_one_"+rcname+"_vertex", 10, 0.0, 10.0);
    event_cf_.at(rcname).add("no_extra_"+rcname+"_vertices", 10, 0.0, 10.0);
    if (isSignal_) {
      event_cf_.at(rcname).add("at_least_one_true_vd", 3, 0.0, 2.0);
      event_cf_.at(rcname).add("no_extra_true_vd", 3, 0.0, 2.0);
    }
    event_cf_.at(rcname).init();
  }

  std::vector<std::string> count_names{"readout","preselected","l1l1","l1l2"};
  for (const auto& cn : count_names) {
    n_vertices_h_[cn] = std::make_unique<TH1F>(
        ("n_"+cn+"_vertices_h").c_str(),
        ("N "+cn+" Vertices in Event").c_str(),
        10,-0.5,9.5
    );
  }
}

void PreSelectAndCategorize::setFile(TFile* out_file) {
  Processor::setFile(out_file);
  // create output TTree in output file
  std::vector<TTree*> tree_handles;
  tree_handles.reserve(2);
  for (const auto& rc: rec_cat_names_) {
    output_trees_.emplace(
        rc,
        std::make_unique<TTree>(
          (rc+"_preselection").c_str(),
          (rc+" PreSelected and Categorized Vertices").c_str(),
          99,
          out_file
        )
    );
    tree_handles.push_back(output_trees_[rc].get());
  }
  bus_.board_output<double>(tree_handles, "weight");
  bus_.board_output<Vertex>(tree_handles, "vertex");
  bus_.board_output<Particle>(tree_handles, "ele");
  bus_.board_output<Particle>(tree_handles, "pos");
  for (const auto& name : {"eleL1","eleL2","posL1","posL2"}) {
    bus_.board_output<bool>(tree_handles, name);
  }
  if (not v0proj_fits_.empty()) {
    for (const auto& name : {
        "vtx_proj_sig",
        "vtx_proj_x", "vtx_proj_x_sig",
        "vtx_proj_y", "vtx_proj_y_sig" 
    }) {
      bus_.board_output<double>(tree_handles, name);
    }
  }
  if (bus_.has(mcColl_) and isSignal_) {
    bus_.board_output<MCParticle>(tree_handles, "true_vd");
    bus_.board_output<bool>(tree_handles, "isRadEle");
  }
}

class RecoCategory {
  bool eleL1;
  bool eleL2;
  bool posL1;
  bool posL2;
 public:
  RecoCategory(std::shared_ptr<AnaHelpers> ah, Track ele, Track pos) {
    ah->InnermostLayerCheck(&ele, this->eleL1, this->eleL2);
    ah->InnermostLayerCheck(&pos, this->posL1, this->posL2);
  }
  bool l1l1() const {
    return eleL1 and posL1;
  }
  bool l1l2() const {
    //return (not l1l1()) and (eleL1 and posL2) or (eleL2 and posL1);
    return (not l1l1()) and (eleL1 or posL1);
  }
  void set(EventBus& bus) const {
    bus.set("eleL1", eleL1);
    bus.set("eleL2", eleL2);
    bus.set("posL1", posL1);
    bus.set("posL2", posL2);
  }
};

bool PreSelectAndCategorize::process(IEvent*) {
  const auto& eh{bus_.get<EventHeader>("EventHeader")};
  int run_number = eh.getRunNumber();

  bool pair1trigger = (not isData_ or eh.isPair1Trigger());
  for (auto& [rcname, cf]: event_cf_) {
    cf.begin_event();
    // re-apply trigger desicion to data sample since it contains
    // other triggers within it
    // MC is created with only this trigger AND the event header
    // is not updated so we need to skip this check for MC
    cf.apply("pair1trigger", pair1trigger);
  }

  if (not pair1trigger) {
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
  std::map<std::string, std::vector<std::tuple<Vertex,Particle,Particle,RecoCategory>>> preselected_vtx;
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
    TVector3 ele_mom(
        ele.getTrack().getMomentum()[0],
        ele.getTrack().getMomentum()[1],
        ele.getTrack().getMomentum()[2]
    );
    TVector3 pos_mom(
        pos.getTrack().getMomentum()[0],
        pos.getTrack().getMomentum()[1],
        pos.getTrack().getMomentum()[2]
    );


    vertex_cf_.begin_event();
    vertex_cf_.apply("abs_ele_track_before_6ns", abs(ele.getTrack().getTrackTime()) <= 6.0);
    vertex_cf_.apply("abs_pos_track_before_6ns", abs(pos.getTrack().getTrackTime()) <= 6.0);
    vertex_cf_.apply("clusters_within_1.45ns", cluster_tdiff <= 1.45);
    vertex_cf_.apply("ele_track_cluster_within_4ns", ele_track_cluster_tdiff <= 4.0);
    vertex_cf_.apply("pos_track_cluster_within_4ns", pos_track_cluster_tdiff <= 4.0);
    vertex_cf_.apply("ele_track_chi2ndf", ele.getTrack().getChi2Ndf() <= 20.0);
    vertex_cf_.apply("pos_track_chi2ndf", pos.getTrack().getChi2Ndf() <= 20.0);
    vertex_cf_.apply("electron_below_1.75GeV", ele.getTrack().getP() <= 1.75);
    vertex_cf_.apply("ele_min_8_hits", ele_nhits >= 7);
    vertex_cf_.apply("pos_min_8_hits", pos_nhits >= 7);
    vertex_cf_.apply("vertex_chi2", vtx->getChi2() <= 20.0);
    double vtxmaxp = (ele_mom+pos_mom).Mag();
    vertex_cf_.apply("vtx_max_p_2.4GeV", vtxmaxp <= 2.4);
  
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
    vertex_cf_.fill_nm1("vtx_max_p_2.4GeV", vtxmaxp);
  
    if (vertex_cf_.keep()) {
      RecoCategory rc(_ah, ele.getTrack(), pos.getTrack());
      std::string name{"other"};
      if (rc.l1l1()) {
        name = "l1l1";
      } else if (rc.l1l2()) {
        name = "l1l2";
      }
      preselected_vtx[name].emplace_back(*vtx, ele, pos, rc);
    }
  }

  std::size_t n_preselected{0};
  for (const auto& [rcname, vertices]: preselected_vtx) {
    n_preselected += vertices.size();
  }

  n_vertices_h_["readout"]->Fill(vtxs.size());
  n_vertices_h_["preselected"]->Fill(n_preselected);
  n_vertices_h_["l1l1"]->Fill(preselected_vtx["l1l1"].size());
  n_vertices_h_["l1l2"]->Fill(preselected_vtx["l1l2"].size());

  for (auto& [rcname, tree] : output_trees_) {
    auto& vertices{preselected_vtx[rcname]};
    event_cf_.at(rcname).apply("at_least_one_"+rcname+"_vertex", vertices.size() > 0);
    event_cf_.at(rcname).apply("no_extra_"+rcname+"_vertices", vertices.size() < 2);
    if (not event_cf_.at(rcname).keep()) {
      // drop event and do not fill output tree for this reco category
      continue;
    }
  
    // correct number of vertices (i.e. only one)
    // unpack the vector of vertices into the single elements
    auto [ vtx, ele, pos, rc ] = vertices.at(0);
    
    // copy reco-category booleans into output bus
    rc.set(bus_);
  
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
      for (const auto& [layer_id, particle_id] : ele.getTrack().getMcpHits()) {
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
       * The removal of these events are still counted within the signal cutflow
       * so they can be confirmed to have a small impact.
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
      event_cf_.at(rcname).apply("at_least_one_true_vd", n_vd > 0);
      event_cf_.at(rcname).apply("no_extra_true_vd", n_vd < 2);
      if (not event_cf_.at(rcname).keep()) {
        continue;
      }
      if (not vd) {
        throw std::runtime_error(
            "ERROR: Logic error: checked for VD earlier but there isn't one."
        );
      }
      bus_.set("true_vd", *vd);
      bus_.set("isRadEle", ele_is_rad_ele);
    } // if signal, get more truth information

    tree->Fill();
  } // loop over reco categories

  return true;
}

void PreSelectAndCategorize::finalize() {
  outF_->cd();
  for (auto& [rcname, tree]: output_trees_) {
    tree->Write();
  }
  for (auto& [_name, h]: n_vertices_h_) {
    h->Write();
  }
  vertex_cf_.save();
  for (auto& [rcname, cf]: event_cf_) {
    cf.save();
  }
}

DECLARE_PROCESSOR(PreSelectAndCategorize);
