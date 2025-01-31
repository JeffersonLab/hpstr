#include "NTupplizer.h"
#include <iomanip>
#include <set>
#include "utilities.h"

NTupplizer::NTupplizer(const std::string& name, Process& process)
	: Processor(name, process) { 
	mmapper_ = new ModuleMapper();
	}

NTupplizer::~NTupplizer() { 
}

void NTupplizer::configure(const ParameterSet& parameters) {

	std::cout << "Configuring TrackPerformance" << std::endl;
	try
	{
		debug_                = parameters.getInteger("debug",debug_);
		trkCollName_          = parameters.getString("trkCollName",trkCollName_);
		fspCollName_	      = parameters.getString("fspCollName",fspCollName_);
		eclusCollName_	      = parameters.getString("eclusCollName",eclusCollName_);
		sclusCollName_	      = parameters.getString("sclusCollName",sclusCollName_);
		baselineFile_ 	      = parameters.getString("baselineFile");
        	timeProfiles_ 	      = parameters.getString("timeProfiles");
		outPutCsv_	      = parameters.getInteger("outPutCsv");
	}
	catch (std::runtime_error& error)
	{
		std::cout << error.what() << std::endl;
	}
}

void NTupplizer::initialize(TTree* tree) {
	if(outPutCsv_==1){
		csvFile_=std::ofstream("outputCSV.csv");
		csvFile_<<"EventNum,TrackNo";
		for(int i = 0; i<12;i++){
			for(int j = 0; j<14;j++){
				csvFile_<<",track_"+std::to_string(i)+"_lay_"+std::to_string(j)+"_lambkink";
				csvFile_<<",track_"+std::to_string(i)+"_lay_"+std::to_string(j)+"_phikink";
			}
		}
		csvFile_<<"\n";
	}
	std::cout<<"I GOT HERE 3"<<std::endl;
	tree_= tree;
	tree_->SetBranchAddress(trkCollName_.c_str(),&tracks_, &btracks_); 
	tree_->SetBranchAddress(fspCollName_.c_str(),&fsps_, &bfsps_);
	tree_->SetBranchAddress(eclusCollName_.c_str(),&eclusters_,&beclusters_);
	tree_->SetBranchAddress(sclusCollName_.c_str(),&sclusters_,&bsclusters_);
    	tree_->SetBranchAddress("EventHeader",&evH_,&bevH_);
	Tracks_ = new TH1F("Tracks","Tracks",10,0,10);
	for(int i=0;i<12;i++){
		for(int j=0;j<14;j++){
			std::string title1="track_"+std::to_string(i)+"_layer_"+std::to_string(j)+"_lamdba";
			std::string title2="track_"+std::to_string(i)+"_layer_"+std::to_string(j)+"_phi";
			Track_Lambda[i][j]=new TH1F(title1.c_str(),"Tracks",100,-1.0,1.0);
			Track_Phi[i][j]=new TH1F(title2.c_str(),"Tracks",100,-1.0,1.0);
		}
	}	
	return;
}

bool NTupplizer::process(IEvent* ievent) {
	int STRIPNUM = 0;
	int STRIP = -1;
	long eventTime = evH_->getEventTime();
	int eventNum = evH_->getEventNumber();
    	int trigPhase = (int)((eventTime%24)/4);
	//std::cout<<"I GOT HERE 1"<<std::endl;
	//HERE IS WHERE WE PUT ALL THE CSV TITLES. FIRST UP IS ALL THE LAMBDA KINKS FOR 12 TRACKS IN ORDER
	std::string helper=std::to_string(eventNum)+","+std::to_string(tracks_->size());
	Tracks_->Fill(tracks_->size());
	//I AM GOING TO ORDER THE TRACKS THEIR ORDER; ONE MAY USE ERROR TO DO A DIFFERENT ORDERING, AND GOING TO CAP AT 12
	for (int itrack = 0; itrack < 12; ++itrack) {
		Track* track{nullptr};	
		if(itrack<tracks_->size()){	
			track = tracks_->at(itrack);
			for(int j=0;j<14;j++){
				helper+=","+std::to_string(track->getLambdaKink(j));
				helper+=","+std::to_string(track->getPhiKink(j));
				Track_Lambda[itrack][j]->Fill(track->getLambdaKink(j));	
				Track_Phi[itrack][j]->Fill(track->getPhiKink(j));	
			}
		}else{
			for(int j=0;j<14;j++){
				helper+=","+std::to_string(-1000.0);
				helper+=","+std::to_string(-1000.0);
				Track_Lambda[itrack][j]->Fill(-1000.0);	
				Track_Phi[itrack][j]->Fill(-1000.0);	
			}	
		}		
	}
	helper+="\n";
	if(outPutCsv_=1){
		csvFile_<<helper;
	}
	return true;
}


float NTupplizer::str_to_float(std::string token){
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

void NTupplizer::WriteRoot() { 
	if(outPutCsv_==1){
		csvFile_.close();
	}
	if(outPutCsv_==0){
		TFile *outputFile;
		outputFile = new TFile("outputRoot.root","RECREATE");
		Tracks_->Write();
		for(int i=0;i<12;i++){
			for(int j=0;j<14;j++){
				Track_Lambda[i][j]->Write();
				Track_Phi[i][j]->Write();
			}
		}
		outputFile->Close();
	}
	return;
}

void NTupplizer::finalize() { 
	WriteRoot();
	return;
}

DECLARE_PROCESSOR(NTupplizer); 
