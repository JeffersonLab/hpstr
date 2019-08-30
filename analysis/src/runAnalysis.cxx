
#include "EventHeader.h"  //CONTAINS LCIO. I don't want that!
#include "Track.h"
#include "TFile.h"
#include "TTree.h"

#include "TBranch.h"

#include <iostream>

int main(int argc, char** argv) {


  TFile* inputFile = new TFile("/Users/Pierfrancesco/HPS/sw/hipster/run/testRun.root");
  TTree* t = (TTree*) inputFile->Get("HPS_Event");
  long nentries = t->GetEntriesFast();
  std::cout<<"n entries "<<nentries<<std::endl;

  TClonesArray *tracks = nullptr;
  TBranch* btracks = nullptr;
  t->SetBranchAddress("GBLTracks",&tracks,&btracks);

  for (int ientry=0; ientry<100;ientry++) {
    t->GetEntry(ientry);
    std::cout<<"Number of Tracks in the event:"<<tracks->GetEntries()<<std::endl;
    
    for (int itrack = 0; itrack<tracks->GetEntries();itrack++) {
      Track *track = (Track*)tracks->ConstructedAt(itrack);
      
      std::cout<<"Tracks number of hits for track "<<itrack<<" "<<track->getNSvtHits()<<std::endl;
      
    }
      
    
  }

  return 0;

    
  
  
}
