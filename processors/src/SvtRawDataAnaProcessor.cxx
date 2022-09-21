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
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

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


TF1* SvtRawDataAnaProcessor::fourPoleFitFunction(){
        TF1* func = new TF1("pulsefit","(TMath::Max(x-[0],0.0)/(x-[0]))*([3])*(([1]^2)/(([1]-[2])^(3))) * ( exp(-(x-[0])/[1]) - ( exp(-(x-[0])/[2]) * ( ((((([1]-[2])/([1]*[2]))*(x-[0]))^(0))) +  ((((([1]-[2])/([1]*[2]))*(x-[0]))^(1))) + ((((([1]-[2])/([1]*[2]))*(x-[0]))^(2))/2) ) ) ) + [4]",0.0,150.0,"");
            return func;
}

void SvtRawDataAnaProcessor::initialize(TTree* tree) {
    if(doSample_){
        //Fill in the Background Arrays
        std::ifstream myfile("/sdf/group/hps/users/rodwyer1/hps_14552_offline_baselines.dat");
        std::ifstream myfile2("/sdf/group/hps/users/rodwyer1/hpssvt_014393_database_svt_pulse_shapes_final.dat");
        std::string s;
        std::string s2;
        std::vector<float [12]> baselines;
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
                    if(feb<=1){
                        times1_[feb][hyb][ch][I-2]=str_to_float(token);
                    }else{
                        times2_[feb][hyb][ch][I-2]=str_to_float(token);
                    }   
                }
            } 
            for(int I=0;I<13;I++){
                if(I>0){
                    if(feb<=1){
                            std::string token=s.substr(0,s.find(" "));
                            baseErr1_[feb][hyb][ch][I-1]=str_to_float(token);
                            s=s.substr(s.find(" ")+1);
                    }else{
                            std::string token=s.substr(0,s.find(" ")); 
                            baseErr2_[feb][hyb][ch][I-1]=str_to_float(token);
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
    }
    
    
    tree_= tree;
    // init histos
    //histos = new RawSvtHitHistos(anaName_.c_str(), mmapper_);
    //histos->loadHistoConfig(histCfgFilename_);
    //histos->DefineHistos();
    //std::cout<<"hello4"<<std::endl;
    //std::cout<<svtHitColl_.c_str()<<std::endl;
    ///std::cout<<svtHits_->size()<<std::endl;
    tree_->SetBranchAddress(svtHitColl_.c_str()  , &svtHits_    , &bsvtHits_    );
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

bool SvtRawDataAnaProcessor::process(IEvent* ievent) {
    //std::cout<<"hello5"<<std::endl;
    Float_t TimeRef=-0.0;
    Float_t AmpRef=1000.0;
    double weight = 1.;int count1=0;int count2=0;
    for(unsigned int i = 0; i < svtHits_->size(); i++){ 
        RawSvtHit * thisHit = svtHits_->at(i);
        int getNum = thisHit->getFitN();
        for (unsigned int i_reg = 0; i_reg < regionSelections_.size(); i_reg++){
            //std::cout<<"\n"<<std::endl;
            for(unsigned int J=0; J<getNum; J++){
                //std::cout<<"\ngetNum:"<<getNum<<std::endl;
                //std::cout<<"region No:"<<regions_[i_reg]<<std::endl;
                
                //std::cout<<"Which Hit:"<<J<<std::endl;
                if(!(reg_selectors_[regions_[i_reg]]->passCutEq("getN_et",getNum,weight))){continue;}
                if(!(reg_selectors_[regions_[i_reg]]->passCutEq("getId_lt",J,weight))){continue;} 
                if(!(reg_selectors_[regions_[i_reg]]->passCutEq("getId_gt",J,weight))){continue;}
                //std::cout<<"getNum:"<<getNum<<std::endl;
                //std::cout<<"region No:"<<regions_[i_reg]<<std::endl;
                //std::cout<<"Which Hit:"<<J<<"\n"<<std::endl;
 
                
                //if((getNum==2)and(i_reg==0)){
                //    if(J==0){count1+=1;std::cout<<"hello"<<std::endl;}else{count2+=1;}
                //} 
                //if(getNum==2){std::cout<<J<<std::endl;}
                
                //std::cout<<"hellO"<<std::endl;           
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
                


                if(!(reg_selectors_[regions_[i_reg]]->passCutLt("amp2_lt",thisHit->getAmp(0),weight))){continue;}
                if(!(reg_selectors_[regions_[i_reg]]->passCutEq("channel", (thisHit->getStrip()),weight))){continue;} 
                int * adcs=thisHit->getADCs();
                int maxx = 0;
                for(unsigned int K=0; K<6; K++){
                    if(maxx<adcs[K]){maxx=adcs[K];}
                }
                if(!(reg_selectors_[regions_[i_reg]]->passCutEq("first_max",adcs[0]-maxx,weight))){continue;}
                //if(!(reg_selectors_[regions_[i_reg]]->passCutEq("second_max",adcs[0]-maxx,weight))){continue;}
                //std::cout<<"getNum:"<<getNum<<std::endl;
                //std::cout<<"region No:"<<regions_[i_reg]<<std::endl;
                //std::cout<<"Which Hit:"<<J<<"\n"<<std::endl;
 
                //if((thisHit->getAmp(J)>900)and(getNum==2)){
                //    adcs_=thisHit->getADCs();
                //    for(unsigned int K=0; K<6; K++){
                //        std::cout<<adcs_[K]<<std::endl;
                //    }
                //}
                //std::cout<<"hellO2"<<std::endl;
                //if(reg_selectors_[regions_[i_reg]]->passCutEq("doing_ft",3.0,weight)){
                //    if(((thisHit->getT0(J))-TimeRef)*((thisHit->getT0(J))-TimeRef)<(thisHit->getT0((J+1)%2)-TimeRef)*(thisHit->getT0((J+1)%2)-TimeRef)){continue;}//std::cout<<"hellO2"<<std::endl;std::cout<<thisHit->getT0(J)<<std::endl;std::cout<<thisHit->getT0((J+1)%2)<<std::endl;continue;}          
                //}else{std::cout<<"hell03"<<std::endl;continue;}
                //std::cout<<"hellO3"<<std::endl;
                // if(reg_selectors_[regions_[i_reg]]->passCutEq("doing_sa",1.0,weight)){
                //    if(!(std::abs((thisHit->getAmp(J))-AmpRef)<std::abs(thisHit->getAmp((J+1)%2)-AmpRef))){continue;}          
                //}
                //std::cout<<"hellO4"<<std::endl;
                //if(reg_selectors_[regions_[i_reg]]->passCutEq("doing_la",1.0,weight)){
                //    if((std::abs((thisHit->getAmp(J))-AmpRef)<std::abs(thisHit->getAmp((J+1)%2)-AmpRef))){continue;}          
                //}
                //std::cout<<"hellO2"<<std::endl;
                Float_t TimeDiff=-42069.0;
                Float_t AmpDiff=-42069.0;
                if(getNum==2){
                    TimeDiff=(thisHit->getT0(J))-(thisHit->getT0((J+1)%2));
                    AmpDiff=(thisHit->getT0(J))-(thisHit->getT0((J+1)%2)); 
                }
                if((doSample_==1) and (readout<40)){
                    std::cout<<"I ACTUALLY DID THIS"<<std::endl;
                    sample(thisHit,regions_[i_reg],(float)readout);
                    readout++; 
                }
                
                reg_histos_[regions_[i_reg]]->FillHistograms(thisHit,weight,J,i,TimeDiff,AmpDiff);
            }
        }
    }
    //std::cout<<count1<<std::endl;
   //std::cout<<count2<<std::endl;
    return true;
}

void SvtRawDataAnaProcessor::sample(RawSvtHit* thisHit,std::string word,float read){
    auto mod = std::to_string(thisHit->getModule());
    auto lay = std::to_string(thisHit->getLayer());
    //swTag= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
    std::string helper = mmapper_->getHwFromSw("ly"+lay+"_m"+mod); 
    char char_array[helper.length()+1];
    std::strcpy(char_array,helper.c_str());
    int feb = (int)char_array[1]-48;
    int hyb = (int)char_array[3]-48;
    int BigCount = 0;
    if(feb<=1){
        BigCount+=feb*2048+hyb*512+(int)(thisHit->getStrip());
    }else{
        BigCount+=4096;
        BigCount+=(feb-2)*2560+hyb*640+(int)(thisHit->getStrip());
    }
    //std::cout<<"I GOT HERE"<<std::endl;
    int * adcs2=thisHit->getADCs(); 
    //std::cout<<regions_[i_reg]<<" "<<readout<<std::endl;
   
    TF1* fitfunc = fourPoleFitFunction();

    fitfunc->FixParameter(0,thisHit->getT0(0));
    fitfunc->FixParameter(3,thisHit->getAmp(0));
    if(feb<=1){
        fitfunc->FixParameter(1,times1_[feb][hyb][(int)thisHit->getStrip()][1]);
        fitfunc->FixParameter(2,times1_[feb][hyb][(int)thisHit->getStrip()][2]);
        fitfunc->FixParameter(4,baseErr1_[feb][hyb][(int)thisHit->getStrip()][0]);
    }else{
        fitfunc->FixParameter(1,times2_[feb-2][hyb][(int)thisHit->getStrip()][1]);
        fitfunc->FixParameter(2,times2_[feb-2][hyb][(int)thisHit->getStrip()][2]);
        fitfunc->FixParameter(4,baseErr2_[feb-2][hyb][(int)thisHit->getStrip()][0]); 
    }   
    std::cout<<word<<std::endl;
    if(word=="OneFit"){ 
        auto gr = new TGraph();
        for(int i=0;i<6;i++){
            std::cout<<adcs2[i]<<std::endl; 
            gr->SetPoint(i,float(i)*24.0,adcs2[i]);
        }
        gr->Draw();
        //fitfunc->Draw();
        //legend->Draw("same");
        std::string helper=word+std::to_string(read)+".png";
        const char *thing = helper.data(); 
        gPad->SaveAs(thing);

        std::cout<<adcs2[0]<<" "<<adcs2[1]<<" "<<adcs2[2]<<" "<<adcs2[3]<<" "<<adcs2[4]<<" "<<adcs2[5]<<" "<<thisHit->getAmp(0)<<" "<<thisHit->getT0(0)<<" "<<BigCount<<" "<<thisHit->getChiSq(0)<<std::endl;
    }
}

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
    //std::cout<<"gotToHEREANDFUCKINGEXPLODED"<<std::endl;
    //histos->saveHistosSVT(outF_, anaName_.c_str());
    //delete histos;
    //histos = nullptr;
    //
    //trkHistos_->saveHistos(outF_,trkCollName_);
    //delete trkHistos_;
    //trkHistos_ = nullptr;
    //if (trkSelector_)
    //    trkSelector_->getCutFlowHisto()->Write();

    //if (truthHistos_) {
    //    truthHistos_->saveHistos(outF_,trkCollName_+"_truth");
    //    delete truthHistos_;
    //    truthHistos_ = nullptr;
    //}

        //std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
        //std::cout << "Setting up region:: " << regname << std::endl;
        //reg_selectors_[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);
        //reg_selectors_[regname]->setDebug(debug_);
        //reg_selectors_[regname]->LoadSelection();

        //reg_histos_[regname] = std::make_shared<RawSvtHitHistos>(regname);
        //reg_histos_[regname]->loadHistoConfig(histCfgFilename_);
        //reg_histos_[regname]->doTrackComparisonPlots(false);
        //reg_histos_[regname]->DefineHistos();

        //regions_.push_back(regname);
            //reg_histos->FillHistograms(svtHits_,1.);

DECLARE_PROCESSOR(SvtRawDataAnaProcessor);
