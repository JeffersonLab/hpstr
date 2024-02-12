#ifndef __RAWSVTHIT_ANAPROCESSOR_H__
#define __RAWSVTHIT_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "RawSvtHit.h"
#include "RawSvtHitHistos.h"
#include "AnaHelpers.h"
#include "Event.h"
#include "BaseSelector.h"
#include "RawSvtHitHistos.h"
#include "EventHeader.h"
#include "VTPData.h"
#include "TSData.h"
#include "CalCluster.h"
#include "Track.h"
#include "TrackerHit.h"

//#include <IMPL/TrackerHitImpl.h>"
//ROOT

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

class TTree;


class SvtRawDataAnaProcessor : public Processor {

    public:

        SvtRawDataAnaProcessor(const std::string& name, Process& process);

        ~SvtRawDataAnaProcessor();

        /*
         *
         *RUNS OVER THE REGION SELECTORS AND CHECKS IF AN EVENT PASSES A SELECTION JSON AND FILLS A RAWSVTHITHISTO
         IF DO SAMPLE IS ON, IT RUNS SAMPLING.
         *
         *
         */

        virtual bool process(IEvent* ievent);

        /*
         *
         *PROCESS INITIALIZER. READS IN THE OFFLINE BASELINES INTO LOCAL BASELINE FILES, READs in the PULSE SHAPES, and FINALLLY
         ESTABLISHES REGIONS WHICH ARE USED ALONG WITH THE REGION SELECTOR CLASS AND CUTS IN ANALYSIS/SELECTION/SVT TO SELECT ON
         EVENTS FOR WHICH HISTOGRAMS IN RAWSVTHISTO IS FILLED.
         *
         *
         */

        virtual void initialize(TTree* tree);
        
        virtual void sample(RawSvtHit* thisHit, std::string word, IEvent* ievent, long t,int i);

        /*
         *
         *FOUR POLE PULSE FUNCTION AND THE SUM OF TWO OF THEM WITH BASELINES BORROWED FROM ALIC
         *
         */

        virtual TF1* fourPoleFitFunction(std::string word, int caser);

        /**
         *
         *THIS METHOD IS IMPLEMENTED BECAUSE C++ std:of METHOD WHICH CONVERTS STRINGS
         *TO FLOATS IS NOT WORKING. WE NEED THIS TO READ IN OFFLINE BASELINES AND CHARACTERISTIC TIMES.
         *
         *
         * */

        virtual float str_to_float(std::string word);

        float reverseEngineerTime(float ti, long t);

        /*
         *FILLS IN HISTOGRAMS
         *
         */

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

        //Containers to hold histogrammer info
        RawSvtHitHistos* histos{nullptr};
        std::string  histCfgFilename_;
        Float_t TimeRef_;
        Float_t AmpRef_;
        ModuleMapper * mmapper_;
        int * adcs_;
        int doSample_;
        int readout[3]={0,0,0};
       
        float times1_[2][4][512][3];
        float times2_[8][4][640][3];
        float baseErr1_[2][4][512][12];
        float baseErr2_[8][4][640][12];
        std::vector<std::string> MatchList_;

        TTree* tree_;
        TBranch* bsvtHits_{nullptr};
        TBranch* bvtpBank_{nullptr};
        TBranch* btsBank_{nullptr};
        TBranch* brecoClu_{nullptr};
        TBranch* bPart_{nullptr};
        TBranch* bTrk_{nullptr};
        TBranch* bClusters_{nullptr};

        TBranch* bevH_;
        /*
        TTree *HitEff_;
        //TBranch* bL1_{nullptr};
        int L1_;
        bool doHitEff_{false};
        */
        std::vector<RawSvtHit*> * svtHits_{};
        VTPData * vtpBank_;
        TSData * tsBank_;
        std::vector<CalCluster*>* recoClu_{};
        std::vector<Track*>* Trk_{};
        std::vector<Particle*>* Part_{};
        std::vector<TrackerHit*>* Clusters_{};
        
        //std::vector<Track> Trk_{};
        EventHeader * evH_;

        std::string anaName_{"rawSvtHitAna"};
        std::string svtHitColl_{"RotatedHelicalTrackHits"};
        std::vector<std::string> regionSelections_;
        std::map<std::string, std::shared_ptr<BaseSelector>> reg_selectors_;
        std::map<std::string,std::shared_ptr<RawSvtHitHistos>> reg_histos_;
        typedef std::map<std::string,std::shared_ptr<RawSvtHitHistos>>::iterator reg_it;
        std::vector<std::string> regions_;
        std::string baselineFile_;
        std::string timeProfiles_;
        int tphase_{6};

        //Debug Level
        int debug_{0};

};

#endif
