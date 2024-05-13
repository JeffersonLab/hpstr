/**
 * @file SvtRawDataAnaProcessor.cxx
 * @brief AnaProcessor used fill histograms to study svt hit fitting
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */     
#include "SvtRawDataAnaProcessor.h"

#include <iostream>

SvtRawDataAnaProcessor::SvtRawDataAnaProcessor(const std::string& name, Process& process) : Processor(name,process){
    mmapper_ = new ModuleMapper();
}
//TODO CHECK THIS DESTRUCTOR
SvtRawDataAnaProcessor::~SvtRawDataAnaProcessor(){}


void SvtRawDataAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring SvtRawDataAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        svtHitColl_     = parameters.getString("trkrHitColl");           
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

/**
 *
 *THIS METHOD IS IMPLEMENTED BECAUSE C++ std:of METHOD WHICH CONVERTS STRINGS
 TO FLOATS IS NOT WORKING. WE NEED THIS TO READ IN OFFLINE BASELINES AND CHARACTERISTIC TIMES.
 *
 *
 * */


float SvtRawDataAnaProcessor::str_to_float(std::string token){
    std::string top1=token.substr(0,token.find("."));
    const char *top=top1.c_str();
    std::string bot1=token.substr(token.find(".")+1);
    const char *bottom=bot1.c_str();
    float base = 0.0;
    for(int J=0;J<std::strlen(top);J++){
        base+=((float)((int)top[J]-48))*pow(10.0,(float)(std::strlen(top)-J-1));
    }
    for(int J=0;J<std::strlen(bottom);J++){
        base+=((float)((int)bottom[J]-48))*pow(10.0,-1*((float)J+1.0));
    } 
    return base;
}

//float reverseEngineerTime(float ti,IEvent* ievent){
//    return ti-1.0;
//}
 

float rETime(float ti,long T){
    bool correctTimeOffset = true;
    bool subtractTriggerTime = true;
    bool syncGood = false; 
    bool isMC = false;

    bool correctChanT0 = false;
    bool correctT0Shift = false;
    
    //bool subtractTOF = true;
    
    
    bool useTimestamps = false;
    
    //sensor=hit.getSensor();
    long OffsetTime = 35;
    long OffsetPhase = 4;
    long trigTimeOffset = 14;
    //THIS ONE
    //std::cout<<"Initial Time: "<<ti<<std::endl;
    
    if (correctTimeOffset) {
        //if (debug)
        //    System.out.println("subtracting svt time offset " + timingConstants.getOffsetTime());
        ti=ti-OffsetTime;
    }
    

    //THIS ONE
    if (subtractTriggerTime) {
        //std::cout<<T<<std::endl;
        //std::cout<<"OffsetPhase Time is "<<((T-4*OffsetPhase)%24)/8<<std::endl;
        float tt = (float)(((T-4*OffsetPhase)%24)-trigTimeOffset);
        //std::cout<<tt<<std::endl;
        if (!syncGood) tt = tt - 8;
        //std::cout<<tt<<std::endl;
        if (!syncGood && (((T-4*OffsetPhase)%24)/8 < 1)) { 
            tt = tt + 24;//std::cout<<"I SHIFTED"<<std::endl;
        }
        //std::cout<<tt<<std::endl;
        //else{std::cout<<"I DID NOT SHIFT"<<std::endl;}
        //if (isMC && ((((int)T - 4 * (int)OffsetPhase) % 24)/8 == 1)) {
        //    tt = tt + 24;
        //} 
        
        ti=ti-tt;
    }
    return ti;
}


/*
 *
 *FOUR POLE PULSE FUNCTION AND THE SUM OF TWO OF THEM WITH BASELINES BORROWED FROM ALIC
 *
 */

TF1* SvtRawDataAnaProcessor::fourPoleFitFunction(std::string word, int caser){ 
    const char *helper = word.data();
    if(caser==0){
        TF1* func = new TF1(helper,"(TMath::Max(x-[0],0.0)/(x-[0]))*([3])*(1/ ( exp(-(3*(([1]*([2]^3))^.25))/[1]) - ( exp(-(3*(([1]*([2]^3))^.25))/[2]) * ( ((((([1]-[2])/([1]*[2]))*(3*(([1]*([2]^3))^.25)))^(0))) +  ((((([1]-[2])/([1]*[2]))*(3*(([1]*([2]^3))^.25)))^(1))) + ((((([1]-[2])/([1]*[2]))*(3*(([1]*([2]^3))^.25)))^(2))/2) ) ) )) * ( exp(-(x-[0])/[1]) - ( exp(-(x-[0])/[2]) * ( ((((([1]-[2])/([1]*[2]))*(x-[0]))^(0))) +  ((((([1]-[2])/([1]*[2]))*(x-[0]))^(1))) + ((((([1]-[2])/([1]*[2]))*(x-[0]))^(2))/2) ) ) )",0.0,150.0);
        return func;
    }
    TF1* func2 = new TF1(helper,"(TMath::Max(x-[0],0.0)/(x-[0]))*([3])*(1/ ( exp(-(3*(([1]*([2]^3))^.25))/[1]) - ( exp(-(3*(([1]*([2]^3))^.25))/[2]) * ( ((((([1]-[2])/([1]*[2]))*(3*(([1]*([2]^3))^.25)))^(0))) +  ((((([1]-[2])/([1]*[2]))*(3*(([1]*([2]^3))^.25)))^(1))) + ((((([1]-[2])/([1]*[2]))*(3*(([1]*([2]^3))^.25)))^(2))/2) ) ) )) * ( exp(-(x-[0])/[1]) - ( exp(-(x-[0])/[2]) * ( ((((([1]-[2])/([1]*[2]))*(x-[0]))^(0))) +  ((((([1]-[2])/([1]*[2]))*(x-[0]))^(1))) + ((((([1]-[2])/([1]*[2]))*(x-[0]))^(2))/2) ) ) )+(TMath::Max(x-[5],0.0)/(x-[5]))*([8])*(1/ ( exp(-(3*(([6]*([7]^3))^.25))/[6]) - ( exp(-(3*(([6]*([7]^3))^.25))/[7]) * ( ((((([6]-[7])/([6]*[7]))*(3*(([6]*([7]^3))^.25)))^(0))) +  ((((([6]-[7])/([6]*[7]))*(3*(([6]*([7]^3))^.25)))^(1))) + ((((([6]-[7])/([6]*[7]))*(3*(([6]*([7]^3))^.25)))^(2))/2) ) ) )) * ( exp(-(x-[5])/[1]) - ( exp(-(x-[5])/[2]) * ( ((((([6]-[7])/([6]*[7]))*(x-[5]))^(0))) +  ((((([6]-[7])/([6]*[7]))*(x-[5]))^(1))) + ((((([6]-[7])/([6]*[7]))*(x-[5]))^(2))/2) ) ) )  ",0.0,150.0);
    return func2;
}

/*
 *
 *PROCESS INITIALIZER. READS IN THE OFFLINE BASELINES INTO LOCAL BASELINE FILES, READs in the PULSE SHAPES, and FINALLLY
 ESTABLISHES REGIONS WHICH ARE USED ALONG WITH THE REGION SELECTOR CLASS AND CUTS IN ANALYSIS/SELECTION/SVT TO SELECT ON
 EVENTS FOR WHICH HISTOGRAMS IN RAWSVTHISTO IS FILLED.
 *
 *
 */

void SvtRawDataAnaProcessor::initialize(TTree* tree) {
    if(doSample_){
        //Fill in the Background Arrays
        std::ifstream myfile(baselineFile_.data());
        std::ifstream myfile2(timeProfiles_.data());
        std::string s;
        std::string s2;
        // std::vector<float [12]> baselines;
        for(int i=0; i<24576; i++){ 
            std::getline(myfile,s);
            std::getline(myfile2,s2);
            int feb=0;
            int hyb=0;
            int ch=0;
            if(i>=4096){
                feb=((i-4096)/2560);
                hyb=(i-4096-feb*2560)/640;
                ch=i-4096-feb*2560-hyb*640;
            }else{
                feb=i/2048;
                hyb=(i-feb*2048)/512;
                ch=i-feb*2048-hyb*512;
            }
            for(int I=0;I<5;I++){
                std::string token=s2.substr(0,s2.find(","));
                s2=s2.substr(s2.find(",")+1);
                if(I>=2){
                    if(i<=4096){
                        times1_[feb][hyb][ch][I-2]=str_to_float(token);
                    }else{
                        times2_[feb][hyb][ch][I-2]=str_to_float(token);
                    }   
                }
            }
            //if(i<2048){
            //std::cout<<i<<" "<<feb<<" "<<hyb<<" "<<ch<<std::endl;
            //std::cout<<s<<std::endl;}
            for(int I=0;I<13;I++){
                if(I>0){
                    if(i<=4096){
                        std::string token=s.substr(0,s.find(" "));
                        if(debug_){
                            std::cout<<i<<" "<<feb<<" "<<hyb<<" "<<ch<<std::endl;
                        }
                        baseErr1_[feb][hyb][ch][I-1]=str_to_float(token);
                        //std::cout<<str_to_float(token)<<std::endl;
                        s=s.substr(s.find(" ")+1);
                    }else{
                        std::string token=s.substr(0,s.find(" ")); 
                        baseErr2_[feb][hyb][ch][I-1]=str_to_float(token);
                        //std::cout<<str_to_float(token)<<std::endl;
                        s=s.substr(s.find(" ")+1);
                        //std::cout<<s<<std::endl;
                    }
                }else{
                    s=s.substr(s.find(" ")+1);
                }
            }

        }
        myfile.close();
        myfile2.close();
        //sleep(2000);
    }


    tree_= tree;
    // init histos
    //histos = new RawSvtHitHistos(anaName_.c_str(), mmapper_);
    //histos->loadHistoConfig(histCfgFilename_);
    //histos->DefineHistos();
    //std::cout<<svtHitColl_.c_str()<<std::endl;
    ///std::cout<<svtHits_->size()<<std::endl;
    tree_->SetBranchAddress(svtHitColl_.c_str()  , &svtHits_    , &bsvtHits_    );
    //tree_->SetBranchAddress("VTPBank", &vtpBank_ , &bvtpBank_ );
    //tree_->SetBranchAddress("TSBank", &tsBank_ , &btsBank_ );
    //tree_->SetBranchAddress("RecoEcalClusters",&recoClu_,&brecoClu_ );
    //tree_->SetBranchAddress("KalmanFullTracks",&Trk_,&bTrk_);
    tree_->SetBranchAddress("FinalStateParticles_KF",&Part_,&bPart_);
    tree_->SetBranchAddress("EventHeader",&evH_,&bevH_);
    
    for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++) 
    {
        std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
        std::cout << "Setting up region:: " << regname << std::endl;
        reg_selectors_[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);
        reg_selectors_[regname]->setDebug(debug_);
        reg_selectors_[regname]->LoadSelection();

        reg_histos_[regname] = std::make_shared<RawSvtHitHistos>(regname,mmapper_);
        reg_histos_[regname]->loadHistoConfig(histCfgFilename_);
        //reg_histos_[regname]->doTrackComparisonPlots(false);
        reg_histos_[regname]->DefineHistos();

        regions_.push_back(regname);
    }
}

/*
 *
 *RUNS OVER THE REGION SELECTORS AND CHECKS IF AN EVENT PASSES A SELECTION JSON AND FILLS A RAWSVTHITHISTO
 IF DO SAMPLE IS ON, IT RUNS SAMPLING.
 *
 *
 */


bool SvtRawDataAnaProcessor::process(IEvent* ievent) {
    Float_t TimeRef=-0.0;
    Float_t AmpRef=1000.0;
    double weight = 1.;int count1=0;int count2=0;
    long eventTime = evH_->getEventTime();
    
    //I AM DOING CLUSTER MATCHING HERE :)
    //std::cout<<"Here is the eventTime"<<eventTime<<std::endl;
    //std::cout<<"Here is the TSBank Trigger Time"<<tsBank_->T<<std::endl;
    bool doClMatch = true;
    
    
    //if((doClMatch)and(not((tsBank_->prescaled.Single_3_Top==1)or(tsBank_->prescaled.Single_3_Bot==1)))){return true;}
    
    
    //ONLY POSITRONS, MAY USE FEE's 
    //ONCE I DETERMINE A CLUSTER WHICH IS IN LINE WITH TRIG, I CAN USE ANY CLUSTERS CLOSE IN TIME.
    
    //std::cout<<"Trigger Time: "<<vtpBank_->singletrigs.at(0).T<<std::endl;
    int trigPhase =  (int)((eventTime%24)/4);
    if((trigPhase!=tphase_)&&(tphase_!=6)){return true;}
    for(unsigned int i = 0; i < svtHits_->size(); i++){ 
        RawSvtHit * thisHit = svtHits_->at(i); 
        int getNum = thisHit->getFitN();//std::cout<<"I got here 10"<<std::endl;
        if(doClMatch){
            bool Continue = true;
            for(int i = 0; i<Part_->size();i++){
                if(Part_->at(i)->getPDG()==22){continue;}
                if(Part_->at(i)->getCluster().getEnergy()<0){continue;}
                if(not((Part_->at(i)->getCluster().getTime()<=40)and(Part_->at(i)->getCluster().getTime()>=36))){continue;}
                //std::cout<<"For each Tracker Hit I now print out Raw Hit Info: "<<std::endl;
                for(int j = 0; j<Part_->at(i)->getTrack().getSvtHits().GetEntries();j++){
                    TrackerHit * tHit = (TrackerHit*)(Part_->at(i)->getTrack().getSvtHits().At(j));
                    //std::cout<<tHit->getTime()<<std::endl;
                    for(int k = 0;k<tHit->getRawHits().GetEntries();k++){
                        RawSvtHit * rHit = (RawSvtHit*)(tHit->getRawHits().At(k));
                        if(rHit->getT0(0)==thisHit->getT0(0)){Continue=false;}
                        //std::cout<<"Raw Hit T0: "<<rHit->getT0(0)<<std::endl;
                    }
                    //std::cout<<" T0: "<<Part_->at(i)->getTrack().getSvtHits().At(j)->getRawHits().At(0).getT0()<<std::endl; 
                }
            }
            if(Continue){
                return true;
            }
        }       
        for(unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++){
            //std::cout<<"\n"<<std::endl;
            for(unsigned int J=0; J<getNum; J++){
                //std::cout<<"\ngetNum:"<<getNum<<std::endl;
                //std::cout<<"region No:"<<regions_[i_reg]<<std::endl;

                //std::cout<<"Which Hit:"<<J<<std::endl;                
                Float_t TimeDiff=-42069.0;
                Float_t AmpDiff=-42069.0;
                
                if(!(reg_selectors_[regions_[i_reg]]->passCutEq("getN_et",getNum,weight))){continue;}

                if(getNum==2){
                    TimeDiff=(thisHit->getT0(J))-(thisHit->getT0((J+1)%2));
                    AmpDiff=(thisHit->getT0(J))-(thisHit->getT0((J+1)%2)); 
                    if(!(reg_selectors_[regions_[i_reg]]->passCutLt("TimeDiff_lt",TimeDiff*TimeDiff,weight))){continue;}
                }                
                if(!(reg_selectors_[regions_[i_reg]]->passCutEq("getId_lt",J,weight))){continue;} 
                if(!(reg_selectors_[regions_[i_reg]]->passCutEq("getId_gt",J,weight))){continue;}   
                if(!(reg_selectors_[regions_[i_reg]]->passCutLt("chi_lt",thisHit->getChiSq(J),weight))){continue;}
                if(!(reg_selectors_[regions_[i_reg]]->passCutGt("chi_gt",thisHit->getChiSq(J),weight))){continue;}
                                
                
                if(!(reg_selectors_[regions_[i_reg]]->passCutLt("doing_ft",(((thisHit->getT0(J))-TimeRef)*((thisHit->getT0(J))-TimeRef)<((thisHit->getT0((J+1)%getNum)-TimeRef)*(thisHit->getT0((J+1)%getNum)-TimeRef)+.00001)),weight))){continue;}
                if(i_reg<regionSelections_.size()-1){
                    if(!(reg_selectors_[regions_[i_reg]]->passCutLt("doing_ct",(((thisHit->getT0(J))-TimeRef)*((thisHit->getT0(J))-TimeRef)>((thisHit->getT0((J+1)%getNum)-TimeRef)*(thisHit->getT0((J+1)%getNum)-TimeRef)+.00001)),weight))){continue;}
                }else{
                    if(getNum==2){
                        if(!(reg_selectors_[regions_[i_reg]]->passCutLt("doing_ct",(((thisHit->getT0(J))-TimeRef)*((thisHit->getT0(J))-TimeRef)>((thisHit->getT0((J+1)%getNum)-TimeRef)*(thisHit->getT0((J+1)%getNum)-TimeRef)+.00001)),weight))){continue;}
                    }
                }
                if(!(reg_selectors_[regions_[i_reg]]->passCutLt("doing_ca",(((thisHit->getAmp(J))-AmpRef)*((thisHit->getAmp(J))-AmpRef)<((thisHit->getAmp((J+1)%getNum)-AmpRef)*(thisHit->getAmp((J+1)%getNum)-AmpRef)+.00001)),weight))){continue;}

                if(!(reg_selectors_[regions_[i_reg]]->passCutLt("doing_fterr",(((thisHit->getT0err(J))-TimeRef)*((thisHit->getT0err(J))-TimeRef)<((thisHit->getT0err((J+1)%getNum)-TimeRef)*(thisHit->getT0err((J+1)%getNum)-TimeRef)+.00001)),weight))){continue;}
                if(!(reg_selectors_[regions_[i_reg]]->passCutLt("doing_cterr",(((thisHit->getT0err(J))-0.0)*((thisHit->getT0err(J))-0.0)>((thisHit->getT0err((J+1)%getNum)-0.0)*(thisHit->getT0err((J+1)%getNum)-0.0)+.00001)),weight))){continue;}

                //if(!(std::abs((thisHit->getT0(J))-TimeRef)<std::abs(thisHit->getT0((J+1)%2)-TimeRef))){continue;}          
                //if(!(reg_selectors_[regions_[i_reg]]->passCutEq("doing_ca",1.0,weight))){continue;}else{
                //if(!(std::abs((thisHit->getT0(J))-TimeRef)<std::abs(thisHit->getT0((J+1)%2)-TimeRef))){continue;}          
                if(!(reg_selectors_[regions_[i_reg]]->passCutLt("amp_lt",thisHit->getAmp(0),weight))){continue;}
                if(!(reg_selectors_[regions_[i_reg]]->passCutGt("amp_gt",thisHit->getAmp(0),weight))){continue;}

                if(!(reg_selectors_[regions_[i_reg]]->passCutLt("time_lt",thisHit->getT0(0),weight))){continue;}
                if(!(reg_selectors_[regions_[i_reg]]->passCutGt("time_gt",thisHit->getT0(0),weight))){continue;}

                //std::cout<<(float)(thisHit->getT0((J+1)%getNum))<<std::endl;
                //std::cout<<!(reg_selectors_[regions_[i_reg]]->passCutLt("Otime_lt",(float)(thisHit->getT0((J+1)%getNum)),weight))<<std::endl;
                //std::cout<<!(reg_selectors_[regions_[i_reg]]->passCutGt("Otime_gt",(float)(thisHit->getT0((J+1)%getNum)),weight))<<std::endl;
                if(!(reg_selectors_[regions_[i_reg]]->passCutLt("Otime_lt",(float)(thisHit->getT0((J+1)%getNum)),weight))){continue;}
                if(!(reg_selectors_[regions_[i_reg]]->passCutGt("Otime_gt",(float)(thisHit->getT0((J+1)%getNum)),weight))){continue;}

                 if(!(reg_selectors_[regions_[i_reg]]->passCutLt("Stime_lt",(float)(thisHit->getT0((J)%getNum)),weight))){continue;}
                if(!(reg_selectors_[regions_[i_reg]]->passCutGt("Stime_gt",(float)(thisHit->getT0((J)%getNum)),weight))){continue;}


                if(!(reg_selectors_[regions_[i_reg]]->passCutLt("amp2_lt",thisHit->getAmp(0),weight))){continue;}
                if(!(reg_selectors_[regions_[i_reg]]->passCutEq("channel", (thisHit->getStrip()),weight))){continue;} 
                int * adcs=thisHit->getADCs();
                int maxx = 0;
                for(unsigned int K=0; K<6; K++){
                    if(maxx<adcs[K]){maxx=adcs[K];}
                }
                if(!(reg_selectors_[regions_[i_reg]]->passCutEq("first_max",adcs[0]-maxx,weight))){continue;}
                
                //if(!(reg_selectors_[regions_[i_reg]]->passCutEq("time_phase",trigPhase,weight))){continue;}
                
                bool helper = false; 
                if(doSample_==1){
                    int len=*(&readout+1)-readout;
                    for(int KK=0;KK<len;KK++){
                        if(readout[KK]<200){
                            helper=true;
                        }
                    }
                }
                if((doSample_==1)and(helper)){
                    int N = evH_->getEventNumber();
                    //std::cout<<T<<std::endl;
                    //if((regions_[i_reg]=="OneFit")and(feb>=2)){continue;}
                    if((regions_[i_reg]=="CTFit")and((thisHit->getT0(J)<26.0)or(thisHit->getT0(J)>30.0))){continue;}
                    sample(thisHit,regions_[i_reg],ievent,eventTime,N); 
                
                }

                reg_histos_[regions_[i_reg]]->FillHistograms(thisHit,weight,J,i,TimeDiff,AmpDiff);
            }
            }
        }
        //std::cout<<count1<<std::endl;
        //std::cout<<count2<<std::endl;
        return true;
    }

    /*
     *
     *READS IN FROM THE LOCAL BASELINE AND TIME ARRAYS THE PULSE SHAPES AND BASELINES AND PLOTS THEM OVER THE ADC COUNTS
     THIS ALLOWS US, WHEN ACTIVATED, TO SEE WHAT DECISIONS ARE BEING MADE BY THE FITTING ALGORITHM GIVEN SOME CUT ON OUR PULSES
     ESTABLISHED BY THE REGION SELECTION IN PROCESS ABOVE.
     *
     *
     */

    void SvtRawDataAnaProcessor::sample(RawSvtHit* thisHit,std::string word, IEvent* ievent,long T,int N){
        auto mod = std::to_string(thisHit->getModule());
        auto lay = std::to_string(thisHit->getLayer());
        //swTag= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
        std::string helper = mmapper_->getHwFromSw("ly"+lay+"_m"+mod); 
        //std::cout<<helper<<std::endl;
        char char_array[helper.length()+1];
        std::strcpy(char_array,helper.c_str());
        int feb = (int)char_array[1]-48;
        int hyb = (int)char_array[3]-48;
        
        
        if((feb>=2)and(word=="OneFit")){return;}
        if((feb<2)and(word=="CTFit")){return;}
        if(thisHit->getChiSq(0)<.85){return;}
        
        
        
        //std::cout<<"Feb "<<feb<<" ,Hyb "<<hyb<<" ,Baseline Feb<=1: "<<baseErr1_[0][0][(int)thisHit->getStrip()][0]<<" ,Baseline Feb>1: "<<baseErr1_[0][1][(int)thisHit->getStrip()][0]<<" More Baselines "<<baseErr1_[0][2][(int)thisHit->getStrip()][0]<<" ,Baseline Feb>1: "<<baseErr1_[0][3][(int)thisHit->getStrip()][0]<<std::endl;
        int BigCount = 0;
        if(feb<=1){
            BigCount+=feb*2048+hyb*512+(int)(thisHit->getStrip());
        }else{
            BigCount+=4096;
            BigCount+=(feb-2)*2560+hyb*640+(int)(thisHit->getStrip());
        }
        //std::cout<<"READ HERE "<<BigCount<<" "<<feb<<" "<<hyb<<" "<<(int)thisHit->getStrip()<<" "<<baseErr1_[feb][hyb][(int)thisHit->getStrip()][0]<<std::endl;
        int * adcs2=thisHit->getADCs(); 
        //std::cout<<regions_[i_reg]<<" "<<readout<<std::endl;

        TF1* fitfunc = fourPoleFitFunction("Pulse 0",0);
        TF1* fitfunc2 = fourPoleFitFunction("Pulse 1",0);
        TF1* fitfunc3 = fourPoleFitFunction("Addition",1);
        //TF1* baseline = new TF1("base","[0]",0.0,150.0);
        //rETime((float(i))*24.0,T)
        float TimeShift[2] = {-1*rETime(-(thisHit->getT0(0)),T),-1*rETime(-(thisHit->getT0(1)),T)};

        fitfunc->FixParameter(0,TimeShift[0]);
        fitfunc->FixParameter(3,thisHit->getAmp(0));    
        if(feb<=1){
            fitfunc->FixParameter(1,times1_[feb][hyb][(int)thisHit->getStrip()][1]);
            fitfunc->FixParameter(2,times1_[feb][hyb][(int)thisHit->getStrip()][2]);
            fitfunc->FixParameter(4,baseErr1_[feb][hyb][(int)thisHit->getStrip()][1]);
            //baseline->FixParameter(0,baseErr1_[feb][hyb][(int)thisHit->getStrip()][1]);
        }else{
            fitfunc->FixParameter(1,times2_[feb-2][hyb][(int)thisHit->getStrip()][1]);
            fitfunc->FixParameter(2,times2_[feb-2][hyb][(int)thisHit->getStrip()][2]);
            fitfunc->FixParameter(4,baseErr2_[feb-2][hyb][(int)thisHit->getStrip()][1]); 
            //baseline->FixParameter(0,baseErr2_[feb-2][hyb][(int)thisHit->getStrip()][1]);
        }
        if(thisHit->getFitN()==2){
            fitfunc2->FixParameter(0,TimeShift[1]);
            fitfunc2->FixParameter(3,thisHit->getAmp(1));

            fitfunc3->FixParameter(0,TimeShift[0]);
            fitfunc3->FixParameter(3,thisHit->getAmp(0));
            fitfunc3->FixParameter(5,TimeShift[1]);
            fitfunc3->FixParameter(8,thisHit->getAmp(1));

            if(feb<=1){
                fitfunc2->FixParameter(1,times1_[feb][hyb][(int)thisHit->getStrip()][1]);
                fitfunc2->FixParameter(2,times1_[feb][hyb][(int)thisHit->getStrip()][2]);
                fitfunc2->FixParameter(4,baseErr1_[feb][hyb][(int)thisHit->getStrip()][1]);

                fitfunc3->FixParameter(1,times1_[feb][hyb][(int)thisHit->getStrip()][1]);
                fitfunc3->FixParameter(2,times1_[feb][hyb][(int)thisHit->getStrip()][2]);
                fitfunc3->FixParameter(4,baseErr1_[feb][hyb][(int)thisHit->getStrip()][1]);

                fitfunc3->FixParameter(6,times1_[feb][hyb][(int)thisHit->getStrip()][1]);
                fitfunc3->FixParameter(7,times1_[feb][hyb][(int)thisHit->getStrip()][2]);

            }else{
                fitfunc2->FixParameter(1,times2_[feb-2][hyb][(int)thisHit->getStrip()][1]);
                fitfunc2->FixParameter(2,times2_[feb-2][hyb][(int)thisHit->getStrip()][2]);
                fitfunc2->FixParameter(4,baseErr2_[feb-2][hyb][(int)thisHit->getStrip()][1]); 

                fitfunc3->FixParameter(1,times2_[feb-2][hyb][(int)thisHit->getStrip()][1]);
                fitfunc3->FixParameter(2,times2_[feb-2][hyb][(int)thisHit->getStrip()][2]);
                fitfunc3->FixParameter(4,baseErr2_[feb-2][hyb][(int)thisHit->getStrip()][1]);

                fitfunc3->FixParameter(6,times2_[feb-2][hyb][(int)thisHit->getStrip()][1]);
                fitfunc3->FixParameter(7,times2_[feb-2][hyb][(int)thisHit->getStrip()][2]);
            } 
        }

        //for(int P=0;P<2;P++){for(int PP=0;PP<4;PP++){ std::cout<<"baseline: "<<baseErr1_[P][PP][(int)thisHit->getStrip()][0]<<std::endl;  }}

        //std::cout<<"baseline: "<<baseErr1_[feb][hyb][(int)thisHit->getStrip()][0]<<" "<<baseErr2_[feb-2][hyb][(int)thisHit->getStrip()][0]<<std::endl;
        //std::cout<<"type for above baseline "<<word<<std::endl;
        int Length=MatchList_.size();
        for(int K=0; K<Length;K++){
            //std::cout<<K<<std::endl;
            if((word==MatchList_[K])and(readout[K]<200)){ 
                //gPad->vRange(0.0,3000.0,150.0,6000.0);
                readout[K]++;
                //auto gr = new TGraph();
                //auto gr2 = new TGraph();

                std::string helper1="Feb: "+std::to_string(feb)+",Hyb: "+std::to_string(hyb)+",ch: "+std::to_string((int)thisHit->getStrip())+", chi_sqr value: "+std::to_string((float)thisHit->getChiSq(0));
                const char *thing1 = helper1.data();
                //gr2->SetPoint(0,0.,3000.);gr2->SetPoint(1,0.,6000.);gr2->SetPoint(2,150.0,6000.);
                //gPad->DrawFrame(0.0,3000.0,150.0,6000.0);
                TCanvas *c1 = new TCanvas("c");
                c1->DrawFrame(0.0,3000.0,150.0,7000.0);
                c1->SetTitle(thing1);
                //auto gr = new TGraphErrors();
                //gr->SetName("ADCs");
                float times[12]; float points[12]; float errors[12]; float zeroes[12];
                for(int i=0;i<6;i++){
                    //std::cout<<adcs2[i]<<std::endl; 
                    //rETime((float(i))*24.0,T)
                    zeroes[i]=0.0;
                    if(feb<=1){
                        times[i]=float(i)*24.0;//-27.0;
                        points[i]=adcs2[i]-baseErr1_[feb][hyb][(int)thisHit->getStrip()][i];
                        errors[i]=baseErr1_[feb][hyb][(int)thisHit->getStrip()][i+6];
                    }else{
                        //std::cout<<(float(i))*24.0-27.0<<"  "<<rETime((float(i))*24.0,T)<<std::endl;
                        times[i]=float(i)*24.0;//rETime((float(i))*24.0,T);//(float(i))*24.0-27.0;
                        points[i]=adcs2[i]-baseErr2_[feb-2][hyb][(int)thisHit->getStrip()][i];
                        errors[i]=baseErr2_[feb-2][hyb][(int)thisHit->getStrip()][i+6];
                    }
                }
                auto gr = new TGraphErrors(6,times,points,zeroes,errors);
                gr->SetName("ADCs");
                gr->SetTitle(thing1);
                gr->GetYaxis()->SetTitle("ADC Counts");
                gr->GetXaxis()->SetTitle("ns");
                gr->GetXaxis()->SetLimits(-10.0,130.);
                gr->GetHistogram()->SetMaximum(2000.);
                gr->GetHistogram()->SetMinimum(-500.);
                //gr2->Draw("");
                gr->Draw("AL*");
                //TF1* helper1 = (TF1*)fitfunc->Clone("ff");

                //baseline->SetLineColor(kBlue);
                //baseline->Draw("same"); 

                fitfunc->Draw("same"); 
                if(thisHit->getFitN()==2){
                    fitfunc2->SetLineColor(kGreen);
                    fitfunc2->Draw("same"); 
                    fitfunc3->SetLineColor(kOrange);
                    fitfunc3->SetTitle(thing1);
                    fitfunc3->Draw("same");


                    //TF1 *add = new TF1("ff+gg-base",);
                    //TF1* fitfunc3 = new TF1("ll","ff+gg+base");
                    //TF1* h = new TF1("hh","ff+gg");
                    //add->Draw("same");
                    //auto ADD = new TF1();
                }
                //gr->Draw("same");
                auto legend = new TLegend(0.1,0.7,.48,.9);
                legend->AddEntry("gr","ADC counts");
                legend->AddEntry("base","Offline Baselines");
                legend->AddEntry("Pulse 0","First Pulse");
                if(thisHit->getFitN()==2){
                    legend->AddEntry("Pulse 1","Second Pulse");
                    legend->AddEntry("Addition","Summed Fit");
                }
                legend->Draw("same");
                std::string helper2=word+std::to_string(readout[K]-1)+".png";
                const char *thing2 = helper2.data(); 
                c1->SaveAs(thing2);
                //gPad->Clear();
                
                
                //if(helper2=="OneFit8.png"){
                    //std::cout<<BigCount<<std::endl;
                    //std::cout<<feb<<" "<<hyb<<" "<<(int)thisHit->getStrip()<<std::endl;
                    //std::cout<<adcs2[0]<<" "<<adcs2[1]<<" "<<adcs2[2]<<" "<<adcs2[3]<<" "<<adcs2[4]<<" "<<adcs2[5]<<std::endl;
                    //std::cout<<adcs2[0]-baseErr1_[feb][hyb][(int)thisHit->getStrip()][0]<<" "<<adcs2[1]-baseErr1_[feb][hyb][(int)thisHit->getStrip()][1]<<" "<<adcs2[2]-baseErr1_[feb][hyb][(int)thisHit->getStrip()][2]<<" "<<adcs2[3]-baseErr1_[feb][hyb][(int)thisHit->getStrip()][3]<<" "<<adcs2[4]-baseErr1_[feb][hyb][(int)thisHit->getStrip()][4]<<" "<<adcs2[5]-baseErr1_[feb][hyb][(int)thisHit->getStrip()][5]<<std::endl;
                //}
                
                
                //std::cout<<helper2<<std::endl;
                //std::cout<<N<<std::endl;
                

                //std::cout<<adcs2[0]<<" "<<adcs2[1]<<" "<<adcs2[2]<<" "<<adcs2[3]<<" "<<adcs2[4]<<" "<<adcs2[5]<<" "<<thisHit->getAmp(0)<<" "<<thisHit->getT0(0)<<" "<<BigCount<<" "<<thisHit->getChiSq(0)<<std::endl;
            }
        }
    }

    /*
     *FILLS IN HISTOGRAMS
     *
     */

    void SvtRawDataAnaProcessor::finalize() {

        outF_->cd();
        for(reg_it it = reg_histos_.begin(); it!=reg_histos_.end(); ++it){
            std::string dirName = it->first;
            (it->second)->saveHistos(outF_,dirName);
            outF_->cd(dirName.c_str());
            //reg_selectors_[it->first]->getCutFlowHisto()->Scale(.5);
            //reg_selectors_[it->first]->getCutFlowHisto()->Write();
        }
        outF_->Close();

    }

DECLARE_PROCESSOR(SvtRawDataAnaProcessor);
