#include "RawSvtHitHistos.h"
#include <math.h>
#include <string>
#include "TCanvas.h"

RawSvtHitHistos::RawSvtHitHistos(const std::string& inputName, ModuleMapper* mmapper) {
    m_name = inputName;
    mmapper_ = mmapper;
}

RawSvtHitHistos::~RawSvtHitHistos() {
}

void RawSvtHitHistos::DefineHistos(){
    //Define vector of hybrid names using ModuleMapper 
    //Use this list to define multiple copies of histograms, one for each hybrid, from json file
    std::vector<std::string> hybridNames;
    mmapper_->getStrings(hybridNames);
    std::string makeMultiplesTag = "SvtHybrids";
    //std::cout<<"hello1"<<std::endl;
    HistoManager::DefineHistos(hybridNames, makeMultiplesTag );
    //std::cout<<"hello2"<<std::endl;
}

void RawSvtHitHistos::FillHistograms(RawSvtHit* rawSvtHit,float weight,int i,unsigned int i2,Float_t TimeDiff,Float_t AmpDiff) {
    std::vector<std::string> hybridStrings={};
    std::string histokey;
    //std::cout<<Event_number<<std::endl;
    //if(Event_number>=10000){return;}
    //if(Event_number==11) std::cout<<nhits<<i<<std::endl;
    if(Event_number%10000 == 0){std::cout << "Event: " << Event_number << std::endl;Event_number++;} 
    //    << " Number of RawSvtHits: " << nhits <<"\t"<<i<< std::endl;
    if(i2==0){
    Event_number++;}
    auto mod = std::to_string(rawSvtHit->getModule());
    auto lay = std::to_string(rawSvtHit->getLayer());
    swTag= mmapper_->getStringFromSw("ly"+lay+"_m"+mod);
    std::string helper = mmapper_->getHwFromSw("ly"+lay+"_m"+mod); 
    char char_array[helper.length()+1];
    std::strcpy(char_array,helper.c_str());
    int feb = (int)char_array[1]-48;
    int hyb = (int)char_array[3]-48;
        
    histokey = swTag + "_SvtHybrids_getFitN_h"; 
    //std::cout<<"hello3"<<std::endl;
    Fill1DHisto(histokey, rawSvtHit->getFitN(),weight);
    histokey = swTag +"_SvtHybrids_T0_h";
    //std::cout<<histokey<<std::endl;
    //std::cout<<rawSvtHit->getT0(i)<<std::endl;
    //std::cout<<rawSvtHit->getAmp(i)<<std::endl;
    //std::cout<<rawSvtHit->getT0err(i)<<std::endl;
    //std::cout<<rawSvtHit->getAmpErr(i)<<std::endl;
    //if(i==0){
    Fill1DHisto(histokey, rawSvtHit->getT0(i),weight);
    //}//else{
    //    Fill1DHisto(histokey, rawSvtHit->getT0(i)-27.0,weight);
    //}
    //std::cout<<"hello6"<<std::endl;
    histokey = swTag + "_SvtHybrids_Am_h"; 
    Fill1DHisto(histokey, rawSvtHit->getAmp(i),weight);
    histokey = swTag + "_SvtHybrids_Chi_Sqr_h";
    Fill1DHisto(histokey, rawSvtHit->getChiSq(i),weight);
    //std::cout<<rawSvtHit->getStrip()<<std::endl;
    histokey = swTag + "_SvtHybrids_ADCcount_hh";
    int * adcs=rawSvtHit->getADCs();
    int maxx = 0;
    for(unsigned int K=0; K<6; K++){
        if(maxx<adcs[K]){maxx=adcs[K];}
        //Fill2DHisto(histokey,24.0*K-(rawSvtHit->getT0(i)),((Float_t)(adcs[K]))/(rawSvtHit->getAmp(i)),weight);
    }
    
    for(unsigned int K=1; K<6; K++){
        if(feb<=1){
            Fill2DHisto(histokey,24.0*K-(rawSvtHit->getT0(i)),((Float_t)(adcs[K])-Float_t(baseErr1_[feb][hyb][(int)(rawSvtHit->getStrip())][K]))/(rawSvtHit->getAmp(i)),weight); 
        }else{
            Fill2DHisto(histokey,24.0*K-(rawSvtHit->getT0(i)),((Float_t)(adcs[K])-Float_t(baseErr2_[feb-2][hyb][(int)(rawSvtHit->getStrip())][K]))/(rawSvtHit->getAmp(i)),weight); 
        }
        //((Float_t)maxx),weight);
    }

    //NOW THE CODE FOR THE TGRAPH SHITE
    //




    histokey = swTag + "_SvtHybrids_ADCcountdeshift_hh";
    for(unsigned int K=1; K<6; K++){
        if(feb<=1){
            if(std::abs(rawSvtHit->getT0(i)+60)<25){
                Fill2DHisto(histokey,K,((Float_t)(adcs[K])-Float_t(baseErr1_[feb][hyb][(int)(rawSvtHit->getStrip())][K])));//(rawSvtHit->getAmp(i)),weight);
            }else{
                Fill2DHisto(histokey,K,((Float_t)(adcs[K])-Float_t(baseErr1_[feb][hyb][(int)(rawSvtHit->getStrip())][K])));//(rawSvtHit->getAmp(i)),weight);
            } 
        }else{
            if(std::abs(rawSvtHit->getT0(i)+60)<25){
                Fill2DHisto(histokey,K,((Float_t)(adcs[K])-Float_t(baseErr2_[feb-2][hyb][(int)(rawSvtHit->getStrip())][K])));//(rawSvtHit->getAmp(i)),weight);
            }else{
                Fill2DHisto(histokey,K,((Float_t)(adcs[K])-Float_t(baseErr2_[feb-2][hyb][(int)(rawSvtHit->getStrip())][K])));//(rawSvtHit->getAmp(i)),weight);
            } 
        }
        //((Float_t)maxx),weight);
    }
    //adcs_=rawSvtHit->getADCs(i);
    //Fill1DHisto(histokey, -(rawSvthit->getT0(i)),weight);

    //std::cout<<"hello7"<<std::endl;
    histokey = swTag + "_SvtHybrids_T0Err_hh";
    Fill2DHisto(histokey, rawSvtHit->getT0(i), rawSvtHit->getT0err(i),weight);
    histokey = swTag + "_SvtHybrids_AmErr_hh";
    //std::cout<<"hello8"<<std::endl;
    Fill2DHisto(histokey, rawSvtHit->getAmp(i), rawSvtHit->getAmpErr(i),weight);
    histokey = swTag + "_SvtHybrids_AmT0_hh";
    //std::cout<<"hello9"<<std::endl;
    Fill2DHisto(histokey, rawSvtHit->getT0(i), rawSvtHit->getAmp(i),weight);
    histokey = swTag + "_SvtHybrids_AmErrT0Err_hh";
    //std::cout<<"hello10"<<std::endl;
    Fill2DHisto(histokey, rawSvtHit->getT0err(i), rawSvtHit->getAmpErr(i),weight);
    
    histokey = swTag + "_SvtHybrids_AmT0Err_hh";
    //std::cout<<"hello10"<<std::endl;
    Fill2DHisto(histokey, rawSvtHit->getT0err(i), rawSvtHit->getAmp(i),weight);

    histokey = swTag + "_SvtHybrids_AmErrT0_hh";
    //std::cout<<"hello10"<<std::endl;
    Fill2DHisto(histokey, rawSvtHit->getT0(i), rawSvtHit->getAmpErr(i),weight);
    
    if(i==1){
        histokey = swTag + "_SvtHybrids_PT1PT2_hh";
        Fill2DHisto(histokey, rawSvtHit->getT0(1),rawSvtHit->getT0(0));
    }else{
        histokey = swTag + "_SvtHybrids_PT1PT2_hh";
        Fill2DHisto(histokey, rawSvtHit->getT0(0),rawSvtHit->getT0(1));
    }

    if(TimeDiff==-42069){return;}
    else{
        histokey = swTag + "_SvtHybrids_TD_h";
        Fill1DHisto(histokey, TimeDiff,weight);
        histokey = swTag + "_SvtHybrids_T0TD_hh";
        Fill2DHisto(histokey, rawSvtHit->getT0(i),TimeDiff,weight);
        histokey = swTag + "_SvtHybrids_AmErrTD_hh";
        Fill2DHisto(histokey, rawSvtHit->getAmpErr(i),TimeDiff,weight); 
        histokey = swTag + "_SvtHybrids_AmpTD_hh";
        Fill2DHisto(histokey, rawSvtHit->getAmp(i),TimeDiff,weight);
        histokey = swTag + "_SvtHybrids_Amp12_hh";
        Fill2DHisto(histokey, rawSvtHit->getAmp(0),rawSvtHit->getAmp(1),weight); 
        histokey = swTag + "_SvtHybrids_ADTD_hh";
        Fill2DHisto(histokey, AmpDiff,TimeDiff,weight); 
    }
    //}
    //std::cout<<"hello11"<<std::endl;
    //}catch(std::exception& e){
        //std::cout<<e.what()<<std::endl;
        //continue;
    //}
    //        } 
            //std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
            //reg_selectors_[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);   
    //    }        
    //}
    return;
}     


