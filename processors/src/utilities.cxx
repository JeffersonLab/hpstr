#include "utilities.h"
#include <algorithm>
#include <memory>
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

Particle* utils::buildParticle(EVENT::ReconstructedParticle* lc_particle,
        std::string trackstate_location,
        EVENT::LCCollection* gbl_kink_data,
        EVENT::LCCollection* track_data)

{ 

    if (!lc_particle) 
        return nullptr;

    Particle* part = new Particle();
    // Set the charge of the HpsParticle    
    part->setCharge(lc_particle->getCharge());

    // Set the HpsParticle type
    part->setType(lc_particle->getType());

    // Set the energy of the HpsParticle
    part->setEnergy(lc_particle->getEnergy());

    // Set the momentum of the HpsParticle
    part->setMomentum(lc_particle->getMomentum());

    // Set the mass of the HpsParticle
    part->setMass(lc_particle->getMass());

    // Set the goodness of PID for the HpsParticle
    part->setGoodnessOfPID(lc_particle->getGoodnessOfPID());

    // Set the PDG ID for the HpsParticle
    part->setPDG(lc_particle->getParticleIDUsed()->getPDG());

    // Set the Track for the HpsParticle
    if (lc_particle->getTracks().size()>0)
    {
        Track * trkPtr = utils::buildTrack(lc_particle->getTracks()[0],trackstate_location, gbl_kink_data, track_data);
        part->setTrack(trkPtr);
        delete trkPtr;
    }

    // Set the Track for the HpsParticle
    if (lc_particle->getClusters().size() > 0)
    {
        CalCluster * clusBuf = utils::buildCalCluster(lc_particle->getClusters()[0]);
        part->setCluster(clusBuf);
        delete clusBuf;
    }

    return part;
}

CalCluster* utils::buildCalCluster(EVENT::Cluster* lc_cluster) 
{ 

    if (!lc_cluster) 
        return nullptr;

    CalCluster* cluster = new CalCluster();
    // Set the cluster position
    cluster->setPosition(lc_cluster->getPosition());

    // Set the cluster energy
    cluster->setEnergy(lc_cluster->getEnergy());

    // Get the ecal hits used to create the cluster
    EVENT::CalorimeterHitVec lc_hits = lc_cluster->getCalorimeterHits();

    // Loop over all of the Ecal hits and add them to the Ecal cluster.  The
    // seed hit is set to be the hit with the highest energy.  The cluster time
    // is set to be the hit time of the seed hit.
    double senergy = 0;
    double stime = 0;
    for(int ihit = 0; ihit < (int) lc_hits.size(); ++ihit) {
        // Get an Ecal hit
        EVENT::CalorimeterHit* lc_hit  = lc_hits[ihit];
        if (senergy < lc_hit->getEnergy()) {
            senergy = lc_hit->getEnergy();
            stime = lc_hit->getTime();
        }
    }

    // Set the time of the cluster
    cluster->setTime(stime);

    return cluster;
}

bool utils::IsSameTrack(Track* trk1, Track* trk2) {
    double tol = 1e-6;
    if (fabs(trk1->getD0()        - trk2->getD0())        > tol ||
            fabs(trk1->getPhi()       - trk2->getPhi())       > tol ||
            fabs(trk1->getOmega()     - trk2->getOmega())     > tol ||
            fabs(trk1->getTanLambda() - trk2->getTanLambda()) > tol ||
            fabs(trk1->getZ0()        - trk2->getZ0())        > tol ||
            fabs(trk1->getChi2Ndf()   - trk2->getChi2Ndf())   > tol 
       ) 
        return false;

    return true;
}


Track* utils::buildTrack(EVENT::Track* lc_track,
        std::string trackstate_location,
        EVENT::LCCollection* gbl_kink_data,
        EVENT::LCCollection* track_data) {

    if (!lc_track)
        return nullptr;

     //TrackState Location map
     std::map<std::string, int> trackstateLocationMap_ = {
        {"", EVENT::TrackState::AtIP},
        {"AtTarget", EVENT::TrackState::LastLocation}
     };

    int loc;
    auto it = trackstateLocationMap_.find(trackstate_location);
    if (it != trackstateLocationMap_.end()){
        loc = it->second;
    }
    else{
        std::cout << "[utilities]::ERROR Track State Location " << trackstate_location << " Doesn't Exist!" << std::endl;
        std::cout << "Check map in utilities::buildTrack for defined locations" << std::endl;
        return nullptr;
    }

    Track* track = new Track();
    //If using track AtIP, get params from lc_track
    if (loc == trackstateLocationMap_[""]){
        // Set the track parameters
        track->setTrackParameters(lc_track->getD0(), 
                lc_track->getPhi(), 
                lc_track->getOmega(), 
                lc_track->getTanLambda(), 
                lc_track->getZ0());

        // Set the track covariance matrix
        track->setCov(static_cast<std::vector<float> > (lc_track->getCovMatrix()));
    }

    //If other TrackState specified, get track params from track state
    else {
        // If track state doesn't exist, no track returned
        const EVENT::TrackState* ts = lc_track->getTrackState(loc);
        if (ts == nullptr){
            return nullptr;
        }
        // Set the track parameters using trackstate
        track->setTrackParameters(ts->getD0(), 
                ts->getPhi(), 
                ts->getOmega(), 
                ts->getTanLambda(), 
                ts->getZ0());

        double position[3] = {
            ts->getReferencePoint()[1],  
            ts->getReferencePoint()[2],  
            ts->getReferencePoint()[0]
        };

        track->setCov(static_cast<std::vector<float> > (ts->getCovMatrix()));

        track->setPosition(position);
    }

    // Set the track id
    track->setID(lc_track->id());

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
        std::shared_ptr<UTIL::LCRelationNavigator> gbl_kink_data_nav = std::make_shared<UTIL::LCRelationNavigator>(gbl_kink_data);

        // Get the list of GBLKinkData associated with the LCIO Track
        EVENT::LCObjectVec gbl_kink_data_list 
            = gbl_kink_data_nav->getRelatedFromObjects(lc_track);

        // The container of GBLKinkData objects should only contain a 
        // single object. If not, throw an exception
        if (gbl_kink_data_list.size() == 1) {

            /*
               std::cout<<"[ Utilities ]: The collection " 
               + std::string(Collections::KINK_DATA)
               + " has the wrong data structure for this track"<<std::endl; 
               */

            // Get the list GBLKinkData GenericObject associated with the LCIO Track
            IMPL::LCGenericObjectImpl* gbl_kink_datum 
                = static_cast<IMPL::LCGenericObjectImpl*>(gbl_kink_data_list.at(0));

            // Set the lambda and phi kink values
            for (int ikink = 0; ikink < gbl_kink_datum->getNDouble(); ++ikink) { 
                track->setLambdaKink(ikink, gbl_kink_datum->getFloatVal(ikink));
                track->setPhiKink(ikink, gbl_kink_datum->getDoubleVal(ikink));
            }

        }//gbl_kink_data has right structure

    } // add gbl kink data

    if (track_data) { 

        // Instantiate an LCRelation navigator which will allow faster access
        // to TrackData objects  
        std::shared_ptr<UTIL::LCRelationNavigator> track_data_nav = std::make_shared<UTIL::LCRelationNavigator>(track_data);

        // Get the list of TrackData associated with the LCIO Track
        EVENT::LCObjectVec track_data_list = track_data_nav->getRelatedFromObjects(lc_track);

        // The container of TrackData objects should only contain a single
        //  object.  If not, throw an exception.
        if (track_data_list.size() == 1) { 

            // Get the TrackData GenericObject associated with the LCIO Track
            IMPL::LCGenericObjectImpl* track_datum = static_cast<IMPL::LCGenericObjectImpl*>(track_data_list.at(0));

            // Check that the TrackData data structure is correct.  If it's
            // not, throw a runtime exception.   
            if (track_datum->getNDouble() > 14 || track_datum->getNFloat() > 7 || track_datum->getNInt() != 1) {
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

            // Set the Track momentum
            if (track_datum->getNFloat()>3)
              track->setMomentum(track_datum->getFloatVal(1),track_datum->getFloatVal(2),track_datum->getFloatVal(3));

            // Set the volume (top/bottom) in which the SvtTrack resides
            track->setTrackVolume(track_datum->getIntVal(0));

            // Set the BfieldY for track state
            double bfieldY = -999.9;
            if(track_datum->getNFloat() > 4){
                if (loc == trackstateLocationMap_[""])
                    bfieldY = track_datum->getFloatVal(4);
                if (loc == trackstateLocationMap_["AtTarget"]){
                    bfieldY = track_datum->getFloatVal(5);
                }
                if (loc == trackstateLocationMap_["AtCalorimeter"])
                    bfieldY = track_datum->getFloatVal(6);
                //Bfield needs factor of -1, not sure why... <-TODO investigate
                track->setMomentum(-bfieldY);
            }
        }

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
        std::shared_ptr<UTIL::LCRelationNavigator> rawTracker_hit_fits_nav = std::make_shared<UTIL::LCRelationNavigator>(raw_svt_hit_fits);


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
        rawHit->setFit(fit_params, 0);
        if(rawTracker_hit_fits_list.size()>1)
        {
            hit_fit_param = static_cast<IMPL::LCGenericObjectImpl*>(rawTracker_hit_fits_list.at(1));
            fit_params[0] = (double)hit_fit_param->getDoubleVal(0); 
            fit_params[1] = (double)hit_fit_param->getDoubleVal(1); 
            fit_params[2] = (double)hit_fit_param->getDoubleVal(2); 
            fit_params[3] = (double)hit_fit_param->getDoubleVal(3); 
            fit_params[4] = (double)hit_fit_param->getDoubleVal(4);

            rawHit->setFit(fit_params, 1);
        }
        rawHit->setFitN(rawTracker_hit_fits_list.size());
    }//raw svt hit fits

    return rawHit;

}//build raw hit

//type = 0 RotatedHelicalTrackHit type = 1 SiCluster
TrackerHit* utils::buildTrackerHit(IMPL::TrackerHitImpl* lc_tracker_hit, bool rotate, int type) { 

    if (!lc_tracker_hit)
        return nullptr;

    TrackerHit* tracker_hit = new TrackerHit();

    // Get the position of the LCIO TrackerHit and set the position of 
    // the TrackerHit
    double hit_position[3] = { 
        lc_tracker_hit->getPosition()[0],  //lcio x
        lc_tracker_hit->getPosition()[1],  //lcio y
        lc_tracker_hit->getPosition()[2]   //lcio z
    };
    tracker_hit->setPosition(hit_position, rotate, type);

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

//type 0 rotatedHelicalHit  type 1 SiClusterHit
bool utils::addRawInfoTo3dHit(TrackerHit* tracker_hit, 
        IMPL::TrackerHitImpl* lc_tracker_hit,
        EVENT::LCCollection* raw_svt_fits, std::vector<RawSvtHit*>* rawHits,int type, bool storeRawHit) {

    if (!tracker_hit || !lc_tracker_hit)
        return false;

    float rawcharge = 0;
    //0 top 1 bottom
    int volume = -1;
    //1-6(7) for rotated  0-13 for SiCluster
    int layer = -1;

    // Get decoders to read cellids
    UTIL::BitField64 decoder("system:6,barrel:3,layer:4,module:12,sensor:1,side:32:-2,strip:12");

    //Get the Raw content of the tracker hits
    EVENT::LCObjectVec lc_rawHits             = lc_tracker_hit->getRawHits();  

    std::vector<int> rawhit_strips;
    for (unsigned int irh = 0 ; irh < lc_rawHits.size(); ++irh) {
        // Get a raw hit from the list of hits
        EVENT::TrackerRawData* rawTracker_hit
            = static_cast<EVENT::TrackerRawData*>(lc_rawHits.at(irh));

        //Decode the cellid
        EVENT::long64 value = EVENT::long64( rawTracker_hit->getCellID0() & 0xffffffff ) |
            ( EVENT::long64( rawTracker_hit->getCellID1() ) << 32 ) ;
        decoder.setValue(value);

        //Get rawhit strip number
        int stripnumber = decoder["strip"];
        rawhit_strips.push_back(stripnumber);

        //TODO useless to build all of it?
        RawSvtHit* rawHit = buildRawHit(rawTracker_hit,raw_svt_fits); 
        rawcharge += rawHit->getAmp(0);
        int currentHitVolume = rawHit->getModule() % 2 ? 1 : 0;
        int currentHitLayer  = (rawHit->getLayer() - 1 ) / 2;
        if (type == 1) 
            currentHitLayer = rawHit->getLayer() - 1;
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

        if(storeRawHit){
            tracker_hit->addRawHit(rawHit);
            if (rawHits)
                rawHits->push_back(rawHit);
        }
        else
            delete rawHit;

    }

    tracker_hit->setRawCharge(rawcharge);
    tracker_hit->setVolume(volume);
    tracker_hit->setLayer(layer);
    tracker_hit->setRawHitStripNumbers(rawhit_strips);

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


bool utils::getParticlesFromVertex(Vertex* vtx, Particle* ele, Particle* pos) {

    for (int ipart = 0; ipart < vtx->getParticles().GetEntries(); ++ipart) {
        int pdg_id = ((Particle*)vtx->getParticles().At(ipart))->getPDG();
        if (pdg_id == 11) {
            ele = (Particle*)vtx->getParticles().At(ipart);
        }
        else if (pdg_id == -11) {
            pos = (Particle*)vtx->getParticles().At(ipart);
        }

        else {
            std::cout<<"Utilities::Wrong particle ID "<< pdg_id <<"associated to vertex. Skip."<<std::endl;
            return false;
        }
    }

    if (!ele || !pos) {
        std::cout<<"Utilities::Vertex formed without ele/pos. Skip."<<std::endl;
        return false;
    }

    return true;
}

double utils::getKalmanTrackL1Isolations(Track* track, std::vector<TrackerHit*>* siClusters){
    double L1_axial_iso = 999999.9;
    double L1_stereo_iso = 999999.9;
    //Loop over hits on track
    for(int i = 0; i < track->getSvtHits().GetEntries(); i++){
        TrackerHit* track_hit = (TrackerHit*)track->getSvtHits().At(i);
        //Track hit info
        int trackhit_id = track_hit->getID();
        int trackhit_layer = track_hit->getLayer();
        int trackhit_volume = track_hit->getVolume();
        double trackhit_y = track_hit->getGlobalY();
        double trackhit_charge = track_hit->getRawCharge();
        double trackhit_time = track_hit->getTime();

        //Only look at L1
        if(trackhit_layer > 1)
            continue;

        //Get rawhit strip information
        std::vector<int> trackhit_rawhits = track_hit->getRawHitStripNumbers();
        if(trackhit_rawhits.size() < 1)
            continue;
        int trackhit_maxstrip = *max_element(trackhit_rawhits.begin(), trackhit_rawhits.end());
        int trackhit_minstrip = *min_element(trackhit_rawhits.begin(), trackhit_rawhits.end());


        //Isolation only calculated for axial sensors (sensitive to global y)
        bool isAxial = false;
        //Axial/Stereo Top/Bot mapping
        if(trackhit_volume == 1){
            if(trackhit_layer%2 == 1)
                isAxial = true;
        }
        if(trackhit_volume == 0){
            if(trackhit_layer%2 == 0)
                isAxial = true;
        }

        //Loop over all SiClusters in event
        //Find closest/best alternative SiCluster to track hit 
        TrackerHit* closest_althit = nullptr;
        double isohit_dy = 999999.9;
        double closest_dcharge = 99999.9;
        double closest_dt = 999999.9;
        double isohit_y = 999999.9;

        if ( siClusters == nullptr )
            return isohit_dy;
        for(int j = 0; j < siClusters->size(); j++){
            TrackerHit* althit = siClusters->at(j);
            int althit_id = althit->getID();
            int althit_volume = althit->getVolume();
            int althit_layer = althit->getLayer();
            double althit_y = althit->getGlobalY();
            double althit_charge = althit->getRawCharge();
            double althit_time = althit->getTime();

            //Skip if SiCluster not on same layer as track hit
            if (althit_layer != trackhit_layer)
                continue;

            //Skip if volume doesn't match
            if(althit_volume != trackhit_volume)
                continue;

            //Skip same hit
            if (althit_id == trackhit_id)
                continue;

            //Only look at hits that are further from beam-axis in Global Y
            if ( (trackhit_volume == 0 && althit_y < trackhit_y) ||
                    (trackhit_volume == 1 && althit_y > trackhit_y))
                continue;

            //Require alternative hits to be within +-30ns (based on SiClustersOnTrack t distr)
            if(std::abs(althit_time) > 30.0)
                continue;

            //Skip adjacent rawhits
            std::vector<int> althit_rawhits = althit->getRawHitStripNumbers();
            int althit_maxstrip = *max_element(althit_rawhits.begin(), althit_rawhits.end());
            int althit_minstrip = *min_element(althit_rawhits.begin(), althit_rawhits.end());
            if(trackhit_minstrip - althit_maxstrip <= 1 && althit_minstrip - trackhit_maxstrip <= 1)
                continue;

            //Pick closest alternative hit
            if (std::abs(trackhit_y - althit_y) < isohit_dy){
                isohit_dy = std::abs(trackhit_y-althit_y);
                closest_althit = althit;
                closest_dcharge = trackhit_charge - althit_charge;
                closest_dt = trackhit_time - althit_time;
                isohit_y = althit_y;
            }
        }

        if(isAxial)
            L1_axial_iso = isohit_dy;
        else
            L1_stereo_iso = isohit_dy;

    }

    if(L1_axial_iso < L1_stereo_iso){
        return L1_axial_iso;
    }
    else
        return L1_stereo_iso;
}

void utils::get2016KFMCTruthHitCodes(Track* ele_trk, Track* pos_trk, std::vector<TrackerHit*>* hits, int& L1L2hitCode, int& L1hitCode, int& L2hitCode){
    //Build map of hits and the associated MC part ids for later
    TRefArray ele_trk_hits = ele_trk->getSvtHits();
    TRefArray pos_trk_hits = pos_trk->getSvtHits();
    std::map<int, std::vector<int> > trueHitIDs;
    if(hits == nullptr)
        return;
    for(int i = 0; i < hits->size(); i++)
    {
        TrackerHit* hit = hits->at(i);
        trueHitIDs[hit->getID()] = hit->getMCPartIDs();
    }
    //Count the number of hits per part on the ele track
    std::map<int, int> nHits4part_ele;
    for(int i = 0; i < ele_trk_hits.GetEntries(); i++)
    {
        TrackerHit* eleHit = (TrackerHit*)ele_trk_hits.At(i);
        for(int idI = 0; idI < trueHitIDs[eleHit->getID()].size(); idI++ )
        {
            int partID = trueHitIDs[eleHit->getID()].at(idI);
            if ( nHits4part_ele.find(partID) == nHits4part_ele.end() )
            {
                // not found
                nHits4part_ele[partID] = 1;
            }
            else
            {
                // found
                nHits4part_ele[partID]++;
            }
        }
    }

    //Determine the MC part with the most hits on the track
    int maxNHits_ele = 0;
    int maxID_ele = 0;
    for (std::map<int,int>::iterator it=nHits4part_ele.begin(); it!=nHits4part_ele.end(); ++it)
    {
        if(it->second > maxNHits_ele)
        {
            maxNHits_ele = it->second;
            maxID_ele = it->first;
        }
    }

    //Count the number of hits per part on the pos track
    std::map<int, int> nHits4part_pos;
    for(int i = 0; i < pos_trk_hits.GetEntries(); i++)
    {
        TrackerHit* posHit = (TrackerHit*)pos_trk_hits.At(i);
        for(int idI = 0; idI < trueHitIDs[posHit->getID()].size(); idI++ )
        {
            int partID = trueHitIDs[posHit->getID()].at(idI);
            if ( nHits4part_pos.find(partID) == nHits4part_pos.end() )
            {
                // not found
                nHits4part_pos[partID] = 1;
            }
            else
            {
                // found
                nHits4part_pos[partID]++;
            }
        }
    }

    //Determine the MC part with the most hits on the track
    int maxNHits_pos = 0;
    int maxID_pos = 0;
    for (std::map<int,int>::iterator it=nHits4part_pos.begin(); it!=nHits4part_pos.end(); ++it)
    {
        if(it->second > maxNHits_pos)
        {
            maxNHits_pos = it->second;
            maxID_pos = it->first;
        }
    }

    //Determine Ele L1 and L2 truth information
    bool ele_trueAxialL1 = false;
    bool ele_trueStereoL1 = false;
    bool ele_trueAxialL2 = false;
    bool ele_trueStereoL2 = false;
    bool pos_trueAxialL1 = false;
    bool pos_trueStereoL1 = false;
    bool pos_trueAxialL2 = false;
    bool pos_trueStereoL2 = false;
    if (ele_trk->isKalmanTrack()){

        for(int i = 0; i < ele_trk_hits.GetEntries(); i++)
        {
            TrackerHit* hit = (TrackerHit*)ele_trk_hits.At(i);
            int trackhit_layer = hit->getLayer();
            int trackhit_volume = hit->getVolume();
            bool isAxial = false;
            bool isStereo = false; 
            bool isL1 = false;
            bool isL2 = false;
            bool isGood = false;

            //L1 and L2 only
            if(trackhit_layer < 2)
                isL1 = true;
            else if(trackhit_layer > 1 && trackhit_layer < 4)
                isL2 = true;
            else
                continue;

            if(trackhit_volume == 1){
                if(trackhit_layer%2 == 1)
                    isAxial = true;
                else
                    isStereo = true;
            }
            if(trackhit_volume == 0){
                if(trackhit_layer%2 == 0)
                    isAxial = true;
                else
                    isStereo = true;
            }
            //Check if truth hit is from truth track MCP
            for(int idI = 0; idI < trueHitIDs[hit->getID()].size(); idI++ )
            {
                int partID = trueHitIDs[hit->getID()].at(idI);
                //hit is from best track MCP
                if(partID == maxID_ele){
                    isGood = true;
                }
            }

            if(isGood){
                if(isAxial){
                    if(isL1)
                        ele_trueAxialL1 = true;
                    if(isL2)
                        ele_trueAxialL2 = true;
                }
                if(isStereo){
                    if(isL1)
                        ele_trueStereoL1 = true;
                    if(isL2)
                        ele_trueStereoL2 = true;
                }
            }
        }
    }

    //Positron
    if (pos_trk->isKalmanTrack()){

        for(int i = 0; i < pos_trk_hits.GetEntries(); i++)
        {
            TrackerHit* hit = (TrackerHit*)pos_trk_hits.At(i);
            int trackhit_layer = hit->getLayer();
            int trackhit_volume = hit->getVolume();
            bool isAxial = false;
            bool isStereo = false; 
            bool isL1 = false;
            bool isL2 = false;
            bool isGood = false;

            //L1 and L2 only
            if(trackhit_layer < 2)
                isL1 = true;
            else if(trackhit_layer > 1 && trackhit_layer < 4)
                isL2 = true;
            else
                continue;

            if(trackhit_volume == 1){
                if(trackhit_layer%2 == 1)
                    isAxial = true;
                else
                    isStereo = true;
            }
            if(trackhit_volume == 0){
                if(trackhit_layer%2 == 0)
                    isAxial = true;
                else
                    isStereo = true;
            }
            //Check if truth hit is from truth track MCP
            for(int idI = 0; idI < trueHitIDs[hit->getID()].size(); idI++ )
            {
                int partID = trueHitIDs[hit->getID()].at(idI);
                //hit is from best track MCP
                if(partID == maxID_pos){
                    isGood = true;
                }
            }

            if(isGood){
                if(isAxial){
                    if(isL1)
                        pos_trueAxialL1 = true;
                    if(isL2)
                        pos_trueAxialL2 = true;
                }
                if(isStereo){
                    if(isL1)
                        pos_trueStereoL1 = true;
                    if(isL2)
                        pos_trueStereoL2 = true;
                }
            }
        }
    }
    //Require both Axial and Stereo truth hits to be 'Good' hit
    if(ele_trueAxialL1 && ele_trueStereoL1) L1L2hitCode = L1L2hitCode | (0x1 << 3);           
    if(pos_trueAxialL1 && pos_trueStereoL1) L1L2hitCode = L1L2hitCode | (0x1 << 2);           
    if(ele_trueAxialL2 && ele_trueStereoL2) L1L2hitCode = L1L2hitCode | (0x1 << 1);           
    if(pos_trueAxialL2 && pos_trueStereoL2) L1L2hitCode = L1L2hitCode | (0x1 << 0);           
    
    
    //Set L1 axial/stereo hit code for ele and positron
    if(ele_trueAxialL1) L1hitCode = L1hitCode | (0x1 << 3);
    if(ele_trueStereoL1) L1hitCode = L1hitCode | (0x1 << 2);
    if(pos_trueAxialL1) L1hitCode = L1hitCode | (0x1 << 1);
    if(pos_trueStereoL1) L1hitCode = L1hitCode | (0x1 << 0);

    //Set L2 axial/stereo hit code for ele and positron
    if(ele_trueAxialL2) L2hitCode = L2hitCode | (0x1 << 3);
    if(ele_trueStereoL2) L2hitCode = L2hitCode | (0x1 << 2);
    if(pos_trueAxialL2) L2hitCode = L2hitCode | (0x1 << 1);
    if(pos_trueStereoL2) L2hitCode = L2hitCode | (0x1 << 0);
}
