#include <MutableTTree.h>

MutableTTree::MutableTTree(TFile* infile, std::string tree_name){
   std::cout << "Reading in tree: " << tree_name << std::endl;
   tree_ = (TTree*)infile->Get(tree_name.c_str()); 
   if(tree_ == nullptr)
       std::cout << "ERROR READING TREE " << tree_name << " from file " << std::endl;
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

//add method to do generic var
void MutableTTree::shiftZ0(double shift){

    double* shifted_ele_z0 = new double{999.9};
    *shifted_ele_z0 = *tuple_["unc_vtx_ele_track_z0"] + shift;
    tuple_["unc_vtx_ele_track_z0"] = shifted_ele_z0;

    double* shifted_pos_z0 = new double{999.9};
    *shifted_pos_z0 = *tuple_["unc_vtx_pos_track_z0"] + shift;
    tuple_["unc_vtx_pos_track_z0"] = shifted_pos_z0;
}

void MutableTTree::addSelectionGreaterThan(std::string selection, double value){
    selectionGT_[selection] = value;
}

void MutableTTree::Fill(){

    for(int e=0; e < tree_->GetEntries(); e++){
        tree_->GetEntry(e);
        
        //Mass Window (if set)
        if(lowMass_ != -999.9 && highMass_ != -999.9){
            if(getValue("unc_vtx_mass")*1000.0 > highMass_) continue; 
            if(getValue("unc_vtx_mass")*1000.0 < lowMass_) continue; 
        }

        for(std::map<std::string, double>::iterator it = selectionGT_.begin(); it != selectionGT_.end(); it++){
            if(getValue(it->first) <= selectionGT_[it->first])
                continue;
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

bool MutableTTree::testImpactParameterCut(){
    double ele_z0 = getValue("unc_vtx_ele_track_z0");
    double pos_z0 = getValue("unc_vtx_pos_track_z0");
    double Z = getValue("unc_vtx_z");
    bool passCut = true;
    if(ele_z0 > 0.0){
        if(ele_z0 < 0.029816*(Z-3.471875))
            passCut = false;
    }
    else{
        if(ele_z0 > -0.029530*(Z-3.471875))
            passCut = false;
    }

    if(pos_z0 > 0.0){
        if(pos_z0 < 0.029816*(Z-3.471875))
            passCut = false;
    }
    else{
        if(pos_z0 > -0.029530*(Z-3.471875))
            passCut = false;
    }

    return passCut;
}

bool MutableTTree::impactParameterCut2016Canonical(double mass){
    mass = mass/1000.0;
    double ele_z0 = getValue("unc_vtx_ele_track_z0");
    double pos_z0 = getValue("unc_vtx_pos_track_z0");
    double Z = getValue("unc_vtx_z");
    //double dz = -0.377 + (13.79*mass) - (55.84*mass*mass) + (84.00*mass*mass*mass);
    //Z = Z + dz;
    double a = -0.2018;
    double b0p = 5.199e-2;
    double b1p = -2.301e-3;
    double b0n = -4.716e-2;
    double b1n = 1.086e-3;
    double z0p_gt = a + b0p*Z + b1p*Z/mass;
    double z0n_lt = -a + b0n*Z + b1n*Z/mass;

    bool passCut = true;
    if(Z < 6.0)
        return passCut;
    if(ele_z0 >= 0.0){
       if(ele_z0 < z0p_gt) passCut = false; 
    }
    else
        if(ele_z0 > z0n_lt) passCut = false;

    if(pos_z0 >= 0.0){
       if(pos_z0 < z0p_gt) passCut = false; 
    }
    else
        if(pos_z0 > z0n_lt) passCut = false;

    return passCut;
}

void::MutableTTree::shiftVariable(std::string variable, double shift){
    std::function<double()> shiftVariableFunc = [&,shift]()->double{
        return *tuple_[variable] = *tuple_[variable] + shift;
    };
    variable_shifts_[variable] = shiftVariableFunc;
}

void::MutableTTree::addNewVariable(std::string new_variable, double param){
    int case_id = new_variable_ids_[new_variable];
    switch(case_id) {
        case 0 : 
            addVariableZalpha(param); 
            break;
        case 1 : 
            addVariableZbravo();
            break;
        case 2 : 
            addVariableZbravosum();
            break;
        case 3 :
            addVariableZbravosumAlpha(param);
            break;
    }
}

void::MutableTTree::addVariableZbravosumAlpha(double slope){
    double* zbravosum_alpha = new double {999.9};
    tuple_["unc_vtx_zbravosumalpha"] = zbravosum_alpha;
    newtree_->Branch("unc_vtx_zbravosumalpha", tuple_["unc_vtx_zbravosumalpha"],"unc_vtx_zbravosumalpha/D");
    new_variables_["unc_vtx_zbravosumalpha"] = zbravosum_alpha;
    std::function<double()> calculateZbravosumalpha = [&,slope]()->double{
        if(*tuple_["unc_vtx_zbravosum"] > 0.0){
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_zbravosum"])/slope)) );
        }
        else{
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_zbravosum"])/(-1.0*slope))) );
        }
    };
    functions_["unc_vtx_zbravosumalpha"] = calculateZbravosumalpha;
}

void::MutableTTree::addVariableZbravosum(){
    double* zbravosum = new double {999.9};
    tuple_["unc_vtx_zbravosum"] = zbravosum;
    newtree_->Branch("unc_vtx_zbravosum", tuple_["unc_vtx_zbravosum"],"unc_vtx_zbravosum/D");
    new_variables_["unc_vtx_zbravosum"] = zbravosum;
    std::function<double()> calculateZbravosum = [&]()->double{
            return *tuple_["unc_vtx_ele_zbravo"] + *tuple_["unc_vtx_pos_zbravo"];
    };
    functions_["unc_vtx_zbravosum"] = calculateZbravosum;
}


void MutableTTree::addVariableZbravoAlpha(double slope){
    //04/24 This variable is defined as the difference between z0, and the 
    //Zbravo line, which is defined by fitting signal z0 v reconz with a line
    //This will depend on signal mass. This is just a starting point
    //double* ele_zbravo = tuple_["unc_vtx_ele_zbravo"];
    //double* pos_bravo = tuple_["unc_vtx_pos_zbravo"];
    //double* recon_z = tuple_["unc_vtx_z"];

    //ele
    double* ele_zbalpha = new double {999.9};
    tuple_["unc_vtx_ele_zbravoalpha"] = ele_zbalpha;
    newtree_->Branch("unc_vtx_ele_zbravoalpha",tuple_["unc_vtx_ele_zbravoalpha"],"unc_vtx_ele_zbravoalpha/D");  
    new_variables_["unc_vtx_ele_zbravoalpha"] = ele_zbalpha;
    double z0_correction = 0.0; //Details of this not clear yet 04/24/23
    std::function<double()> calculateZbravoalpha_ele = [&,slope]()->double{
        if(*tuple_["unc_vtx_ele_zbravo"] > -z0_correction){
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_zbravo"]+z0_correction)/slope)) );
        }
        else{
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_zbravo"]+z0_correction)/(-1.0*slope))) );
        }
    };
    functions_["unc_vtx_ele_zbravoalpha"] = calculateZbravoalpha_ele;

    //pos
    double* pos_zbalpha = new double {999.9};
    tuple_["unc_vtx_pos_zbravoalpha"] = pos_zbalpha;
    newtree_->Branch("unc_vtx_pos_zbravoalpha",tuple_["unc_vtx_pos_zbravoalpha"],"unc_vtx_pos_zbravoalpha/D");  
    new_variables_["unc_vtx_pos_zbravoalpha"] = pos_zbalpha;
    std::function<double()> calculateZbravoalpha_pos = [&,slope]()->double{
        if(*tuple_["unc_vtx_pos_zbravo"] > -z0_correction){
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_zbravo"]+z0_correction)/slope)) );
        }
        else{
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_zbravo"]+z0_correction)/(-1.0*slope))) );
        }
    };
    functions_["unc_vtx_pos_zbravoalpha"] = calculateZbravoalpha_pos;

}

void MutableTTree::addVariableZbravo(){
    //04/24 This variable is defined as the difference between z0, and the 
    //Zbravo line, which is defined by fitting signal z0 v reconz with a line
    //This will depend on signal mass. This is just a starting point
    double* ele_z0 = tuple_["unc_vtx_ele_track_z0"];
    double* pos_z0 = tuple_["unc_vtx_pos_track_z0"];
    double* recon_z = tuple_["unc_vtx_z"];

    //ele
    double* ele_zbravo = new double {999.9};
    tuple_["unc_vtx_ele_zbravo"] = ele_zbravo;
    newtree_->Branch("unc_vtx_ele_zbravo",tuple_["unc_vtx_ele_zbravo"],"unc_vtx_ele_zbravo/D");  
    new_variables_["unc_vtx_ele_zbravo"] = ele_zbravo;
    std::function<double()> calculateZbravo_ele = [&]()->double{
        if(*tuple_["unc_vtx_ele_track_tanLambda"] > 0.0){
            return *tuple_["unc_vtx_ele_track_z0"] - (-0.039151**tuple_["unc_vtx_z"] - 0.031282);
        }
        else{
            return *tuple_["unc_vtx_ele_track_z0"] - (0.040086**tuple_["unc_vtx_z"] + 0.016186);
        }
    };
    functions_["unc_vtx_ele_zbravo"] = calculateZbravo_ele;

    //pos
    double* pos_zbravo = new double {999.9};
    tuple_["unc_vtx_pos_zbravo"] = pos_zbravo;
    newtree_->Branch("unc_vtx_pos_zbravo",tuple_["unc_vtx_pos_zbravo"],"unc_vtx_pos_zbravo/D");  
    new_variables_["unc_vtx_pos_zbravo"] = pos_zbravo;
    std::function<double()> calculateZbravo_pos = [&]()->double{
        if(*tuple_["unc_vtx_pos_track_tanLambda"] > 0.0){
            return *tuple_["unc_vtx_pos_track_z0"] - (-0.037899**tuple_["unc_vtx_z"] - 0.0094);
        }
        else{
            return *tuple_["unc_vtx_pos_track_z0"] - (0.039501**tuple_["unc_vtx_z"] + 0.004176);
        }
    };
    functions_["unc_vtx_pos_zbravo"] = calculateZbravo_pos;
}

void MutableTTree::addVariableZalpha(double slope){

    double* ele_zalpha = new double{999.9};
    tuple_["unc_vtx_ele_track_zalpha"] = ele_zalpha;
    newtree_->Branch("unc_vtx_ele_track_zalpha",tuple_["unc_vtx_ele_track_zalpha"],"unc_vtx_ele_track_zalpha/D");  
    new_variables_["unc_vtx_ele_track_zalpha"] = ele_zalpha;

    //I think I messed up the signs of things here
    //Define lambda function to calculate zalpha
    std::function<double()> calculateZalpha_ele = [&, slope]()->double{
        if(*tuple_["unc_vtx_ele_track_z0"] > 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/slope)) );
        else
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/(-1*slope)) ));
    };
    functions_["unc_vtx_ele_track_zalpha"] = calculateZalpha_ele;

    double* pos_zalpha = new double{999.9};
    tuple_["unc_vtx_pos_track_zalpha"] = pos_zalpha;
    newtree_->Branch("unc_vtx_pos_track_zalpha",tuple_["unc_vtx_pos_track_zalpha"],"unc_vtx_pos_track_zalpha/D");  
    new_variables_["unc_vtx_pos_track_zalpha"] = pos_zalpha;

    //Define lambda function to calculate zalpha
    std::function<double()> calculateZalpha_pos = [&,slope]()->double{
        if(*tuple_["unc_vtx_pos_track_z0"] > 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"])/slope)) );
        else
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"]))/(-1*slope)) );
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
}

void MutableTTree::addNewBranch(std::string branch){
    double* value = new double{999.9};
    tuple_[branch] = value;
    tree_->Branch(branch.c_str(),tuple_[branch],(branch+"/D").c_str());  
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


