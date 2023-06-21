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

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);
        
        virtual void sample(RawSvtHit* thisHit, std::string word, IEvent* ievent, long t,int i);

        virtual TF1* fourPoleFitFunction(std::string word, int caser);

        virtual float str_to_float(std::string word);

        float reverseEngineerTime(float ti, long t);

        //virtual int maximum(int arr[]);
        
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

        TBranch* bevH_;

        std::vector<RawSvtHit*> * svtHits_{};
        VTPData * vtpBank_;
        TSData * tsBank_;
        std::vector<CalCluster*>* recoClu_{};
        std::vector<Track*>* Trk_{};
        std::vector<Particle*>* Part_{};
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
