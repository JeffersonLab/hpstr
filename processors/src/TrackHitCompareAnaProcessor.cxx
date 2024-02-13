/**
 * @file TrackHitAnaProcessor.cxx
 * @brief AnaProcessor used to compare two means of reconstruction directly. Fills histograms to study cluster reconstruction algorithms and dead channels.
 * Does not feature a region selector or histomanager (i.e. no configurable json files), rather 
 * for the limited featured plots allows for more specific manipulation and control.
 * @author Rory O'Dwyer and Cameron Bravo, SLAC National Accelerator Laboratory
 */     
#include "TrackHitCompareAnaProcessor.h"
#include <iostream>

TrackHitAnaProcessor::TrackHitAnaProcessor(const std::string& name, Process& process) : Processor(name,process){
    mmapper_ = new ModuleMapper(2021);
}
TrackHitAnaProcessor::~TrackHitAnaProcessor(){}

void TrackHitAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring TrackHitAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        layer_           = parameters.getInteger("layer");
        module_          = parameters.getInteger("module");
        isMC_            = parameters.getInteger("isMC");
        doingTracks_     = (parameters.getInteger("doTrack")==1);
        pcut_            = (float)parameters.getDouble("cut");
	badchann_	 = parameters.getString("badchannels");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}


void TrackHitAnaProcessor::initialize(TTree* tree) {
    fillDeads();
    tree_= tree;   
    if(isMC_==1){
        layers1_=new TH1F("layers","MC Strip Width for All Clusters",12,0.0,12.0);
        layersOnTrk1_=new TH1F("layersOnTrk","MC Strip Width for Clusters on Track",12,0.0,12.0);
        layersOffTrk1_=new TH1F("layersOffTrk","MC Strip Width for Clusters off Track",12,0.0,12.0);
        charges1_=new TH1F("charges","MC Charge Distribution for All Clusters",1000,0.0,0.000016);
        chargesOnTrk1_=new TH1F("chargesOnTrk","MC Charge Distribution for On Track",1000,0.0,.000016);
        chargesOffTrk1_=new TH1F("chargesOffTrk","MC Charge Distribution for Off Track",1000,0.0,0.000016);

        layersNTD1_=new TH1F("layersNTD","MC Strip Width for All Clusters",12,0.0,12.0);
        layersOnTrkNTD1_=new TH1F("layersOnTrkNTD","MC Strip Width for Clusters on Track",12,0.0,12.0);
        layersOffTrkNTD1_=new TH1F("layersOffTrkNTD","MC Strip Width for Clusters off Track",12,0.0,12.0);
        chargesNTD1_=new TH1F("chargesNTD","MC Charge Distribution for All Clusters",1000,0.0,0.000016);
        chargesOnTrkNTD1_=new TH1F("chargesOnTrkNTD","MC Charge Distribution for On Track",1000,0.0,.000016);
        chargesOffTrkNTD1_=new TH1F("chargesOffTrkNTD","MC Charge Distribution for Off Track",1000,0.0,0.000016);

        positions1_=new TH1F("Positions","MC Location of Cluster Hit;Layer;Hits",14,0.0,14.0);
        positionsOnTrk1_=new TH1F("PositionsOnTrk","MC Location of Cluster Hit for On Track",14,0.0,14.0);
        ClusDistances1_=new TH1F("Minimum Cluster Difference","MC Minimum Distance Between Clusters",14,0.0,14.0);
        ClusDistancesNTD1_=new TH1F("Minimum Cluster Difference","MC Minimum Distance Between Clusters",14,0.0,14.0);

        times1_=new TH1F("Times","MC Time of Cluster Hit",1000,-60.0,60.0);
        timesOnTrk1_=new TH1F("TimesOnTrk","MC Time of On Track Cluster Hit",1000,-60.0,60.0);
        timesOffTrk1_=new TH1F("TimesOffTrk","MC Time of Off Cluster Hit",1000,-60.0,60.0);
        timesNTD1_=new TH1F("TimesNTD","MC Time of Cluster Hit NTD",1000,-60.0,60.0);
        timesOnTrkNTD1_=new TH1F("TimesOnTrkNTD","MC Time of On Track Cluster Hit NTD",1000,-60.0,60.0);
        timesOffTrkNTD1_=new TH1F("TimesOffTrkNTD","MC Time of Off Cluster Hit NTD",1000,-60.0,60.0);

        tree_->SetBranchAddress("SiClusters",&Clusters_,&bClusters_);
        tree_->SetBranchAddress("SiClustersOnTrack_KF",&ClustersKF_,&bClustersKF_);
        tree_->SetBranchAddress("SVTRawTrackerHits",&svtraw_,&bsvtraw_);
        return;
    }
    //Instantiating the first layer
    
    layers1_=new TH1F("layers","Strip Width for All Clusters",12,0.0,12.0);
    layersOnTrk1_=new TH1F("layersOnTrk","Strip Width for Clusters on Track",12,0.0,12.0);
    layersOffTrk1_=new TH1F("layersOffTrk","Strip Width for Clusters off Track",12,0.0,12.0);
    charges1_=new TH1F("charges","Charge Distribution for All Clusters",1000,0.0,0.000016);
    chargesOnTrk1_=new TH1F("chargesOnTrk","Charge Distribution for On Track",1000,0.0,.000016);
    chargesOffTrk1_=new TH1F("chargesOffTrk","Charge Distribution for Off Track",1000,0.0,0.000016);

    layersNTD1_=new TH1F("layersNTD","Strip Width for All Clusters",12,0.0,12.0);
    layersOnTrkNTD1_=new TH1F("layersOnTrkNTD","Strip Width for Clusters on Track",12,0.0,12.0);
    layersOffTrkNTD1_=new TH1F("layersOffTrkNTD","Strip Width for Clusters off Track",12,0.0,12.0);
    chargesNTD1_=new TH1F("chargesNTD","Charge Distribution for All Clusters",1000,0.0,0.000016);
    chargesOnTrkNTD1_=new TH1F("chargesOnTrkNTD","Charge Distribution for On Track",1000,0.0,.000016);
    chargesOffTrkNTD1_=new TH1F("chargesOffTrkNTD","Charge Distribution for Off Track",1000,0.0,0.000016);

    positions1_=new TH1F("Positions","Location of Cluster Hit;Layer;Hits",14,0.0,14.0);
    positionsOnTrk1_=new TH1F("PositionsOnTrk","Location of Cluster Hit for On Track",14,0.0,14.0);
    ClusDistances1_=new TH1F("Minimum Cluster Difference","Minimum Distance Between Clusters",14,0.0,14.0);
    ClusDistancesNTD1_=new TH1F("Minimum Cluster Difference","Minimum Distance Between Clusters",14,0.0,14.0);

    times1_=new TH1F("Times","Time of Cluster Hit",1000,-60.0,60.0);
    timesOnTrk1_=new TH1F("TimesOnTrk","Time of On Track Cluster Hit",1000,-60.0,60.0);
    timesOffTrk1_=new TH1F("TimesOffTrk","Time of Off Cluster Hit",1000,-60.0,60.0);
    timesNTD1_=new TH1F("TimesNTD","Time of Cluster Hit NTD",1000,-60.0,60.0);
    timesOnTrkNTD1_=new TH1F("TimesOnTrkNTD","Time of On Track Cluster Hit NTD",1000,-60.0,60.0);
    timesOffTrkNTD1_=new TH1F("TimesOffTrkNTD","Time of Off Cluster Hit NTD",1000,-60.0,60.0);
   
    if(doingTracks_){
        Z0VNShare2Hist1_= new TH2F("Z0VNShare2Hist","Z0 versus Number of Shared Hits No Cut",100,0,3,8,0,8);
        Z0VNShare2HistCut1_= new TH2F("Z0VNShare2HistCut","Z0 versus Number of Shared Hits Momentum Cut",100,0,3,8,0,8);
        SharedAmplitudes1_= new TH1F("SharedAmplitudes","The Amplitudes of Clusters Shared Between Tracks",1000,0.0,0.000016);
        UnSharedAmplitudes1_= new TH1F("UnSharedAmplitudes","The Amplitudes of Clusters Not Shared Between Tracks",1000,0.0,0.000016);
        SharedTimes1_= new TH1F("SharedTimes","The Times of Clusters Shared Between Tracks",1000,-60.0,60.0);
        UnSharedTimes1_= new TH1F("UnSharedTimes","The Times of Clusters Not Shared Between Tracks",1000,-60.0,60.0);    
        TrackMomentumInTime1_ = new TH1F("TrackMomentumInTime","The Momentum of In Time Tracks",1000,0.0,7.0);
        TrackMomentumOutTime1_ =  new TH1F("TrackMomentumOutTime","The Momentum of Out of Time Tracks",1000,0.0,7.0);
        TrackMomentumAllTime1_ = new TH1F("TrackMomentumAll","The Momentum of All Tracks",1000,0.0,7.0);

        TrackMomentumTInTime1_ = new TH1F("TrackMomentumTInTime","The Transverse Momentum of In Time Tracks",1000,0.0,7.0);
        TrackMomentumTOutTime1_ =  new TH1F("TrackMomentumTOutTime","The Transverse Momentum of Out of Time Tracks",1000,0.0,7.0);
        TrackMomentumTAllTime1_ = new TH1F("TrackMomentumTAll","The Transverse Momentum of All Tracks",1000,0.0,7.0);
    }


    //Instantiating the first layer
    
    layers2_=new TH1F("layers","Strip Width for All Clusters",12,0.0,12.0);
    layersOnTrk2_=new TH1F("layersOnTrk","Strip Width for Clusters on Track",12,0.0,12.0);
    layersOffTrk2_=new TH1F("layersOffTrk","Strip Width for Clusters off Track",12,0.0,12.0);
    charges2_=new TH1F("charges","Charge Distribution for All Clusters",1000,0.0,0.000016);
    chargesOnTrk2_=new TH1F("chargesOnTrk","Charge Distribution for On Track",1000,0.0,.000016);
    chargesOffTrk2_=new TH1F("chargesOffTrk","Charge Distribution for Off Track",1000,0.0,0.000016);

    layersNTD2_=new TH1F("layersNTD","Strip Width for All Clusters",12,0.0,12.0);
    layersOnTrkNTD2_=new TH1F("layersOnTrkNTD","Strip Width for Clusters on Track",12,0.0,12.0);
    layersOffTrkNTD2_=new TH1F("layersOffTrkNTD","Strip Width for Clusters off Track",12,0.0,12.0);
    chargesNTD2_=new TH1F("chargesNTD","Charge Distribution for All Clusters",1000,0.0,0.000016);
    chargesOnTrkNTD2_=new TH1F("chargesOnTrkNTD","Charge Distribution for On Track",1000,0.0,.000016);
    chargesOffTrkNTD2_=new TH1F("chargesOffTrkNTD","Charge Distribution for Off Track",1000,0.0,0.000016);

    times2_=new TH1F("Times","Time of Cluster Hit",1000,-60.0,60.0);
    timesOnTrk2_=new TH1F("TimesOnTrk","Time of On Track Cluster Hit",1000,-60.0,60.0);
    timesOffTrk2_=new TH1F("TimesOffTrk","Time of Off Cluster Hit",1000,-60.0,60.0);
    timesNTD2_=new TH1F("TimesNTD","Time of Cluster Hit NTD",1000,-60.0,60.0);
    timesOnTrkNTD2_=new TH1F("TimesOnTrkNTD","Time of On Track Cluster Hit NTD",1000,-60.0,60.0);
    timesOffTrkNTD2_=new TH1F("TimesOffTrkNTD","Time of Off Cluster Hit NTD",1000,-60.0,60.0);
    
    positions2_=new TH1F("Positions","Location of Cluster Hit;Layer;Hits",14,0.0,14.0);
    positionsOnTrk2_=new TH1F("PositionsOnTrk","Location of Cluster Hit for On Track",14,0.0,14.0);
    ClusDistances2_=new TH1F("Minimum Cluster Difference","Minimum Distance Between Clusters",14,0.0,14.0);
    ClusDistancesNTD2_=new TH1F("Minimum Cluster Difference","Minimum Distance Between Clusters",14,0.0,14.0);

    if(doingTracks_){
        Z0VNShare2Hist2_= new TH2F("Z0VNShare2Hist","Z0 versus Number of Shared Hits No Cut",100,0,3,8,0,8);
        Z0VNShare2HistCut2_= new TH2F("Z0VNShare2HistCut","Z0 versus Number of Shared Hits Momentum Cut",100,0,3,8,0,8);
        SharedAmplitudes2_= new TH1F("SharedAmplitudes","The Amplitudes of Clusters Shared Between Tracks",1000,0.0,0.000016);
        UnSharedAmplitudes2_= new TH1F("UnSharedAmplitudes","The Amplitudes of Clusters Not Shared Between Tracks",1000,0.0,0.000016);
        SharedTimes2_= new TH1F("SharedTimes","The Times of Clusters Shared Between Tracks",1000,-60.0,60.0);
        UnSharedTimes2_= new TH1F("UnSharedTimes","The Times of Clusters Not Shared Between Tracks",1000,-60.0,60.0); 
        
        TrackMomentumInTime2_ = new TH1F("TrackMomentumInTime","The Momentum of In Time Tracks",1000,0.0,7.0);
        TrackMomentumOutTime2_ =  new TH1F("TrackMomentumOutTime","The Momentum of Out of Time Tracks",1000,0.0,7.0);
        TrackMomentumAllTime2_ = new TH1F("TrackMomentumAll","The Momentum of All Tracks",1000,0.0,7.0);
    
        TrackMomentumTInTime2_ = new TH1F("TrackMomentumTInTime","The Transverse Momentum of In Time Tracks",1000,0.0,7.0);
        TrackMomentumTOutTime2_ =  new TH1F("TrackMomentumTOutTime","The Transverse Momentum of Out of Time Tracks",1000,0.0,7.0);
        TrackMomentumTAllTime2_ = new TH1F("TrackMomentumTAll","The Transverse Momentum of All Tracks",1000,0.0,7.0);
}
    tree_->SetBranchAddress("SiClusters",&Clusters_,&bClusters_);
    tree_->SetBranchAddress("SiClustersOnTrack_KF",&ClustersKF_,&bClustersKF_);
    tree_->SetBranchAddress("SVTRawTrackerHits",&svtraw_,&bsvtraw_);
    tree_->SetBranchAddress("Identifier",&ident_,&bident_);
    if(doingTracks_){
        tree_->SetBranchAddress("KalmanFullTracks",&tracks_,&btracks_);
    }
}

bool TrackHitAnaProcessor::process(IEvent* ievent) {
    if(doingTracks_){
        for(int i = 0;i<tracks_->size();i++){
            Track* track = tracks_->at(i);
            if(track->getTrackTime()*track->getTrackTime()<100.0){
                if(ident_<1.5){Z0VNShare2Hist1_->Fill(track->getZ0Err(),track->getNShared());}else{Z0VNShare2Hist2_->Fill(track->getZ0Err(),track->getNShared());}
                if(track->getP()<pcut_){
                    if(ident_<1.5){Z0VNShare2HistCut1_->Fill(track->getZ0Err(),track->getNShared());}else{Z0VNShare2HistCut2_->Fill(track->getZ0Err(),track->getNShared());}
                }
            }
        
            //Track Momentum for In and Out of time hits (All not just transverse)
            if(ident_<1.5){TrackMomentumAllTime1_->Fill(track->getP());}else{TrackMomentumAllTime2_->Fill(track->getP());}
        
            if(track->getTrackTime()*track->getTrackTime()<16.0){
                if(ident_<1.5){TrackMomentumInTime1_->Fill(track->getP());}else{TrackMomentumInTime2_->Fill(track->getP());}
            }else{
                if(ident_<1.5){TrackMomentumOutTime1_->Fill(track->getP());}else{TrackMomentumOutTime2_->Fill(track->getP());}
            }
          
            //Transverse Track Momentum for same cuts
            if(ident_<1.5){TrackMomentumTAllTime1_->Fill(track->getPt());}else{TrackMomentumTAllTime2_->Fill(track->getPt());}
        
            if(track->getTrackTime()*track->getTrackTime()<16.0){
                if(ident_<1.5){TrackMomentumTInTime1_->Fill(track->getPt());}else{TrackMomentumTInTime2_->Fill(track->getPt());}
            }else{
                if(ident_<1.5){TrackMomentumTOutTime1_->Fill(track->getPt());}else{TrackMomentumTOutTime2_->Fill(track->getPt());}
            }
        }
    }
    for(int i = 0; i < Clusters_->size(); i++){ 
        TrackerHit * clu = Clusters_->at(i);
        Int_t layc = -1;
        Int_t modc = -1;      
 
        RawSvtHit * seed = (RawSvtHit*)(clu->getRawHits().At(0));

        layc=clu->getLayer();
        modc=seed->getModule();

        if(doingTracks_){
            bool isShared = false;int increment = 0;
            for(int i = 0;i<tracks_->size();i++){
                Track* track = tracks_->at(i);     
                for(int j = 0; j<track->getSvtHits().GetEntries();j++){
                    TrackerHit * test = (TrackerHit *)(track->getSvtHits().At(j));
                    if(clu->getTime()==test->getTime()){
                        increment+=1;
                    }
                }
            }
            if(increment>1){
                isShared=true;
            }
            if(increment>0){
                bool general = ((layer_==-1)||(module_==-1));
                if(((layc==layer_)&&(modc==module_))||(general)){
                    if(isShared){
                        if(ident_<1.5){SharedAmplitudes1_->Fill(clu->getCharge());}else{SharedAmplitudes2_->Fill(clu->getCharge());}
                        if(ident_<1.5){SharedTimes1_->Fill(clu->getTime());}else{SharedTimes2_->Fill(clu->getTime());}
                    }else{
                        if(ident_<1.5){UnSharedAmplitudes1_->Fill(clu->getCharge());}else{UnSharedAmplitudes2_->Fill(clu->getCharge());}
                        if(ident_<1.5){UnSharedTimes1_->Fill(clu->getTime());}else{UnSharedTimes2_->Fill(clu->getTime());}
                    }
                }
            }
        }
        
        
        float seedStrip = (float)(seed->getStrip());
        float nLayers = (float)(clu->getRawHits().GetEntries());
        float ncharges = (float)(clu->getCharge());
        float ntimes = (float)(clu->getTime());
        bool onTrk = false;
        bool NTD = false;
        for(unsigned int j = 0; j < ClustersKF_->size(); j++){
            if(clu->getID()==(ClustersKF_->at(j)->getID())){
                onTrk = true;
            }
        }
        std::string input = "ly"+std::to_string(layc+1)+"_m"+std::to_string(modc);
        std::string helper = mmapper_->getHwFromSw(input);
        int feb=std::stoi(helper.substr(1,1));
        int hyb=std::stoi(helper.substr(3,1));
              
        int channelL=seedStrip-1;
        int channelR=seedStrip+1;
        if(channelL>=0){
            NTD=(NTD)||(Deads_[GetStrip(feb,hyb,channelL)]==1);
        }
        if(((feb<=1)&&(channelR<=511))||((feb>1)&&(channelR<=639))){
            NTD=(NTD)||(Deads_[GetStrip(feb,hyb,channelR)]==1); 
        }
        bool general = ((layer_==-1)||(module_==-1));
        if(((layc==layer_)&&(modc==module_))||(general)){
            //NOW IS THE PART WHERE I FILL THE CLUSTER DISTANCE HISTOGRAM
            float Dist=69420;
            for(int p = 0; p < Clusters_->size(); p++){ 
                if(p==i){continue;}
                TrackerHit * clu2 = Clusters_->at(p);
                RawSvtHit * seed2 = (RawSvtHit*)(clu2->getRawHits().At(0));
                float layc2=clu->getLayer();
                float modc2=seed->getModule();
                if((not(layc2==layc))or(not(modc2==modc))){continue;}
                float dist = ((float)(seed2->getStrip()))-seedStrip;
                if(dist<0){dist*=-1.0;}
                if(dist<Dist){Dist=dist;}
            }
            if(Dist<69420){
                if(ident_<1.5){ClusDistances1_->Fill(Dist);}else{ClusDistances2_->Fill(Dist);}
                if(NTD){if(ident_<1.5){ClusDistancesNTD1_->Fill(Dist);}else{ClusDistancesNTD2_->Fill(Dist);}}
            }
            if(ident_<1.5){layers1_->Fill(nLayers);}else{layers2_->Fill(nLayers);}
            if(ident_<1.5){charges1_->Fill(ncharges);}else{charges2_->Fill(ncharges);}
            if(ident_<1.5){positions1_->Fill(clu->getLayer());}else{positions2_->Fill(clu->getLayer());}
            if(ident_<1.5){times1_->Fill(ntimes);}else{times2_->Fill(ntimes);}
            if(onTrk){
                if(ident_<1.5){layersOnTrk1_->Fill(nLayers);}else{layersOnTrk2_->Fill(nLayers);}
                if(ident_<1.5){chargesOnTrk1_->Fill(ncharges);}else{chargesOnTrk2_->Fill(ncharges);}
                if(ident_<1.5){timesOnTrk1_->Fill(ntimes);}else{timesOnTrk2_->Fill(ntimes);} 
                if(ident_<1.5){positionsOnTrk1_->Fill(clu->getLayer());}else{positionsOnTrk2_->Fill(clu->getLayer());}
            }else{
                if(ident_<1.5){layersOffTrk1_->Fill(nLayers);}else{layersOffTrk2_->Fill(nLayers);}
                if(ident_<1.5){chargesOffTrk1_->Fill(ncharges);}else{chargesOffTrk2_->Fill(ncharges);}
                if(ident_<1.5){timesOffTrk1_->Fill(ntimes);}else{timesOffTrk2_->Fill(ntimes);}
            }
            if(NTD){
                if(ident_<1.5){layersNTD1_->Fill(nLayers);}else{layersNTD2_->Fill(nLayers);}
                if(ident_<1.5){chargesNTD1_->Fill(ncharges);}else{chargesNTD2_->Fill(ncharges);}
                if(ident_<1.5){timesNTD1_->Fill(ntimes);}else{timesNTD2_->Fill(ntimes);}
                if(onTrk){
                    if(ident_<1.5){layersOnTrkNTD1_->Fill(nLayers);}else{layersOnTrkNTD2_->Fill(nLayers);}
                    if(ident_<1.5){chargesOnTrkNTD1_->Fill(ncharges);}else{chargesOnTrkNTD2_->Fill(ncharges);}
                    if(ident_<1.5){timesOnTrkNTD1_->Fill(ntimes);}else{timesOnTrkNTD2_->Fill(ntimes);}
                }else{
                    if(ident_<1.5){layersOffTrkNTD1_->Fill(nLayers);}else{layersOffTrkNTD2_->Fill(nLayers);}
                    if(ident_<1.5){chargesOffTrkNTD1_->Fill(ncharges);}else{chargesOffTrkNTD2_->Fill(ncharges);}
                    if(ident_<1.5){timesOffTrkNTD1_->Fill(ntimes);}else{timesOffTrkNTD2_->Fill(ntimes);}
                } 
            }
        }      
    }
    return true;
}

void TrackHitAnaProcessor::fillDeads(){
    for(int i = 0;i<24576;i++){
        Deads_[i]=0.0;
    }
    std::string FILENAME=badchann_;
    std::ifstream file(FILENAME.c_str());
    std::string line;
    std::getline(file,line);
    while (std::getline(file, line)) {
        int value = std::atoi(line.c_str());
        Deads_[value]=1.0;
    }
    file.close();
    return;
}

int TrackHitAnaProcessor::GetStrip(int feb,int hyb,int strip){
	int BigCount = 0;
    if(feb<=1){
        BigCount+=feb*2048+hyb*512+strip;
    }else{
        BigCount+=4096;
        BigCount+=(feb-2)*2560+hyb*640+strip;
    }
    return BigCount;
}

void TrackHitAnaProcessor::PlotClusterLayers(){
    TCanvas *c1 = new TCanvas("c");
    gPad->SetLogy(true);
    c1->cd();
    layers1_->GetXaxis()->SetTitle("Width (strip)");
    layers1_->GetYaxis()->SetTitle("Hits"); 
    layersOnTrk1_->GetXaxis()->SetTitle("Width (strip)");
    layersOnTrk1_->GetYaxis()->SetTitle("Hits"); 
    layersOffTrk1_->GetXaxis()->SetTitle("Width (strip)");
    layersOffTrk1_->GetYaxis()->SetTitle("Hits"); 
    layers1_->SetLineColor(kRed);
    layersOnTrk1_->SetLineColor(kRed);
    layersOffTrk1_->SetLineColor(kRed);


    layers2_->GetXaxis()->SetTitle("Width (strip)");
    layers2_->GetYaxis()->SetTitle("Hits"); 
    layersOnTrk2_->GetXaxis()->SetTitle("Width (strip)");
    layersOnTrk2_->GetYaxis()->SetTitle("Hits"); 
    layersOffTrk2_->GetXaxis()->SetTitle("Width (strip)");
    layersOffTrk2_->GetYaxis()->SetTitle("Hits"); 
    layers2_->SetLineColor(kBlue);
    layersOnTrk2_->SetLineColor(kBlue);
    layersOffTrk2_->SetLineColor(kBlue);

    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("Layers for All Clusters");
    layers1_->Draw("e");
    layers2_->Draw("e same");
    auto legend1 = new TLegend(0.3,0.8,.68,.9);
    legend1->AddEntry(layers1_,"Layers Rec 1");
    legend1->AddEntry(layers2_,"Layers Rec 2");
    legend1->Draw("same e");
    c1->SaveAs("allClusters.png");     
    c1->Clear();

    c1->SetTitle("Layers for On Track Clusters");
    layersOnTrk1_->Draw("e");
    layersOnTrk2_->Draw("e same");
    auto legend2 = new TLegend(0.3,0.8,.68,.9);
    legend2->AddEntry(layersOnTrk1_,"Layers On Track Rec 1");
    legend2->AddEntry(layersOnTrk2_,"Layers On Track Rec 2");
    legend2->Draw("same e");
    c1->SaveAs("onClusters.png");     
    c1->Clear();
    
    c1->SetTitle("Layers for Off Track Clusters");
    layersOffTrk1_->Draw("e");
    layersOffTrk2_->Draw("e same");
    auto legend3 = new TLegend(0.3,0.8,.68,.9);
    legend3->AddEntry(layersOffTrk1_,"Layers Off Track Rec 1");
    legend3->AddEntry(layersOffTrk2_,"Layers Off Track Rec 2");
    legend3->Draw("same e");
    c1->SaveAs("offClusters.png");     
    c1->Clear();
    return;
}

void TrackHitAnaProcessor::PlotClusterCharges(){
    TCanvas *c1 = new TCanvas("c");
    c1->cd();
    
    charges1_->GetXaxis()->SetTitle("Charge");
    charges1_->GetYaxis()->SetTitle("Hits"); 
    chargesOffTrk1_->GetXaxis()->SetTitle("Charge");
    chargesOffTrk1_->GetYaxis()->SetTitle("Hits"); 
    chargesOnTrk1_->GetXaxis()->SetTitle("Charge");
    chargesOnTrk1_->GetYaxis()->SetTitle("Hits"); 
    charges1_->SetLineColor(kRed);
    chargesOnTrk1_->SetLineColor(kRed);
    chargesOffTrk1_->SetLineColor(kRed);

    charges2_->GetXaxis()->SetTitle("Charge");
    charges2_->GetYaxis()->SetTitle("Hits"); 
    chargesOffTrk2_->GetXaxis()->SetTitle("Charge");
    chargesOffTrk2_->GetYaxis()->SetTitle("Hits"); 
    chargesOnTrk2_->GetXaxis()->SetTitle("Charge");
    chargesOnTrk2_->GetYaxis()->SetTitle("Hits"); 
    charges2_->SetLineColor(kBlue);
    chargesOnTrk2_->SetLineColor(kBlue);
    chargesOffTrk2_->SetLineColor(kBlue);


    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("Charges for All Clusters");
    charges1_->Draw("e");
    charges2_->Draw("e same");
    auto legend1 = new TLegend(0.3,0.8,.68,.9);
    legend1->AddEntry(charges1_,"Charges Rec 1");
    legend1->AddEntry(charges2_,"Charges Rec 2");
    legend1->Draw("same e");
    c1->SaveAs("allClustersCharge.png");     
    c1->Clear();

    c1->SetTitle("Charges for On Track Clusters");
    chargesOnTrk1_->Draw("e");
    chargesOnTrk2_->Draw("e same");
    auto legend2 = new TLegend(0.3,0.8,.68,.9);
    legend2->AddEntry(chargesOnTrk1_,"Charges On Track Rec 1");
    legend2->AddEntry(chargesOnTrk2_,"Charges On Track Rec 2");
    legend2->Draw("same e");
    c1->SaveAs("onClustersCharge.png");     
    c1->Clear();
    
    c1->SetTitle("Charges for Off Track Clusters");
    chargesOffTrk1_->Draw("e");
    chargesOffTrk2_->Draw("e same");
    auto legend3 = new TLegend(0.3,0.8,.68,.9);
    legend3->AddEntry(chargesOffTrk1_,"Charges Off Track Rec 1");
    legend3->AddEntry(chargesOffTrk2_,"Charges Off Track Rec 2");
    legend3->Draw("same e");
    c1->SaveAs("offClustersCharge.png");     
    c1->Clear();
    return;
}

void TrackHitAnaProcessor::PlotClusterTimes(){
    TCanvas *c1 = new TCanvas("c");
    c1->cd();
    
    times1_->GetXaxis()->SetTitle("Charge");
    times1_->GetYaxis()->SetTitle("Hits"); 
    timesOffTrk1_->GetXaxis()->SetTitle("Charge");
    timesOffTrk1_->GetYaxis()->SetTitle("Hits"); 
    timesOnTrk1_->GetXaxis()->SetTitle("Charge");
    timesOnTrk1_->GetYaxis()->SetTitle("Hits"); 
    times1_->SetLineColor(kRed);
    timesOnTrk1_->SetLineColor(kRed);
    timesOffTrk1_->SetLineColor(kRed);


    times2_->GetXaxis()->SetTitle("Charge");
    times2_->GetYaxis()->SetTitle("Hits"); 
    timesOffTrk2_->GetXaxis()->SetTitle("Charge");
    timesOffTrk2_->GetYaxis()->SetTitle("Hits"); 
    timesOnTrk2_->GetXaxis()->SetTitle("Charge");
    timesOnTrk2_->GetYaxis()->SetTitle("Hits"); 
    times2_->SetLineColor(kBlue);
    timesOnTrk2_->SetLineColor(kBlue);
    timesOffTrk2_->SetLineColor(kBlue);


    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("Times for All Clusters");
    times1_->Draw("e");
    times2_->Draw("e same");
    auto legend1 = new TLegend(0.3,0.8,.68,.9);
    legend1->AddEntry(times1_,"Times Rec 1");
    legend1->AddEntry(times2_,"Times Rec 2");
    legend1->Draw("same e");
    c1->SaveAs("alltimes.png");     
    c1->Clear();

    c1->SetTitle("Times for On Track Clusters");
    timesOnTrk1_->Draw("e");
    timesOnTrk2_->Draw("e same");
    auto legend2 = new TLegend(0.3,0.8,.68,.9);
    legend2->AddEntry(timesOnTrk1_,"Times On Track Rec 1");
    legend2->AddEntry(timesOnTrk2_,"Times On Track Rec 2");
    legend2->Draw("same e");
    c1->SaveAs("ontimes.png");     
    c1->Clear();
    
    c1->SetTitle("Times for Off Track Clusters");
    timesOffTrk1_->Draw("e");
    timesOffTrk2_->Draw("e same");
    auto legend3 = new TLegend(0.3,0.8,.68,.9);
    legend3->AddEntry(timesOffTrk1_,"Times Off Track Rec 1");
    legend3->AddEntry(timesOffTrk2_,"Times Off Track Rec 2");
    legend3->Draw("same e");
    c1->SaveAs("offtimes.png");     
    c1->Clear();       
    return;
}

void TrackHitAnaProcessor::PlotClusterPositions(){
    TCanvas *c1 = new TCanvas("c");
    c1->cd();
    
    positions1_->GetXaxis()->SetTitle("Position");
    positions1_->GetYaxis()->SetTitle("Hits"); 
    positionsOnTrk1_->GetXaxis()->SetTitle("Position");
    positionsOnTrk1_->GetYaxis()->SetTitle("Hits"); 

    positions2_->GetXaxis()->SetTitle("Position");
    positions2_->GetYaxis()->SetTitle("Hits"); 
    positionsOnTrk2_->GetXaxis()->SetTitle("Position");
    positionsOnTrk2_->GetYaxis()->SetTitle("Hits"); 

    positions1_->SetLineColor(kRed);
    positionsOnTrk1_->SetLineColor(kRed);
    positions2_->SetLineColor(kBlue);
    positionsOnTrk2_->SetLineColor(kBlue);


    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("Position for All Clusters");
    positions1_->Draw("e");
    positions2_->Draw("e same");
    auto legend1 = new TLegend(0.3,0.8,.68,.9);
    legend1->AddEntry(positions1_,"Position Rec 1");
    legend1->AddEntry(positions2_,"Position Rec 2");
    legend1->Draw("same e");
    c1->SaveAs("positions.png");     
    c1->Clear();

    c1->SetTitle("Position for On Track Clusters");
    positionsOnTrk1_->Draw("e");
    positionsOnTrk2_->Draw("e same");
    auto legend2 = new TLegend(0.3,0.8,.68,.9);
    legend2->AddEntry(positionsOnTrk1_,"Position On Track Rec 1");
    legend2->AddEntry(positionsOnTrk2_,"Position On Track Rec 2");
    legend2->Draw("same e");
    c1->SaveAs("onpositions.png");     
    c1->Clear();
    
    c1->SetTitle("Cluster Distances");
    positionsOnTrk1_->Draw("e");
    positionsOnTrk2_->Draw("e same");
    auto legend3 = new TLegend(0.3,0.8,.68,.9);
    legend3->AddEntry(positionsOnTrk1_,"Cluster Distances Rec 1");
    legend3->AddEntry(positionsOnTrk2_,"Cluster Distances Rec 2");
    legend3->Draw("same e");
    c1->SaveAs("clusDistances.png");     
    c1->Clear();

    return;
}

void TrackHitAnaProcessor::TrackMomenta(){
    TCanvas *c1 = new TCanvas("c");
    c1->cd();
    gPad->SetLogy(false);
    TrackMomentumAllTime1_->GetXaxis()->SetTitle("Track Number");
    TrackMomentumAllTime1_->GetYaxis()->SetTitle("Momentum");
    TrackMomentumInTime1_->GetXaxis()->SetTitle("Track Number");
    TrackMomentumInTime1_->GetYaxis()->SetTitle("Momentum");
    TrackMomentumOutTime1_->GetXaxis()->SetTitle("Track Number");
    TrackMomentumOutTime1_->GetYaxis()->SetTitle("Momentum");
    
    TrackMomentumAllTime1_->SetLineColor(kRed);
    TrackMomentumInTime1_->SetLineColor(kRed);
    TrackMomentumOutTime1_->SetLineColor(kRed);
    TrackMomentumAllTime2_->SetLineColor(kBlue);
    TrackMomentumInTime2_->SetLineColor(kBlue);
    TrackMomentumOutTime2_->SetLineColor(kBlue);
    
    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("Momentum for All Tracks");
    TrackMomentumAllTime1_->Draw("e");
    TrackMomentumAllTime2_->Draw("e same");
    auto legend1 = new TLegend(0.3,0.8,.68,.9);
    legend1->AddEntry(TrackMomentumAllTime1_,"Momenta Rec 1");
    legend1->AddEntry(TrackMomentumAllTime2_,"Momenta Rec 2");
    legend1->Draw("same e");
    c1->SaveAs("allTracksMomenta.png");     
    c1->Clear();

    c1->SetTitle("Momentum for In Time Tracks");
    TrackMomentumInTime1_->Draw("e");
    TrackMomentumInTime2_->Draw("e same");
    auto legend2 = new TLegend(0.3,0.8,.68,.9);
    legend2->AddEntry(TrackMomentumInTime1_,"Momenta Rec 1");
    legend2->AddEntry(TrackMomentumInTime2_,"Momenta Rec 2");
    legend2->Draw("same e");
    c1->SaveAs("inTimeTracksMomenta.png");     
    c1->Clear();
    
    gPad->SetLogy(true);
    c1->SetTitle("Momentum for Out Time Tracks");
    TrackMomentumOutTime1_->Draw("e");
    TrackMomentumOutTime2_->Draw("e same");
    auto legend3 = new TLegend(0.3,0.8,.68,.9);
    legend3->AddEntry(TrackMomentumOutTime1_,"Momenta Rec 1");
    legend3->AddEntry(TrackMomentumOutTime2_,"Momenta Rec 2");
    legend3->Draw("same e");
    c1->SaveAs("outTimeTracksMomenta.png");     
    c1->Clear(); 
    return;    
}

void TrackHitAnaProcessor::TrackTransverseMomenta(){
    TCanvas *c1 = new TCanvas("c");
    c1->cd();
    gPad->SetLogy(false); 
    TrackMomentumTAllTime1_->GetXaxis()->SetTitle("Track Number");
    TrackMomentumTAllTime1_->GetYaxis()->SetTitle("Momentum");
    TrackMomentumTInTime1_->GetXaxis()->SetTitle("Track Number");
    TrackMomentumTInTime1_->GetYaxis()->SetTitle("Momentum");
    TrackMomentumTOutTime1_->GetXaxis()->SetTitle("Track Number");
    TrackMomentumTOutTime1_->GetYaxis()->SetTitle("Momentum");
    
    TrackMomentumTAllTime1_->SetLineColor(kRed);
    TrackMomentumTInTime1_->SetLineColor(kRed);
    TrackMomentumTOutTime1_->SetLineColor(kRed);
    TrackMomentumTAllTime2_->SetLineColor(kBlue);
    TrackMomentumTInTime2_->SetLineColor(kBlue);
    TrackMomentumTOutTime2_->SetLineColor(kBlue);
    
    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("Transverse Momentum for All Tracks");
    TrackMomentumTAllTime1_->Draw("e");
    TrackMomentumTAllTime2_->Draw("e same");
    auto legend1 = new TLegend(0.3,0.8,.68,.9);
    legend1->AddEntry(TrackMomentumTAllTime1_,"Momenta Rec 1");
    legend1->AddEntry(TrackMomentumTAllTime2_,"Momenta Rec 2");
    legend1->Draw("same e");
    c1->SaveAs("allTracksMomentaT.png");     
    c1->Clear();

    c1->SetTitle("Transverse Momentum for In Time Tracks");
    TrackMomentumTInTime1_->Draw("e");
    TrackMomentumTInTime2_->Draw("e same");
    auto legend2 = new TLegend(0.3,0.8,.68,.9);
    legend2->AddEntry(TrackMomentumTInTime1_,"Momenta Rec 1");
    legend2->AddEntry(TrackMomentumTInTime2_,"Momenta Rec 2");
    legend2->Draw("same e");
    c1->SaveAs("inTimeTracksMomentaT.png");     
    c1->Clear();
    gPad->SetLogy(true);
    c1->SetTitle("Transverse Momentum for Out Time Tracks");
    TrackMomentumTOutTime1_->Draw("e");
    TrackMomentumTOutTime2_->Draw("e same");
    auto legend3 = new TLegend(0.3,0.8,.68,.9);
    legend3->AddEntry(TrackMomentumTOutTime1_,"Momenta Rec 1");
    legend3->AddEntry(TrackMomentumTOutTime2_,"Momenta Rec 2");
    legend3->Draw("same e");
    c1->SaveAs("outTimeTracksMomentaT.png");     
    c1->Clear(); 
    return;    
}

void TrackHitAnaProcessor::finalize() {
    PlotClusterLayers();  
    PlotClusterCharges();
    PlotClusterTimes();
    PlotClusterPositions();
    if(doingTracks_){
        TrackMomenta()();
        TrackTransverseMomenta()(); 
    }
    return;
}
DECLARE_PROCESSOR(TrackHitAnaProcessor);
