/**
 * @file SvtRawDataProcessor.h
 * @brief Processor used to add Raw SVT data to tree
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */
#include "SvtRawDataProcessor.h" 
#include "utilities.h"

SvtRawDataProcessor::SvtRawDataProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

SvtRawDataProcessor::~SvtRawDataProcessor() { 
}

void SvtRawDataProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring SvtRawDataProcessor" << std::endl;
    try
    {
        debug_         = parameters.getInteger("debug", debug_);
        hitCollLcio_   = parameters.getString("hitCollLcio", hitCollLcio_);
        hitfitCollLcio_   = parameters.getString("hitfitCollLcio", hitfitCollLcio_);
        hitCollRoot_   = parameters.getString("hitCollRoot", hitCollRoot_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}


void SvtRawDataProcessor::initialize(TTree* tree) {

    tree->Branch(hitCollRoot_.c_str(),&rawhits_);
}

bool SvtRawDataProcessor::process(IEvent* ievent) {

    Event* event = static_cast<Event*>(ievent);
    UTIL::LCRelationNavigator* rawTracker_hit_fits_nav;
    // Get the collection of 3D hits from the LCIO event. If no such collection 
    // exist, a DataNotAvailableException is thrown
    EVENT::LCCollection* raw_svt_hits{nullptr};
    try
    {
        raw_svt_hits = event->getLCCollection(hitCollLcio_.c_str());
    }
    catch (EVENT::DataNotAvailableException e) 
    {
        std::cout << e.what() << std::endl;
    }

    //Check to see if fits are in the file
    auto evColls = event->getLCEvent()->getCollectionNames();
    auto it = std::find (evColls->begin(), evColls->end(), hitfitCollLcio_.c_str());
    bool hasFits = true;
    EVENT::LCCollection* raw_svt_hit_fits;
    if(it == evColls->end()) hasFits = false;
    if(hasFits) 
    {
        raw_svt_hit_fits = event->getLCCollection(hitfitCollLcio_.c_str()); 
        // Heap an LCRelation navigator which will allow faster access 
        rawTracker_hit_fits_nav = new UTIL::LCRelationNavigator(raw_svt_hit_fits);

    }

    // Get decoders to read cellids
    UTIL::BitField64 decoder("system:6,barrel:3,layer:4,module:12,sensor:1,side:32:-2,strip:12");
    //decoder[field] returns the value

    // Loop over all of the raw SVT hits in the LCIO event and add them to the 
    // HPS event
    for(int i = 0; i < rawhits_.size(); i++) delete rawhits_.at(i);
    rawhits_.clear();

    for (int ihit = 0; ihit < raw_svt_hits->getNumberOfElements(); ++ihit) {

        // Get a raw hit from the list of hits
        EVENT::TrackerRawData* rawTracker_hit 
            = static_cast<EVENT::TrackerRawData*>(raw_svt_hits->getElementAt(ihit));
        //Decode the cellid
        EVENT::long64 value = EVENT::long64( rawTracker_hit->getCellID0() & 0xffffffff ) | 
            ( EVENT::long64( rawTracker_hit->getCellID1() ) << 32 ) ;
        decoder.setValue(value);

        // Add a raw tracker hit to the event
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
            (int)rawTracker_hit->getADCValues().at(5)
        };
        rawHit->setADCs(hit_adcs);


        if (hasFits)
        {
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
        }
        rawhits_.push_back(rawHit);
    }

    //Clean up
    if (hasFits) delete rawTracker_hit_fits_nav;

    return true;
}

void SvtRawDataProcessor::finalize() { 
}

DECLARE_PROCESSOR(SvtRawDataProcessor); 
