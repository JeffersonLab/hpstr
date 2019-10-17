/**
 * @file Collection.h
 * @brief List of collection names.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#ifndef _COLLECTIONS_H_
#define _COLLECTIONS_H_

namespace Collections { 

    /** Name of the tracks collection. */
    constexpr const char* GBL_TRACKS{"GBLTracks"};

    /** Name of the Raw tracker hits collection. */
    constexpr const char* RAW_SVT_HITS{"SVTRawTrackerHits"};

    /** Name of the Raw tracker hits collection. */
    constexpr const char* RAW_SVT_HITS_ON_TRACK{"SVTRawHitsOnTrack"};

    /** Name of the Raw tracker fit relations collection. */
    constexpr const char* RAW_SVT_HIT_FITS{"SVTFittedRawTrackerHits"};

    /** Name of the Raw tracker fit relations collection. */
    constexpr const char* RAW_SVT_HIT_FITSP{"SVTShapeFitParameters"};

    /** Name of the tracker hits collection. */
    constexpr const char* TRACKER_HITS{"RotatedHelicalTrackHits"};

    /** The name of the collection containing GBL kink data GenericObjects */
    constexpr const char* KINK_DATA{"GBLKinkData"};

    /** Name of the collection containing Track to GBLKinkData LCRelations. */
    constexpr const char* KINK_DATA_REL{"GBLKinkDataRelations"};

    /** Name of the collection of TrackData GenericObjects */ 
    constexpr const char* TRACK_DATA{"TrackData"}; 

    /** Name of the collection containing Track to TrackData LCRelations */
    constexpr const char* TRACK_DATA_REL{"TrackDataRelations"};

    /** Name of the collection containing Final State Particles. */
    constexpr const char* FINAL_STATE_PARTICLES{"FinalStateParticles"}; 

    /** Name of the collection containing MC Particles. */
    constexpr const char* MC_PARTICLES{"MCParticle"}; 

    /** Name of time corrected ECal hits collection. */
    constexpr const char* ECAL_TIME_CORR_HITS{"TimeCorrEcalHits"}; 

    /** Name of ECal hits collection. */
    constexpr const char* ECAL_HITS{"EcalCalHits"}; 

    /** Name of ECal clusters collection. */
    constexpr const char* ECAL_CLUSTERS{"EcalClustersCorr"};

    /** Name of collection containing "other electrons". */
    constexpr const char* OTHER_ELECTRONS{"OtherElectrons"};

    /** Name of the collection of event headers. */
    constexpr const char* EVENT_HEADERS{"EventHeader"}; 
    
    /** Name of trigger bank collection. */
    constexpr const char* TRIGGER_BANK{"TriggerBank"}; 

    /** Name of vtp (JLab VME Trigger Processor) bank collection. */
    constexpr const char* VTP_BANK{"VTPBank"}; 

    /** Name of trigger supervisor bank collection. */
    constexpr const char* TS_BANK{"TSBank"}; 

    /** Name of RF hits collection. */
    constexpr const char* RF_HITS{"RFHits"};

    /** There are three typical type of V0Candidates: BeamspotConstrained, TargetConstrained and Unconstrained */
  
    /** Base name of the V0Candidates collection. */
    constexpr const char* V0CANDIDATES{"V0Candidates"};

    /** Name of the BSC V0Candidates collection. */
    constexpr const char* BSC_V0CANDIDATES{"BeamspotConstrainedV0Candidates"};

    /** Name of the TC V0Candidates collection. */
    constexpr const char* TC_V0CANDIDATES{"TargetConstrainedV0Candidates"};

    /** Name of the UC V0Candidates collection. */
    constexpr const char* UC_V0CANDIDATES{"UnConstrainedV0Candidates"};

    /** Name of the V0Vertices collection. */
    constexpr const char* V0VERTICES{"V0Vertices"};

    /** Name of the BSC V0Vertices collection. */
    constexpr const char* BSC_V0VERTICES{"BeamspotConstrainedV0Vertices"};

    /** Name of the TC V0Candidates collection. */
    constexpr const char* TC_V0Vertices{"TargetConstrainedV0Vertices"};

    /** Name of the UC V0Candidates collection. */
    constexpr const char* UC_V0VERTICES{"UnConstrainedV0Vertices"};
   
}

#endif // _COLLECTION_H_

