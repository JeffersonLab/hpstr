void AddIdentity(){
    TFile *f = new TFile("rootcopy0.root","update");
    TTree *tree = (TTree *)(f->Get("HPS_Event"));
    Float_t helper = 1.0;
    auto my_new_branch = tree->Branch("Identifier",&helper,"Identifier/F");
    for (Long64_t entry = 0 ; entry < tree->GetEntries() ; entry++ ) {
        //tree->GetEntry();
        /* something to compute my_local_variable */
        std::cout<<entry<<std::endl;
        my_new_branch->Fill();
    }
    tree->Write("",TObject::kOverwrite);
}
