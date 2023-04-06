#include <MutableTTree.h>

MutableTTree::MutableTTree(TFile* infile, std::string tree_name){
   tree_ = (TTree*)infile->Get(tree_name.c_str()); 
   initializeFlatTuple(tree_, tuple_);
   newtree_ = new TTree();
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

        for(std::map<std::string,double*>::iterator it = new_variables_.begin(); it != new_variables_.end(); it++){
            *new_variables_[it->first] = functions_[it->first]();
        }
        newtree_->Fill();
    }
    
    delete tree_;
    tree_ = newtree_;
}

void MutableTTree::addVariableZalpha(double slope){

    double* ele_zalpha = new double{999.9};
    tuple_["unc_vtx_ele_track_zalpha"] = ele_zalpha;
    newtree_->Branch("unc_vtx_ele_track_zalpha",tuple_["unc_vtx_ele_track_zalpha"],"unc_vtx_ele_track_zalpha/D");  
    new_variables_["unc_vtx_ele_track_zalpha"] = ele_zalpha;

    //I think I messed up the signs of things here
    //Define lambda function to calculate zalpha
    std::function<double()> calculateZalpha_ele = [&, slope]()->double{
        if(*tuple_["unc_vtx_ele_track_z0"] > 0)
            return *tuple_["unc_vtx_z"] - ((*tuple_["unc_vtx_ele_track_z0"]/slope));
        else
            return *tuple_["unc_vtx_z"] - ((*tuple_["unc_vtx_ele_track_z0"])/(-1*slope));
    };
    functions_["unc_vtx_ele_track_zalpha"] = calculateZalpha_ele;

    double* pos_zalpha = new double{999.9};
    tuple_["unc_vtx_pos_track_zalpha"] = pos_zalpha;
    newtree_->Branch("unc_vtx_pos_track_zalpha",tuple_["unc_vtx_pos_track_zalpha"],"unc_vtx_pos_track_zalpha/D");  
    new_variables_["unc_vtx_pos_track_zalpha"] = pos_zalpha;

    //Define lambda function to calculate zalpha
    std::function<double()> calculateZalpha_pos = [&,slope]()->double{
        if(*tuple_["unc_vtx_pos_track_z0"] > 0)
            return *tuple_["unc_vtx_z"] - ((*tuple_["unc_vtx_pos_track_z0"]/slope));
        else
            return *tuple_["unc_vtx_z"] - ((*tuple_["unc_vtx_pos_track_z0"])/(-1*slope));
    };
    functions_["unc_vtx_pos_track_zalpha"] = calculateZalpha_pos;
}

//Add comment
void MutableTTree::addVariableZalpha(double y_intercept, double slope, double alpha_z){

    double* ele_zalpha = new double{999.9};
    tuple_["unc_vtx_ele_track_zalpha"] = ele_zalpha;
    newtree_->Branch("unc_vtx_ele_track_zalpha",tuple_["unc_vtx_ele_track_zalpha"],"unc_vtx_ele_track_zalpha/D");  
    new_variables_["unc_vtx_ele_track_zalpha"] = ele_zalpha;

    //I think I messed up the signs of things here
    //Define lambda function to calculate zalpha
    std::function<double()> calculateZalpha_ele = [&, y_intercept, slope, alpha_z]()->double{
        if(*tuple_["unc_vtx_ele_track_z0"] > 0)
            return *tuple_["unc_vtx_z"] - ( ((*tuple_["unc_vtx_ele_track_z0"]-y_intercept)/slope) - alpha_z );
        else
            return *tuple_["unc_vtx_z"] - ( ((*tuple_["unc_vtx_ele_track_z0"]+y_intercept)/(-1*slope)) - alpha_z );
    };
    functions_["unc_vtx_ele_track_zalpha"] = calculateZalpha_ele;

    double* pos_zalpha = new double{999.9};
    tuple_["unc_vtx_pos_track_zalpha"] = pos_zalpha;
    newtree_->Branch("unc_vtx_pos_track_zalpha",tuple_["unc_vtx_pos_track_zalpha"],"unc_vtx_pos_track_zalpha/D");  
    new_variables_["unc_vtx_pos_track_zalpha"] = pos_zalpha;

    //Define lambda function to calculate zalpha
    std::function<double()> calculateZalpha_pos = [&, y_intercept, slope, alpha_z]()->double{
        if(*tuple_["unc_vtx_pos_track_z0"] > 0)
            return *tuple_["unc_vtx_z"] - ( ((*tuple_["unc_vtx_pos_track_z0"]-y_intercept)/slope) - alpha_z);
        else
            return *tuple_["unc_vtx_z"] - ( ((*tuple_["unc_vtx_pos_track_z0"]+y_intercept)/(-1*slope)) - alpha_z );
    };
    functions_["unc_vtx_pos_track_zalpha"] = calculateZalpha_pos;
    /*
    //Define lambda function to calculate zalpha
    std::function<double()> calculateZalpha_ele = [&, y_intercept, slope, alpha_z]()->double{
        if(*tuple_["unc_vtx_ele_track_z0"] > 0)
            return *tuple_["unc_vtx_z"] - ( ((*tuple_["unc_vtx_ele_track_z0"]-y_intercept)/slope) - alpha_z );
        else
            return *tuple_["unc_vtx_z"] - ( ((*tuple_["unc_vtx_ele_track_z0"]-y_intercept)/(-1*slope)) - alpha_z );
    };
    functions_["unc_vtx_ele_track_zalpha"] = calculateZalpha_ele;

    double* pos_zalpha = new double{999.9};
    tuple_["unc_vtx_pos_track_zalpha"] = pos_zalpha;
    newtree_->Branch("unc_vtx_pos_track_zalpha",tuple_["unc_vtx_pos_track_zalpha"],"unc_vtx_pos_track_zalpha/D");  
    new_variables_["unc_vtx_pos_track_zalpha"] = pos_zalpha;

    //Define lambda function to calculate zalpha
    std::function<double()> calculateZalpha_pos = [&, y_intercept, slope, alpha_z]()->double{
        if(*tuple_["unc_vtx_pos_track_z0"] > 0)
            return *tuple_["unc_vtx_z"] - ( ((*tuple_["unc_vtx_pos_track_z0"]-y_intercept)/slope) - alpha_z);
        else
            return *tuple_["unc_vtx_z"] - ( ((*tuple_["unc_vtx_pos_track_z0"]-y_intercept)/(-1*slope)) - alpha_z );
    };
    functions_["unc_vtx_pos_track_zalpha"] = calculateZalpha_pos;
    */
        

        /*
        double a_p = impact_parameter_cut_[0];
        double b_p = impact_parameter_cut_[1];
        double a_d = impact_parameter_cut_[2];
        double b_d = impact_parameter_cut_[3];
        double beta = impact_parameter_cut_[4];
        double z_alpha = impact_parameter_cut_[5];

        if(*tuple_["unc_vtx_ele_track_z0"] > 0)
            return *tuple_["unc_vtx_z"] - ( ((*tuple_["unc_vtx_ele_track_z0"]-a_p)/b_p) - z_alpha );
        else
            return *tuple_["unc_vtx_z"] - ( ((*tuple_["unc_vtx_ele_track_z0"]-a_d)/b_d) - z_alpha );
            */

    //double (*myfunc_ptr)();
    //*myfunc_ptr = myfunc;


    //my_functions_["zalpha"] = Function of lambda(param) 
    //auto lambda = []() { return  MATH(*tuple_["unc_vtx_z"]) };
}

void MutableTTree::addNewBranch(std::string branch){
    double* value = new double{999.9};
    tuple_[branch] = value;
    tree_->Branch(branch.c_str(),tuple_[branch],(branch+"/D").c_str());  
    //TBranch* br = (TBranch*)tree_->GetBranch(branch.c_str());
    //new_branches[branch] = br;
}

void MutableTTree::fillNewBranch(std::string branch, double value){
    setBranchValue(branch, value);
    new_branches[branch]->Fill();
}

void MutableTTree::printEvent(){
    for(std::map<std::string,double*>::iterator it = tuple_.begin(); it != tuple_.end(); it ++){
        std::cout << it->first << ": " << *it->second << std::endl;
    }
    std::cout << "End print" << std::endl;
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


