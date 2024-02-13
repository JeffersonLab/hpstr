/**
 * @file SVTClusterAnaProcessor.cxx
 * @brief AnaProcessor used fill histograms to study cluster reconstruction algorithms and dead channels.
 * Does not feature a region selector or histomanager (i.e. no configurable json files), rather 
 * for the limited featured plots allows for more specific manipulation and control.
 * @author Rory O'Dwyer and Cameron Bravo, SLAC National Accelerator Laboratory
 */     
#include "SVTClusterAnaProcessor.h"
#include <iostream>

SVTClusterAnaProcessor::SVTClusterAnaProcessor(const std::string& name, Process& process) : Processor(name,process){
    mmapper_ = new ModuleMapper(2021);
}
SVTClusterAnaProcessor::~SVTClusterAnaProcessor(){}

void SVTClusterAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring SVTClusterAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        layer_           = parameters.getInteger("layer");
        module_          = parameters.getInteger("module");
        isMC_            = parameters.getInteger("isMC");
        doingTracks_     = (parameters.getInteger("doTrack")==1);
        pcut_            = (float)parameters.getDouble("cut");
	badchann_	 = parameters.getString("badchannels");
        //anaName_         = parameters.getString("anaName");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void SVTClusterAnaProcessor::initialize(TTree* tree) {
    fillDeads();
    tree_= tree;
   
    if(isMC_==1){
        layers_=new TH1F("layers","MC Strip Width for All Clusters",12,0.0,12.0);
        layersOnTrk_=new TH1F("layersOnTrk","MC Strip Width for Clusters on Track",12,0.0,12.0);
        layersOffTrk_=new TH1F("layersOffTrk","MC Strip Width for Clusters off Track",12,0.0,12.0);
        charges_=new TH1F("charges","MC Charge Distribution for All Clusters",1000,0.0,0.000016);
        chargesOnTrk_=new TH1F("chargesOnTrk","MC Charge Distribution for On Track",1000,0.0,.000016);
        chargesOffTrk_=new TH1F("chargesOffTrk","MC Charge Distribution for Off Track",1000,0.0,0.000016);

        layersNTD_=new TH1F("layersNTD","MC Strip Width for All Clusters",12,0.0,12.0);
        layersOnTrkNTD_=new TH1F("layersOnTrkNTD","MC Strip Width for Clusters on Track",12,0.0,12.0);
        layersOffTrkNTD_=new TH1F("layersOffTrkNTD","MC Strip Width for Clusters off Track",12,0.0,12.0);
        chargesNTD_=new TH1F("chargesNTD","MC Charge Distribution for All Clusters",1000,0.0,0.000016);
        chargesOnTrkNTD_=new TH1F("chargesOnTrkNTD","MC Charge Distribution for On Track",1000,0.0,.000016);
        chargesOffTrkNTD_=new TH1F("chargesOffTrkNTD","MC Charge Distribution for Off Track",1000,0.0,0.000016);

        positions_=new TH1F("Positions","MC Location of Cluster Hit;Layer;Hits",14,0.0,14.0);
        positionsOnTrk_=new TH1F("PositionsOnTrk","MC Location of Cluster Hit for On Track",14,0.0,14.0);
        ClusDistances_=new TH1F("Minimum Cluster Difference","MC Minimum Distance Between Clusters",14,0.0,14.0);
        ClusDistancesNTD_=new TH1F("Minimum Cluster Difference","MC Minimum Distance Between Clusters",14,0.0,14.0);

        times_=new TH1F("Times","MC Time of Cluster Hit",1000,-60.0,60.0);
        timesOnTrk_=new TH1F("TimesOnTrk","MC Time of On Track Cluster Hit",1000,-60.0,60.0);
        timesOffTrk_=new TH1F("TimesOffTrk","MC Time of Off Cluster Hit",1000,-60.0,60.0);
        timesNTD_=new TH1F("TimesNTD","MC Time of Cluster Hit NTD",1000,-60.0,60.0);
        timesOnTrkNTD_=new TH1F("TimesOnTrkNTD","MC Time of On Track Cluster Hit NTD",1000,-60.0,60.0);
        timesOffTrkNTD_=new TH1F("TimesOffTrkNTD","MC Time of Off Cluster Hit NTD",1000,-60.0,60.0);

        tree_->SetBranchAddress("SiClusters",&Clusters_,&bClusters_);
        tree_->SetBranchAddress("SiClustersOnTrack_KF",&ClustersKF_,&bClustersKF_);
        tree_->SetBranchAddress("SVTRawTrackerHits",&svtraw_,&bsvtraw_);
        return;
    }
    layers_=new TH1F("layers","Strip Width for All Clusters",12,0.0,12.0);
    layersOnTrk_=new TH1F("layersOnTrk","Strip Width for Clusters on Track",12,0.0,12.0);
    layersOffTrk_=new TH1F("layersOffTrk","Strip Width for Clusters off Track",12,0.0,12.0);
    charges_=new TH1F("charges","Charge Distribution for All Clusters",1000,0.0,0.000016);
    chargesOnTrk_=new TH1F("chargesOnTrk","Charge Distribution for On Track",1000,0.0,.000016);
    chargesOffTrk_=new TH1F("chargesOffTrk","Charge Distribution for Off Track",1000,0.0,0.000016);

    layersNTD_=new TH1F("layersNTD","Strip Width for All Clusters",12,0.0,12.0);
    layersOnTrkNTD_=new TH1F("layersOnTrkNTD","Strip Width for Clusters on Track",12,0.0,12.0);
    layersOffTrkNTD_=new TH1F("layersOffTrkNTD","Strip Width for Clusters off Track",12,0.0,12.0);
    chargesNTD_=new TH1F("chargesNTD","Charge Distribution for All Clusters",1000,0.0,0.000016);
    chargesOnTrkNTD_=new TH1F("chargesOnTrkNTD","Charge Distribution for On Track",1000,0.0,.000016);
    chargesOffTrkNTD_=new TH1F("chargesOffTrkNTD","Charge Distribution for Off Track",1000,0.0,0.000016);

    positions_=new TH1F("Positions","Location of Cluster Hit;Layer;Hits",14,0.0,14.0);
    positionsOnTrk_=new TH1F("PositionsOnTrk","Location of Cluster Hit for On Track",14,0.0,14.0);
    ClusDistances_=new TH1F("Minimum Cluster Difference","Minimum Distance Between Clusters",14,0.0,14.0);
    ClusDistancesNTD_=new TH1F("Minimum Cluster Difference","Minimum Distance Between Clusters",14,0.0,14.0);

    times_=new TH1F("Times","Time of Cluster Hit",1000,-60.0,60.0);
    timesOnTrk_=new TH1F("TimesOnTrk","Time of On Track Cluster Hit",1000,-60.0,60.0);
    timesOffTrk_=new TH1F("TimesOffTrk","Time of Off Cluster Hit",1000,-60.0,60.0);
    timesNTD_=new TH1F("TimesNTD","Time of Cluster Hit NTD",1000,-60.0,60.0);
    timesOnTrkNTD_=new TH1F("TimesOnTrkNTD","Time of On Track Cluster Hit NTD",1000,-60.0,60.0);
    timesOffTrkNTD_=new TH1F("TimesOffTrkNTD","Time of Off Cluster Hit NTD",1000,-60.0,60.0);
    if(doingTracks_){
        Z0VNShare2Hist_= new TH2F("Z0VNShare2Hist","Z0 versus Number of Shared Hits No Cut",100,0,3,8,0,8);
        Z0VNShare2HistCut_= new TH2F("Z0VNShare2HistCut","Z0 versus Number of Shared Hits Momentum Cut",100,0,3,8,0,8);
        SharedAmplitudes_= new TH1F("SharedAmplitudes","The Amplitudes of Clusters Shared Between Tracks",1000,0.0,0.000016);
        UnSharedAmplitudes_= new TH1F("UnSharedAmplitudes","The Amplitudes of Clusters Not Shared Between Tracks",1000,0.0,0.000016);
        SharedTimes_= new TH1F("SharedTimes","The Times of Clusters Shared Between Tracks",1000,-60.0,60.0);
        UnSharedTimes_= new TH1F("UnSharedTimes","The Times of Clusters Not Shared Between Tracks",1000,-60.0,60.0); 
    }
    tree_->SetBranchAddress("SiClusters",&Clusters_,&bClusters_);
    tree_->SetBranchAddress("SiClustersOnTrack_KF",&ClustersKF_,&bClustersKF_);
    tree_->SetBranchAddress("SVTRawTrackerHits",&svtraw_,&bsvtraw_);
    if(doingTracks_){
        tree_->SetBranchAddress("KalmanFullTracks",&tracks_,&btracks_);
    }
}

bool SVTClusterAnaProcessor::process(IEvent* ievent) {
    if(doingTracks_){
        for(int i = 0;i<tracks_->size();i++){
            Track* track = tracks_->at(i);
            if(track->getTrackTime()*track->getTrackTime()<100.0){
                Z0VNShare2Hist_->Fill(track->getZ0Err(),track->getNShared());
                if(track->getP()<pcut_){
                    Z0VNShare2HistCut_->Fill(track->getZ0Err(),track->getNShared());
                }
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
                        SharedAmplitudes_->Fill(clu->getCharge());
                        SharedTimes_->Fill(clu->getTime());
                    }else{
                        UnSharedAmplitudes_->Fill(clu->getCharge());
                        UnSharedTimes_->Fill(clu->getTime());
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
            //Now is the part where I fill the cluster distance histogram.
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
                ClusDistances_->Fill(Dist);
                if(NTD){ClusDistancesNTD_->Fill(Dist);}
            }
            layers_->Fill(nLayers);
            charges_->Fill(ncharges);
            positions_->Fill(clu->getLayer());
            times_->Fill(ntimes);
            if(onTrk){
                layersOnTrk_->Fill(nLayers);
                chargesOnTrk_->Fill(ncharges);
                timesOnTrk_->Fill(ntimes);
            }else{
                layersOffTrk_->Fill(nLayers);
                chargesOffTrk_->Fill(ncharges);
                timesOffTrk_->Fill(ntimes);
            }
            if(NTD){
                layersNTD_->Fill(nLayers);
                chargesNTD_->Fill(ncharges);
                positionsOnTrk_->Fill(clu->getLayer());
                timesNTD_->Fill(ntimes);
                if(onTrk){
                    layersOnTrkNTD_->Fill(nLayers);
                    chargesOnTrkNTD_->Fill(ncharges);
                    timesOnTrkNTD_->Fill(ntimes);
                }else{
                    layersOffTrkNTD_->Fill(nLayers);
                    chargesOffTrkNTD_->Fill(ncharges);
                    timesOffTrkNTD_->Fill(ntimes);
                } 
            }
        }      
    }
    return true;
}

void SVTClusterAnaProcessor::fillDeads(){
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

int SVTClusterAnaProcessor::GetStrip(int feb,int hyb,int strip){
	int BigCount = 0;
    if(feb<=1){
        BigCount+=feb*2048+hyb*512+strip;
    }else{
        BigCount+=4096;
        BigCount+=(feb-2)*2560+hyb*640+strip;
    }
    return BigCount;
}

void SVTClusterAnaProcessor::PlotClusterLayers(){
    TCanvas *c1 = new TCanvas("c");
    gPad->SetLogy(true);
    c1->cd();
    layers_->GetXaxis()->SetTitle("Width (strip)");
    layers_->GetYaxis()->SetTitle("Hits"); 
    layersOnTrk_->GetXaxis()->SetTitle("Width (strip)");
    layersOnTrk_->GetYaxis()->SetTitle("Hits"); 
    layersOffTrk_->GetXaxis()->SetTitle("Width (strip)");
    layersOffTrk_->GetYaxis()->SetTitle("Hits"); 

    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("Layers for All Clusters");
    layers_->Draw("e");
    c1->SaveAs("allClusters.png");     
    c1->Clear();

    c1->SetTitle("Layers for On Track Clusters");
    layersOnTrk_->Draw("e");
    c1->SaveAs("onClusters.png");     
    c1->Clear();
    
    c1->SetTitle("Layers for Off Track Clusters");
    layersOffTrk_->Draw("e");
    c1->SaveAs("offClusters.png");     
    c1->Clear();

    layers_->SetTitle("Cluster Strip Width for all Cluster Cuts");
    auto legend = new TLegend(0.3,0.8,.68,.9);
    layers_->SetLineColor(kCyan);
    layersOnTrk_->SetLineColor(kGreen);
    legend->AddEntry(layers_,"Layers");
    legend->AddEntry(layersOnTrk_,"Layers On Track");
    legend->AddEntry(layersOffTrk_,"Layers Off Track");
    layers_->Draw("e");
    layersOnTrk_->Draw("same e");
    layersOffTrk_->Draw("same e");
    legend->Draw("same e");
    c1->SaveAs("PutOnTogether.png");
    c1->Clear();
    return;
}

void SVTClusterAnaProcessor::PlotClusterCharges(){
    TCanvas *c1 = new TCanvas("c");
    gPad->SetLogy(true);
    c1->cd();
    
    charges_->GetXaxis()->SetTitle("Charge");
    charges_->GetYaxis()->SetTitle("Hits"); 
    chargesOffTrk_->GetXaxis()->SetTitle("Charge");
    chargesOffTrk_->GetYaxis()->SetTitle("Hits"); 
    chargesOnTrk_->GetXaxis()->SetTitle("Charge");
    chargesOnTrk_->GetYaxis()->SetTitle("Hits"); 
    
    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("Charges for All Clusters");
    charges_->Draw("e");
    c1->SaveAs("allClustersCharge.png");     
    c1->Clear();

    c1->SetTitle("Charges for On Track Clusters");
    chargesOnTrk_->Draw("e");
    c1->SaveAs("onClustersCharge.png");     
    c1->Clear();
    
    c1->SetTitle("Charges for Off Track Clusters");
    chargesOffTrk_->Draw("e");
    c1->SaveAs("offClustersCharge.png");     
    c1->Clear();

    layers_->SetTitle("Putting all Charges Together");
    auto legend = new TLegend(0.3,0.8,.68,.9);
    charges_->SetLineColor(kCyan);
    chargesOnTrk_->SetLineColor(kGreen);
    legend->AddEntry(charges_,"Charges");
    legend->AddEntry(chargesOnTrk_,"Charges On Track");
    legend->AddEntry(chargesOffTrk_,"Charges Off Track");
    charges_->Draw("e");
    chargesOnTrk_->Draw("same e");
    chargesOffTrk_->Draw("same e");
    legend->Draw("same e");
    c1->SaveAs("PutOnTogetherCharges.png");
    c1->Clear();
    return;
}

void SVTClusterAnaProcessor::PlotClusterLayersNTD(){
    TCanvas *c1 = new TCanvas("c");
    gPad->SetLogy(true);
    c1->cd();
    layersNTD_->GetXaxis()->SetTitle("Width (strip)");
    layersNTD_->GetYaxis()->SetTitle("Hits"); 
    layersOnTrkNTD_->GetXaxis()->SetTitle("Width (strip)");
    layersOnTrkNTD_->GetYaxis()->SetTitle("Hits"); 
    layersOffTrkNTD_->GetXaxis()->SetTitle("Width (strip)");
    layersOffTrkNTD_->GetYaxis()->SetTitle("Hits"); 

    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("NTD Layers for All Clusters");
    layersNTD_->Draw("e");
    c1->SaveAs("allClustersNTD.png");     
    c1->Clear();

    c1->SetTitle("NTD Layers for On Track Clusters");
    layersOnTrkNTD_->Draw("e");
    c1->SaveAs("onClustersNTD.png");     
    c1->Clear();
    
    c1->SetTitle("NTD Layers for Off Track Clusters");
    layersOffTrkNTD_->Draw("e");
    c1->SaveAs("offClustersNTD.png");     
    c1->Clear();

    layersNTD_->SetTitle("NTD Cluster Strip Width for all Cluster Cuts");
    auto legend = new TLegend(0.3,0.8,.68,.9);
    layersNTD_->SetLineColor(kCyan);
    layersOnTrkNTD_->SetLineColor(kGreen);
    legend->AddEntry(layersNTD_,"Layers");
    legend->AddEntry(layersOnTrkNTD_,"Layers On Track");
    legend->AddEntry(layersOffTrkNTD_,"Layers Off Track");
    layersNTD_->Draw("e");
    layersOnTrkNTD_->Draw("same e");
    layersOffTrkNTD_->Draw("same e");
    legend->Draw("same e");
    c1->SaveAs("PutOnTogetherNTD.png");
    c1->Clear();
    return;
}

void SVTClusterAnaProcessor::PlotClusterChargesNTD(){
    TCanvas *c1 = new TCanvas("c");
    gPad->SetLogy(true);
    c1->cd();
    
    chargesNTD_->GetXaxis()->SetTitle("Charge");
    chargesNTD_->GetYaxis()->SetTitle("Hits"); 
    chargesOffTrkNTD_->GetXaxis()->SetTitle("Charge");
    chargesOffTrkNTD_->GetYaxis()->SetTitle("Hits"); 
    chargesOnTrkNTD_->GetXaxis()->SetTitle("Charge");
    chargesOnTrkNTD_->GetYaxis()->SetTitle("Hits"); 
    
    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("NTD Charges for All Clusters");
    chargesNTD_->Draw("e");
    c1->SaveAs("allClustersChargeNTD.png");     
    c1->Clear();

    c1->SetTitle("NTD Charges for On Track Clusters");
    chargesOnTrkNTD_->Draw("e");
    c1->SaveAs("onClustersChargeNTD.png");     
    c1->Clear();
    
    c1->SetTitle("NTD Charges for Off Track Clusters");
    chargesOffTrkNTD_->Draw("e");
    c1->SaveAs("offClustersChargeNTD.png");     
    c1->Clear();

    chargesNTD_->SetTitle("NTD Charge Distribution for all Charges");
    auto legend = new TLegend(0.3,0.8,.68,.9);
    chargesNTD_->SetLineColor(kCyan);
    chargesOnTrkNTD_->SetLineColor(kGreen);
    legend->AddEntry(chargesNTD_,"Charges");
    legend->AddEntry(chargesOnTrkNTD_,"Charges On Track");
    legend->AddEntry(chargesOffTrkNTD_,"Charges Off Track");
    chargesNTD_->Draw("e");
    chargesOnTrkNTD_->Draw("same e");
    chargesOffTrkNTD_->Draw("same e");
    legend->Draw("same e");
    c1->SaveAs("PutOnTogetherChargesNTD.png");
    c1->Clear();
    return;
}

void SVTClusterAnaProcessor::PlotClusterPositions(){
    TCanvas *c1 = new TCanvas("c");
    gPad->SetLogy(true);
    c1->cd();
    positions_->GetXaxis()->SetTitle("Layer");
    positions_->GetYaxis()->SetTitle("Hits"); 

    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("Cluster Position for all Clusters");
    positions_->Draw("e");
    c1->SaveAs("positions.png");     
    c1->Clear();

    positionsOnTrk_->GetXaxis()->SetTitle("Layer");
    positionsOnTrk_->GetYaxis()->SetTitle("Hits"); 

    c1->SetTitle("Cluster Position for NTD Clusters");
    positionsOnTrk_->Draw("e");
    c1->SaveAs("positionsNTD.png");     
    c1->Clear();

    ClusDistances_->GetXaxis()->SetTitle("Separation");
    ClusDistances_->GetYaxis()->SetTitle("Pairs"); 

    c1->SetTitle("Cluster Separation on Same Sensors");
    ClusDistances_->Draw("e");
    c1->SaveAs("clusDistances.png");     
    c1->Clear();

    ClusDistancesNTD_->GetXaxis()->SetTitle("Separation");
    ClusDistancesNTD_->GetYaxis()->SetTitle("Pairs"); 

    c1->SetTitle("Cluster Separation on Same Sensors for NTD");
    ClusDistancesNTD_->Draw("e");
    c1->SaveAs("clusDistancesNTD.png");     
    c1->Clear();
}

void SVTClusterAnaProcessor::PlotClusterTimes(){
    TCanvas *c1 = new TCanvas("c");
    gPad->SetLogy(true);
    c1->cd();
    times_->GetXaxis()->SetTitle("Times (ns)");
    times_->GetYaxis()->SetTitle("Hits"); 
    timesOnTrk_->GetXaxis()->SetTitle("Time (ns)");
    timesOnTrk_->GetYaxis()->SetTitle("Hits"); 
    timesOffTrk_->GetXaxis()->SetTitle("Time (ns)");
    timesOffTrk_->GetYaxis()->SetTitle("Hits"); 

    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("Times for All Clusters");
    times_->Draw();
    c1->SaveAs("alltimes.png");     
    c1->Clear();

    c1->SetTitle("Times for On Track Clusters");
    timesOnTrk_->Draw("e");
    c1->SaveAs("ontimes.png");     
    c1->Clear();
    
    c1->SetTitle("Times for Off Track Clusters");
    timesOffTrk_->Draw("e");
    c1->SaveAs("offtimes.png");     
    c1->Clear();

    times_->SetTitle("Cluster Times for all Cluster Cuts");
    auto legend = new TLegend(0.3,0.8,.68,.9);
    times_->SetLineColor(kCyan);
    timesOnTrk_->SetLineColor(kGreen);
    legend->AddEntry(times_,"Times");
    legend->AddEntry(timesOnTrk_,"Times On Track");
    legend->AddEntry(timesOffTrk_,"Times Off Track");
    times_->Draw("e");
    timesOnTrk_->Draw("same e");
    timesOffTrk_->Draw("same e");
    legend->Draw("same e");
    c1->SaveAs("PutOnTogetherTimes.png");
    c1->Clear();
    return;
}

void SVTClusterAnaProcessor::PlotClusterTimesNTD(){
    TCanvas *c1 = new TCanvas("c");
    gPad->SetLogy(true);
    c1->cd();
    timesNTD_->GetXaxis()->SetTitle("Times (ns)");
    timesNTD_->GetYaxis()->SetTitle("Hits"); 
    timesOnTrkNTD_->GetXaxis()->SetTitle("Time (ns)");
    timesOnTrkNTD_->GetYaxis()->SetTitle("Hits"); 
    timesOffTrkNTD_->GetXaxis()->SetTitle("Time (ns)");
    timesOffTrkNTD_->GetYaxis()->SetTitle("Hits"); 

    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("Times for All Clusters");
    timesNTD_->Draw("e");
    c1->SaveAs("alltimesNTD.png");     
    c1->Clear();

    c1->SetTitle("Times for On Track Clusters");
    timesOnTrkNTD_->Draw("e");
    c1->SaveAs("ontimesNTD.png");     
    c1->Clear();
    
    c1->SetTitle("Times for Off Track Clusters");
    timesOffTrkNTD_->Draw("e");
    c1->SaveAs("offtimesNTD.png");     
    c1->Clear();

    timesNTD_->SetTitle("Cluster Times for all Cluster Cuts");
    auto legend = new TLegend(0.3,0.8,.68,.9);
    timesNTD_->SetLineColor(kCyan);
    timesOnTrkNTD_->SetLineColor(kGreen);
    legend->AddEntry(timesNTD_,"Times");
    legend->AddEntry(timesOnTrkNTD_,"Times On Track");
    legend->AddEntry(timesOffTrkNTD_,"Times Off Track");
    timesNTD_->Draw("e");
    timesOnTrkNTD_->Draw("same e");
    timesOffTrkNTD_->Draw("same e");
    legend->Draw("same e");
    c1->SaveAs("PutOnTogetherTimesNTD.png");
    c1->Clear();
    return;
}

void SVTClusterAnaProcessor::TrackPlot(){
    TCanvas *c1 = new TCanvas("c");
    c1->cd();
    //FIRST I DO THE PROFILES OF THE NSHARED HITS PLOTS
    
    TProfile* p1 = Z0VNShare2Hist_->ProfileX("p1");
    TProfile* p2 = Z0VNShare2HistCut_->ProfileX("p2");
    auto legend = new TLegend(0.3,0.8,.68,.9);
    p2->SetLineColor(kRed);
    p2->SetLineWidth(2.0);
    p1->SetLineWidth(2.0);
    legend->AddEntry(p1,"Shared Profile Not Cut");
    legend->AddEntry(p2,"Shared Profile Cut");
    p1->Draw("e");
    p2->Draw("same e");
    legend->Draw("same e");
    c1->SaveAs("Z0VSharedProfile.png");
    c1->Clear();

    //Now I do the charge distribution for shared and unshared hits overlayed.
    legend = new TLegend(0.3,0.8,.68,.9);
    SharedAmplitudes_->SetLineColor(kRed);
    SharedAmplitudes_->SetLineWidth(2.0);
    UnSharedAmplitudes_->SetLineWidth(2.0);
    legend->AddEntry(SharedAmplitudes_,"Charge Distribution for Shared Clusters");
    legend->AddEntry(UnSharedAmplitudes_,"Charge Distribution for UnShared Clusters");
    UnSharedAmplitudes_->Draw("e");
    SharedAmplitudes_->Draw("same e");
    legend->Draw("same e");
    gPad->SetLogy(true);
    c1->SaveAs("SharedVUnSharedChargeDist.png");
    c1->Clear();

    //Now I do the time distribution for shared and unshared hits overlayed.
    SharedTimes_->SetTitle("The Time of Clusters Shared Between Tracks");
    legend = new TLegend(0.3,0.8,.68,.9);
    SharedTimes_->SetLineColor(kRed);
    SharedTimes_->SetLineWidth(2.0);
    UnSharedTimes_->SetLineWidth(2.0);
    legend->AddEntry(SharedTimes_,"Time Distribution for Shared Clusters");
    legend->AddEntry(UnSharedTimes_,"Time Distribution for UnShared Clusters");
    UnSharedTimes_->Draw("e");
    SharedTimes_->Draw("same e");
    legend->Draw("same e");
    c1->SaveAs("SharedVUnSharedTimeDist.png");
    gPad->SetLogy(false);
    c1->Clear();
    return;    
}

void SVTClusterAnaProcessor::finalize() {
    PlotClusterLayers();
    PlotClusterLayersNTD();
    PlotClusterCharges();
    PlotClusterChargesNTD();
    PlotClusterPositions();
    PlotClusterTimes();
    PlotClusterTimesNTD();
    if(doingTracks_){
        TrackPlot();
    }
    return;
}
DECLARE_PROCESSOR(SVTClusterAnaProcessor);
