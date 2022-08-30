/**
 * @file ECalDataProcessor.h
 * @brief Processor used to convert ECal LCIO data to ROOT. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef __ECAL_DATA_PROCESSOR_H__
#define __ECAL_DATA_PROCESSOR_H__

//----------------//
//   C++ StdLib   //
//----------------//
#include <string>
#include <vector>

//----------//
//   LCIO   //
//----------//
#include <EVENT/CalorimeterHit.h>
#include <IMPL/CalorimeterHitImpl.h>
#include <IMPL/ClusterImpl.h>
#include <UTIL/BitField64.h>

//----------//
//   ROOT   //
//----------//
#include <TClonesArray.h>

//-----------//
//   hpstr   //
//-----------//
#include "CalCluster.h"
#include "CalHit.h"
#include "Collections.h"
#include "Processor.h"

typedef long long long64;

// Forward declarations
class TTree;

/**
 * @brief Insert description here.
 * more details
 */
class ECalDataProcessor : public Processor { 

    public: 

        /**
         * @brief Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        ECalDataProcessor(const std::string& name, Process& process); 

        /** Destructor */
        ~ECalDataProcessor(); 

        /**
         * @brief Callback for the Processor to configure itself from the given set of parameters.
         * 
         * @param parameters ParameterSet for configuration.
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events starts.
         * 
         * @param tree
         */
        virtual void initialize(TTree* tree);

        /**
         * @brief Process the event and put new data products into it.
         * 
         * @param event The Event to process.
         */
        virtual bool process(IEvent* event);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events finishes.
         */
        virtual void finalize();

    private: 

        /**
         * @brief Method to unpack field value from a calorimeter hit ID.
         * 
         * @param field The field ID to unpack
         * @param hit The CalorimeterHit whose ID will be used to unpack the 
         *            the field value. 
         */
        UTIL::BitFieldValue getIdentifierFieldValue(std::string field, EVENT::CalorimeterHit* hit);

        /** TClonesArray collection containing all ECal hits. */ 
        std::vector<CalHit*> cal_hits_; 
        std::string hitCollLcio_{"EcalCalHits"}; //!< description
        std::string hitCollRoot_{"RecoEcalHits"}; //!< description

        /** TClonesArray collection containing all ECal clusters. */
        std::vector<CalCluster*> clusters_; 
        std::string clusCollLcio_{"EcalClustersCorr"}; //!< description
        std::string clusCollRoot_{"RecoEcalClusters"}; //!< description

        /** Encoding string describing cell ID. */
        const std::string encoder_string_{"system:6,layer:2,ix:-8,iy:-6"};

        int debug_{0}; //!< Debug Level

}; // ECalDataProcessor

#endif // __ECAL_DATA_PROCESSOR_H__
