#include "RawSvtHitHistos.h"
#include <math.h>
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
    //std::vector<std::string> hybridNames2;
    //if(debug_ > 0){
    //for(int i = 0; i< hybridNames.size(); i++){ 
        //std::cout << "Hybrid: " << hybridNames.at(i) << std::endl;
        //regions_ = {"OneFit","BothFit","CTFit","FirstFit"};//,"SecondFit"};
        //for (std::string region : regions_){
        //    hybridNames2.push_back(hybridNames.at(i)+"_"+region);
        //}
    //}
    //}
    //Define histos
    //All histogram keys in the JSON file that contain special tag will have multiple copies of that histogram template made, one for each string
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
    histokey = swTag + "_SvtHybrids_getFitN_h"; 
    //std::cout<<"hello3"<<std::endl;
    Fill1DHisto(histokey, rawSvtHit->getFitN(),weight);
    histokey = swTag +"_SvtHybrids_T0_h";
    //std::cout<<histokey<<std::endl;
    //std::cout<<rawSvtHit->getT0(i)<<std::endl;
    //std::cout<<rawSvtHit->getAmp(i)<<std::endl;
    //std::cout<<rawSvtHit->getT0err(i)<<std::endl;
    //std::cout<<rawSvtHit->getAmpErr(i)<<std::endl;
    Fill1DHisto(histokey, rawSvtHit->getT0(i),weight); 
    //std::cout<<"hello6"<<std::endl;
    histokey = swTag + "_SvtHybrids_Am_h"; 
    Fill1DHisto(histokey, rawSvtHit->getAmp(i),weight);
    //std::cout<<"hello7"<<std::endl;
    histokey = swTag + "_SvtHybrids_T0Err_hh";
    Fill2DHisto(histokey, rawSvtHit->getT0(i), rawSvtHit->getT0err(i),weight);
    histokey = swTag + "_SvtHybrids_AmErr_hh";
    //std::cout<<"hello8"<<std::endl;
    Fill2DHisto(histokey, rawSvtHit->getAmp(i), rawSvtHit->getAmpErr(i),weight);
    histokey = swTag + "_SvtHybrids_AmT0_hh";
    //std::cout<<"hello9"<<std::endl;
    Fill2DHisto(histokey, rawSvtHit->getT0(i), rawSvtHit->getAmp(i),weight);
    histokey = swTag + "_SvtHybrids_AmT0Err_hh";
    //std::cout<<"hello10"<<std::endl;
    Fill2DHisto(histokey, rawSvtHit->getT0err(i), rawSvtHit->getAmpErr(i),weight);
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

//void RawSvtHitHistos::saveHistosSVT(TFile* outF,std::string folder) {
//    std::cout<<"hello34"<<std::endl;
//    if (outF) outF->cd();
//    TDirectory* dir=outF;
//    std::cout<<"hello0"<<std::endl;
//    std::cout<<folder.c_str()<<std::endl;
    //if (!folder.empty()) {
    //    dir = outF->mkdir(folder.c_str());
    //    dir->cd();
    //}
//    std::cout<<"hello1"<<std::endl;
//    int counter = 0;
//    outF->mkdir("OneFit");
//    outF->mkdir("BothFit");
//    outF->mkdir("CTFit");
//    for (int i =0; i < 4; i++)
//    {
//        for (int j = 1; j < 15; j++)
//        {
//            if (!(j<9 && i>1))
//            {   
//                if(counter%3==0){dir->cd("OneFit");}
//                if(counter%3==1){dir->cd("BothFit");}
//                if(counter%3==2){dir->cd("CTFit");}
//                
//                counter++;
//                for (it3d it = histos3d.begin(); it!=histos3d.end(); ++it) {
//                    if (!it->second){
//                        std::cout<<it->first<<" Null ptr in saving.."<<std::endl;
//                        continue;
//                    }
//                    it->second->Write();
//                }
//                std::cout<<"hello2"<<std::endl;
//                for (it2d it = histos2d.begin(); it!=histos2d.end(); ++it) {
//                    if (!(it->second)) {
//                        std::cout<<it->first<<" Null ptr in saving.."<<std::endl;
//                        continue;
//                    }
//                    it->second->Write();
//                }
//                for (it1d it = histos1d.begin(); it!=histos1d.end(); ++it) {
//                    if (!it->second){
//                        std::cout<<it->first<<" Null ptr in saving.."<<std::endl;
//                        continue;
//                    }
//                    it->second->Write();
//                }
                //std::cout<<svtHybMulti[i][j]<<std::endl;
                //Fill1DHisto(swTag+ "_SvtHybridsHitN_h", static_cast< float >(svtHybMulti[i][j]),weight);
//                dir->cd("..");
//            }
//        }
//    }
    //dir->Write();
    //if (dir) {delete dir; dir=0;}

//    Clear();

//}
