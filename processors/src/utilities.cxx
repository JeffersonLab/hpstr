#include "utilities.h"
#include <algorithm>
/*
   void utils::buildTrackCollection(std::vector<Track*>& tracks, 
   Event* event,
   const char* LCTrackCollection)
   {

   EVENT::LCCollection* lc_tracks event->getLCCollection(LCTrackCollection);


   }

*/


bool utils::hasCollection(EVENT::LCEvent* lc_event,const std::string& collection) {

    if (!lc_event || collection.empty())
        return false;

    auto evColls = lc_event->getCollectionNames();
    auto it = std::find(evColls->begin(),evColls->end(), collection);
    if (it!=evColls->end()) 
        return true;
    return false;
}


Vertex* utils::buildVertex(EVENT::Vertex* lc_vertex) { 

    if (!lc_vertex) 
        return nullptr;

    //TODO move the static cast outside?

    Vertex* vertex = new Vertex();
    vertex->setChi2         (lc_vertex->getChi2());
    vertex->setProbability  (lc_vertex->getProbability());
    vertex->setID           (lc_vertex->id());
    vertex->setType         (lc_vertex->getAlgorithmType());
    vertex->setVtxParameters((std::vector<float>)lc_vertex->getParameters());

    //TODO Rotate the covariance matrix!
    vertex->setCovariance   ((std::vector<float>)lc_vertex->getCovMatrix());
    //std::cout<<lc_vertex->getVertexParameterNames[0]<<std::endl;
    vertex->setType         (lc_vertex->getAlgorithmType());

    vertex->setPos          (lc_vertex->getPosition(),false);


    return vertex;
}

Track* utils::buildTrack(EVENT::Track* lc_track,
        EVENT::LCCollection* gbl_kink_data,
        EVENT::LCCollection* track_data) {

    if (!lc_track)
        return nullptr;

    Track* track = new Track();
    // Set the track parameters
    track->setTrackParameters(lc_track->getD0(), 
            lc_track->getPhi(), 
            lc_track->getOmega(), 
            lc_track->getTanLambda(), 
            lc_track->getZ0());

    // Set the track type
    track->setType(lc_track->getType()); 

    // Set the track fit chi^2
    track->setChi2(lc_track->getChi2());

    // Set the track ndf 
    track->setNdf(lc_track->getNdf());

    // Set the position of the extrapolated track at the ECal face.  The
    // extrapolation uses the full 3D field map.
    const EVENT::TrackState* track_state 
        = lc_track->getTrackState(EVENT::TrackState::AtCalorimeter); 

    if (track_state) {
        double position_at_ecal[3] = { 
            track_state->getReferencePoint()[1],  
            track_state->getReferencePoint()[2],  
            track_state->getReferencePoint()[0]
        };
        track->setPositionAtEcal(position_at_ecal); 
    }

    if (gbl_kink_data) {
        // Instantiate an LCRelation navigator which will allow faster access 
        // to GBLKinkData object
        UTIL::LCRelationNavigator* gbl_kink_data_nav 
            = new UTIL::LCRelationNavigator(gbl_kink_data);

        // Get the list of GBLKinkData associated with the LCIO Track
        EVENT::LCObjectVec gbl_kink_data_list 
            = gbl_kink_data_nav->getRelatedFromObjects(lc_track);

        // The container of GBLKinkData objects should only contain a 
        // single object. If not, throw an exception
        if (gbl_kink_data_list.size() != 1) { 
            throw std::runtime_error("[ TrackingProcessor ]: The collection " 
                    + std::string(Collections::TRACK_DATA_REL)
                    + " has the wrong data structure."); 
        }

        // Get the list GBLKinkData GenericObject associated with the LCIO Track
        IMPL::LCGenericObjectImpl* gbl_kink_datum 
            = static_cast<IMPL::LCGenericObjectImpl*>(gbl_kink_data_list.at(0));

        // Set the lambda and phi kink values
        for (int ikink = 0; ikink < gbl_kink_datum->getNDouble(); ++ikink) { 
            track->setLambdaKink(ikink, gbl_kink_datum->getFloatVal(ikink));
            track->setPhiKink(ikink, gbl_kink_datum->getDoubleVal(ikink));
        }

        delete gbl_kink_data_nav; 

    } // add gbl kink data

    if (track_data) { 

        // Instantiate an LCRelation navigator which will allow faster access
        // to TrackData objects  
        UTIL::LCRelationNavigator* track_data_nav 
            = new UTIL::LCRelationNavigator(track_data);

        // Get the list of TrackData associated with the LCIO Track
        EVENT::LCObjectVec track_data_list = track_data_nav->getRelatedFromObjects(lc_track);

        // The container of TrackData objects should only contain a single
        //  object.  If not, throw an exception.
        if (track_data_list.size() == 1) { 

            // Get the TrackData GenericObject associated with the LCIO Track
            IMPL::LCGenericObjectImpl* track_datum = static_cast<IMPL::LCGenericObjectImpl*>(track_data_list.at(0));

            // Check that the TrackData data structure is correct.  If it's
            // not, throw a runtime exception.   
            if (track_datum->getNDouble() > 14 || track_datum->getNFloat() != 1 
                    || track_datum->getNInt() != 1) {
                throw std::runtime_error("[ TrackingProcessor ]: The collection " 
                        + std::string(Collections::TRACK_DATA)
                        + " has the wrong structure.");
            }

            // Set the SvtTrack isolation values
            for (int iso_index = 0; iso_index < track_datum->getNDouble(); ++iso_index) { 
                track->setIsolation(iso_index, track_datum->getDoubleVal(iso_index));
            }

            // Set the SvtTrack time
            track->setTrackTime(track_datum->getFloatVal(0));

            // Set the volume (top/bottom) in which the SvtTrack resides
            track->setTrackVolume(track_datum->getIntVal(0));
        }
        delete track_data_nav;

    } //add track data  

    return track;
}

RawSvtHit* utils::buildRawHit(EVENT::TrackerRawData* rawTracker_hit,
        EVENT::LCCollection* raw_svt_hit_fits) {

    EVENT::long64 value =
        EVENT::long64(rawTracker_hit->getCellID0() & 0xffffffff) |
        ( EVENT::long64(rawTracker_hit->getCellID1() ) << 32       );
    decoder.setValue(value);

    RawSvtHit* rawHit = new RawSvtHit();
    rawHit->setSystem(decoder["system"]);
    rawHit->setBarrel(decoder["barrel"]);
    rawHit->setLayer(decoder["layer"]);
    rawHit->setModule(decoder["module"]);
    rawHit->setSensor(decoder["sensor"]);
    rawHit->setSide(decoder["side"]);
    rawHit->setStrip(decoder["strip"]);

    // Extract ADC values for this hit
    int hit_adcs[6] = { 
        (int)rawTracker_hit->getADCValues().at(0), 
        (int)rawTracker_hit->getADCValues().at(1), 
        (int)rawTracker_hit->getADCValues().at(2), 
        (int)rawTracker_hit->getADCValues().at(3), 
        (int)rawTracker_hit->getADCValues().at(4), 
        (int)rawTracker_hit->getADCValues().at(5)};

    rawHit->setADCs(hit_adcs);
    if (raw_svt_hit_fits) {
        UTIL::LCRelationNavigator* rawTracker_hit_fits_nav = new UTIL::LCRelationNavigator(raw_svt_hit_fits);

        // Get the list of fit params associated with the raw tracker hit
        EVENT::LCObjectVec rawTracker_hit_fits_list
            = rawTracker_hit_fits_nav->getRelatedToObjects(rawTracker_hit);

        // Get the list SVTFittedRawTrackerHit GenericObject associated with the SVTRawTrackerHit
        IMPL::LCGenericObjectImpl* hit_fit_param
            = static_cast<IMPL::LCGenericObjectImpl*>(rawTracker_hit_fits_list.at(0));

        double fit_params[5] = { 
            (double)hit_fit_param->getDoubleVal(0), 
            (double)hit_fit_param->getDoubleVal(1), 
            (double)hit_fit_param->getDoubleVal(2), 
            (double)hit_fit_param->getDoubleVal(3), 
            (double)hit_fit_param->getDoubleVal(4)
        };

        rawHit->setFit(fit_params);
        if (rawTracker_hit_fits_nav)
            delete rawTracker_hit_fits_nav;
        rawTracker_hit_fits_nav = nullptr;
    }//raw svt hits

    return rawHit;

}//build raw hit


TrackerHit* utils::buildTrackerHit(IMPL::TrackerHitImpl* lc_tracker_hit) { 

    if (!lc_tracker_hit)
        return nullptr;

    TrackerHit* tracker_hit = new TrackerHit();

    // Get the position of the LCIO TrackerHit and set the position of 
    // the TrackerHit
    double hit_position[3] = { 
        lc_tracker_hit->getPosition()[0], 
        lc_tracker_hit->getPosition()[1], 
        lc_tracker_hit->getPosition()[2]
    };
    tracker_hit->setPosition(hit_position, true);

    // Set the covariance matrix of the SvtHit
    tracker_hit->setCovarianceMatrix(lc_tracker_hit->getCovMatrix());

    // Set the time of the SvtHit
    tracker_hit->setTime(lc_tracker_hit->getTime());

    // Set the charge of the SvtHit
    tracker_hit->setCharge(lc_tracker_hit->getEDep());

    // Set the LCIO id
    tracker_hit->setID(lc_tracker_hit->id());

    return tracker_hit;


}

bool utils::addRawInfoTo3dHit(TrackerHit* tracker_hit, 
        IMPL::TrackerHitImpl* lc_tracker_hit,
        EVENT::LCCollection* raw_svt_fits, std::vector<RawSvtHit*>* rawHits) {

    if (!tracker_hit || !lc_tracker_hit)
        return false;

    float rawcharge = 0;
    //0 top 1 bottom
    int volume = -1;
    //1-6
    int layer = -1;

    //Get the Raw content of the tracker hits
    EVENT::LCObjectVec lc_rawHits             = lc_tracker_hit->getRawHits();  

    for (unsigned int irh = 0 ; irh < lc_rawHits.size(); ++irh) {

        //TODO useless to build all of it?
        RawSvtHit* rawHit = buildRawHit(static_cast<EVENT::TrackerRawData*>(lc_rawHits.at(irh)),raw_svt_fits); 
        rawcharge += rawHit->getAmp();
        int currentHitVolume = rawHit->getModule() % 2 ? 1 : 0;
        int currentHitLayer  = (rawHit->getLayer() - 1 ) / 2;
        if (volume == -1 )
            volume = currentHitVolume;
        else {
            if ( currentHitVolume != volume)
                std::cout<<"[ ERROR ] : utils::addRawInfoTo3dHit raw hits with inconsistent volume found" <<std::endl;
        }

        if (layer == -1 )
            layer = currentHitLayer;
        else {
            if (currentHitLayer != layer)
                std::cout<<"[ ERROR ] : utils::addRawInfoTo3dHit raw hits with inconsistent layer found" <<std::endl;
        }

        //TODO:: store only if asked
        tracker_hit->addRawHit(rawHit);
        if (rawHits)
            rawHits->push_back(rawHit);

    }

    tracker_hit->setRawCharge(rawcharge);
    tracker_hit->setVolume(volume);
    tracker_hit->setLayer(layer);

    return true;
}

//TODO-improve shared finding algorithm 

bool utils::isUsedByTrack(IMPL::TrackerHitImpl* lc_tracker_hit,
        EVENT::Track* lc_track) {

    EVENT::TrackerHitVec trk_lc_tracker_hits = lc_track->getTrackerHits();

    for (auto trk_lc_tracker_hit : trk_lc_tracker_hits) {
        //std::cout<<lc_tracker_hit->id()<<" " <<trk_lc_tracker_hit->id()<<std::endl;
        if (lc_tracker_hit -> id() == trk_lc_tracker_hit -> id())
            return true;
    }
    return false;
}

bool utils::isUsedByTrack(TrackerHit* tracker_hit,
        EVENT::Track* lc_track) {

    EVENT::TrackerHitVec trk_lc_tracker_hits = lc_track->getTrackerHits();

    for (auto trk_lc_tracker_hit : trk_lc_tracker_hits) {
        if (tracker_hit->getID() ==  trk_lc_tracker_hit->id())
            return true;
    }
    return false;
}
