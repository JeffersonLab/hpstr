#include "ClusterOnTrackProcessor.h"
#include "TBranch.h"

ClusterOnTrackProcessor::ClusterOnTrackProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
ClusterOnTrackProcessor::~ClusterOnTrackProcessor(){}

void ClusterOnTrackProcessor::initialize(TTree* tree) {
  clusterHistos = new ClusterHistos("default_");
  tree_=tree;
  //TODO Change this.
  tree_->SetBranchAddress("GBLTracks",&tracks_,&btracks_);
  
}

void ClusterOnTrackProcessor::process(IEvent* ievent) {


  for (int itrack = 0; itrack<tracks_->GetEntries();itrack++) {
    Track *track = (Track*)tracks_->ConstructedAt(itrack);
    
    std::cout<<"Tracks number of hits for track "<<itrack<<" "<<track->getNSvtHits()<<std::endl;
  }
  
}


void ClusterOnTrackProcessor::finalize() {
  
}


DECLARE_PROCESSOR(ClusterOnTrackProcessor);
