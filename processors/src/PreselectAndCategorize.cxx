/*
 * @file PreselectAndCategorize.cxx
 * @author Tom Eichlersmith
 * @date Dec 2024
 * @author Sarah Gaiser
 * @date Apr 2025
 */

#include "PreselectAndCategorize.h"


void PreselectAndCategorize::configure(const ParameterSet& parameters) {
    auto pSmearingFile = parameters.getString("pSmearingFile");
    auto beamPosCfg = parameters.getString("beamPosCfg");
    auto v0ProjectionFitsCfg = parameters.getString("v0ProjectionFitsCfg");
    auto trackBiasCfg = parameters.getString("trackBiasCfg");

    calTimeOffset_ = parameters.getDouble("calTimeOffset");
    isData_ = parameters.getInteger("isData") != 0;
    isSignal_ = parameters.getInteger("isSignal") != 0;
}

void PreselectAndCategorize::initialize(TTree* tree) {
    _ah =  std::make_shared<AnaHelpers>();

    //init Reading Tree
    bus_.board_input<EventHeader>(tree, "EventHeader");
    bus_.board_input<std::vector<Vertex*>>(tree, vtxColl_);
    if(not isData_ and not mcColl_.empty())
        bus_.board_input<std::vector<MCParticle*>>(tree, mcColl_);
    
    /* pre-selection on vertices */
    vertex_cf_.add("ele_track_chi2ndf", 100, 0.0, 20.0);
    vertex_cf_.add("pos_track_chi2ndf", 100, 0.0, 20.0);
    vertex_cf_.add("vertex_chi2", 100,0.0,50.0);
    vertex_cf_.init();
    
    /* event selection after vertex selection */
    event_cf_.add("at_least_one_vertex", 10, 0.0, 10.0);
    event_cf_.add("no_extra_vertices", 10, 0.0, 10.0);
    event_cf_.init();

    n_vertices_h_ = std::make_unique<TH2F>(
        "n_vertices_h",
        "N Vertices in Event (readout and preselected)",
        10,-0.5,9.5,
        10,-0.5,9.5
    );
}

void PreselectAndCategorize::setFile(TFile* out_file) {
    Processor::setFile(out_file);
    
    // create output TTree in output file
    output_tree_ = std::make_unique<TTree>("preselection",
                                           "Preselected and Categorized Vertices",
                                           99, out_file);
    bus_.board_output<double>(output_tree_.get(), "weight");
    bus_.board_output<Vertex>(output_tree_.get(), "vertex");
    bus_.board_output<Particle>(output_tree_.get(), "ele");
    bus_.board_output<Particle>(output_tree_.get(), "pos");
    bus_.board_output<double>(output_tree_.get(), "psum");

    for (const auto& name : {"eleL1", "eleL2", "posL1", "posL2"}) {
        bus_.board_output<bool>(output_tree_.get(), name);
    }

    if (not v0proj_fits_.empty()) {
        for (const auto& name : {"vtx_proj_sig", "vtx_proj_x", "vtx_proj_x_sig",
                                 "vtx_proj_y", "vtx_proj_y_sig"}) {
            bus_.board_output<double>(output_tree_.get(), name);
        }
    }
}

bool PreselectAndCategorize::process(IEvent*) {
    const auto& eh{bus_.get<EventHeader>("EventHeader")};
    int run_number = eh.getRunNumber();

    event_cf_.begin_event();

    const auto& vtxs{bus_.get<std::vector<Vertex*>>(vtxColl_)};

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
    
        // put tracks back into their particles
        // with their new data
        ele.setTrack(&ele_trk);
        pos.setTrack(&pos_trk);

        vertex_cf_.begin_event();
        vertex_cf_.apply("ele_track_chi2ndf", ele.getTrack().getChi2Ndf() <= 20.0);
        vertex_cf_.apply("pos_track_chi2ndf", pos.getTrack().getChi2Ndf() <= 20.0);
        vertex_cf_.apply("vertex_chi2", vtx->getChi2() <= 20.0);
    
        vertex_cf_.fill_nm1("ele_track_chi2ndf", ele.getTrack().getChi2Ndf());
        vertex_cf_.fill_nm1("pos_track_chi2ndf", pos.getTrack().getChi2Ndf());
        vertex_cf_.fill_nm1("vertex_chi2", vtx->getChi2());
    
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
    bool eleL1{false}, eleL2{false}, posL1{false}, posL2{false};
    Track ele_trk{ele.getTrack()}, pos_trk{pos.getTrack()};
    _ah->InnermostLayerCheck(&ele_trk, eleL1, eleL2);
    _ah->InnermostLayerCheck(&pos_trk, posL1, posL2);
    bus_.set("eleL1", eleL1);
    bus_.set("eleL2", eleL2);
    bus_.set("posL1", posL1);
    bus_.set("posL2", posL2);

    TVector3 ele_mom(
        ele_trk.getMomentum()[0],
        ele_trk.getMomentum()[1],
        ele_trk.getMomentum()[2]
    );
    TVector3 pos_mom(
        pos_trk.getMomentum()[0],
        pos_trk.getMomentum()[1],
        pos_trk.getMomentum()[2]
    );
    bus_.set("psum", ele_mom.Mag() + pos_mom.Mag());

    bus_.set("weight", 1.);
    bus_.set("vertex", vtx);
    bus_.set("ele", ele);
    bus_.set("pos", pos);

    output_tree_->Fill();
    return true;
}

void PreselectAndCategorize::finalize() {
    outF_->cd();
    output_tree_->Write();
    n_vertices_h_->Write();
    vertex_cf_.save();
    event_cf_.save();
}

DECLARE_PROCESSOR(PreselectAndCategorize);
