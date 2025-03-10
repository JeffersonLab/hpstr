/** 
 * @file VertexProcessor.h
 * @brief Class used to convert LCIO Vertex collections into ROOT collections.
 * @author Cameron Bravo, SLAC
 */
#include "VertexProcessor.h" 
#include "utilities.h"

VertexProcessor::VertexProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

VertexProcessor::~VertexProcessor() { 
}

void VertexProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring VertexProcessor" << std::endl;
    try
    {
        debug_             = parameters.getInteger("debug", debug_);
        vtxCollLcio_       = parameters.getString("vtxCollLcio", vtxCollLcio_);
        vtxCollRoot_       = parameters.getString("vtxCollRoot", vtxCollRoot_);
        partCollRoot_      = parameters.getString("partCollRoot", partCollRoot_);
        kinkRelCollLcio_   = parameters.getString("kinkRelCollLcio", kinkRelCollLcio_);
        trkRelCollLcio_    = parameters.getString("trkRelCollLcio", trkRelCollLcio_);
        trackStateLocation_= parameters.getString("trackStateLocation", trackStateLocation_);
        hitFitsCollLcio_   = parameters.getString("hitFitsCollLcio", hitFitsCollLcio_);
        trkhitCollRoot_    = parameters.getString("trkhitCollRoot",trkhitCollRoot_);
        rawhitCollRoot_    = parameters.getString("rawhitCollRoot",rawhitCollRoot_);
        mcPartRelLcio_     = parameters.getString("mcPartRelLcio", mcPartRelLcio_);
        useTrackerHits_    = parameters.getInteger("useTrackerHits",useTrackerHits_);
        bfield_            = parameters.getDouble("bfield",bfield_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void VertexProcessor::initialize(TTree* tree) {
    // Add branches to tree
    tree->Branch(vtxCollRoot_.c_str(),  &vtxs_);
    tree->Branch(partCollRoot_.c_str(), &parts_);
    if (!trkhitCollRoot_.empty())
        tree->Branch(trkhitCollRoot_.c_str(), &hits_);

    if (!rawhitCollRoot_.empty())
        tree->Branch(rawhitCollRoot_.c_str(), &rawhits_);
}

bool VertexProcessor::process(IEvent* ievent) {

    if (debug_ > 0) std::cout << "VertexProcessor: Clear output vector" << std::endl;

    if (hits_.size() > 0) {
        for (std::vector<TrackerHit *>::iterator it = hits_.begin(); it != hits_.end(); ++it) {
            delete *it;
        }
        hits_.clear();
    }

    if (rawhits_.size() > 0) {
        for (std::vector<RawSvtHit *>::iterator it = rawhits_.begin(); it != rawhits_.end(); ++it) {
            delete *it;
        }
        rawhits_.clear();
    }

    for(int i = 0; i < vtxs_.size(); i++) delete vtxs_.at(i);
    vtxs_.clear();
    for(int i = 0; i < parts_.size(); i++) delete parts_.at(i);
    parts_.clear();

    Event* event = static_cast<Event*> (ievent);
    UTIL::LCRelationNavigator* mcPartRel_nav;

    // Get the collection of vertices from the LCIO event. If no such collection 
    // exist, a DataNotAvailableException is thrown
    if (debug_ > 0) std::cout << "VertexProcessor: Get LCIO Collection " << vtxCollLcio_ << std::endl;
    EVENT::LCCollection* lc_vtxs = nullptr;
    try
    {
        lc_vtxs = event->getLCCollection(vtxCollLcio_.c_str()); 
    }
    catch (EVENT::DataNotAvailableException e) 
    {
        std::cout << e.what() << std::endl;
        return false;
    }

    // Get the collection of LCRelations between GBL tracks and kink data and track data variables.
    EVENT::LCCollection* gbl_kink_data{nullptr};
    EVENT::LCCollection* track_data{nullptr};

    try
    {
        if (!kinkRelCollLcio_.empty())
            gbl_kink_data = static_cast<EVENT::LCCollection*>(event->getLCCollection(kinkRelCollLcio_.c_str()));
        if (!trkRelCollLcio_.empty())
            track_data = static_cast<EVENT::LCCollection*>(event->getLCCollection(trkRelCollLcio_.c_str()));
    }
    catch (EVENT::DataNotAvailableException e)
    {
        std::cout << e.what() << std::endl;
        if (!gbl_kink_data)
            std::cout<<"Failed retrieving " << kinkRelCollLcio_ <<std::endl;
        if (!track_data)
            std::cout<<"Failed retrieving " << trkRelCollLcio_ <<std::endl;

    }

    bool rotateHits = true;
    int hitType = 0;
    EVENT::LCCollection* raw_svt_hit_fits = nullptr;

    auto evColls = event->getLCEvent()->getCollectionNames();
    auto it = std::find (evColls->begin(), evColls->end(), hitFitsCollLcio_.c_str());
    bool hasFits = true;
    if(it == evColls->end()) hasFits = false;
    if(hasFits)
    {
        raw_svt_hit_fits = event->getLCCollection(hitFitsCollLcio_.c_str());
    }
    //Check to see if MC Particles are in the file
    it = std::find (evColls->begin(), evColls->end(), mcPartRelLcio_.c_str());
    bool hasMCParts = true;
    EVENT::LCCollection* mcPartRel;
    if(it == evColls->end()) hasMCParts = false;
    if(hasMCParts)
    {
        mcPartRel = event->getLCCollection(mcPartRelLcio_.c_str());
        // Heap an LCRelation navigator which will allow faster access 
        mcPartRel_nav = new UTIL::LCRelationNavigator(mcPartRel);

    }

    if (debug_ > 0) std::cout << "VertexProcessor: Converting Verteces" << std::endl;
    for (int ivtx = 0 ; ivtx < lc_vtxs->getNumberOfElements(); ++ivtx) 
    {
        if (debug_ > 0) std::cout << "VertexProcessor: Converting Vertex " << ivtx << std::endl;
        EVENT::Vertex * lc_vtx{nullptr};
        lc_vtx = static_cast<EVENT::Vertex*>(lc_vtxs->getElementAt(ivtx));

        if (debug_ > 0) std::cout << "VertexProcessor: Build Vertex" << std::endl;
        Vertex* vtx = utils::buildVertex(lc_vtx);

        if (debug_ > 0) std::cout << "VertexProcessor: Get Particles" << std::endl;
        std::vector<EVENT::ReconstructedParticle*> lc_parts = lc_vtx->getAssociatedParticle()->getParticles();
        for(auto lc_part : lc_parts)
        {
            if (debug_ > 0) std::cout << "VertexProcessor: Build particle" << std::endl;
            Particle * part = utils::buildParticle(lc_part,trackStateLocation_, gbl_kink_data, track_data);
            if (lc_part->getTracks().size()>0){
                EVENT::Track* lc_track = static_cast<EVENT::Track*>(lc_part->getTracks()[0]);
                Track* track = utils::buildTrack(lc_track,trackStateLocation_,gbl_kink_data,track_data);
                int nHits = 0;
                if (bfield_ > 0.0) track->setMomentum(bfield_);
                if (track->isKalmanTrack()) hitType = 1; //SiClusters

                if(!useTrackerHits_){
                    auto hitPattern = lc_track->getSubdetectorHitNumbers();
                    for( int h=0; h<hitPattern.size(); h++){
                        if( hitPattern[h]>0 ){
                            track->addHitLayer(h);
                            nHits++;
                        }
                    }
                    if(nHits==0){
                        std::cout << "[VertexProcessor::process] WARNING Track with no hits -- likely getSubdetectorHitNumbers isn't filled!" << std::endl;
                        std::cout << "[VertexProcessor::process] WARNING If hit collections are saved, you may want to turn on useTrackerHits" << std::endl;
                    }
                    track->setTrackerHitCount(nHits);
                } 
                else{
		    EVENT::TrackerHitVec lc_tracker_hits = lc_track->getTrackerHits();
		    for (auto lc_tracker_hit : lc_tracker_hits) {
			TrackerHit* tracker_hit = utils::buildTrackerHit(static_cast<IMPL::TrackerHitImpl*>(lc_tracker_hit),rotateHits,hitType);
			std::vector<RawSvtHit*> rawSvthitsOn3d;
			utils::addRawInfoTo3dHit(tracker_hit,static_cast<IMPL::TrackerHitImpl*>(lc_tracker_hit),
				raw_svt_hit_fits,&rawSvthitsOn3d,hitType);

			int hitLayer = tracker_hit->getLayer();
			nHits++;
			EVENT::LCObjectVec rawHits = lc_tracker_hit->getRawHits();
			for(int irawhit = 0; irawhit < rawHits.size(); ++irawhit){
			    IMPL::TrackerHitImpl* rawhit = static_cast<IMPL::TrackerHitImpl*>(rawHits.at(irawhit));
			    if(debug_ > 0)
				std::cout << "rawhit on track has lcio id: " << rawhit->id() << std::endl;

			    if (hasMCParts)
			    {
				// Get the list of fit params associated with the raw tracker hit
				EVENT::LCObjectVec lc_simtrackerhits = mcPartRel_nav->getRelatedToObjects(rawhit);

				//Loop over SimTrackerHits to get MCParticles
				for(int isimhit = 0; isimhit < lc_simtrackerhits.size(); isimhit++){
				    IMPL::SimTrackerHitImpl* lc_simhit = static_cast<IMPL::SimTrackerHitImpl*>(lc_simtrackerhits.at(isimhit));
				    IMPL::MCParticleImpl* lc_mcp = static_cast<IMPL::MCParticleImpl*>(lc_simhit->getMCParticle());
				    if(lc_mcp == nullptr)
					std::cout << "mcp is null" << std::endl;
				    track->addMcpHit(hitLayer, lc_mcp->id());
				    if(debug_ > 0) {
					std::cout << "simtrackerhit lcio id: " << lc_simhit->id() << std::endl;
					std::cout << "mcp lcio id: " << lc_mcp->id() << std::endl;
				    }
				}
			    }
			} // Loop over raw tracker hits

			track->addHitLayer(hitLayer);
			hits_.push_back(tracker_hit);
                        for (std::vector<RawSvtHit *>::iterator it = rawSvthitsOn3d.begin(); it != rawSvthitsOn3d.end(); ++it) {
                          delete *it;
                        }
                        rawSvthitsOn3d.clear();	

                    } // Loop over hits on track

                    track->setTrackerHitCount(nHits);

                } // !useTrackerHits

                part->setTrack(track); 
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

    if(hasMCParts) delete mcPartRel_nav;
    if (debug_ > 0) std::cout << "VertexProcessor: End process" << std::endl;
    return true;
}

void VertexProcessor::finalize() { 
}

DECLARE_PROCESSOR(VertexProcessor); 
