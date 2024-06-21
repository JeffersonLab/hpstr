/**
 * @file RoryClusterCompareAnaProcessor.cxx
 * @brief AnaProcessor used to compare two means of reconstruction directly.
 * @author Rory O'Dwyer and Cameron Bravo, SLAC National Accelerator Laboratory
 */     
#include "RoryClusterCompareAnaProcessor.h"
//#include "Int_t.h"
#include <iostream>

RoryClusterCompareAnaProcessor::RoryClusterCompareAnaProcessor(const std::string& name, Process& process) : Processor(name,process){
    mmapper_ = new ModuleMapper(2021);
}
//TODO CHECK THIS DESTRUCTOR
RoryClusterCompareAnaProcessor::~RoryClusterCompareAnaProcessor(){}

void RoryClusterCompareAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring RoryClusterCompareAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        layer_           = parameters.getInteger("layer");
        module_          = parameters.getInteger("module");
        isMC_            = parameters.getInteger("isMC");
        doingTracks_     = (parameters.getInteger("doTrack")==1);
        pcut_            = (float)parameters.getDouble("cut");
        //anaName_         = parameters.getString("anaName");
    	fileOne_	 = parameters.getString("fileName1");
	fileTwo_	 = parameters.getString("fileName2");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}


void RoryClusterCompareAnaProcessor::initialize(std::string inFilename, std::string outFilename){
    fillDeads();   
    std::cout<<"DO I MAKE IT HERE1"<<std::endl;
    file1_ = new HpsEventFile(fileOne_, "hello1.root");
    file2_ = new HpsEventFile(fileTwo_, "hello2.root");
    std::cout<<"DO I MAKE IT HERE2"<<std::endl;
    file1_->setupEvent(&event1_);
    file2_->setupEvent(&event2_);
    std::cout<<"DO I MAKE IT HERE3"<<std::endl;
    tree1_= event1_.getTree();tree2_= event2_.getTree(); 
    std::cout<<"DO I MAKE IT HERE4"<<std::endl;
    //tree_->Print();
   
    //Instantiating the first layer
    
    layers1_=new TH1F("layers","Strip Width for All Clusters",12,0.0,12.0);
    layersOnTrk1_=new TH1F("layersOnTrk","Strip Width for Clusters on Track",12,0.0,12.0);
    layersOffTrk1_=new TH1F("layersOffTrk","Strip Width for Clusters off Track",12,0.0,12.0);
    charges1_=new TH1F("charges","Charge Distribution for All Clusters",1000,0.0,0.000016);
    charges2D_=new TH2F("Charge Distribution for Both Reco Techniques","Charge Distribution for Both Reco Techniques",500,0.0,0.000016,500,0.0,0.000016);
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
    	HitsOnTrack1_ = new TH1F("HitsOnTrack","Hits On Tracks",15,-.5,14.5);
    }


    //Instantiating the first layer
    
    layers2_=new TH1F("layers2","Strip Width for All Clusters2",12,0.0,12.0);
    layersOnTrk2_=new TH1F("layersOnTrk2","Strip Width for Clusters on Track2",12,0.0,12.0);
    layersOffTrk2_=new TH1F("layersOffTrk2","Strip Width for Clusters off Track2",12,0.0,12.0);
    charges2_=new TH1F("charges2","Charge Distribution for All Clusters2",1000,0.0,0.000016);
    chargesOnTrk2_=new TH1F("chargesOnTrk2","Charge Distribution for On Track2",1000,0.0,.000016);
    chargesOffTrk2_=new TH1F("chargesOffTrk2","Charge Distribution for Off Track2",1000,0.0,0.000016);

    layersNTD2_=new TH1F("layersNTD2","Strip Width for All Clusters2",12,0.0,12.0);
    layersOnTrkNTD2_=new TH1F("layersOnTrkNTD2","Strip Width for Clusters on Track2",12,0.0,12.0);
    layersOffTrkNTD2_=new TH1F("layersOffTrkNTD2","Strip Width for Clusters off Track2",12,0.0,12.0);
    chargesNTD2_=new TH1F("chargesNTD2","Charge Distribution for All Clusters2",1000,0.0,0.000016);
    chargesOnTrkNTD2_=new TH1F("chargesOnTrkNTD2","Charge Distribution for On Track2",1000,0.0,.000016);
    chargesOffTrkNTD2_=new TH1F("chargesOffTrkNTD2","Charge Distribution for Off Track2",1000,0.0,0.000016);

    times2_=new TH1F("Times2","Time of Cluster Hit2",1000,-60.0,60.0);
    timesOnTrk2_=new TH1F("TimesOnTrk2","Time of On Track Cluster Hit2",1000,-60.0,60.0);
    timesOffTrk2_=new TH1F("TimesOffTrk2","Time of Off Cluster Hit2",1000,-60.0,60.0);
    timesNTD2_=new TH1F("TimesNTD2","Time of Cluster Hit NTD2",1000,-60.0,60.0);
    timesOnTrkNTD2_=new TH1F("TimesOnTrkNTD2","Time of On Track Cluster Hit NTD2",1000,-60.0,60.0);
    timesOffTrkNTD2_=new TH1F("TimesOffTrkNTD2","Time of Off Cluster Hit NTD2",1000,-60.0,60.0);
    
    positions2_=new TH1F("Positions2","Location of Cluster Hit;Layer;Hits2",14,0.0,14.0);
    positionsOnTrk2_=new TH1F("PositionsOnTrk2","Location of Cluster Hit for On Track2",14,0.0,14.0);
    ClusDistances2_=new TH1F("Minimum Cluster Difference2","Minimum Distance Between Clusters2",14,0.0,14.0);
    ClusDistancesNTD2_=new TH1F("Minimum Cluster Difference2","Minimum Distance Between Clusters2",14,0.0,14.0);

    
    //Event by event comparisions of variables
    DifferenceInPositions_=new TH1F("Difference in Cluster Positions","Difference in Cluster Positions",100,-0.05,.5);
    //DifferenceInPositions_=new TH1F("Difference in Cluster Positions","Difference in Cluster Positions",100,-0.0000000005,.000000005);
    DifferenceInCharges_=new TH1F("Difference in Charge Positions","Difference in Charge Positions",100,-.000005,.00005);
    DifferenceInPositionsVStripNo_= new TH2F("Difference in Cluster Positions vs. Strip No","Difference in Cluster Positions vs. Strip No",100,-0.05,.5,6,0,6);
    DifferenceInPositionVsCharge_ = new TH2F("Difference in Cluster Positions vs. Charge Difference","Difference in Cluster Positions vs. Charge Difference",100,-0.05,.5,100,-.000005,.00005);
    DifferenceInPositionVsChargeNTD_ = new TH2F("Difference in Cluster Positions vs. Charge Difference NTD","Difference in Cluster Positions vs. Charge Difference NTD",100,-0.05,.5,100,-.000005,.00005);
    MinComboPosition0_ =  new TH2F("Position X of First Cluster v.s Position of Closest Cluster","Position X of First Cluster v.s Position of Closest Cluster",200,-.5,.5,200,-.5,.5); 
    MinComboPosition1_ =  new TH2F("Position Y of First Cluster v.s Position of Closest Cluster","Position Y of First Cluster v.s Position of Closest Cluster",200,-.5,.5,200,-.5,.5); 
    MinComboPosition2_ =  new TH2F("Position Z of First Cluster v.s Position of Closest Cluster","Position Z of First Cluster v.s Position of Closest Cluster",200,-.5,.5,200,-.5,.5); 
    MinComboChargeNTD_ = new TH2F("Cluster of Smaller One vs. Combined Cluster","Cluster of Smaller One vs. Combined Cluster",100,-0.000005,.00005,100,-.000005,.00005);
    DeltaTVsDeltaCharge_ = new TH2F("Time Between NTD vs. Did the Bind","Time Between NTD vs. Did the Bind",200,-100.0,100.0,2000,-.00005,.00005);
    DeltaTVsDeltaChargeV2_ = new TH2F("Time Between NTD vs. Did the Bind 2","Time Between NTD vs. Did the Bind 2",2000,-220.0,220.0,2000,-.00005,.00005);
    ChargeShared2D_ = new TH2F("Cluster of Smaller One vs. Larger One","Cluster of Smaller One vs. Larger One",100,-0.000005,.00005,100,-.000005,.00005);


    if(doingTracks_){
        Z0VNShare2Hist2_= new TH2F("Z0VNShare2Hist2","Z0 versus Number of Shared Hits No Cut2",100,0,3,8,0,8);
        Z0VNShare2HistCut2_= new TH2F("Z0VNShare2HistCut2","Z0 versus Number of Shared Hits Momentum Cut2",100,0,3,8,0,8);
        SharedAmplitudes2_= new TH1F("SharedAmplitudes2","The Amplitudes of Clusters Shared Between Tracks2",1000,0.0,0.000016);
        UnSharedAmplitudes2_= new TH1F("UnSharedAmplitudes2","The Amplitudes of Clusters Not Shared Between Tracks2",1000,0.0,0.000016);
        SharedTimes2_= new TH1F("SharedTimes2","The Times of Clusters Shared Between Tracks2",1000,-60.0,60.0);
        UnSharedTimes2_= new TH1F("UnSharedTimes2","The Times of Clusters Not Shared Between Tracks2",1000,-60.0,60.0); 
        
        TrackMomentumInTime2_ = new TH1F("TrackMomentumInTime2","The Momentum of In Time Tracks2",1000,0.0,7.0);
        TrackMomentumOutTime2_ =  new TH1F("TrackMomentumOutTime2","The Momentum of Out of Time Tracks2",1000,0.0,7.0);
        TrackMomentumAllTime2_ = new TH1F("TrackMomentumAll2","The Momentum of All Tracks2",1000,0.0,7.0);
    
        TrackMomentumTInTime2_ = new TH1F("TrackMomentumTInTime2","The Transverse Momentum of In Time Tracks2",1000,0.0,7.0);
        TrackMomentumTOutTime2_ =  new TH1F("TrackMomentumTOutTime2","The Transverse Momentum of Out of Time Tracks2",1000,0.0,7.0);
        TrackMomentumTAllTime2_ = new TH1F("TrackMomentumTAll2","The Transverse Momentum of All Tracks2",1000,0.0,7.0);
    	HitsOnTrack2_ = new TH1F("HitsOnTrack2","Hits On Tracks",15,-.5,14.5);
    }
    std::cout<<"I GET HERE 1"<<std::endl;
    tree1_->SetBranchAddress("SiClusters",&Clusters1_,&bClusters1_);
    tree1_->SetBranchAddress("SiClustersOnTrack_KF",&ClustersKF1_,&bClustersKF1_);
    tree1_->SetBranchAddress("SVTRawTrackerHits",&svtraw1_,&bsvtraw1_);
    
    tree2_->SetBranchAddress("SiClusters",&Clusters2_,&bClusters2_);
    tree2_->SetBranchAddress("SiClustersOnTrack_KF",&ClustersKF2_,&bClustersKF2_);
    tree2_->SetBranchAddress("SVTRawTrackerHits",&svtraw2_,&bsvtraw2_);
    
    if(doingTracks_){
        tree1_->SetBranchAddress("KalmanFullTracks", &tracks1_, &btracks1_);
        tree2_->SetBranchAddress("KalmanFullTracks", &tracks2_, &btracks2_);
    }
    std::cout<<"I GET HERE 2"<<std::endl;
}


bool RoryClusterCompareAnaProcessor::process() {
    int n_events_processed=0;
    while((file1_->nextEvent())and(n_events_processed<40000000)) {
	file2_->nextEvent();
        if (n_events_processed%1000 == 0){std::cout<<"Event:"<<n_events_processed<<std::endl;}
	loop(&event1_,&event2_);
	n_events_processed++;
    }
    return true;
}


void RoryClusterCompareAnaProcessor::loop(IEvent *event1,IEvent *event2) {
    //std::cout<<"We have "<< ClustersKF_->size()<<" hits"<<std::endl;
    
    //if(ident_>1.0){std::cout<<ident_<<std::endl;}
    for(int ident_=1;ident_<3;ident_++){
	int size=-1;
	if(doingTracks_){
	    if(ident_<1.5){size=tracks1_->size();}else{size=tracks2_->size();}
            for(int i = 0;i<size;i++){
                Track* track;
	        if(ident_<1.5){track = tracks1_->at(i);}else{track=tracks2_->at(i);}
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
    	int size2=-1;
	if(ident_<1.5){size2=Clusters1_->size();}else{size2=Clusters2_->size();}
    	for(int i = 0; i < size2; i++){ 
            TrackerHit * clu;
	    if(ident_<1.5){clu = Clusters1_->at(i);}else{clu=Clusters2_->at(i);}
            Int_t LAYER = -1;
            Int_t MODULE = -1;      
 
            RawSvtHit * seed = (RawSvtHit*)(clu->getRawHits().At(0));

            LAYER=clu->getLayer();
            MODULE=seed->getModule(); 
	    
	    
	    float seedStrip = (float)(seed->getStrip());
            float nLayers = (float)(clu->getRawHits().GetEntries());
            float ncharges = (float)(clu->getCharge());
            float ntimes = (float)(clu->getTime());
            
	    bool onTrk = false;
            bool NTD = false;
	    int size3 = -1;
	    if(ident_<1.5){size3=ClustersKF1_->size();}else{size3=ClustersKF2_->size();}
            for(unsigned int j = 0; j < size3; j++){
		int ID=-1;
	    	if(ident_<1.5){ID=ClustersKF1_->at(j)->getID();}else{ID=ClustersKF2_->at(j)->getID();}	
                if(clu->getID()==ID){
                    onTrk = true;
                }
            }

            std::string input = "ly"+std::to_string(LAYER+1)+"_m"+std::to_string(MODULE);
            std::string helper = mmapper_->getHwFromSw(input);
        
	    int feb=std::stoi(helper.substr(1,1));
            int hyb=std::stoi(helper.substr(3,1));
            //std::cout<<"I GOT HERE HELLO COMPARE"<<std::endl; 
            int channelL=seedStrip-1;
            int channelR=seedStrip+1;
            if(channelL>=0){
                NTD=(NTD)||(Deads_[GetStrip(feb,hyb,channelL)]==1);
            }
            if(((feb<=1)&&(channelR<=511))||((feb>1)&&(channelR<=639))){
                NTD=(NTD)||(Deads_[GetStrip(feb,hyb,channelR)]==1); 
            }
	    //if(NTD){std::cout<<"THE NTD IS "<<NTD<<std::endl;}
	    if(ident_<1.5){
	    	//WE ARE GOING TO DO ALL OUR EVENT BY EVENT ANALYSIS HERE
		bool general = ((layer_==-1)||(module_==-1));
                if(((LAYER==layer_)&&(MODULE==module_))||(general)){
		    float minCharge=100000000;
		    float minDist=10000000;
		    int minCoor=0;int secCoor=-1;
		    std::vector<double> pos1 = clu->getPosition();
		    for(int j = 0; j<Clusters2_->size(); j++){
			std::vector<double> pos2 = Clusters2_->at(j)->getPosition();
			float curDist=0.0;
			float curCharge=std::abs(clu->getCharge()-Clusters2_->at(j)->getCharge());
			for(int d=0;d<3;d++){
			    curDist+=(pos2[d]-pos1[d])*(pos2[d]-pos1[d]);
			}
			curDist=std::sqrt(curDist);
			if(curDist<minDist){
			    secCoor=minCoor;
			    minCoor=j;
			    minDist=curDist;
		            minCharge=curCharge;
			}
		    }
		    DifferenceInPositions_->Fill(minDist);
		    DifferenceInCharges_->Fill(minCharge);
		    charges2D_->Fill(clu->getCharge(),Clusters2_->at(minCoor)->getCharge());
		    DifferenceInPositionsVStripNo_->Fill(minDist,clu->getRawHits().GetEntries());
		    DifferenceInPositionVsCharge_->Fill(minDist,minCharge); 
		    std::vector<double> pos2 = Clusters2_->at(minCoor)->getPosition();
		    MinComboPosition0_->Fill(pos1[0],pos2[0]);
		    MinComboPosition1_->Fill(pos1[1],pos2[1]);
		    MinComboPosition2_->Fill(pos1[2],pos2[2]);
		    bool condition1 = (Deads_[GetStrip(feb,hyb,channelL)]==1)or(Deads_[GetStrip(feb,hyb,channelR)]==1); 
		    //std::cout<<"I GOT HERE HELLO COMPARE 2"<<std::endl; 
		    if((secCoor>=0)and(condition1)){
			//std::cout<<"I GOT HERE HELLO COMPARE 3"<<std::endl; 
			double MTIME = 0.0;
			double MTIMEERRINV = 0.0;
			//Clusters2_->at(minCoor)->getRawHits().GetEntries()
			for(int PP=0;PP<Clusters2_->at(minCoor)->getNHits();PP++){
				//std::cout<<"I GOT HERE HELLO COMPARE 4"<<std::endl; 
				RawSvtHit * hithold = (RawSvtHit *)(Clusters2_->at(minCoor)->getRawHits().At(PP));
				for(int PPP=0;PPP<hithold->getFitN();PPP++){
					MTIME+=hithold->getT0(PPP)/hithold->getT0err(PPP);
					MTIMEERRINV+=(1.0/hithold->getT0err(PPP));
				}
			}
			MTIME=MTIME/MTIMEERRINV;
			//std::cout<<"I GOT HERE HELLO COMPARE 3"<<std::endl; 
			double STIME = 0.0;
			double STIMEERRINV = 0.0;
			//Clusters2_->at(secCoor)->getRawHits().GetEntries()
			for(int PP=0;PP<Clusters2_->at(secCoor)->getNHits();PP++){
				RawSvtHit * hithold = (RawSvtHit *)(Clusters2_->at(secCoor)->getRawHits().At(PP));
				for(int PPP=0;PPP<hithold->getFitN();PPP++){
					STIME+=hithold->getT0(PPP)/hithold->getT0err(PPP);
					STIMEERRINV+=(1.0/hithold->getT0err(PPP));
				}
			}
			STIME=STIME/STIMEERRINV;
			if(Clusters2_->at(minCoor)->getCharge()<Clusters2_->at(secCoor)->getCharge()){
			    if(NTD){MinComboChargeNTD_->Fill(Clusters2_->at(minCoor)->getCharge(),clu->getCharge());
		            if(clu->getCharge()-(Clusters2_->at(minCoor)->getCharge()+Clusters2_->at(minCoor)->getCharge())<.0000001){ChargeShared2D_->Fill(Clusters2_->at(secCoor)->getCharge(),Clusters2_->at(minCoor)->getCharge());}
			    DeltaTVsDeltaCharge_->Fill(Clusters2_->at(secCoor)->getTime()-Clusters2_->at(minCoor)->getTime(),clu->getCharge()-Clusters2_->at(minCoor)->getCharge());}
			    //HERE IS THE TIME CUT SHENANIGANS MIN COOR SMALLER
			    if(minCoor<secCoor){
			   	RawSvtHit* seedLeft = (RawSvtHit*)(Clusters2_->at(secCoor)->getRawHits().At(0));	
				//if(clu->getCharge()-(Clusters2_->at(minCoor)->getCharge()+Clusters2_->at(minCoor)->getCharge())<.0000001){
					DeltaTVsDeltaChargeV2_->Fill((MTIME-seedLeft->getT0(0))/seedLeft->getT0err(0),clu->getCharge()-Clusters2_->at(minCoor)->getCharge());
			    	//}
			    }else{
			    	RawSvtHit* seedLeft = (RawSvtHit*)(Clusters2_->at(minCoor)->getRawHits().At(0));	
				//if(clu->getCharge()-(Clusters2_->at(minCoor)->getCharge()+Clusters2_->at(minCoor)->getCharge())<.0000001){
					DeltaTVsDeltaChargeV2_->Fill((STIME-seedLeft->getT0(0))/seedLeft->getT0err(0),clu->getCharge()-Clusters2_->at(minCoor)->getCharge());
			    	//}
			    }
			}else{
			    if(NTD){MinComboChargeNTD_->Fill(Clusters2_->at(minCoor)->getCharge(),clu->getCharge());
		            if(clu->getCharge()-(Clusters2_->at(minCoor)->getCharge()+Clusters2_->at(minCoor)->getCharge())<.0000001){
				    ChargeShared2D_->Fill(Clusters2_->at(minCoor)->getCharge(),Clusters2_->at(secCoor)->getCharge());
			    }
			    DeltaTVsDeltaCharge_->Fill(Clusters2_->at(secCoor)->getTime()-Clusters2_->at(minCoor)->getTime(),clu->getCharge()-Clusters2_->at(secCoor)->getCharge());}
			 //HERE IS THE TIME CUT SHENANIGANS SEC CORR SMALLER
			    if(minCoor<secCoor){
			   	RawSvtHit* seedLeft = (RawSvtHit*)(Clusters2_->at(secCoor)->getRawHits().At(0));		
				//if(clu->getCharge()-(Clusters2_->at(minCoor)->getCharge()+Clusters2_->at(minCoor)->getCharge())<.0000001){
					DeltaTVsDeltaChargeV2_->Fill((MTIME-seedLeft->getT0(0))/seedLeft->getT0err(0),clu->getCharge()-Clusters2_->at(secCoor)->getCharge());
			    	//}
		            }else{
			    	RawSvtHit* seedLeft = (RawSvtHit*)(Clusters2_->at(minCoor)->getRawHits().At(0));	
				//if(clu->getCharge()-(Clusters2_->at(minCoor)->getCharge()+Clusters2_->at(minCoor)->getCharge())<.0000001){
					DeltaTVsDeltaChargeV2_->Fill((STIME-seedLeft->getT0(0))/seedLeft->getT0err(0),clu->getCharge()-Clusters2_->at(secCoor)->getCharge());
			        //}
			    }

			}
		    }
		    //std::cout<<"DINDADIDDLYDO"<<std::endl;
		    if(NTD){DifferenceInPositionVsChargeNTD_->Fill(minDist,minCharge);}
		}
	    } 
	    
	    
            if(doingTracks_){
                bool isShared = false;int increment = 0;
                for(int i = 0;i<size;i++){
                    Track* track;     
	            if(ident_<1.5){
			    track = tracks1_->at(i);
			    HitsOnTrack1_->Fill(track->getSvtHits().GetEntries());
		    }else{
			    track=tracks2_->at(i);
			    HitsOnTrack2_->Fill(track->getSvtHits().GetEntries());
		    }
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
                    if(((LAYER==layer_)&&(MODULE==module_))||(general)){
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
            
            bool general = ((layer_==-1)||(module_==-1));
            if(((LAYER==layer_)&&(MODULE==module_))||(general)){
                //NOW IS THE PART WHERE I FILL THE CLUSTER DISTANCE HISTOGRAM
                float Dist=69420;
                for(int p = 0; p < size2; p++){ 
                    if(p==i){continue;}
	            TrackerHit * clu2;
	    	    if(ident_<1.5){clu2 = Clusters1_->at(p);}else{clu2=Clusters2_->at(p);}
                    RawSvtHit * seed2 = (RawSvtHit*)(clu2->getRawHits().At(0));
                    float LAYER2=clu->getLayer();
                    float MODULE2=seed->getModule();
                    if((not(LAYER2==LAYER))or(not(MODULE2==MODULE))){continue;}
                    float dist = ((float)(seed2->getStrip()))-seedStrip;
                    if(dist<0){dist*=-1.0;}
                    if(dist<Dist){Dist=dist;}
                }
                if(Dist<69420){
                    if(ident_<1.5){ClusDistances1_->Fill(Dist);}else{/*std::cout<<"1 "<<Dist<<std::endl;*/ClusDistances2_->Fill(Dist);}
                    if(NTD){if(ident_<1.5){ClusDistancesNTD1_->Fill(Dist);}else{ClusDistancesNTD2_->Fill(Dist);}}
                }
                //std::cout<<"HELLO"<<std::endl;
                if(ident_<1.5){layers1_->Fill(nLayers);}else{layers2_->Fill(nLayers);}
                if(ident_<1.5){charges1_->Fill(ncharges);}else{charges2_->Fill(ncharges);}
                if(ident_<1.5){positions1_->Fill(clu->getLayer());}else{/*std::cout<<"2"<<std::endl;*/positions2_->Fill(clu->getLayer());}
                if(ident_<1.5){times1_->Fill(ntimes);}else{times2_->Fill(ntimes);}
                if(onTrk){
                    if(ident_<1.5){layersOnTrk1_->Fill(nLayers);}else{layersOnTrk2_->Fill(nLayers);}
                    if(ident_<1.5){chargesOnTrk1_->Fill(ncharges);}else{chargesOnTrk2_->Fill(ncharges);}
                    if(ident_<1.5){timesOnTrk1_->Fill(ntimes);}else{timesOnTrk2_->Fill(ntimes);} 
                    if(ident_<1.5){positionsOnTrk1_->Fill(clu->getLayer());}else{positionsOnTrk2_->Fill(clu->getLayer());}//std::cout<<"3"<<std::endl;
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
    }
    return;
}


void RoryClusterCompareAnaProcessor::fillDeads(){
    for(int i = 0;i<24576;i++){
        Deads_[i]=0.0;
    }
    std::string FILENAME="/fs/ddn/sdf/group/hps/users/rodwyer1/run/cluster_study/badchannels2021.dat";
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

int RoryClusterCompareAnaProcessor::GetStrip(int feb,int hyb,int strip){
	int BigCount = 0;
    if(feb<=1){
        BigCount+=feb*2048+hyb*512+strip;
    }else{
        BigCount+=4096;
        BigCount+=(feb-2)*2560+hyb*640+strip;
    }
    return BigCount;
}

void RoryClusterCompareAnaProcessor::Plot1(){
    std::cout<<"I AM IN THE FINAL STEP"<<std::endl; 
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
    //std::cout<<"I AM IN THE FINAL STEP 2"<<std::endl; 
    //std::cout<<layers_->GetEntries()<<std::endl;
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

void RoryClusterCompareAnaProcessor::Plot2(){
    TCanvas *c1 = new TCanvas("c");
    //gPad->SetLogy(true);
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

void RoryClusterCompareAnaProcessor::Plot3(){
    std::cout<<"I AM IN THE FINAL STEP"<<std::endl; 
    TCanvas *c1 = new TCanvas("c");
    //gPad->SetLogy(true);
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

void RoryClusterCompareAnaProcessor::Plot4(){
    std::cout<<"I AM IN THE FINAL STEP"<<std::endl; 
    TCanvas *c1 = new TCanvas("c");
    //gPad->SetLogy(true);
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

void RoryClusterCompareAnaProcessor::TrackPlot1(){
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

void RoryClusterCompareAnaProcessor::TrackPlot2(){
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

void RoryClusterCompareAnaProcessor::PlotShared1() {
    TCanvas *c1 = new TCanvas("c");
    c1->cd();
    gPad->SetLogy(true); 
    
    DifferenceInPositions_->GetXaxis()->SetTitle("Closest Cluster Position");
    DifferenceInCharges_->GetXaxis()->SetTitle("Closest Cluster Charge");
    DifferenceInPositions_->GetYaxis()->SetTitle("No Hits");
    DifferenceInCharges_->GetYaxis()->SetTitle("No Hits");

    DifferenceInPositions_->SetLineColor(kRed);
    DifferenceInCharges_->SetLineColor(kRed);
    
    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("Closest Associated Hit Position");
    DifferenceInPositions_->Draw("e");
    c1->SaveAs("PositionDifference.png");     
    c1->Clear();

    c1->DrawFrame(0.0,3000.0,150.0,7000.0);
    c1->SetTitle("Closest Associated Hit Charge");
    DifferenceInCharges_->Draw("e");
    c1->SaveAs("ChargeDifference.png");     
    c1->Clear();
    
    return;
}
void RoryClusterCompareAnaProcessor::WriteRoot(){
    TFile *outputFile;
    outputFile = new TFile("picHistos.root","RECREATE");
   
    layers1_->Write();
    layersOnTrk1_->Write();
    layersOffTrk1_->Write();
    charges1_->Write();
    chargesOnTrk1_->Write();
    chargesOffTrk1_->Write();
    charges2D_->Write();

    layersNTD1_->Write();
    layersOnTrkNTD1_->Write();
    layersOffTrkNTD1_->Write();
    chargesNTD1_->Write();
    chargesOnTrkNTD1_->Write();
    chargesOffTrkNTD1_->Write();

    positions1_->Write();
    positionsOnTrk1_->Write();
    ClusDistances1_->Write();
    ClusDistancesNTD1_->Write();

    times1_->Write();
    timesOnTrk1_->Write();
    timesOffTrk1_->Write();
    timesNTD1_->Write();
    timesOnTrkNTD1_->Write();
    timesOffTrkNTD1_->Write();

    //FOR THE SECOND FILE
        
    layers2_->Write();
    layersOnTrk2_->Write();
    layersOffTrk2_->Write();
    charges2_->Write();
    chargesOnTrk2_->Write();
    chargesOffTrk2_->Write();

    layersNTD2_->Write();
    layersOnTrkNTD2_->Write();
    layersOffTrkNTD2_->Write();
    chargesNTD2_->Write();
    chargesOnTrkNTD2_->Write();
    chargesOffTrkNTD2_->Write();

    positions2_->Write();
    positionsOnTrk2_->Write();
    ClusDistances2_->Write();
    ClusDistancesNTD2_->Write();

    times2_->Write();
    timesOnTrk2_->Write();
    timesOffTrk2_->Write();
    timesNTD2_->Write();
    timesOnTrkNTD2_->Write();
    timesOffTrkNTD2_->Write();

    //SHARED DISTRIBUTIONS
    
    DifferenceInPositions_->Write();
    DifferenceInCharges_->Write();
    DifferenceInPositionsVStripNo_->Write();    
    DifferenceInPositionVsCharge_->Write();
    DifferenceInPositionVsChargeNTD_->Write();
    MinComboPosition0_->Write();
    MinComboPosition1_->Write();
    MinComboPosition2_->Write();
    MinComboChargeNTD_->Write();
    DeltaTVsDeltaCharge_->Write(); 
    DeltaTVsDeltaChargeV2_->Write(); 
    ChargeShared2D_->Write();

    //TRACKING RELATED VARIABLES

    Z0VNShare2Hist1_->Write();
    Z0VNShare2HistCut1_->Write();
    SharedAmplitudes1_->Write();
    UnSharedAmplitudes1_->Write();
    SharedTimes1_->Write();
    UnSharedTimes1_->Write();
    TrackMomentumInTime1_->Write();
    TrackMomentumOutTime1_->Write();
    TrackMomentumAllTime1_->Write();
    TrackMomentumTInTime1_->Write();
    TrackMomentumTOutTime1_->Write();
    TrackMomentumTAllTime1_->Write();
    HitsOnTrack1_->SetLineColor(kRed);
    HitsOnTrack1_->Write();

    Z0VNShare2Hist2_->Write();
    Z0VNShare2HistCut2_->Write();
    SharedAmplitudes2_->Write();
    UnSharedAmplitudes2_->Write();
    SharedTimes2_->Write();
    UnSharedTimes2_->Write();
    TrackMomentumInTime2_->Write();
    TrackMomentumOutTime2_->Write();
    TrackMomentumAllTime2_->Write();
    TrackMomentumTInTime2_->Write();
    TrackMomentumTOutTime2_->Write();
    TrackMomentumTAllTime2_->Write();
    HitsOnTrack2_->SetLineColor(kBlue);
    HitsOnTrack2_->Write();

    outputFile->Close();
    return;
}
void RoryClusterCompareAnaProcessor::finalize() {
    Plot1();  
    Plot2();
    Plot3();
    Plot4();
    if(doingTracks_){
        TrackPlot1();
        TrackPlot2(); 
    }
    PlotShared1();
    WriteRoot();
    //ClusterFit();
    return;
}
DECLARE_PROCESSOR(RoryClusterCompareAnaProcessor);
