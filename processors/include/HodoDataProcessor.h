/**
 *  @file   HodoDataProcessor.cxx
 *  @brief  Processor for the Hodoscope, to convert LCIO to the HodoCluster and HodoHit classes in event.
 *  @author Maurik Holtrop, University of New Hampshire, 11/13/19.
 */

#ifndef HodoDataProcessor_h
#define HodoDataProcessor_h

#include <stdio.h>
#include "Event.h"

#include "Processor.h"
#include "HodoHit.h"
#include "HodoCluster.h"

//----------//
//   LCIO   //
//----------//
#include "EVENT/CalorimeterHit.h"

#include "IMPL/LCGenericObjectImpl.h"
#include "IMPL/CalorimeterHitImpl.h"
#include "IMPL/ClusterImpl.h"
#include "UTIL/BitField64.h"

// Forward declarations
class TTree;
class HodoDataProcessor : public Processor {
    
public:
    
    /**
     * Class constructor.
     *
     * @param name Name for this instance of the class.
     * @param process The Process class associated with Processor, provided
     *                by the processing framework.
     */
    HodoDataProcessor(const std::string& name, Process& process): Processor(name, process) {};
    
    /** Destructor */
    ~HodoDataProcessor(){};
    
    /**
     * Callback from ConfigurePython step for the Processor to configure itself from the given set of parameters.
     * Parameters are set in the Python configuration file as: hodo.parameters["debug"] = 0  where "debug" is the name of the parameter.
     * @param parameters ParameterSet for configuration.
     */
    virtual void configure(const ParameterSet& parameters);

    
    /**
     * Process the event and put new data products into it.
     * @param event The Event to process.
     */
    virtual bool process(IEvent* event);
    
    /**
     * Callback for the Processor to take any necessary
     * action when the processing of events starts.
     */
    virtual void initialize(TTree* tree);
    
    /**
     * Callback for the Processor to take any necessary
     * action when the processing of events finishes.
     */
    virtual void finalize(){};
    
// private:
    
    /**
     * Method to unpack field value from a hodoscope hit ID from the ID string.
     * The string itself comes from the detector description (in hps-java/detector-data/...), and should be identical to that one.
     *
     * @param field The field ID to unpack
     * @param hit The CalorimeterHit whose ID will be used to unpack the
     *            the field value.
     */
    UTIL::BitFieldValue getIdentifierFieldValue(std::string field, EVENT::CalorimeterHit* hit);
    
    /** TClonesArray collection containing all ECal hits. */
    std::vector<HodoHit*> hits_;

    /** TClonesArray collection containing all ECal clusters. */
    std::vector<HodoCluster*> clusters_;
    
    /** Encoding string describing cell ID. */
    const std::string encoder_string_{"system:6,barrel:3,layer:4,ix:4,iy:-3,hole:-3"};
    
    /** Configurable parameters.
     * Defaults are passed to the "configure" step, so can be set here.
     */
    int debug_{1};  // Debug level. 0 is very quiet, 1 info, 2 debug

    std::string hitCollLcio_{"HodoCalHits"};
    std::string hitCollLcio_Generic_{"HodoGenericHits"};
    std::string hitCollRoot_{"HodoHits"};
    
    std::string clusCollLcio_{"HodoGenericClusters"};
    std::string clusCollRoot_{"HodoClusters"};
    

}; // ECalDataProcessor



#endif /* HodoDataProcessor_h */
