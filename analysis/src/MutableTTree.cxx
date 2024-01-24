#include <MutableTTree.h>

MutableTTree::MutableTTree(TFile* infile, std::string tree_name){
   std::cout << "[MutableTTree]::Reading in tree: " << tree_name << std::endl;
   tree_ = (TTree*)infile->Get(tree_name.c_str()); 
   if(tree_ == nullptr)
       std::cout << "[MutableTTree]::ERROR READING TREE " << tree_name << " from file " << std::endl;
   initializeFlatTuple(tree_, tuple_);
   newtree_ = new TTree();
   copyTTree();
}

double MutableTTree::getValue(std::string branch_name){
    if(tuple_.find(branch_name) == tuple_.end()){
        return -9999.9;
    }
    else
        return *tuple_[branch_name];
}

void MutableTTree::copyTTree(){
    //Make new TTree. Copy branches from original. Add new branch. Fill new TTree with old TTree
    int nBr = tree_->GetListOfBranches()->GetEntries();
    for(int iBr = 0; iBr < nBr; iBr++){
        TBranch *br = dynamic_cast<TBranch*>(tree_->GetListOfBranches()->At(iBr));
        std::string varname = (std::string)br->GetFullName();
        newtree_->Branch(varname.c_str(),tuple_[varname],(varname+"/D").c_str());  
    }
}

void MutableTTree::defineMassWindow(double lowMass, double highMass){
    lowMass_ = lowMass;
    highMass_ = highMass;
}

bool MutableTTree::variableExists(std::string variable){
    if(tuple_.find(variable) != tuple_.end())
        return true;
    else
        return false;
}

std::vector<std::string> MutableTTree::getAllVariables(){
    std::vector<std::string> variables;
    for(std::map<std::string,double*>::iterator it = tuple_.begin(); it != tuple_.end(); it++){
       variables.push_back(it->first); 
    }

    return variables;
}

void MutableTTree::Fill(){

    for(int e=0; e < tree_->GetEntries(); e++){
        tree_->GetEntry(e);
        
        //Mass Window (if set)
        if(lowMass_ != -999.9 && highMass_ != -999.9){
            if(getValue("unc_vtx_mass")*1000.0 > highMass_) continue; 
            if(getValue("unc_vtx_mass")*1000.0 < lowMass_) continue; 
        }

        //Apply varible shifts here
        for(std::map<std::string,std::function<double()>>::iterator it = variable_shifts_.begin(); 
                it != variable_shifts_.end(); it++){
            variable_shifts_[it->first]();
        }

        //Add new variables here
        for(std::map<std::string,double*>::iterator it = new_variables_.begin(); it != new_variables_.end(); it++){
            *new_variables_[it->first] = functions_[it->first]();
        }
        
        newtree_->Fill();
    }
    
    delete tree_;
    tree_ = newtree_;
}

void MutableTTree::shiftVariable(std::string variable, double shift){
    std::cout << "[MutableTTree]::Shifting Variable " << variable << " by " << shift << std::endl;
    std::function<double()> shiftVariableFunc = [&,shift]()->double{
        return *tuple_[variable] = *tuple_[variable] + shift;
    };
    variable_shifts_[variable] = shiftVariableFunc;
}

void MutableTTree::addNewBranch(std::string branch){
    double* value = new double{999.9};
    tuple_[branch] = value;
    tree_->Branch(branch.c_str(),tuple_[branch],(branch+"/D").c_str());  
}

void MutableTTree::printEvent(){
    for(std::map<std::string,double*>::iterator it = tuple_.begin(); it != tuple_.end(); it ++){
        std::cout << it->first << ": " << *it->second << std::endl;
    }
    std::cout << "[MutableTree}::End print" << std::endl;
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

void MutableTTree::addVariableToTBranch(const std::string& variableName){
    double* variable = new double{999.9};
    tuple_[variableName] = variable;
    newtree_->Branch(variableName.c_str(), tuple_[variableName], (variableName+"/D").c_str());
    new_variables_[variableName] = variable;
}

