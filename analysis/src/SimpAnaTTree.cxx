#include <SimpAnaTTree.h>

bool SimpAnaTTree::testImpactParameterCut(){
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

bool SimpAnaTTree::impactParameterCut2016Canonical(double mass){
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


void SimpAnaTTree::addVariable_unc_vtx_ele_zalpha(double slope){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_ele_zalpha with slope " << slope << std::endl;

    //Create branch to hold new variable
    double* ele_zalpha = new double{999.9};
    tuple_["unc_vtx_ele_zalpha"] = ele_zalpha;
    newtree_->Branch("unc_vtx_ele_zalpha",tuple_["unc_vtx_ele_zalpha"],"unc_vtx_ele_zalpha/D");  
    new_variables_["unc_vtx_ele_zalpha"] = ele_zalpha;

    //Define lambda function to calculate ele zalpha
    std::function<double()> calculate_ele_zalpha = [&, slope]()->double{
        if(*tuple_["unc_vtx_ele_track_z0"] > 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/slope)) );
        else
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/(-1*slope)) ));
    };
    functions_["unc_vtx_ele_zalpha"] = calculate_ele_zalpha;
}

void SimpAnaTTree::addVariable_unc_vtx_pos_zalpha(double slope){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_pos_zalpha with slope " << slope << std::endl;

    //Create branch to hold new variable
    double* pos_zalpha = new double{999.9};
    tuple_["unc_vtx_pos_zalpha"] = pos_zalpha;
    newtree_->Branch("unc_vtx_pos_zalpha",tuple_["unc_vtx_pos_zalpha"],"unc_vtx_pos_zalpha/D");  
    new_variables_["unc_vtx_pos_zalpha"] = pos_zalpha;

    //Define lambda function to calculate pos zalpha
    std::function<double()> calculate_pos_zalpha = [&,slope]()->double{
        if(*tuple_["unc_vtx_pos_track_z0"] > 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"])/slope)) );
        else
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"]))/(-1*slope)) );
    };
    functions_["unc_vtx_pos_zalpha"] = calculate_pos_zalpha;
}

void SimpAnaTTree::addVariable_unc_vtx_zalpha_max(double slope){
    std::cout << "[SimpAnaTTree]::Add variable unc_vtx_zalpha_max with slope " << slope << std::endl;

    //Create branch to hold new variable
    double* zalpha_max = new double{999.9};
    tuple_["unc_vtx_zalpha_max"] = zalpha_max;
    newtree_->Branch("unc_vtx_zalpha_max",tuple_["unc_vtx_zalpha_max"],"unc_vtx_zalpha_max/D");  
    new_variables_["unc_vtx_zalpha_max"] = zalpha_max;

    //Define lambda function to calculate ele zalpha
    std::function<double()> calculate_zalpha_max = [&, slope]()->double{
        double ele_zalpha;
        double pos_zalpha;
        if(*tuple_["unc_vtx_ele_track_z0"] > 0.0)
            ele_zalpha = ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/slope)) );
        else
            ele_zalpha = ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/(-1*slope)) ));

        if(*tuple_["unc_vtx_pos_track_z0"] > 0.0)
            pos_zalpha = ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"])/slope)) );
        else
            pos_zalpha = ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"])/(-1*slope)) ));
        if(ele_zalpha > pos_zalpha)
            return ele_zalpha;
        else
            return pos_zalpha;
    };
    functions_["unc_vtx_zalpha_max"] = calculate_zalpha_max;
}

void SimpAnaTTree::addVariable_unc_vtx_zalpha_min(double slope){
    std::cout << "[SimpAnaTTree]::Add variable unc_vtx_zalpha_min with slope " << slope << std::endl;

    //Create branch to hold new variable
    double* zalpha_min = new double{999.9};
    tuple_["unc_vtx_zalpha_min"] = zalpha_min;
    newtree_->Branch("unc_vtx_zalpha_min",tuple_["unc_vtx_zalpha_min"],"unc_vtx_zalpha_min/D");  
    new_variables_["unc_vtx_zalpha_min"] = zalpha_min;

    //Define lambda function to calculate ele zalpha
    std::function<double()> calculate_zalpha_min = [&, slope]()->double{
        double ele_zalpha;
        double pos_zalpha;
        if(*tuple_["unc_vtx_ele_track_z0"] > 0.0)
            ele_zalpha = ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/slope)) );
        else
            ele_zalpha = ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/(-1*slope)) ));

        if(*tuple_["unc_vtx_pos_track_z0"] > 0.0)
            pos_zalpha = ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"])/slope)) );
        else
            pos_zalpha = ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"])/(-1*slope)) ));
        if(ele_zalpha < pos_zalpha)
            return ele_zalpha;
        else
            return pos_zalpha;
    };
    functions_["unc_vtx_zalpha_min"] = calculate_zalpha_min;
}

void SimpAnaTTree::addVariable_unc_vtx_ele_iso_z0err(){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_ele_iso_z0err " << std::endl;
    double* ele_iso_z0err = new double{9999.9};
    tuple_["unc_vtx_ele_iso_z0err"] = ele_iso_z0err;
    newtree_->Branch("unc_vtx_ele_iso_z0err", tuple_["unc_vtx_ele_iso_z0err"], "unc_vtx_ele_iso_z0err/D");
    new_variables_["unc_vtx_ele_iso_z0err"] = ele_iso_z0err;

    std::function<double()> calculate_ele_iso_z0err = [&]()->double{
        return 2.0* *tuple_["unc_vtx_ele_track_L1_isolation"] / *tuple_["unc_vtx_ele_track_z0Err"];
    };
    functions_["unc_vtx_ele_iso_z0err"] = calculate_ele_iso_z0err;
}

void SimpAnaTTree::addVariable_unc_vtx_pos_iso_z0err(){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_pos_iso_z0err " << std::endl;
    double* pos_iso_z0err = new double{9999.9};
    tuple_["unc_vtx_pos_iso_z0err"] = pos_iso_z0err;
    newtree_->Branch("unc_vtx_pos_iso_z0err", tuple_["unc_vtx_pos_iso_z0err"], "unc_vtx_pos_iso_z0err/D");
    new_variables_["unc_vtx_pos_iso_z0err"] = pos_iso_z0err;

    std::function<double()> calculate_pos_iso_z0err = [&]()->double{
        return 2.0* *tuple_["unc_vtx_pos_track_L1_isolation"] / *tuple_["unc_vtx_pos_track_z0Err"];
    };
    functions_["unc_vtx_pos_iso_z0err"] = calculate_pos_iso_z0err;
}

void SimpAnaTTree::addVariable_unc_vtx_ele_z0_z0err(){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_ele_z0_z0err " << std::endl;
    double* ele_z0_z0err = new double{9999.9};
    tuple_["unc_vtx_ele_z0_z0err"] = ele_z0_z0err;
    newtree_->Branch("unc_vtx_ele_z0_z0err", tuple_["unc_vtx_ele_z0_z0err"], "unc_vtx_ele_z0_z0err/D");
    new_variables_["unc_vtx_ele_z0_z0err"] = ele_z0_z0err;

    std::function<double()> calculate_ele_z0_z0err = [&]()->double{
        return std::abs(*tuple_["unc_vtx_ele_track_z0"])/ *tuple_["unc_vtx_ele_track_z0Err"];
    };
    functions_["unc_vtx_ele_z0_z0err"] = calculate_ele_z0_z0err;
}

void SimpAnaTTree::addVariable_unc_vtx_pos_z0_z0err(){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_pos_z0_z0err " << std::endl;
    double* pos_z0_z0err = new double{9999.9};
    tuple_["unc_vtx_pos_z0_z0err"] = pos_z0_z0err;
    newtree_->Branch("unc_vtx_pos_z0_z0err", tuple_["unc_vtx_pos_z0_z0err"], "unc_vtx_pos_z0_z0err/D");
    new_variables_["unc_vtx_pos_z0_z0err"] = pos_z0_z0err;

    std::function<double()> calculate_pos_z0_z0err = [&]()->double{
        return std::abs(*tuple_["unc_vtx_pos_track_z0"])/ *tuple_["unc_vtx_pos_track_z0Err"];
    };
    functions_["unc_vtx_pos_z0_z0err"] = calculate_pos_z0_z0err;
}

void SimpAnaTTree::addVariable_unc_vtx_ele_isolation_cut(){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_ele_isolation_cut" << std::endl;
    double* ele_isolation = new double{9999.9};
    tuple_["unc_vtx_ele_isolation_cut"] = ele_isolation;
    newtree_->Branch("unc_vtx_ele_isolation_cut", tuple_["unc_vtx_ele_isolation_cut"], "unc_vtx_ele_isolation_cut/D");
    new_variables_["unc_vtx_ele_isolation_cut"] = ele_isolation;

    std::function<double()> calculate_ele_isolation_cut = [&]()->double{

        return ( (2.0* *tuple_["unc_vtx_ele_track_L1_isolation"] / *tuple_["unc_vtx_ele_track_z0Err"]) - (std::abs(*tuple_["unc_vtx_ele_track_z0"])/ *tuple_["unc_vtx_ele_track_z0Err"]) );
    };
    functions_["unc_vtx_ele_isolation_cut"] = calculate_ele_isolation_cut;
}

void SimpAnaTTree::addVariable_unc_vtx_pos_isolation_cut(){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_pos_isolation_cut" << std::endl;
    double* pos_isolation = new double{9999.9};
    tuple_["unc_vtx_pos_isolation_cut"] = pos_isolation;
    newtree_->Branch("unc_vtx_pos_isolation_cut", tuple_["unc_vtx_pos_isolation_cut"], "unc_vtx_pos_isolation_cut/D");
    new_variables_["unc_vtx_pos_isolation_cut"] = pos_isolation;

    std::function<double()> calculate_pos_isolation_cut = [&]()->double{

        return ( (2.0* *tuple_["unc_vtx_pos_track_L1_isolation"] / *tuple_["unc_vtx_pos_track_z0Err"]) - (std::abs(*tuple_["unc_vtx_pos_track_z0"])/ *tuple_["unc_vtx_pos_track_z0Err"]) );
    };
    functions_["unc_vtx_pos_isolation_cut"] = calculate_pos_isolation_cut;
}

void SimpAnaTTree::addVariable_unc_vtx_ele_z0tanlambda(){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_ele_z0tanlambda" << std::endl;
    double* ele_z0tanlambda = new double{999.9};
    tuple_["unc_vtx_ele_z0tanlambda"] = ele_z0tanlambda;
    newtree_->Branch("unc_vtx_ele_z0tanlambda", tuple_["unc_vtx_ele_z0tanlambda"], "unc_vtx_ele_z0tanlambda/D");
    new_variables_["unc_vtx_ele_z0tanlambda"] = ele_z0tanlambda;
    std::function<double()> calculate_ele_z0tanlambda = [&]()->double{
        return *tuple_["unc_vtx_ele_track_z0"] /  *tuple_["unc_vtx_ele_track_tanLambda"];
    };
    functions_["unc_vtx_ele_z0tanlambda"] = calculate_ele_z0tanlambda;
}

void SimpAnaTTree::addVariable_unc_vtx_pos_z0tanlambda(){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_pos_z0tanlambda" << std::endl;
    double* pos_z0tanlambda = new double{999.9};
    tuple_["unc_vtx_pos_z0tanlambda"] = pos_z0tanlambda;
    newtree_->Branch("unc_vtx_pos_z0tanlambda", tuple_["unc_vtx_pos_z0tanlambda"], "unc_vtx_pos_z0tanlambda/D");
    new_variables_["unc_vtx_pos_z0tanlambda"] = pos_z0tanlambda;
    std::function<double()> calculate_pos_z0tanlambda = [&]()->double{
        return *tuple_["unc_vtx_pos_track_z0"] /  *tuple_["unc_vtx_pos_track_tanLambda"];
    };
    functions_["unc_vtx_pos_z0tanlambda"] = calculate_pos_z0tanlambda;
}

void SimpAnaTTree::addVariable_unc_vtx_ele_z0tanlambda_right(double slope){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_ele_z0tanlambda_right with slope " << slope <<  std::endl;
    double* ele_z0tanlambda_right = new double{999.9};
    tuple_["unc_vtx_ele_z0tanlambda_right"] = ele_z0tanlambda_right;
    newtree_->Branch("unc_vtx_ele_z0tanlambda_right", tuple_["unc_vtx_ele_z0tanlambda_right"], "unc_vtx_ele_z0tanlambda_right/D");
    new_variables_["unc_vtx_ele_z0tanlambda_right"] = ele_z0tanlambda_right;
    std::function<double()> calculate_ele_z0tanlambda_right = [&,slope]()->double{
        return (*tuple_["unc_vtx_ele_track_z0"] /  *tuple_["unc_vtx_ele_track_tanLambda"]) + (-*tuple_["unc_vtx_z"]/-slope);
    };
    functions_["unc_vtx_ele_z0tanlambda_right"] = calculate_ele_z0tanlambda_right;
}

void SimpAnaTTree::addVariable_unc_vtx_pos_z0tanlambda_right(double slope){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_pos_z0tanlambda_right with slope " << slope <<  std::endl;
    double* pos_z0tanlambda_right = new double{999.9};
    tuple_["unc_vtx_pos_z0tanlambda_right"] = pos_z0tanlambda_right;
    newtree_->Branch("unc_vtx_pos_z0tanlambda_right", tuple_["unc_vtx_pos_z0tanlambda_right"], "unc_vtx_pos_z0tanlambda_right/D");
    new_variables_["unc_vtx_pos_z0tanlambda_right"] = pos_z0tanlambda_right;
    std::function<double()> calculate_pos_z0tanlambda_right = [&, slope]()->double{
        return (*tuple_["unc_vtx_pos_track_z0"] /  *tuple_["unc_vtx_pos_track_tanLambda"]) + (-*tuple_["unc_vtx_z"]/-slope);
    };
    functions_["unc_vtx_pos_z0tanlambda_right"] = calculate_pos_z0tanlambda_right;
}

void SimpAnaTTree::addVariable_unc_vtx_ele_z0tanlambda_left(double slope){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_ele_z0tanlambda_left with slope " << slope <<  std::endl;
    double* ele_z0tanlambda_left = new double{999.9};
    tuple_["unc_vtx_ele_z0tanlambda_left"] = ele_z0tanlambda_left;
    newtree_->Branch("unc_vtx_ele_z0tanlambda_left", tuple_["unc_vtx_ele_z0tanlambda_left"], "unc_vtx_ele_z0tanlambda_left/D");
    new_variables_["unc_vtx_ele_z0tanlambda_left"] = ele_z0tanlambda_left;
    std::function<double()> calculate_ele_z0tanlambda_left = [&,slope]()->double{
        return (*tuple_["unc_vtx_ele_track_z0"] /  *tuple_["unc_vtx_ele_track_tanLambda"]) + (-*tuple_["unc_vtx_z"]/-slope);
    };
    functions_["unc_vtx_ele_z0tanlambda_left"] = calculate_ele_z0tanlambda_left;
}

void SimpAnaTTree::addVariable_unc_vtx_pos_z0tanlambda_left(double slope){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_pos_z0tanlambda_left with slope " << slope <<  std::endl;
    double* pos_z0tanlambda_left = new double{999.9};
    tuple_["unc_vtx_pos_z0tanlambda_left"] = pos_z0tanlambda_left;
    newtree_->Branch("unc_vtx_pos_z0tanlambda_left", tuple_["unc_vtx_pos_z0tanlambda_left"], "unc_vtx_pos_z0tanlambda_left/D");
    new_variables_["unc_vtx_pos_z0tanlambda_left"] = pos_z0tanlambda_left;
    std::function<double()> calculate_pos_z0tanlambda_left = [&, slope]()->double{
        return (*tuple_["unc_vtx_pos_track_z0"] /  *tuple_["unc_vtx_pos_track_tanLambda"]) + (-*tuple_["unc_vtx_z"]/-slope);
    };
    functions_["unc_vtx_pos_z0tanlambda_left"] = calculate_pos_z0tanlambda_left;
}

/*
void SimpAnaTTree::addVariable_unc_vtx_zbravosumAlpha(double slope){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_zbravosumAlpha" << std::endl;
    double* zbravosumAlpha = new double {999.9};
    tuple_["unc_vtx_zbravosumAlpha"] = zbravosumAlpha;
    newtree_->Branch("unc_vtx_zbravosumAlpha", tuple_["unc_vtx_zbravosumAlpha"],"unc_vtx_zbravosumAlpha/D");
    new_variables_["unc_vtx_zbravosumAlpha"] = zbravosumAlpha;
    std::function<double()> calculatezbravosumAlpha = [&,slope]()->double{
        if(*tuple_["unc_vtx_zbravosum"] > 0.0){
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_zbravosum"])/slope)) );
        }
        else{
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_zbravosum"])/(-1.0*slope))) );
        }
    };
    functions_["unc_vtx_zbravosumAlpha"] = calculatezbravosumAlpha;
}

void SimpAnaTTree::addVariable_unc_vtx_zbravosum(){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_zbravosum" << std::endl;
    double* zbravosum = new double {999.9};
    tuple_["unc_vtx_zbravosum"] = zbravosum;
    newtree_->Branch("unc_vtx_zbravosum", tuple_["unc_vtx_zbravosum"],"unc_vtx_zbravosum/D");
    new_variables_["unc_vtx_zbravosum"] = zbravosum;
    std::function<double()> calculateZbravosum = [&]()->double{
            return *tuple_["unc_vtx_ele_zbravo"] + *tuple_["unc_vtx_pos_zbravo"];
    };
    functions_["unc_vtx_zbravosum"] = calculateZbravosum;
}

void SimpAnaTTree::addVariable_unc_vtx_zbravoAlphaTop(double slope){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_zbravoAlphaTop" << std::endl;
    double* zbravoAlphaTop = new double{999.9};
    tuple_["unc_vtx_zbravoAlphaTop"] = zbravoAlphaTop;
    newtree_->Branch("unc_vtx_zbravoAlphaTop", tuple_["unc_vtx_zbravoAlphaTop"],
        "unc_vtx_zbravoAlpha/D");
    new_variables_["unc_vtx_zbravoAlphaTop"] = zbravoAlphaTop;

    //Lambda function to calculate zbravoAlpha
    std::function<double()> calculate_zbravoAlphaTop = [&, slope]()->double{
        if(*tuple_["unc_vtx_ele_zbravo"] > 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_zbravo"])/slope)) );
        else
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_zbravo"])/slope)) );
    };
    functions_["unc_vtx_zbravoAlphaTop"] = calculate_zbravoAlphaTop;
}

void SimpAnaTTree::addVariable_unc_vtx_zbravoAlphaBot(double slope){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_zbravoAlphaBot" << std::endl;
    double* zbravoAlphaBot = new double{999.9};
    tuple_["unc_vtx_zbravoAlphaBot"] = zbravoAlphaBot;
    newtree_->Branch("unc_vtx_zbravoAlphaBot", tuple_["unc_vtx_zbravoAlphaBot"],
        "unc_vtx_zbravoAlpha/D");
    new_variables_["unc_vtx_zbravoAlphaBot"] = zbravoAlphaBot;

    //Lambda function to calculate zbravoAlpha
    std::function<double()> calculate_zbravoAlphaBot = [&, slope]()->double{
        if(*tuple_["unc_vtx_ele_zbravo"] > 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_zbravo"])/slope)) );
        else
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_zbravo"])/slope)) );
    };
    functions_["unc_vtx_zbravoAlphaBot"] = calculate_zbravoAlphaBot;
}

void SimpAnaTTree::addVariable_unc_vtx_ele_zbravoAlpha(double slope){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_ele_zbravoAlpha with slope " << slope << std::endl;
    //04/24 This variable is defined as the difference between z0, and the 
    //Zbravo line, which is defined by fitting signal z0 v reconz with a line
    //This will depend on signal mass. This is just a starting point

    //ele
    double* ele_zbravoAlpha = new double {999.9};
    tuple_["unc_vtx_ele_zbravoAlpha"] = ele_zbravoAlpha;
    newtree_->Branch("unc_vtx_ele_zbravoAlpha",tuple_["unc_vtx_ele_zbravoAlpha"],"unc_vtx_ele_zbravoAlpha/D");  
    new_variables_["unc_vtx_ele_zbravoAlpha"] = ele_zbravoAlpha;
    double z0_correction = 0.0; //Details of this not clear yet 04/24/23
    std::function<double()> calculate_ele_zbravoAlpha = [&,slope]()->double{
        if(*tuple_["unc_vtx_ele_zbravo"] > -z0_correction){
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_zbravo"]+z0_correction)/slope)) );
        }
        else{
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_zbravo"]+z0_correction)/(-1.0*slope))) );
        }
    };
    functions_["unc_vtx_ele_zbravoAlpha"] = calculate_ele_zbravoAlpha;
}
void SimpAnaTTree::addVariable_unc_vtx_pos_zbravoAlpha(double slope){
    std::cout << "[SimpAnaTTree]::Adding variable unc_vtx_pos_zbravoAlpha with slope " << slope << std::endl;
    //04/24 This variable is defined as the difference between z0, and the 
    //Zbravo line, which is defined by fitting signal z0 v reconz with a line
    //This will depend on signal mass. This is just a starting point
    //
    //pos
    double* pos_zbravoAlpha = new double {999.9};
    tuple_["unc_vtx_pos_zbravoAlpha"] = pos_zbravoAlpha;
    newtree_->Branch("unc_vtx_pos_zbravoAlpha",tuple_["unc_vtx_pos_zbravoAlpha"],"unc_vtx_pos_zbravoAlpha/D");  
    new_variables_["unc_vtx_pos_zbravoAlpha"] = pos_zbravoAlpha;
    std::function<double()> calculate_pos_zbravoAlpha = [&,slope]()->double{
        if(*tuple_["unc_vtx_pos_zbravo"] > -z0_correction){
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_zbravo"]+z0_correction)/slope)) );
        }
        else{
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_zbravo"]+z0_correction)/(-1.0*slope))) );
        }
    };
    functions_["unc_vtx_pos_zbravoAlpha"] = calculate_pos_zbravoAlpha;
}

void SimpAnaTTree::addVariable_unc_vtx_ele_zbravo(){
    std::cout << "[SimpAnaTTree]::Adding variable ele_zbravo" << std::endl;
    double* ele_z0 = tuple_["unc_vtx_ele_track_z0"];
    double* recon_z = tuple_["unc_vtx_z"];

    //ele
    double* ele_zbravo = new double {999.9};
    tuple_["unc_vtx_ele_zbravo"] = ele_zbravo;
    newtree_->Branch("unc_vtx_ele_zbravo",tuple_["unc_vtx_ele_zbravo"],"unc_vtx_ele_zbravo/D");  
    new_variables_["unc_vtx_ele_zbravo"] = ele_zbravo;
    std::function<double()> calculate_ele_zbravo = [&]()->double{
        if(*tuple_["unc_vtx_ele_track_tanLambda"] > 0.0){
            return *tuple_["unc_vtx_ele_track_z0"] - (-0.039151**tuple_["unc_vtx_z"] - 0.031282);
        }
        else{
            return *tuple_["unc_vtx_ele_track_z0"] - (0.040086**tuple_["unc_vtx_z"] + 0.016186);
        }
    };
    functions_["unc_vtx_ele_zbravo"] = ele_zbravo;
}

void SimpAnaTTree::addVariable_unc_vtx_pos_zbravo(){
    std::cout << "[SimpAnaTTree]::Adding variable pos_zbravo" << std::endl;
    double* pos_z0 = tuple_["unc_vtx_pos_track_z0"];
    double* recon_z = tuple_["unc_vtx_z"];

    //pos
    double* pos_zbravo = new double {999.9};
    tuple_["unc_vtx_pos_zbravo"] = pos_zbravo;
    newtree_->Branch("unc_vtx_pos_zbravo",tuple_["unc_vtx_pos_zbravo"],"unc_vtx_pos_zbravo/D");  
    new_variables_["unc_vtx_pos_zbravo"] = pos_zbravo;
    std::function<double()> calculate_pos_zbravo = [&]()->double{
        if(*tuple_["unc_vtx_pos_track_tanLambda"] > 0.0){
            return *tuple_["unc_vtx_pos_track_z0"] - (-0.037899**tuple_["unc_vtx_z"] - 0.0094);
        }
        else{
            return *tuple_["unc_vtx_pos_track_z0"] - (0.039501**tuple_["unc_vtx_z"] + 0.004176);
        }
    };
    functions_["unc_vtx_pos_zbravo"] = calculate_pos_zbravo;
}

void SimpAnaTTree::addVariableZalphaTop(double slope){

    double* zalpha_top = new double{999.9};
    tuple_["unc_vtx_track_zalpha_top"] = zalpha_top;
    newtree_->Branch("unc_vtx_track_zalpha_top", tuple_["unc_vtx_track_zalpha_top"],
        "unc_vtx_track_zalpha_top/D");
    new_variables_["unc_vtx_track_zalpha_top"] = zalpha_top;

    //Lambda function to calculate zalpha
    std::function<double()> calculate_zalpha_top = [&, slope]()->double{
        if(*tuple_["unc_vtx_ele_track_z0"] > 0.0 && *tuple_["unc_vtx_pos_track_z0"] < 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/slope)) );
        if(*tuple_["unc_vtx_pos_track_z0"] > 0.0 && *tuple_["unc_vtx_ele_track_z0"] < 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"])/slope)) );
        if(*tuple_["unc_vtx_ele_track_z0"] < 0.0 && *tuple_["unc_vtx_pos_track_z0"] < 0.0)
            return skipCutVarValue_;
        if(*tuple_["unc_vtx_ele_track_z0"] > 0.0 && *tuple_["unc_vtx_pos_track_z0"] > 0.0){
            double ele_zalpha = ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/slope)) );
            double pos_zalpha = ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"])/slope)) );
            if(ele_zalpha > pos_zalpha)
                return ele_zalpha;
            else
                return pos_zalpha;
        }
    };
    functions_["unc_vtx_track_zalpha_top"] = calculate_zalpha_top;
}

void SimpAnaTTree::addVariableZalphaBot(double slope){

    double* zalpha_bot = new double{999.9};
    tuple_["unc_vtx_track_zalpha_bot"] = zalpha_bot;
    newtree_->Branch("unc_vtx_track_zalpha_bot", tuple_["unc_vtx_track_zalpha_bot"],
            "unc_vtx_track_zalpha_bot/D");
    new_variables_["unc_vtx_track_zalpha_bot"] = zalpha_bot;

    //Lambda function to calculate zalpha
    std::function<double()> calculate_zalpha_bot = [&, slope]()->double{
        if(*tuple_["unc_vtx_ele_track_z0"] < 0.0 && *tuple_["unc_vtx_pos_track_z0"] > 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/-slope)) );
        if(*tuple_["unc_vtx_pos_track_z0"] < 0.0 && *tuple_["unc_vtx_ele_track_z0"] > 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"])/-slope)) );
        if(*tuple_["unc_vtx_ele_track_z0"] > 0.0 && *tuple_["unc_vtx_pos_track_z0"] > 0.0)
            return skipCutVarValue_;
        if(*tuple_["unc_vtx_ele_track_z0"] < 0.0 && *tuple_["unc_vtx_pos_track_z0"] < 0.0){
            double ele_zalpha = ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/-slope)) );
            double pos_zalpha = ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"])/-slope)) );
            if(ele_zalpha > pos_zalpha)
                return ele_zalpha;
            else
                return pos_zalpha;
        }
    };

    functions_["unc_vtx_track_zalpha_bot"] = calculate_zalpha_bot;
}

void SimpAnaTTree::addVariableZalphaTopPos(double slope){
    double* zalpha_top_pos = new double{999.9};
    tuple_["unc_vtx_track_zalpha_top_pos"] = zalpha_top_pos;
    newtree_->Branch("unc_vtx_track_zalpha_top_pos", tuple_["unc_vtx_track_zalpha_top_pos"],
            "unc_vtx_track_zalpha_top_pos/D");
    new_variables_["unc_vtx_track_zalpha_top_pos"] = zalpha_top_pos;
    std::function<double()> calculate_zalpha_top_pos = [&, slope]()->double{
        if(*tuple_["unc_vtx_pos_track_z0"] > 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"])/slope)) );
        else
            return skipCutVarValue_;
    };
    functions_["unc_vtx_track_zalpha_top_pos"] = calculate_zalpha_top_pos;
}

void SimpAnaTTree::addVariableZalphaTopEle(double slope){
    double* zalpha_top_ele = new double{999.9};
    tuple_["unc_vtx_track_zalpha_top_ele"] = zalpha_top_ele;
    newtree_->Branch("unc_vtx_track_zalpha_top_ele", tuple_["unc_vtx_track_zalpha_top_ele"],
            "unc_vtx_track_zalpha_top_ele/D");
    new_variables_["unc_vtx_track_zalpha_top_ele"] = zalpha_top_ele;
    std::function<double()> calculate_zalpha_top_ele = [&, slope]()->double{
        if(*tuple_["unc_vtx_ele_track_z0"] > 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/slope)) );
        else
            return skipCutVarValue_;
    };
    functions_["unc_vtx_track_zalpha_top_ele"] = calculate_zalpha_top_ele;
}

void SimpAnaTTree::addVariableZalphaBotEle(double slope){
    double* zalpha_bot_ele = new double{999.9};
    tuple_["unc_vtx_track_zalpha_bot_ele"] = zalpha_bot_ele;
    newtree_->Branch("unc_vtx_track_zalpha_bot_ele", tuple_["unc_vtx_track_zalpha_bot_ele"],
            "unc_vtx_track_zalpha_bot_ele/D");
    new_variables_["unc_vtx_track_zalpha_bot_ele"] = zalpha_bot_ele;
    std::function<double()> calculate_zalpha_bot_ele = [&, slope]()->double{
        if(*tuple_["unc_vtx_ele_track_z0"] < 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/-slope)) );
        else
            return skipCutVarValue_;
    };
    functions_["unc_vtx_track_zalpha_bot_ele"] = calculate_zalpha_bot_ele;
}

void SimpAnaTTree::addVariableZalphaBotPos(double slope){
    double* zalpha_bot_pos = new double{999.9};
    tuple_["unc_vtx_track_zalpha_bot_pos"] = zalpha_bot_pos;
    newtree_->Branch("unc_vtx_track_zalpha_bot_pos", tuple_["unc_vtx_track_zalpha_bot_pos"],"unc_vtx_track_zalpha_bot_pos/D");
    new_variables_["unc_vtx_track_zalpha_bot_pos"] = zalpha_bot_pos;
    std::function<double()> calculate_zalpha_bot_pos = [&, slope]()->double{
        if(*tuple_["unc_vtx_pos_track_z0"] < 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"])/-slope)) );
        else
            return skipCutVarValue_;
    };
    functions_["unc_vtx_track_zalpha_bot_pos"] = calculate_zalpha_bot_pos;
}

void SimpAnaTTree::addVariableZalphaTopBot(double top_slope, double bot_slope){
    addVariableZalphaTop(top_slope);
    addVariableZalphaBot(bot_slope);
}

*/
