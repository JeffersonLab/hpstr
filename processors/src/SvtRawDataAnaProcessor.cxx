/**
 * @file SvtRawDataAnaProcessor.cxx
 * @brief AnaProcessor used fill histograms to study svt hit fitting
 * @author Rory O'Dwyer and Cameron Bravo, SLAC National Accelerator Laboratory
 */     
#include "SvtRawDataAnaProcessor.h"

#include <iostream>

SvtRawDataAnaProcessor::SvtRawDataAnaProcessor(const std::string& name, Process& process) : Processor(name,process){
    mmapper_ = new ModuleMapper();
}
SvtRawDataAnaProcessor::~SvtRawDataAnaProcessor(){}


void SvtRawDataAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring SvtRawDataAnaProcessor" << std::endl;
    try
    {
        debug_ = parameters.getInteger("debug");
        anaName_ = parameters.getString("anaName");
        svtHitColl_ = parameters.getString("trkrHitColl");           
        histCfgFilename_ = parameters.getString("histCfg");
        regionSelections_ = parameters.getVString("regionDefinitions");
        TimeRef_ = parameters.getDouble("timeref");
        AmpRef_ = parameters.getDouble("ampref");
        doSample_ = parameters.getInteger("sample");
        MatchList_ = parameters.getVString("MatchList");
        baselineFile_ = parameters.getString("baselineFile");
        timeProfiles_ = parameters.getString("timeProfiles");
        tphase_ = parameters.getInteger("tphase");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

float SvtRawDataAnaProcessor::str_to_float(std::string token){
    std::string top1 = token.substr(0, token.find("."));
    const char *top = top1.c_str();
    std::string bot1 = token.substr(token.find(".") + 1);
    const char *bottom = bot1.c_str();
    float base = 0.0;
    for (int j=0; j<std::strlen(top); j++){
        base += ((float)((int)top[j]-48))*pow(10.0, (float)(std::strlen(top)-j-1));
    }
    for (int j=0; j<std::strlen(bottom); j++){
        base += ((float)((int)bottom[j]-48))*pow(10.0, -1*((float)j+1.0));
    } 
    return base;
}

float rETime(float ti,long T){
    bool correctTimeOffset = true;
    bool subtractTriggerTime = true;
    bool syncGood = false; 
    bool isMC = false;

    bool correctChanT0 = false;
    bool correctT0Shift = false;
    bool useTimestamps = false;
    
    long OffsetTime = 35;
    long OffsetPhase = 4;
    long trigTimeOffset = 14;
        
    if (correctTimeOffset) {
        ti = ti-OffsetTime;
    }
    
    if (subtractTriggerTime) {
        float tt = (float)(((T-4*OffsetPhase)%24)-trigTimeOffset);
        if (!syncGood) tt = tt - 8;
        if (!syncGood && (((T-4*OffsetPhase)%24)/8 < 1)) { 
            tt = tt + 24;
        }        
        ti = ti-tt;
    }
    return ti;
}

TF1* SvtRawDataAnaProcessor::fourPoleFitFunction(std::string word, int caser){ 
    const char *helper = word.data();
    if(caser==0){
        TF1* func = new TF1(helper,"(TMath::Max(x-[0],0.0)/(x-[0]))*([3])*(1/ ( exp(-(3*(([1]*([2]^3))^.25))/[1]) - ( exp(-(3*(([1]*([2]^3))^.25))/[2]) * ( ((((([1]-[2])/([1]*[2]))*(3*(([1]*([2]^3))^.25)))^(0))) +  ((((([1]-[2])/([1]*[2]))*(3*(([1]*([2]^3))^.25)))^(1))) + ((((([1]-[2])/([1]*[2]))*(3*(([1]*([2]^3))^.25)))^(2))/2) ) ) )) * ( exp(-(x-[0])/[1]) - ( exp(-(x-[0])/[2]) * ( ((((([1]-[2])/([1]*[2]))*(x-[0]))^(0))) +  ((((([1]-[2])/([1]*[2]))*(x-[0]))^(1))) + ((((([1]-[2])/([1]*[2]))*(x-[0]))^(2))/2) ) ) )",0.0,150.0);
        return func;
    }
    TF1* func2 = new TF1(helper,"(TMath::Max(x-[0],0.0)/(x-[0]))*([3])*(1/ ( exp(-(3*(([1]*([2]^3))^.25))/[1]) - ( exp(-(3*(([1]*([2]^3))^.25))/[2]) * ( ((((([1]-[2])/([1]*[2]))*(3*(([1]*([2]^3))^.25)))^(0))) +  ((((([1]-[2])/([1]*[2]))*(3*(([1]*([2]^3))^.25)))^(1))) + ((((([1]-[2])/([1]*[2]))*(3*(([1]*([2]^3))^.25)))^(2))/2) ) ) )) * ( exp(-(x-[0])/[1]) - ( exp(-(x-[0])/[2]) * ( ((((([1]-[2])/([1]*[2]))*(x-[0]))^(0))) +  ((((([1]-[2])/([1]*[2]))*(x-[0]))^(1))) + ((((([1]-[2])/([1]*[2]))*(x-[0]))^(2))/2) ) ) )+(TMath::Max(x-[5],0.0)/(x-[5]))*([8])*(1/ ( exp(-(3*(([6]*([7]^3))^.25))/[6]) - ( exp(-(3*(([6]*([7]^3))^.25))/[7]) * ( ((((([6]-[7])/([6]*[7]))*(3*(([6]*([7]^3))^.25)))^(0))) +  ((((([6]-[7])/([6]*[7]))*(3*(([6]*([7]^3))^.25)))^(1))) + ((((([6]-[7])/([6]*[7]))*(3*(([6]*([7]^3))^.25)))^(2))/2) ) ) )) * ( exp(-(x-[5])/[1]) - ( exp(-(x-[5])/[2]) * ( ((((([6]-[7])/([6]*[7]))*(x-[5]))^(0))) +  ((((([6]-[7])/([6]*[7]))*(x-[5]))^(1))) + ((((([6]-[7])/([6]*[7]))*(x-[5]))^(2))/2) ) ) )  ",0.0,150.0);
    return func2;
}

void SvtRawDataAnaProcessor::initialize(TTree* tree) {
    if(doSample_){
        //Fill in the Background Arrays
        std::ifstream myfile(baselineFile_.data());
        std::ifstream myfile2(timeProfiles_.data());
        std::string s;
        std::string s2;
        // std::vector<float [12]> baselines;
        for(int i=0; i<24576; i++){ 
            std::getline(myfile, s);
            std::getline(myfile2, s2);
            int feb = 0;
            int hyb = 0;
            int ch = 0;
            if (i>=4096){
                feb = ((i-4096)/2560);
                hyb = (i-4096-feb*2560)/640;
                ch = i-4096-feb*2560-hyb*640;
            } else {
                feb = i/2048;
                hyb = (i-feb*2048)/512;
                ch = i-feb*2048-hyb*512;
            }
            for (int j=0; j<5; j++){
                std::string token = s2.substr(0,s2.find(","));
                s2 = s2.substr(s2.find(",")+1);
                if (j>=2){
                    if (i<=4096){
                        times1_[feb][hyb][ch][j-2] = str_to_float(token);
                    } else {
                        times2_[feb][hyb][ch][j-2] = str_to_float(token);
                    }   
                }
            }
            for(int j=0; j<13; j++){
                if (j>0){
                    if (i<=4096){
                        std::string token = s.substr(0, s.find(" "));
                        if (debug_){
                            std::cout<<i<<" "<<feb<<" "<<hyb<<" "<<ch<<std::endl;
                        }
                        baseErr1_[feb][hyb][ch][j-1] = str_to_float(token);
                        //std::cout<<str_to_float(token)<<std::endl;
                        s = s.substr(s.find(" ")+1);
                    } else {
                        std::string token = s.substr(0,s.find(" ")); 
                        baseErr2_[feb][hyb][ch][j-1] = str_to_float(token);
                        //std::cout<<str_to_float(token)<<std::endl;
                        s = s.substr(s.find(" ")+1);
                        //std::cout<<s<<std::endl;
                    }
                } else {
                    s = s.substr(s.find(" ")+1);
                }
            }

        }
        myfile.close();
        myfile2.close();
        //sleep(2000);
    }

    tree_ = tree;
    tree_->SetBranchAddress(svtHitColl_.c_str(), &svtHits_, &bsvtHits_);
    tree_->SetBranchAddress("FinalStateParticles_KF", &Part_, &bPart_);
    tree_->SetBranchAddress("SiClusters", &Clusters_, &bClusters_);
    tree_->SetBranchAddress("EventHeader", &evH_, &bevH_);

    for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++) 
    {
        std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
        std::cout << "Setting up region:: " << regname << std::endl;
        reg_selectors_[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);
        reg_selectors_[regname]->setDebug(debug_);
        reg_selectors_[regname]->LoadSelection();

        reg_histos_[regname] = std::make_shared<RawSvtHitHistos>(regname,mmapper_);
        reg_histos_[regname]->loadHistoConfig(histCfgFilename_);
        reg_histos_[regname]->DefineHistos();

        regions_.push_back(regname);
    }
}

bool SvtRawDataAnaProcessor::process(IEvent* ievent) {
    Float_t TimeRef = -0.0;
    Float_t AmpRef = 1000.0;
    double weight = 1.;
    int count1 = 0;
    int count2 = 0;
    long eventTime = evH_->getEventTime();
    bool doClMatch = true;
    
    int stripID = -10000;
    int hitc = 0;
    int hitl = 0;
    float otherTime = 69420.0;
    
    //ONLY POSITRONS, MAY USE FEE's 
    //ONCE I DETERMINE A CLUSTER WHICH IS IN LINE WITH TRIG, I CAN USE ANY CLUSTERS CLOSE IN TIME.
    
    int trigPhase = (int)((eventTime%24)/4);
    if ((trigPhase!=tphase_)&&(tphase_!=6)) { return true; }
    
    for (unsigned int i = 0; i<Clusters_->size(); i++){
        Clusters_->at(i)->getLayer();
    }

    for (unsigned int I = 0; I < svtHits_->size(); I++){ 
        RawSvtHit * thisHit = svtHits_->at(I); 
        int getNum = thisHit->getFitN();
        if (doClMatch){
            bool continue_flag = true;
            for (int i = 0; i<Part_->size(); i++){
                if (Part_->at(i)->getPDG()==22) { continue; }
                if (Part_->at(i)->getCluster().getEnergy() < 0) { continue; }
                if (!((Part_->at(i)->getCluster().getTime()<=40)&&(Part_->at(i)->getCluster().getTime()>=36))) { continue; }
                for (int j = 0; j<Part_->at(i)->getTrack().getSvtHits().GetEntries(); j++){
                    TrackerHit * tHit = (TrackerHit*)(Part_->at(i)->getTrack().getSvtHits().At(j));
                    double TrackTime = Part_->at(i)->getTrack().getTrackTime();
                    for (int k = 0; k<tHit->getRawHits().GetEntries(); k++){
                        RawSvtHit * rHit = (RawSvtHit*)(tHit->getRawHits().At(k));
                        int mode = 0;
                        if ((rHit->getT0(0)==thisHit->getT0(0))&&(mode==0)){//or(mode==2))){
                            // This is the HIT ON TRACK Modes
                            bool inCluster = false;
                                int layc = 0; // THE PURPOSE OF layc IS TO COUNT THE NUMBER OF HITS PER LAYER
                                for (int cl = 0; cl < Clusters_->size(); cl++){
                                    for (int clh = 0; clh < Clusters_->at(cl)->getRawHits().GetEntries(); clh++){
                                        RawSvtHit * cluHit = (RawSvtHit*)(Clusters_->at(cl)->getRawHits().At(clh));
                                        if ((cluHit->getLayer()==thisHit->getLayer())&&(cluHit->getModule()==thisHit->getModule())){
                                            layc++;
                                        }
                                        if ((cluHit->getT0(0)==thisHit->getT0(0))){ // and(not(Clusters_->at(cl)->getID()==tHit->getID()))){
                                            inCluster = true;
                                            hitc = Clusters_->at(cl)->getRawHits().GetEntries();
                                            hitl = layc;
                                            if (Clusters_->at(cl)->getRawHits().GetEntries()==2){
                                                RawSvtHit * otherHit = (RawSvtHit*)(Clusters_->at(cl)->getRawHits().At((clh+1)%2));
                                                otherTime = otherHit->getT0(0);
                                            }
                                        }
                                    }
                                }
                            if (inCluster){
                                continue_flag = false;
                            }
                        }
                        
                        if ((rHit->getT0(0)<=-30)&&(not(rHit->getT0(0)==thisHit->getT0(0)))&&(mode==1)){
                            // This is the HIT OFF TRACK Mode
                            // You are looking at the really early time on track hits, and specifically at other hits in the same layer and module
                            // to see if you have evidence of a misplaced hit.
                            
                            if ((rHit->getLayer()==thisHit->getLayer())&&(rHit->getModule()==thisHit->getModule())){
                                stripID = rHit->getStrip();
                                // This is conditioned on it being in clusters.
                                bool inCluster = false;
                                int layc = 0; // THE PURPOSE OF layc IS TO COUNT THE NUMBER OF HITS PER LAYER
                                for( int cl = 0; cl < Clusters_->size(); cl++){
                                    for (int clh = 0; clh < Clusters_->at(cl)->getRawHits().GetEntries(); clh++){
                                        RawSvtHit * cluHit = (RawSvtHit*)(Clusters_->at(cl)->getRawHits().At(clh));
                                        if ((cluHit->getLayer()==thisHit->getLayer())&&(cluHit->getModule()==thisHit->getModule())){
                                            layc++;
                                        }
                                        if ((cluHit->getT0(0)==thisHit->getT0(0))&&(!(Clusters_->at(cl)->getID()==tHit->getID()))){
                                            inCluster = true;
                                            hitc = Clusters_->at(cl)->getRawHits().GetEntries();
                                            hitl = layc;
                                        }
                                    }
                                }
                                if(inCluster){
                                    continue_flag = false;
                                }
                            }
                        }
                    }
                }
            }
            if(continue_flag){
                return true;
            }
        }
     
        for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++){
            for (unsigned int j=0; j<getNum; j++){
                Float_t TimeDiff = -42069.0;
                Float_t AmpDiff = -42069.0;
                
                if (!(reg_selectors_[regions_[i_reg]]->passCutEq("getN_et", getNum,weight))) { continue; }

                if (getNum==2){
                    TimeDiff = (thisHit->getT0(j))-(thisHit->getT0((j+1)%2));
                    AmpDiff = (thisHit->getT0(j))-(thisHit->getT0((j+1)%2)); 
                    if (!(reg_selectors_[regions_[i_reg]]->passCutLt("TimeDiff_lt", TimeDiff*TimeDiff, weight))) { continue; }
                }
                
                if (!(reg_selectors_[regions_[i_reg]]->passCutEq("getId_lt", j, weight))) { continue; } 
                if (!(reg_selectors_[regions_[i_reg]]->passCutEq("getId_gt", j, weight))) { continue; }   
                if (!(reg_selectors_[regions_[i_reg]]->passCutLt("chi_lt", thisHit->getChiSq(j), weight))) { continue; }
                if (!(reg_selectors_[regions_[i_reg]]->passCutGt("chi_gt", thisHit->getChiSq(j), weight))) { continue; }
                                
                
                if (!(reg_selectors_[regions_[i_reg]]->passCutLt("doing_ft", (((thisHit->getT0(j))-TimeRef)*((thisHit->getT0(j))-TimeRef) < ((thisHit->getT0((j+1)%getNum)-TimeRef)*(thisHit->getT0((j+1)%getNum)-TimeRef)+.00001)), weight))) { continue; }
                if (i_reg < regionSelections_.size()-1){
                    if (!(reg_selectors_[regions_[i_reg]]->passCutLt("doing_ct", (((thisHit->getT0(j))-TimeRef)*((thisHit->getT0(j))-TimeRef) > ((thisHit->getT0((j+1)%getNum)-TimeRef)*(thisHit->getT0((j+1)%getNum)-TimeRef)+.00001)), weight))) { continue; }
                } else {
                    if (getNum==2){
                        if (!(reg_selectors_[regions_[i_reg]]->passCutLt("doing_ct", (((thisHit->getT0(j))-TimeRef)*((thisHit->getT0(j))-TimeRef) > ((thisHit->getT0((j+1)%getNum)-TimeRef)*(thisHit->getT0((j+1)%getNum)-TimeRef)+.00001)), weight))) {  continue; }
                    }
                }
                if (!(reg_selectors_[regions_[i_reg]]->passCutLt("doing_ca", (((thisHit->getAmp(j))-AmpRef)*((thisHit->getAmp(j))-AmpRef) < ((thisHit->getAmp((j+1)%getNum)-AmpRef)*(thisHit->getAmp((j+1)%getNum)-AmpRef)+.00001)), weight))) { continue; }

                if (!(reg_selectors_[regions_[i_reg]]->passCutLt("doing_fterr", (((thisHit->getT0err(j))-TimeRef)*((thisHit->getT0err(j))-TimeRef) < ((thisHit->getT0err((j+1)%getNum)-TimeRef)*(thisHit->getT0err((j+1)%getNum)-TimeRef)+.00001)), weight))) { continue; }
                if (!(reg_selectors_[regions_[i_reg]]->passCutLt("doing_cterr", (((thisHit->getT0err(j))-0.0)*((thisHit->getT0err(j))-0.0) > ((thisHit->getT0err((j+1)%getNum)-0.0)*(thisHit->getT0err((j+1)%getNum)-0.0)+.00001)),weight))) { continue; }
   
                if (!(reg_selectors_[regions_[i_reg]]->passCutLt("amp_lt", thisHit->getAmp(0), weight))) { continue; }
                if (!(reg_selectors_[regions_[i_reg]]->passCutGt("amp_gt", thisHit->getAmp(0), weight))) { continue; }

                if (!(reg_selectors_[regions_[i_reg]]->passCutLt("time_lt", thisHit->getT0(0), weight))) { continue; }
                if (!(reg_selectors_[regions_[i_reg]]->passCutGt("time_gt", thisHit->getT0(0), weight))) { continue; }

                if (!(reg_selectors_[regions_[i_reg]]->passCutLt("Otime_lt", (float)(thisHit->getT0((j+1)%getNum)), weight))) { continue; }
                if (!(reg_selectors_[regions_[i_reg]]->passCutGt("Otime_gt", (float)(thisHit->getT0((j+1)%getNum)), weight))) { continue; }

                if (!(reg_selectors_[regions_[i_reg]]->passCutLt("Stime_lt", (float)(thisHit->getT0((j)%getNum)), weight))) { continue; }
                if (!(reg_selectors_[regions_[i_reg]]->passCutGt("Stime_gt", (float)(thisHit->getT0((j)%getNum)), weight))) { continue; }

                if (!(reg_selectors_[regions_[i_reg]]->passCutLt("amp2_lt", thisHit->getAmp(0), weight))) { continue; }
                if (!(reg_selectors_[regions_[i_reg]]->passCutEq("channel", (thisHit->getStrip()), weight))) { continue; } 
                int * adcs = thisHit->getADCs();
                int maxx = 0;
                for (unsigned int k=0; k<6; k++){
                    if (maxx<adcs[k]) { maxx = adcs[k]; }
                }
                if (!(reg_selectors_[regions_[i_reg]]->passCutEq("first_max", adcs[0]-maxx, weight))) { continue; }
                
                bool helper = false; 
                if (doSample_==1){
                    int len=*(&readout+1)-readout;
                    for (int k=0; k<len; k++){
                        if(readout[k]<200){
                            helper = true;
                        }
                    }
                }
                if ((doSample_==1)&&(helper)){
                    int N = evH_->getEventNumber();
                    if ((regions_[i_reg]=="CTFit")&&((thisHit->getT0(j)<26.0)||(thisHit->getT0(j)>30.0))) { continue; }
                    sample(thisHit,regions_[i_reg], ievent, eventTime, N); 
                
                }
                reg_histos_[regions_[i_reg]]->FillHistograms(thisHit, weight, j, I, TimeDiff, AmpDiff, stripID, hitc, hitl, otherTime);
                }
            }
        }
        return true;
    }    

    void SvtRawDataAnaProcessor::sample(RawSvtHit* thisHit, std::string word, IEvent* ievent, long T, int N){
        auto mod = std::to_string(thisHit->getModule());
        auto lay = std::to_string(thisHit->getLayer());
        //swTag= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
        std::string helper = mmapper_->getHwFromSw("ly" + lay + "_m" + mod); 
        //std::cout<<helper<<std::endl;
        char char_array[helper.length()+1];
        std::strcpy(char_array,helper.c_str());
        int feb = (int)char_array[1]-48;
        int hyb = (int)char_array[3]-48;
        
        
        if ((feb>=2) && (word=="OneFit")) { return; }
        if ((feb<2) && (word=="CTFit")) { return; }
        if (thisHit->getChiSq(0) < .85) { return; }
                
        int count = 0;
        if (feb <=1 ){
            count += feb*2048 + hyb*512 + (int)(thisHit->getStrip());
        } else {
            count += 4096;
            count += (feb-2)*2560 + hyb*640 + (int)(thisHit->getStrip());
        }
        int * adcs2 = thisHit->getADCs(); 

        TF1* fitfunc = fourPoleFitFunction("Pulse 0",0);
        TF1* fitfunc2 = fourPoleFitFunction("Pulse 1",0);
        TF1* fitfunc3 = fourPoleFitFunction("Addition",1);
        float TimeShift[2] = {-1*rETime(-(thisHit->getT0(0)), T), -1*rETime(-(thisHit->getT0(1)), T)};

        fitfunc->FixParameter(0, TimeShift[0]);
        fitfunc->FixParameter(3, thisHit->getAmp(0));    
        if (feb <=1 ){
            fitfunc->FixParameter(1, times1_[feb][hyb][(int)thisHit->getStrip()][1]);
            fitfunc->FixParameter(2, times1_[feb][hyb][(int)thisHit->getStrip()][2]);
            fitfunc->FixParameter(4, baseErr1_[feb][hyb][(int)thisHit->getStrip()][1]);
            //baseline->FixParameter(0,baseErr1_[feb][hyb][(int)thisHit->getStrip()][1]);
        } else {
            fitfunc->FixParameter(1, times2_[feb-2][hyb][(int)thisHit->getStrip()][1]);
            fitfunc->FixParameter(2, times2_[feb-2][hyb][(int)thisHit->getStrip()][2]);
            fitfunc->FixParameter(4, baseErr2_[feb-2][hyb][(int)thisHit->getStrip()][1]); 
        }
        if (thisHit->getFitN()==2){
            fitfunc2->FixParameter(0, TimeShift[1]);
            fitfunc2->FixParameter(3, thisHit->getAmp(1));

            fitfunc3->FixParameter(0, TimeShift[0]);
            fitfunc3->FixParameter(3, thisHit->getAmp(0));
            fitfunc3->FixParameter(5, TimeShift[1]);
            fitfunc3->FixParameter(8, thisHit->getAmp(1));

            if (feb <=1 ){
                fitfunc2->FixParameter(1, times1_[feb][hyb][(int)thisHit->getStrip()][1]);
                fitfunc2->FixParameter(2, times1_[feb][hyb][(int)thisHit->getStrip()][2]);
                fitfunc2->FixParameter(4, baseErr1_[feb][hyb][(int)thisHit->getStrip()][1]);

                fitfunc3->FixParameter(1, times1_[feb][hyb][(int)thisHit->getStrip()][1]);
                fitfunc3->FixParameter(2, times1_[feb][hyb][(int)thisHit->getStrip()][2]);
                fitfunc3->FixParameter(4, baseErr1_[feb][hyb][(int)thisHit->getStrip()][1]);

                fitfunc3->FixParameter(6, times1_[feb][hyb][(int)thisHit->getStrip()][1]);
                fitfunc3->FixParameter(7, times1_[feb][hyb][(int)thisHit->getStrip()][2]);

            } else {
                fitfunc2->FixParameter(1, times2_[feb-2][hyb][(int)thisHit->getStrip()][1]);
                fitfunc2->FixParameter(2, times2_[feb-2][hyb][(int)thisHit->getStrip()][2]);
                fitfunc2->FixParameter(4, baseErr2_[feb-2][hyb][(int)thisHit->getStrip()][1]); 

                fitfunc3->FixParameter(1, times2_[feb-2][hyb][(int)thisHit->getStrip()][1]);
                fitfunc3->FixParameter(2, times2_[feb-2][hyb][(int)thisHit->getStrip()][2]);
                fitfunc3->FixParameter(4, baseErr2_[feb-2][hyb][(int)thisHit->getStrip()][1]);

                fitfunc3->FixParameter(6, times2_[feb-2][hyb][(int)thisHit->getStrip()][1]);
                fitfunc3->FixParameter(7, times2_[feb-2][hyb][(int)thisHit->getStrip()][2]);
            } 
        }

        int Length = MatchList_.size();
        for (int k=0; k<Length; k++){
            if ((word==MatchList_[k])&&(readout[k]<200)){ 
                readout[k]++;
                std::string helper1 = "Feb: " + std::to_string(feb) + ",Hyb: " + std::to_string(hyb) + ",ch: " + std::to_string((int)thisHit->getStrip()) + ", chi_sqr value: " + std::to_string((float)thisHit->getChiSq(0));
                const char *thing1 = helper1.data();
                TCanvas *c1 = new TCanvas("c");
                c1->DrawFrame(0.0, 3000.0, 150.0, 7000.0);
                c1->SetTitle(thing1);
                float times[12]; float points[12]; float errors[12]; float zeroes[12];
                for(int i=0; i<6; i++){
                    zeroes[i] = 0.0;
                    if (feb <=1 ){
                        times[i] = float(i)*24.0;//-27.0;
                        points[i] = adcs2[i]-baseErr1_[feb][hyb][(int)thisHit->getStrip()][i];
                        errors[i] = baseErr1_[feb][hyb][(int)thisHit->getStrip()][i+6];
                    } else {
                        times[i] = float(i)*24.0;//rETime((float(i))*24.0,T);//(float(i))*24.0-27.0;
                        points[i] = adcs2[i]-baseErr2_[feb-2][hyb][(int)thisHit->getStrip()][i];
                        errors[i] = baseErr2_[feb-2][hyb][(int)thisHit->getStrip()][i+6];
                    }
                }
                auto gr = new TGraphErrors(6, times, points, zeroes, errors);
                gr->SetName("ADCs");
                gr->SetTitle(thing1);
                gr->GetYaxis()->SetTitle("ADC Counts");
                gr->GetXaxis()->SetTitle("ns");
                gr->GetXaxis()->SetLimits(-10.0, 130.);
                gr->GetHistogram()->SetMaximum(2000.);
                gr->GetHistogram()->SetMinimum(-500.);
                gr->Draw("AL*");
                
                fitfunc->Draw("same"); 
                if (thisHit->getFitN()==2){
                    fitfunc2->SetLineColor(kGreen);
                    fitfunc2->Draw("same"); 
                    fitfunc3->SetLineColor(kOrange);
                    fitfunc3->SetTitle(thing1);
                    fitfunc3->Draw("same");

                }
                auto legend = new TLegend(0.1,0.7,.48,.9);
                legend->AddEntry("gr", "ADC counts");
                legend->AddEntry("base", "Offline Baselines");
                legend->AddEntry("Pulse 0", "First Pulse");
                if (thisHit->getFitN()==2){
                    legend->AddEntry("Pulse 1", "Second Pulse");
                    legend->AddEntry("Addition", "Summed Fit");
                }
                legend->Draw("same");
                std::string helper2 = word + std::to_string(readout[k]-1) + ".png";
                const char *thing2 = helper2.data(); 
                c1->SaveAs(thing2);
            }
        }
    }

    void SvtRawDataAnaProcessor::finalize() {

        outF_->cd();
        for(reg_it it = reg_histos_.begin(); it!=reg_histos_.end(); ++it){
            std::string dirName = it->first;
            (it->second)->saveHistos(outF_,dirName);
            outF_->cd(dirName.c_str());
        }
        outF_->Close();

    }

DECLARE_PROCESSOR(SvtRawDataAnaProcessor);
