/**
 * @file RoryClusterCompareAnaProcessor.cxx
 * @brief AnaProcessor used to compare two means of reconstruction directly.
 * @author Rory O'Dwyer and Cameron Bravo, SLAC National Accelerator Laboratory
 */     
#include "RoryClusterCompareSeedAnaProcessor.h"
//#include "Int_t.h"
#include <iostream>

RoryClusterCompareSeedAnaProcessor::RoryClusterCompareSeedAnaProcessor(const std::string& name, Process& process) : Processor(name,process){
    mmapper_ = new ModuleMapper(2021);
}
//TODO CHECK THIS DESTRUCTOR
RoryClusterCompareSeedAnaProcessor::~RoryClusterCompareSeedAnaProcessor(){}

void RoryClusterCompareSeedAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring RoryClusterCompareSeedAnaProcessor" << std::endl;
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


void RoryClusterCompareSeedAnaProcessor::initialize(std::string inFilename, std::string outFilename){
    fillDeads();   
    std::cout<<"DO I MAKE IT HERE1"<<std::endl;
    file1_ = new HpsEventFile(fileOne_, "hello1.root");
    file2_ = new HpsEventFile(fileTwo_, "hello2.root");
    std::cout<<"DO I MAKE IT HERE2"<<std::endl;
    file1_->setupEvent(&event1_);
    file2_->setupEvent(&event2_);
    std::cout<<"DO I MAKE IT HERE3"<<std::endl;
    tree1_= event1_.getTree();tree2_= event2_.getTree(); 
    std::cout<<"DO I MAKE IT HERE4 initialize"<<std::endl;
    
    helper_=false;
    countWeird_=0.0;

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


    TwoTimes_ = new TH2F("Time of First One v.s Time of Second One","Time of First One v.s Time of Second One",200,-100.0,100.0,200,-100.0,100.0);
    UnChangedTimes_ = new TH1F("UnChanged Hit Time","UnChanged Hit Time",200,-100.0,100.0);
    TwoCharges_ = new TH2F("Charge of First One v.s Charge of Second One","Charge of First One v.s Charge of Second One",100,-.000005,.00005,100,-.000005,.00005);
    UnChangedCharges_ = new TH1F("UnChanged Hit Charge","UnChanged Hit Charge",100,-.000005,.00005);
    SharpPlot_ = new TH1F("Distance from Cluster Time","Distance from Cluster Time",100,-20.0,20.0);
    DidItAdd_ = new TH2F("Charge Change vs. Distance of Potentially Added Hit","Charge Change vs. Distance of Potentially Added Hit",100,-5,5,100,0,.00005);

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
    std::cout<<"I GET HERE 2 DING"<<std::endl;
}


bool RoryClusterCompareSeedAnaProcessor::process() {
    int n_events_processed=0;
    helper_ = false;
    //std::cout<<"DID I MAKE IT TO PROCESS"<<std::endl;
    while((file1_->nextEvent())and(n_events_processed<4000000)) {
	file2_->nextEvent();
        if (n_events_processed%1000 == 0){std::cout<<"Event:"<<n_events_processed<<std::endl;}
	//if ((n_events_processed>8000)and(n_events_processed<9000)){helper_=true;std::cout<<n_events_processed<<std::endl;}
	//std::cout<<"HELLO LOOP 1"<<std::endl;
	loop(&event1_,&event2_);
	//std::cout<<"HELLO LOOP 2"<<std::endl;
	n_events_processed++;
    }
    return true;
}


void RoryClusterCompareSeedAnaProcessor::loop(IEvent *event1,IEvent *event2) {
    //std::cout<<"I ATLEAST MADE IT TO THE FUNCTION CALL"<<std::endl;
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
    	float COUNT=0.0;
	float DENOM=0.0;
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
              
            int channelL=seedStrip-1;
            int channelR=seedStrip+1;
            if(channelL>=0){
                NTD=(NTD)||(Deads_[GetStrip(feb,hyb,channelL)]==1);
            }
            if(((feb<=1)&&(channelR<=511))||((feb>1)&&(channelR<=639))){
                NTD=(NTD)||(Deads_[GetStrip(feb,hyb,channelR)]==1); 
            }    
	    if(ident_<1.5){
	    	//WE ARE GOING TO DO ALL OUR EVENT BY EVENT ANALYSIS HERE
		bool general = ((layer_==-1)||(module_==-1));
		if(((LAYER==layer_)&&(MODULE==module_))||(general)){
		    float minDist=10000000;
		    int minCoor=-1;
		    //std::vector<double> pos1 = clu->getPosition();
		    //ABSOLUTELY FORCES EQUALITY
		    for(int j = 0; j<Clusters2_->size(); j++){
			RawSvtHit * justHoldin = (RawSvtHit *)(Clusters2_->at(j)->getRawHits().At(0));
			if((not(LAYER==Clusters2_->at(j)->getLayer()))or(not(MODULE==justHoldin->getModule()))){continue;}
			if(justHoldin->getStrip()==seedStrip){
				minCoor=j;
				minDist=0;
			}	
		    }
		    if(not(minCoor==-1)){
			RawSvtHit * justHoldin = (RawSvtHit *)(Clusters2_->at(minCoor)->getRawHits().At(0));    
			if(not(abs(clu->getTime()-Clusters2_->at(minCoor)->getTime())==0.0)){
			    TwoTimes_->Fill(clu->getTime(),Clusters2_->at(minCoor)->getTime());
			}else{
			    UnChangedTimes_->Fill(clu->getTime());
		    	}
			if(not(abs(clu->getCharge()-Clusters2_->at(minCoor)->getCharge())==0.0)){
			    if(Clusters2_->at(minCoor)->getCharge()<clu->getCharge()){
				    countWeird_+=1.0;std::cout<<countWeird_<<std::endl;
				    int End=clu->getNHits();
				    if(Clusters2_->at(minCoor)->getNHits()>End){
				    	End=Clusters2_->at(minCoor)->getNHits();
				    }
				    for(int II=0;II<End;II++){
					float T1=-1.0;
					float T2=-1.0;
					float T1err=-1.0;
					float T2err=-1.0;
					float strip1=-1.0;
					float strip2=-1.0;
				    	if(II<clu->getNHits()){
						RawSvtHit * rawhit1 = (RawSvtHit *)(clu->getRawHits().At(II));
						T1=rawhit1->getT0(0);
						T1err=rawhit1->getT0err(0);
						strip1=(float)(rawhit1->getStrip());
					}
					if(II<Clusters2_->at(minCoor)->getNHits()){
						RawSvtHit * rawhit2 = (RawSvtHit *)(Clusters2_->at(minCoor)->getRawHits().At(II));
						T2=rawhit2->getT0(0);
						T2err=rawhit2->getT0err(0);
						strip2=(float)(rawhit2->getStrip());
					}
				    	//if(Deads_[GetStrip(feb,hyb,channelR)]==1){
						std::cout<<T1<<" "<<T1err<<" "<<strip1<<" "<<T2<<" "<<T2err<<" "<<strip2<<"\n"<<std::endl;
					//}
					//if(Deads_[GetStrip(feb,hyb,channelL)]==1){
					//	std::cout<<T1<<" "<<T1err<<" "<<strip1<<" "<<T2<<" "<<T2err<<" "<<strip2<<" "<<channelL<<"\n"<<std::endl;	
					//}
				    }
			    }
			    TwoCharges_->Fill(clu->getCharge(),Clusters2_->at(minCoor)->getCharge());
			}else{
			    UnChangedCharges_->Fill(clu->getCharge());
		    	}

		    }
		    //if(helper_){
		    //DENOM+=1.0;
		    if(minCoor>-1){
			//COUNT+=1.0;
			float chargeSub = 0.0;
		        if(NTD){
				//std::cout<<"GOT HERE 1"<<std::endl;
				float cltime=-1000.0;
				if(Clusters2_->at(minCoor)->getNHits()>1){
					//std::cout<<"GOT HERE 2"<<std::endl;
					RawSvtHit * cl2 = (RawSvtHit *)(Clusters2_->at(minCoor)->getRawHits().At(1));
					cltime=(seed->getT0(0)/seed->getT0err(0)+cl2->getT0(0)/cl2->getT0err(0))/(1.0/seed->getT0err(0)+1.0/cl2->getT0err(0));
				}else{
					//std::cout<<"GOT HERE 3"<<std::endl;
					cltime=seed->getT0(0);
				}
				//std::cout<<"GOT HERE 4"<<std::endl;
				chargeSub = (Clusters2_->at(minCoor)->getCharge()-clu->getCharge());
				//std::cout<<"GOT HERE 5"<<std::endl;
				//for(int indexer=0;indexer<Clusters2_->at(minCoor)->getNHits();indexer++){
				if(clu->getNHits()<Clusters2_->at(minCoor)->getNHits()){
					//std::cout<<"HELLO I GOT TO NHITS"<<std::endl;
					RawSvtHit * deadCandidate = (RawSvtHit *)(Clusters2_->at(minCoor)->getRawHits().At(clu->getNHits()));
					DidItAdd_->Fill((cltime-deadCandidate->getT0(0))/deadCandidate->getT0err(0),chargeSub);
				}
				//if(cond1){std::cout<<"PASSED 1"<<std::endl;}	
				//bool cond1 = (cond1)or((deadCandidate->getStrip()==seedStrip-2)and(Deads_[GetStrip(feb,hyb,channelL)]==1));
				//if(cond1){std::cout<<"PASSED 2"<<std::endl;}	
				//if(cond1){DidItAdd_->Fill((cltime-deadCandidate->getT0(0))/deadCandidate->getT0err(0),chargeSub);}
				//cond1 = (cond1)and((LAYER==Clusters1_->at(indexer)->getLayer())and(MODULE=deadCandidate->getModule()));
					
				//}
				//std::cout<<chargeSub<<std::endl;
				/*for(int indexer=0;indexer<Clusters1_->size();indexer++){
					//std::cout<<"GOT HERE 5"<<std::endl;
					RawSvtHit * deadCandidate = (RawSvtHit *)(Clusters1_->at(indexer)->getRawHits().At(0));
					bool cond1 = (deadCandidate->getStrip()==seedStrip+2)and(Deads_[GetStrip(feb,hyb,channelR)]==1);
					if(cond1){std::cout<<"PASSED 1"<<std::endl;}	
					cond1 = (cond1)or((deadCandidate->getStrip()==seedStrip-2)and(Deads_[GetStrip(feb,hyb,channelL)]==1));
					if(cond1){std::cout<<"PASSED 2"<<std::endl;}	
					cond1 = (cond1)and((LAYER==Clusters1_->at(indexer)->getLayer())and(MODULE=deadCandidate->getModule()));
					if(cond1){
						std::cout<<"GOT HERE 6"<<std::endl;
					}
				}*/
			}	    
		        for(int G=0;G<Clusters2_->at(minCoor)->getNHits();G++){
			    //std::cout<<"\n"<<std::endl;
			    try{
			        RawSvtHit * justHoldin = (RawSvtHit *)(Clusters2_->at(minCoor)->getRawHits().At(G));  
				if((not(LAYER==Clusters2_->at(minCoor)->getLayer()))or(not(MODULE==justHoldin->getModule()))){continue;} 
				//THIS LINE BELOW FINDS OUT IF YOUR CLUSTER2 MATCH CONTAINS THE ACROSS DEAD CHANNEL	    
			 	//if(justHoldin->getStrip()==seedStrip-2){
				//if(Clusters2_->at(minCoor)->getNHits()>1){std::cout<<justHoldin->getStrip()<<" "<<channelL<<std::endl;}
				float timeHold = 0.0;
				float timeErr = 0.0;
				//std::cout<<"DID I GET HERE HELPER 3"<<std::endl;
				//THIS LINE HERE SUMS OVER ALL THE HIT TIMES THE OLD CLUSTER TO CHECK IF YOU'D BE ADDED
				for(int GG=0;GG<G;GG++){
			        try{
				    RawSvtHit * justHoldin2 = (RawSvtHit *)(Clusters2_->at(minCoor)->getRawHits().At(GG));
				    if((not(LAYER==Clusters2_->at(minCoor)->getLayer()))or(not(MODULE==justHoldin2->getModule()))){continue;} 
				    timeHold+=justHoldin2->getT0(0)/justHoldin2->getT0err(0);
				    timeErr+=1.0/justHoldin2->getT0err(0);	
				    }catch(...){
					std::cout<<"There was an encased exception"<<std::endl;
					std::cout<<Clusters2_->at(minCoor)->getNHits()<<std::endl;
					std::cout<<GG<<std::endl;
				    }
				}
				timeHold/=timeErr;
				//std::cout<<"I actually did fill this histogram"<<std::endl;
				SharpPlot_->Fill((timeHold-justHoldin->getT0(0))/(justHoldin->getT0err(0)));
			        //}
			    }catch(...){
				std::cout<<"There was an exception"<<std::endl;
				std::cout<<Clusters2_->at(minCoor)->getNHits()<<std::endl;
				std::cout<<G<<std::endl;
			    }
			}
		    }
		}
	    } 
	    
            if(doingTracks_){
                bool isShared = false;int increment = 0;
                for(int i = 0;i<size;i++){
                    Track* track;     
	            if(ident_<1.5){track = tracks1_->at(i);}else{track=tracks2_->at(i);}
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
        }
    	//if(ident_<1.5){std::cout<<"THE AMOUNT OF MATCHING CLUSTERS IS "<<COUNT/DENOM<<std::endl;}
    }
    return;
}
    //THIS LINE HERE SUMS OVER ALL THE HIT TIMES THE OLD CLUSTER TO CHECK IF YOU'D BE ADDED
				    /*for(int GG=0;GG<clu->getNHits();GG++){
			   		try{
					    RawSvtHit * justHoldin2 = (RawSvtHit *)(clu->getRawHits().At(GG));
					    timeHold+=justHoldin2->getT0(0);
					    timeErr+=justHoldin2->getT0err(0);	
				        }catch(...){
					    std::cout<<"There was an encased exception"<<std::endl;
					    std::cout<<clu->getNHits()<<std::endl;
					    std::cout<<GG<<std::endl;
					}
				    }*/
				    //Now INSTEAD WE SUM OVER SAME HITS.
//std::cout<<"DID I GET HERE HELPER 2"<<std::endl;
			/*if(Clusters2_->at(minCoor)->getNHits()>4){
			    float cluTime = 0.0;
			    float cluErr = 0.0;
		            for(int G=0;G<Clusters2_->at(minCoor)->getNHits();G++){
			    	RawSvtHit * printer = (RawSvtHit *)(Clusters2_->at(minCoor)->getRawHits().At(G));
				cluTime+=printer->getT0(0)/printer->getT0err(0);
				cluErr+=1/(printer->getT0err(0));
				std::cout<<printer->getStrip()<<", Hit Time: "<<printer->getT0(0)<<", Hit Error: "<<printer->getT0err(0)<<", Cluster Time: "<<cluTime/cluErr<<std::endl;
			    }
			    std::cout<<"\n"<<std::endl;
			}*/
/*bool general = ((layer_==-1)||(module_==-1));
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
                    if(ident_<1.5){ClusDistances1_->Fill(Dist);}else{std::cout<<"1 "<<Dist<<std::endl;ClusDistances2_->Fill(Dist);}
                    if(NTD){if(ident_<1.5){ClusDistancesNTD1_->Fill(Dist);}else{ClusDistancesNTD2_->Fill(Dist);}}
                }
                //std::cout<<"HELLO"<<std::endl;
                if(ident_<1.5){layers1_->Fill(nLayers);}else{layers2_->Fill(nLayers);}
                if(ident_<1.5){charges1_->Fill(ncharges);}else{charges2_->Fill(ncharges);}
                if(ident_<1.5){positions1_->Fill(clu->getLayer());}else{std::cout<<"2"<<std::endl;positions2_->Fill(clu->getLayer());}
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
*/

void RoryClusterCompareSeedAnaProcessor::fillDeads(){
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

int RoryClusterCompareSeedAnaProcessor::GetStrip(int feb,int hyb,int strip){
	int BigCount = 0;
    if(feb<=1){
        BigCount+=feb*2048+hyb*512+strip;
    }else{
        BigCount+=4096;
        BigCount+=(feb-2)*2560+hyb*640+strip;
    }
    return BigCount;
}

void RoryClusterCompareSeedAnaProcessor::WriteRoot(){
    TFile *outputFile;
    outputFile = new TFile("picHistos.root","RECREATE");
   
    /*layers1_->Write();
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
    */

    TwoTimes_->Write();
    UnChangedTimes_->Write();
    TwoCharges_->Write();
    UnChangedCharges_->Write(); 
    SharpPlot_->Write();
    DidItAdd_->Write();
    
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

    outputFile->Close();
    return;
}
void RoryClusterCompareSeedAnaProcessor::finalize() {
    WriteRoot();
    //ClusterFit();
    return;
}
DECLARE_PROCESSOR(RoryClusterCompareSeedAnaProcessor);
