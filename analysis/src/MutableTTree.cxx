#include <MutableTTree.h>

MutableTTree::MutableTTree(TFile* infile){
   tree_ = (TTree*)infile->Get(tree_name.c_str()); 
}

MutableTTree::MutableTTree(TFile* infile, std::string tree_name){
   tree_ = infile->Get(tree_name.c_str()); 
   initializeFlatTuple(tree_, tuple_);
}

double MutableTTree::getValue(std::string branch_name){
    if(tuple_.find(branch_name) == tuple_.end()){
        std::cout << "[MutableTTree] ERROR: No branch named " << branch_name << " found inside TTree" << std::endl;
        return -9999.9;
    }
    else
        return tuple_[branch_name];
}

void MutableTTree::addBranch(std::branch_name){
    if(tuple_.find(branch_name) == tuple_.end()){
        std::cout << "Branch Already Exists in TTree" << std::endl;
        return;
    }

    double* value = new double;
    tuple_[branch_name] = value; 

    //Make new TTree. Copy branches from original. Add new branch. Fill new TTree with old TTree
    TTree *newtree = new TTree();
    newtree->Branch(branch_name.c_str(),&tuple_[branch_name],branch_name.c_str());  

    int nBr = tree_->GetListOfBranches()->GetEntries();
    for(int iBr = 0; iBr < nBr; iBr++){
        TBranch *br = dynamic_cast<TBranch*>(tree->GetListOfBranches()->At(iBr));
        std::string varname = (std::string)br->GetFullName();
        newtree->Branch(varname.c_str(),&tuple_[varname],varname.c_str());  
    }

    for(int e=0; e < tree_->GetEntries(); e++){
        tree_->GetEntry(e);
        newtree->Fill();
    }
    
    tree_ = &newtree;
    delete newtree;
}

void MutableTTree::initializeFlatTuple(TTree* tree, std::map<std::string, double*> &tuple_map){
    int nBr = tree->GetListOfBranches()->GetEntries();
    for(int iBr = 0; iBr < nBr; iBr++){
        TBranch *br = dynamic_cast<TBranch*>(tree->GetListOfBranches()->At(iBr));
        double* value = new double;
        std::string varname = (std::string)br->GetFullName();
        tuple_map[varname] = value;
        tree->SetBranchAddress(varname.c_str(),tuple_map[varname]);
    }
}


