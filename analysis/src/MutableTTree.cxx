#include <MutableTTree.h>

MutableTTree::MutableTTree(TFile* infile, std::string tree_name){
   tree_ = (TTree*)infile->Get(tree_name.c_str()); 
   initializeFlatTuple(tree_, tuple_);
   copyTTree();
}

double MutableTTree::getValue(std::string branch_name){
    if(tuple_.find(branch_name) == tuple_.end()){
        std::cout << "[MutableTTree] ERROR: No branch named " << branch_name << " found inside TTree" << std::endl;
        return -9999.9;
    }
    else
        return *tuple_[branch_name];
}

void MutableTTree::copyTTree(){

    //Make new TTree. Copy branches from original. Add new branch. Fill new TTree with old TTree
    newtree_ = new TTree();
    int nBr = tree_->GetListOfBranches()->GetEntries();
    for(int iBr = 0; iBr < nBr; iBr++){
        TBranch *br = dynamic_cast<TBranch*>(tree_->GetListOfBranches()->At(iBr));
        std::string varname = (std::string)br->GetFullName();
        newtree_->Branch(varname.c_str(),tuple_[varname],(varname+"/D").c_str());  
    }

    for(int e=0; e < tree_->GetEntries(); e++){
        tree_->GetEntry(e);
        newtree_->Fill();
    }

    delete tree_;
    tree_ = newtree_;

}

void MutableTTree::addNewBranch(std::string branch){
    double* value = new double{999.9};
    tuple_[branch] = value;
    tree_->Branch(branch.c_str(),tuple_[branch],(branch+"/D").c_str());  
    TBranch* br = (TBranch*)tree_->GetBranch(branch.c_str());
    new_branches[branch] = br;
}

void MutableTTree::fillNewBranch(std::string branch, double value){
    setBranchValue(branch, value);
    new_branches[branch]->Fill();
}

/*
void MutableTTree::addBranch(std::string branch_name){
    if(tuple_.find(branch_name) != tuple_.end()){
        std::cout << "Branch Already Exists in TTree" << std::endl;
        return;
    }

    //Make new TTree. Copy branches from original. Add new branch. Fill new TTree with old TTree
    newtree_ = new TTree();
    double* value = new double{999.9};
    tuple_[branch_name] = value;
    newtree_->Branch(branch_name.c_str(),tuple_[branch_name],(branch_name+"/D").c_str());  

    std::cout << "Copying Branches from original TTree" << std::endl;
    int nBr = tree_->GetListOfBranches()->GetEntries();
    for(int iBr = 0; iBr < nBr; iBr++){
        TBranch *br = dynamic_cast<TBranch*>(tree_->GetListOfBranches()->At(iBr));
        std::string varname = (std::string)br->GetFullName();
        newtree_->Branch(varname.c_str(),tuple_[varname],(varname+"/D").c_str());  
    }

    for(int e=0; e < 5; e++){
        tree_->GetEntry(e);
        newtree_->Fill();
    }

    delete tree_;
    tree_ = newtree_;
}
*/

void MutableTTree::printEvent(){
    for(std::map<std::string,double*>::iterator it = tuple_.begin(); it != tuple_.end(); it ++){
        std::cout << it->first << ": " << *it->second << std::endl;
    }
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


