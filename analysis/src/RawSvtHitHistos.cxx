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
    /*
    std::ifstream myfile("/sdf/group/hps/users/rodwyer1/hps_14552_offline_baselines.dat");
    //std::ifstream myfile2("/sdf/group/hps/users/rodwyer1/hpssvt_014393_database_svt_pulse_shapes_final.dat");
    std::string s;
    //std::string s2;
    std::vector<float [12]> baselines;
    for(int i=0; i<24576; i++){ 
        std::getline(myfile,s);
        //std::getline(myfile2,s2);
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
        /*
        for(int I=0;I<5;I++){
            std::string token=s2.substr(0,s2.find(","));
            s2=s2.substr(s2.find(",")+1);
            if(I>=2){
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
                if(feb<=1){
                    times1_[feb][hyb][ch][I-2]=base;
                }else{
                    times2_[feb][hyb][ch][I-2]=base;
                }
            }
        }
        
        for(int I=0;I<13;I++){
            if(I>0){
                if(feb<=1){
                    try{
                        std::string token=s.substr(0,s.find(" "));
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
                        baseErr1_[feb][hyb][ch][I-1]=base;
                        s=s.substr(s.find(" ")+1);
                    }catch(...){std::cout<<feb<<" "<<hyb<<" "<<ch<<" "<<I-1<<std::endl;}
                    //std::cout<<s<<std::endl;
                }else{
                    try{
                        std::string token=s.substr(0,s.find(" "));
                        std::string top1=token.substr(0,s.find("."));
                        const char *top=top1.c_str();
                        std::string bot1=token.substr(s.find(".")+1);
                        const char *bottom=bot1.c_str();
                        float base = 0.0;
                        for(int J=0;J<std::strlen(top);J++){
                            base+=((float)((int)top[J]-48))*pow(10.0,(float)(std::strlen(top)-J-1));
                        }
                        for(int J=0;J<std::strlen(bottom);J++){
                            base+=((float)((int)bottom[J]-48))*pow(10.0,-1*((float)J+1.0));
                        }
                        baseErr2_[feb][hyb][ch][I-1]=base;
                        s=s.substr(s.find(" ")+1);
                    }catch(...){std::cout<<feb<<" "<<hyb<<" "<<ch<<" "<<I-1<<std::endl;}
                    //std::cout<<s<<std::endl;
                }
            }else{
                s=s.substr(s.find(" ")+1);
            }
        }
    }
    myfile.close();
    //myfile2.close(); */
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
    /*
    int BigChan = 0;
    if(feb<=1){
        BigChan+=2048*feb+512*hyb+(int)(rawSvtHit->getStrip());
    }else{
        BigChan+=4096+2560*feb+640*hyb+(int)(rawSvtHit->getStrip());
    }
    std::string s;
    std::ifstream myfile("/sdf/group/hps/users/rodwyer1/hps_14552_offline_baselines.dat");
    for(int i=0;i<=BigChan;i++){
        std::getline(myfile,s);
    }
    
    float baser[12];
    for(int I=0;I<13;I++){
        float base = 0.0;
        if(I>0){
            std::string token=s.substr(0,s.find(" "));
            std::string top1=token.substr(0,s.find("."));
            const char *top=top1.c_str();
            std::string bot1=token.substr(s.find(".")+1);
            const char *bottom=bot1.c_str();
            for(int J=0;J<std::strlen(top);J++){
                base+=((float)((int)top[J]-48))*pow(10.0,(float)(std::strlen(top)-J-1));
            }
            for(int J=0;J<std::strlen(bottom);J++){
                base+=((float)((int)bottom[J]-48))*pow(10.0,-1*((float)J+1.0));
            }
            baser[I]=base;
            //std::cout<<base<<std::endl;
            s=s.substr(s.find(" ")+1); 
        }else{
            s=s.substr(s.find(" ")+1);
        }
    }*/
    
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
