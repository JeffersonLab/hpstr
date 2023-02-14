#include <MutableTTree.h>

MutableTTree::MutableTTree(TFile* infile, std::string tree_name){
   tree_ = (TTree*)infile->Get(tree_name.c_str()); 
   initializeFlatTuple(tree_, tuple_);
}

double MutableTTree::getValue(std::string branch_name){
    if(tuple_.find(branch_name) == tuple_.end()){
        std::cout << "[MutableTTree] ERROR: No branch named " << branch_name << " found inside TTree" << std::endl;
        return -9999.9;
    }
    else
        return *tuple_[branch_name];
}

//void MutableTTree::fillNewBranch(std::string branch_name, double &value_reference){
//    TBranch* br = (TBranch*)tree_->GetBranch(branch_name.c_str());
//    tuple_[branch_name] = 
//}

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

/*
void MutableTTree::addBranch(std::string branch_name){
    if(tuple_.find(branch_name) != tuple_.end()){
        std::cout << "Branch Already Exists in TTree" << std::endl;
        return;
    }

    double value;
    writeTuple_[branch_name] = value; 

    //Make new TTree. Copy branches from original. Add new branch. Fill new TTree with old TTree
    TTree *newtree = new TTree();
    newtree->Branch(branch_name.c_str(),&writeTuple_[branch_name],(branch_name+"/D").c_str());  

    std::cout << "Copying Branches from original TTree" << std::endl;
    int nBr = tree_->GetListOfBranches()->GetEntries();
    for(int iBr = 0; iBr < nBr; iBr++){
        TBranch *br = dynamic_cast<TBranch*>(tree_->GetListOfBranches()->At(iBr));
        std::string varname = (std::string)br->GetFullName();
        newtree->Branch(varname.c_str(),&writeTuple_[varname],(varname+"/D").c_str());  
    }

    std::cout << "################## COPY OF TREE" << std::endl;
    for(int e=0; e < 5; e++){
        tree_->GetEntry(e);
        std::cout << "Printing Event " << e << std::endl;
        printEvent();
        std::cout << "Writing original TTree to New TTree" << std::endl;
        for(std::map<std::string,double*>::iterator it=tuple_.begin(); it != tuple_.end(); it++){
            writeTuple_[it->first] = *tuple_[it->first];
        }
        newtree->Fill();
    }

    initializeFlatTuple(newtree, readTuple_);
    newtree->SetBranchAddress(branch_name.c_str(),readTuple_[branch_name]);

    //Print values in the new tree
    for(int e=0; e < 5; e++){
        newtree->GetEntry(e);
        std::cout << "Printing Event " << e << std::endl;
        for(std::map<std::string,double*>::iterator it = readTuple_.begin(); it != readTuple_.end(); it ++){
            std::cout << it->first << ": " << *it->second << std::endl;
        }
    }

    //Need to somehow make tree_ refer to this new tree, and the tuple_ refer to this new readTuple_
    std::cout << "Converting original Tree to the New Tree" << std::endl;
    delete tree_;
    tree_ = newtree;
    tuple_ = readTuple_;
    initializeFlatTuple(tree_,tuple_); 
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


