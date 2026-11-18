/**
 * @file VertexProcessor.h
 * @brief Class used to convert LCIO Vertex collections into ROOT collections.
 * @author Cameron Bravo, SLAC
 */
#include "VertexProcessor.h"

#include <cmath>
#include <iomanip>
#include <sstream>

#include "utilities.h"

VertexProcessor::VertexProcessor(const std::string& name, Process& process) : Processor(name, process) {}

VertexProcessor::~VertexProcessor() {}

void VertexProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring VertexProcessor" << std::endl;
    try {
        debug_ = parameters.getInteger("debug", debug_);
        vtxCollLcio_ = parameters.getString("vtxCollLcio", vtxCollLcio_);
        vtxCollRoot_ = parameters.getString("vtxCollRoot", vtxCollRoot_);
        partCollRoot_ = parameters.getString("partCollRoot", partCollRoot_);
        kinkRelCollLcio_ = parameters.getString("kinkRelCollLcio", kinkRelCollLcio_);
        trkRelCollLcio_ = parameters.getString("trkRelCollLcio", trkRelCollLcio_);
        trackStateLocation_ = parameters.getString("trackStateLocation", trackStateLocation_);
        hitFitsCollLcio_ = parameters.getString("hitFitsCollLcio", hitFitsCollLcio_);
        trkhitCollRoot_ = parameters.getString("trkhitCollRoot", trkhitCollRoot_);
        rawhitCollRoot_ = parameters.getString("rawhitCollRoot", rawhitCollRoot_);
        mcPartRelLcio_ = parameters.getString("mcPartRelLcio", mcPartRelLcio_);
        mcPartCollLcio_ = parameters.getString("mcPartCollLcio", mcPartCollLcio_);
        rawHitCollLcio_ = parameters.getString("rawHitCollLcio", rawHitCollLcio_);
        truthClosure_ = parameters.getInteger("truthClosure", truthClosure_);
        useTrackerHits_ = parameters.getInteger("useTrackerHits", useTrackerHits_);
        bfield_ = parameters.getDouble("bfield", bfield_);
    } catch (std::runtime_error& error) {
        std::cout << error.what() << std::endl;
    }
}

void VertexProcessor::initialize(TTree* tree) {
    // Add branches to tree
    tree->Branch(vtxCollRoot_.c_str(), &vtxs_);
    tree->Branch(partCollRoot_.c_str(), &parts_);
    if (!trkhitCollRoot_.empty()) tree->Branch(trkhitCollRoot_.c_str(), &hits_);

    if (!rawhitCollRoot_.empty()) tree->Branch(rawhitCollRoot_.c_str(), &rawhits_);
}

bool VertexProcessor::process(IEvent* ievent) {
    if (debug_ > 0) std::cout << "VertexProcessor: Clear output vector" << std::endl;

    if (hits_.size() > 0) {
        for (std::vector<TrackerHit*>::iterator it = hits_.begin(); it != hits_.end(); ++it) {
            delete *it;
        }
        hits_.clear();
    }

    if (rawhits_.size() > 0) {
        for (std::vector<RawSvtHit*>::iterator it = rawhits_.begin(); it != rawhits_.end(); ++it) {
            delete *it;
        }
        rawhits_.clear();
    }

    for (int i = 0; i < vtxs_.size(); i++) delete vtxs_.at(i);
    vtxs_.clear();
    for (int i = 0; i < parts_.size(); i++) delete parts_.at(i);
    parts_.clear();

    Event* event = static_cast<Event*>(ievent);
    UTIL::LCRelationNavigator* mcPartRel_nav;

    // Get the collection of vertices from the LCIO event. If no such collection
    // exist, a DataNotAvailableException is thrown
    if (debug_ > 0) std::cout << "VertexProcessor: Get LCIO Collection " << vtxCollLcio_ << std::endl;
    EVENT::LCCollection* lc_vtxs = nullptr;
    try {
        lc_vtxs = event->getLCCollection(vtxCollLcio_.c_str());
    } catch (EVENT::DataNotAvailableException e) {
        std::cout << e.what() << std::endl;
        return false;
    }

    // Get the collection of LCRelations between GBL tracks and kink data and track data variables.
    EVENT::LCCollection* gbl_kink_data{nullptr};
    EVENT::LCCollection* track_data{nullptr};

    try {
        if (!kinkRelCollLcio_.empty())
            gbl_kink_data = static_cast<EVENT::LCCollection*>(event->getLCCollection(kinkRelCollLcio_.c_str()));
        if (!trkRelCollLcio_.empty())
            track_data = static_cast<EVENT::LCCollection*>(event->getLCCollection(trkRelCollLcio_.c_str()));
    } catch (EVENT::DataNotAvailableException e) {
        std::cout << e.what() << std::endl;
        if (!gbl_kink_data) std::cout << "Failed retrieving " << kinkRelCollLcio_ << std::endl;
        if (!track_data) std::cout << "Failed retrieving " << trkRelCollLcio_ << std::endl;
    }

    bool rotateHits = true;
    int hitType = 0;
    EVENT::LCCollection* raw_svt_hit_fits = nullptr;

    auto evColls = event->getLCEvent()->getCollectionNames();
    auto it = std::find(evColls->begin(), evColls->end(), hitFitsCollLcio_.c_str());
    bool hasFits = true;
    if (it == evColls->end()) hasFits = false;
    if (hasFits) {
        raw_svt_hit_fits = event->getLCCollection(hitFitsCollLcio_.c_str());
    }
    // Check to see if MC Particles are in the file
    it = std::find(evColls->begin(), evColls->end(), mcPartRelLcio_.c_str());
    bool hasMCParts = true;
    EVENT::LCCollection* mcPartRel;
    if (it == evColls->end()) hasMCParts = false;
    if (hasMCParts) {
        mcPartRel = event->getLCCollection(mcPartRelLcio_.c_str());
        // Heap an LCRelation navigator which will allow faster access
        mcPartRel_nav = new UTIL::LCRelationNavigator(mcPartRel);
    }

    /**
     * Closure test bookkeeping. The map is the ground truth for "does this id
     * mean anything downstream": MCParticleProcessor stores exactly these ids
     * (MCParticleProcessor.cxx:99 also uses lc_particle->id()), so an id that is
     * absent here is one PreselectAndCategorize2021 can never resolve.
     */
    std::map<int, int> mcp_pdg;
    if (truthClosure_) {
        closure_.n_events++;
        if (hasMCParts) closure_.n_events_with_rel++;

        auto mcp_it = std::find(evColls->begin(), evColls->end(), mcPartCollLcio_.c_str());
        if (mcp_it != evColls->end()) {
            closure_.n_events_with_mcp++;
            EVENT::LCCollection* lc_mcps = event->getLCCollection(mcPartCollLcio_.c_str());
            for (int impc = 0; impc < lc_mcps->getNumberOfElements(); ++impc) {
                auto* mcp = static_cast<EVENT::MCParticle*>(lc_mcps->getElementAt(impc));
                mcp_pdg[mcp->id()] = mcp->getPDG();
            }
        }

        /**
         * Event-level control: what fraction of ALL raw hits carry a relation.
         * Without it the on-track number has nothing to be compared against --
         * under pulser overlay most raw hits are data and are expected to have
         * no relation at all.
         */
        auto raw_it = std::find(evColls->begin(), evColls->end(), rawHitCollLcio_.c_str());
        if (raw_it != evColls->end() and hasMCParts) {
            EVENT::LCCollection* lc_raws = event->getLCCollection(rawHitCollLcio_.c_str());
            closure_.n_raw_event += lc_raws->getNumberOfElements();
            for (int iraw = 0; iraw < lc_raws->getNumberOfElements(); ++iraw) {
                if (not mcPartRel_nav->getRelatedToObjects(lc_raws->getElementAt(iraw)).empty()) {
                    closure_.n_raw_event_with_rel++;
                }
            }
        }
    }

    if (debug_ > 0) std::cout << "VertexProcessor: Converting Verteces" << std::endl;
    for (int ivtx = 0; ivtx < lc_vtxs->getNumberOfElements(); ++ivtx) {
        if (debug_ > 0) std::cout << "VertexProcessor: Converting Vertex " << ivtx << std::endl;
        EVENT::Vertex* lc_vtx{nullptr};
        lc_vtx = static_cast<EVENT::Vertex*>(lc_vtxs->getElementAt(ivtx));

        if (debug_ > 0) std::cout << "VertexProcessor: Build Vertex" << std::endl;
        Vertex* vtx = utils::buildVertex(lc_vtx);

        if (debug_ > 0) std::cout << "VertexProcessor: Get Particles" << std::endl;
        std::vector<EVENT::ReconstructedParticle*> lc_parts = lc_vtx->getAssociatedParticle()->getParticles();
        for (auto lc_part : lc_parts) {
            if (debug_ > 0) std::cout << "VertexProcessor: Build particle" << std::endl;
            Particle* part = utils::buildParticle(lc_part, trackStateLocation_, gbl_kink_data, track_data);
            if (lc_part->getTracks().size() > 0) {
                EVENT::Track* lc_track = static_cast<EVENT::Track*>(lc_part->getTracks()[0]);
                if (debug_ > 0)
                    std::cout << "[VertexProcessor] calling buildTrack: vtx=" << ivtx
                              << " trackStateLocation=" << trackStateLocation_
                              << " has_gbl_kink=" << (gbl_kink_data ? "yes" : "no")
                              << " has_track_data=" << (track_data ? "yes" : "no") << std::endl;
                Track* track = utils::buildTrack(lc_track, trackStateLocation_, gbl_kink_data, track_data);
                int nHits = 0;
                if (bfield_ > 0.0) track->setMomentum(bfield_);
                if (track->isKalmanTrack()) hitType = 1;  // SiClusters

                if (!useTrackerHits_) {
                    auto hitPattern = lc_track->getSubdetectorHitNumbers();
                    for (int h = 0; h < hitPattern.size(); h++) {
                        if (hitPattern[h] > 0) {
                            track->addHitLayer(h);
                            nHits++;
                        }
                    }
                    if (nHits == 0) {
                        std::cout << "[VertexProcessor::process] WARNING Track with no hits -- likely "
                                     "getSubdetectorHitNumbers isn't filled!"
                                  << std::endl;
                        std::cout << "[VertexProcessor::process] WARNING If hit collections are saved, you may want to "
                                     "turn on useTrackerHits"
                                  << std::endl;
                    }
                    track->setTrackerHitCount(nHits);
                } else {
                    EVENT::TrackerHitVec lc_tracker_hits = lc_track->getTrackerHits();
                    for (auto lc_tracker_hit : lc_tracker_hits) {
                        TrackerHit* tracker_hit = utils::buildTrackerHit(
                            static_cast<IMPL::TrackerHitImpl*>(lc_tracker_hit), rotateHits, hitType);
                        std::vector<RawSvtHit*> rawSvthitsOn3d;
                        utils::addRawInfoTo3dHit(tracker_hit, static_cast<IMPL::TrackerHitImpl*>(lc_tracker_hit),
                                                 raw_svt_hit_fits, &rawSvthitsOn3d, hitType);

                        int hitLayer = tracker_hit->getLayer();
                        nHits++;
                        if (truthClosure_) closure_.n_trackerhits++;
                        EVENT::LCObjectVec rawHits = lc_tracker_hit->getRawHits();
                        if (truthClosure_) closure_.n_raw += rawHits.size();
                        for (int irawhit = 0; irawhit < rawHits.size(); ++irawhit) {
                            EVENT::LCObject* rawhit = rawHits.at(irawhit);
                            if (debug_ > 0) std::cout << "rawhit on track has lcio id: " << rawhit->id() << std::endl;

                            if (hasMCParts) {
                                // Get the list of fit params associated with the raw tracker hit
                                EVENT::LCObjectVec lc_simtrackerhits = mcPartRel_nav->getRelatedToObjects(rawhit);

                                if (truthClosure_) {
                                    if (not lc_simtrackerhits.empty()) closure_.n_raw_with_rel++;
                                    closure_.n_simhits += lc_simtrackerhits.size();
                                }

                                // Loop over SimTrackerHits to get MCParticles
                                for (int isimhit = 0; isimhit < lc_simtrackerhits.size(); isimhit++) {
                                    EVENT::SimTrackerHit* lc_simhit =
                                        static_cast<EVENT::SimTrackerHit*>(lc_simtrackerhits.at(isimhit));
                                    EVENT::MCParticle* lc_mcp = lc_simhit->getMCParticle();
                                    /**
                                     * A null MCParticle used to be printed and then dereferenced on
                                     * the next line. Skip it instead, and count it -- a SimTrackerHit
                                     * whose particle did not survive into the output is one of the
                                     * ways the truth link goes missing.
                                     */
                                    if (lc_mcp == nullptr) {
                                        if (truthClosure_) closure_.n_simhits_null_mcp++;
                                        if (debug_ > 0) std::cout << "mcp is null" << std::endl;
                                        continue;
                                    }
                                    if (truthClosure_) {
                                        closure_.n_expected_entries++;
                                        if (not mcp_pdg.empty() and mcp_pdg.count(lc_mcp->id()) == 0) {
                                            closure_.n_simhits_dangling++;
                                        }
                                    }
                                    track->addMcpHit(hitLayer, lc_mcp->id());
                                    if (debug_ > 0) {
                                        std::cout << "simtrackerhit lcio id: " << lc_simhit->id() << std::endl;
                                        std::cout << "mcp lcio id: " << lc_mcp->id() << std::endl;
                                    }
                                }
                            }
                        }  // Loop over raw tracker hits

                        track->addHitLayer(hitLayer);
                        track->addHit(tracker_hit);
                        hits_.push_back(tracker_hit);
                        for (std::vector<RawSvtHit*>::iterator it = rawSvthitsOn3d.begin(); it != rawSvthitsOn3d.end();
                             ++it) {
                            delete *it;
                        }
                        rawSvthitsOn3d.clear();

                    }  // Loop over hits on track

                    track->setTrackerHitCount(nHits);

                }  // !useTrackerHits

                part->setTrack(track);
                /**
                 * Close on the copy that is actually written out, not on the local
                 * Track: Particle::setTrack assigns by value, so this is the object
                 * PreselectAndCategorize2021 will read back from the tuple.
                 */
                if (truthClosure_ and useTrackerHits_) {
                    Track stored = part->getTrack();
                    closeTruthLink(&stored, mcp_pdg, not mcp_pdg.empty());
                }
                delete track;
            }
            // =========================================
            if (debug_ > 0) std::cout << "VertexProcessor: Add particle" << std::endl;
            parts_.push_back(part);
            vtx->addParticle(part);
        }

        if (debug_ > 0) std::cout << "VertexProcessor: Add Vertex" << std::endl;
        vtxs_.push_back(vtx);
    }

    if (hasMCParts) delete mcPartRel_nav;
    if (debug_ > 0) std::cout << "VertexProcessor: End process" << std::endl;
    return true;
}

void VertexProcessor::closeTruthLink(Track* track, const std::map<int, int>& mcp_pdg, bool have_mcp) {
    closure_.n_tracks++;

    const auto mcp_hits = track->getMcpHits();
    closure_.n_stored_entries += mcp_hits.size();

    if (mcp_hits.empty()) {
        closure_.n_trk_no_entry++;
        return;
    }

    /**
     * Reproduce the plurality exactly as PreselectAndCategorize2021.cxx:1104-1117
     * does -- over entries, so a wide cluster contributes once per strip and the
     * layer half of the pair is ignored. Recompute it a second time over unique
     * (layer, id) pairs to measure how often that weighting changes the answer.
     */
    std::map<int, int> per_strip;
    std::map<int, int> per_layer;
    std::set<std::pair<int, int>> seen;
    for (const auto& [layer_id, particle_id] : mcp_hits) {
        per_strip[particle_id]++;
        if (seen.insert(std::make_pair(layer_id, particle_id)).second) per_layer[particle_id]++;
    }

    auto plurality = [](const std::map<int, int>& counts) {
        int best_id{-1}, best_n{0};
        for (const auto& [particle_id, count] : counts) {
            if (count > best_n) {
                best_id = particle_id;
                best_n = count;
            }
        }
        return best_id;
    };

    const int strip_id = plurality(per_strip);
    if (strip_id != plurality(per_layer)) closure_.n_trk_plurality_differs++;

    if (not have_mcp) return;

    /**
     * The classification the downstream code cannot make for itself:
     * utils::getTruthPDG returns 0 both when the id resolves to nothing and when
     * it resolves to a non-e+-, and hasTruthMatch collapses both into "no truth
     * link" -- the same value a track with no relations at all gets.
     */
    auto found = mcp_pdg.find(strip_id);
    if (found == mcp_pdg.end()) {
        closure_.n_trk_dangling++;
    } else if (std::abs(found->second) != 11) {
        closure_.n_trk_not_electron++;
    } else {
        closure_.n_trk_electron++;
    }
}

void VertexProcessor::reportTruthLinkClosure() const {
    const auto& c = closure_;
    auto frac = [](long num, long den) {
        std::ostringstream os;
        if (den > 0) os << " (" << std::fixed << std::setprecision(4) << double(num) / double(den) << ")";
        return os.str();
    };

    std::cout << "\n===== VertexProcessor truth-link closure [" << vtxCollLcio_ << "] =====\n"
              << "  events                                : " << c.n_events << "\n"
              << "    with " << mcPartRelLcio_ << " : " << c.n_events_with_rel << "\n"
              << "    with " << mcPartCollLcio_ << " : " << c.n_events_with_mcp << "\n";

    if (c.n_events_with_rel == 0) {
        std::cout << "  no relation collection seen -- nothing to close.\n"
                  << "==========================================================\n" << std::endl;
        return;
    }

    std::cout << "  -- relation table, whole event (" << rawHitCollLcio_ << ") --\n"
              << "    raw hits                            : " << c.n_raw_event << "\n"
              << "    with >=1 relation                   : " << c.n_raw_event_with_rel
              << frac(c.n_raw_event_with_rel, c.n_raw_event) << "\n"
              << "  -- relation table, on track --\n"
              << "    tracks                              : " << c.n_tracks << "\n"
              << "    tracker hits                        : " << c.n_trackerhits << "\n"
              << "    raw hits                            : " << c.n_raw << "\n"
              << "    with >=1 relation                   : " << c.n_raw_with_rel
              << frac(c.n_raw_with_rel, c.n_raw) << "\n"
              << "    with 0 relations                    : " << (c.n_raw - c.n_raw_with_rel)
              << frac(c.n_raw - c.n_raw_with_rel, c.n_raw) << "\n"
              << "    SimTrackerHits reached              : " << c.n_simhits << "\n"
              << "      null MCParticle                   : " << c.n_simhits_null_mcp
              << frac(c.n_simhits_null_mcp, c.n_simhits) << "\n"
              << "      MCParticle id NOT in " << mcPartCollLcio_ << " : " << c.n_simhits_dangling
              << frac(c.n_simhits_dangling, c.n_simhits) << "\n"
              << "  -- closure: LCIO walk vs stored Track::getMcpHits() --\n"
              << "    entries from the walk               : " << c.n_expected_entries << "\n"
              << "    entries on the stored Track         : " << c.n_stored_entries << "\n"
              << "    "
              << (c.n_expected_entries == c.n_stored_entries ? "CLOSED" : "*** MISMATCH: entries were lost ***")
              << "\n"
              << "  -- what the plurality resolves to (what PreselectAndCategorize2021 sees) --\n"
              << "    tracks with no mcp-hit entry        : " << c.n_trk_no_entry
              << frac(c.n_trk_no_entry, c.n_tracks) << "\n"
              << "    plurality id not in MCParticle coll : " << c.n_trk_dangling
              << frac(c.n_trk_dangling, c.n_tracks) << "\n"
              << "    plurality id is not an e+-          : " << c.n_trk_not_electron
              << frac(c.n_trk_not_electron, c.n_tracks) << "\n"
              << "    plurality id IS an e+-              : " << c.n_trk_electron
              << frac(c.n_trk_electron, c.n_tracks) << "\n"
              << "    (the first three are indistinguishable downstream:\n"
              << "     utils::hasTruthMatch returns false for all of them)\n"
              << "  -- weighting cross-check --\n"
              << "    strip-level and layer-unique plurality disagree : " << c.n_trk_plurality_differs
              << frac(c.n_trk_plurality_differs, c.n_tracks) << "\n"
              << "==========================================================\n" << std::endl;
}

void VertexProcessor::finalize() {
    if (truthClosure_) reportTruthLinkClosure();
}

DECLARE_PROCESSOR(VertexProcessor);
