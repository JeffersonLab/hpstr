#ifndef __NTUPPLIZER_H__
#define __NTUPPLIZER_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>
#include <string>

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"

//-----------//
//   hpstr   //
//-----------//
#include "Processor.h"
#include "BaseSelector.h"
#include "Track.h"
#include "Event.h"
#include "TrackHistos.h"
#include "TrackerHit.h"
#include "Particle.h"
#include "CalCluster.h"
#include "MCParticle.h"
#include "RawSvtHit.h"
#include "ModuleMapper.h"


#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TAxis.h"
#include "TROOT.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TLegend.h"



// Forward declarations
class TTree; 

/**
 * @brief Insert description here.
 * more details
 */
class NTupplizer : public Processor { 

    public:
        /**
         * @brief Class constructor. 
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        NTupplizer(const std::string& name, Process& process); 

        /** Destructor */
        ~NTupplizer(); 

        /**
         * @brief Configure the Ana Processor
         * 
         * @param parameters The configuration parameters
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * @brief Process the event and put new data products into it.
         * 
         * @param ievent The Event to process.
         */
        virtual bool process(IEvent* ievent);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events starts.
         * 
         * @param tree
         */
        virtual void initialize(TTree* tree);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events finishes.
         */
        virtual void finalize();

	virtual void WriteRoot();
        
	virtual float str_to_float(std::string word);

    private: 

	TTree* tree_;

	TH1F* Tracks_;
	TH1F* Track_Lambda[12][14];	
	TH1F* Track_Phi[12][14];	


	std::ofstream csvFile_{nullptr};
	int outPutCsv_{1};

	int counter_{0};
	int counter2_{0};
        std::string baselineFile_;
        std::string timeProfiles_;
	ModuleMapper * mmapper_;
	/** Container to hold all Track objects. */
        std::vector<Track*>* tracks_{};
        TBranch* btracks_{nullptr}; //!< description

	std::vector<Particle*>* fsps_{};
	TBranch* bfsps_{nullptr};

	std::vector<CalCluster*>* eclusters_{}; 
        TBranch* beclusters_{nullptr};

	std::vector<TrackerHit*>* sclusters_{};
        TBranch* bsclusters_{nullptr};
		

	float times1_[2][4][512][3];
        float times2_[8][4][640][3];
        float baseErr1_[2][4][512][12];
        float baseErr2_[8][4][640][12];

	TBranch* bevH_;
        EventHeader * evH_;

	std::vector<MCParticle*>* mcpart_{};
	TBranch* bmcpart_{nullptr};

        //std::vector<Track*>* trtracks_{};
	//TBranch* btrtracks_{nullptr}; //!< description

        std::string trkCollName_{"KalmanFullTracks"}; //!< Track Collection name
	std::string fspCollName_{"FinalStateParticles_KF"};
	std::string eclusCollName_{"RecoEcalClusters"}; //!< description
	std::string mcCollName_{"MCParticle"};
	std::string sclusCollName_{"SiClusters"};
	//std::string num_{"0"};

	//std::string truthtrkCollName_{"Truth_KFTracks"};
        
	int readout_{0};
	
	int debug_{0}; //!< debug level

};

#endif
