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

void SimpAnaTTree::addVariableZbravosumAlpha(double slope){
    std::cout << "[MutableTTree]::addVariableZbravosumalpha" << std::endl;
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

void SimpAnaTTree::addVariableZbravosum(){
    std::cout << "[MutableTTree]::addVariableZbravosum" << std::endl;
    double* zbravosum = new double {999.9};
    tuple_["unc_vtx_zbravosum"] = zbravosum;
    newtree_->Branch("unc_vtx_zbravosum", tuple_["unc_vtx_zbravosum"],"unc_vtx_zbravosum/D");
    new_variables_["unc_vtx_zbravosum"] = zbravosum;
    std::function<double()> calculateZbravosum = [&]()->double{
            return *tuple_["unc_vtx_ele_track_zbravo"] + *tuple_["unc_vtx_pos_track_zbravo"];
    };
    functions_["unc_vtx_zbravosum"] = calculateZbravosum;
}

void SimpAnaTTree::addVariableZbravoAlphaTop(double slope){
    double* zbravoalpha = new double{999.9};
    tuple_["unc_vtx_track_zbravoalpha_top"] = zbravoalpha;
    newtree_->Branch("unc_vtx_track_zbravoalpha_top", tuple_["unc_vtx_track_zbravoalpha_top"],
        "unc_vtx_track_zbravoalpha/D");
    new_variables_["unc_vtx_track_zbravoalpha_top"] = zbravoalpha;

    //Lambda function to calculate zbravoalpha
    std::function<double()> calculate_zbravoalpha_top = [&, slope]()->double{
        if(*tuple_["unc_vtx_ele_track_zbravo"] > 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_zbravo"])/slope)) );
        else
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_zbravo"])/slope)) );
    };
    functions_["unc_vtx_track_zbravoalpha_top"] = calculate_zbravoalpha_top;
}

void SimpAnaTTree::addVariableZbravoAlphaBot(double slope){
    double* zbravoalpha = new double{999.9};
    tuple_["unc_vtx_track_zbravoalpha_bot"] = zbravoalpha;
    newtree_->Branch("unc_vtx_track_zbravoalpha_bot", tuple_["unc_vtx_track_zbravoalpha_bot"],
        "unc_vtx_track_zbravoalpha/D");
    new_variables_["unc_vtx_track_zbravoalpha_bot"] = zbravoalpha;

    //Lambda function to calculate zbravoalpha
    std::function<double()> calculate_zbravoalpha_bot = [&, slope]()->double{
        if(*tuple_["unc_vtx_ele_track_zbravo"] < 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_zbravo"])/slope)) );
        else
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_zbravo"])/slope)) );
    };
    functions_["unc_vtx_track_zbravoalpha_bot"] = calculate_zbravoalpha_bot;
}


void SimpAnaTTree::addVariableZbravoAlpha(double slope){
    std::cout << "[MutableTTree]::addVariableZbravoAlpha with slope " << slope << std::endl;
    //04/24 This variable is defined as the difference between z0, and the 
    //Zbravo line, which is defined by fitting signal z0 v reconz with a line
    //This will depend on signal mass. This is just a starting point
    //double* ele_zbravo = tuple_["unc_vtx_ele_track_zbravo"];
    //double* pos_bravo = tuple_["unc_vtx_pos_track_zbravo"];
    //double* recon_z = tuple_["unc_vtx_z"];

    //ele
    double* ele_zbalpha = new double {999.9};
    tuple_["unc_vtx_ele_track_zbravoalpha"] = ele_zbalpha;
    newtree_->Branch("unc_vtx_ele_track_zbravoalpha",tuple_["unc_vtx_ele_track_zbravoalpha"],"unc_vtx_ele_track_zbravoalpha/D");  
    new_variables_["unc_vtx_ele_track_zbravoalpha"] = ele_zbalpha;
    double z0_correction = 0.0; //Details of this not clear yet 04/24/23
    std::function<double()> calculateZbravoalpha_ele = [&,slope]()->double{
        if(*tuple_["unc_vtx_ele_track_zbravo"] > -z0_correction){
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_zbravo"]+z0_correction)/slope)) );
        }
        else{
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_zbravo"]+z0_correction)/(-1.0*slope))) );
        }
    };
    functions_["unc_vtx_ele_track_zbravoalpha"] = calculateZbravoalpha_ele;

    //pos
    double* pos_zbalpha = new double {999.9};
    tuple_["unc_vtx_pos_track_zbravoalpha"] = pos_zbalpha;
    newtree_->Branch("unc_vtx_pos_track_zbravoalpha",tuple_["unc_vtx_pos_track_zbravoalpha"],"unc_vtx_pos_track_zbravoalpha/D");  
    new_variables_["unc_vtx_pos_track_zbravoalpha"] = pos_zbalpha;
    std::function<double()> calculateZbravoalpha_pos = [&,slope]()->double{
        if(*tuple_["unc_vtx_pos_track_zbravo"] > -z0_correction){
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_zbravo"]+z0_correction)/slope)) );
        }
        else{
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_zbravo"]+z0_correction)/(-1.0*slope))) );
        }
    };
    functions_["unc_vtx_pos_track_zbravoalpha"] = calculateZbravoalpha_pos;

}

//Need to parameterize this transformation
void SimpAnaTTree::addVariableZbravo(){
    std::cout << "[MutableTTree]::addVariableZbravo" << std::endl;
    double* ele_z0 = tuple_["unc_vtx_ele_track_z0"];
    double* pos_z0 = tuple_["unc_vtx_pos_track_z0"];
    double* recon_z = tuple_["unc_vtx_z"];

    //ele
    double* ele_zbravo = new double {999.9};
    tuple_["unc_vtx_ele_track_zbravo"] = ele_zbravo;
    newtree_->Branch("unc_vtx_ele_track_zbravo",tuple_["unc_vtx_ele_track_zbravo"],"unc_vtx_ele_track_zbravo/D");  
    new_variables_["unc_vtx_ele_track_zbravo"] = ele_zbravo;
    std::function<double()> calculateZbravo_ele = [&]()->double{
        if(*tuple_["unc_vtx_ele_track_tanLambda"] > 0.0){
            return *tuple_["unc_vtx_ele_track_z0"] - (-0.039151**tuple_["unc_vtx_z"] - 0.031282);
        }
        else{
            return *tuple_["unc_vtx_ele_track_z0"] - (0.040086**tuple_["unc_vtx_z"] + 0.016186);
        }
    };
    functions_["unc_vtx_ele_track_zbravo"] = calculateZbravo_ele;

    //pos
    double* pos_zbravo = new double {999.9};
    tuple_["unc_vtx_pos_track_zbravo"] = pos_zbravo;
    newtree_->Branch("unc_vtx_pos_track_zbravo",tuple_["unc_vtx_pos_track_zbravo"],"unc_vtx_pos_track_zbravo/D");  
    new_variables_["unc_vtx_pos_track_zbravo"] = pos_zbravo;
    std::function<double()> calculateZbravo_pos = [&]()->double{
        if(*tuple_["unc_vtx_pos_track_tanLambda"] > 0.0){
            return *tuple_["unc_vtx_pos_track_z0"] - (-0.037899**tuple_["unc_vtx_z"] - 0.0094);
        }
        else{
            return *tuple_["unc_vtx_pos_track_z0"] - (0.039501**tuple_["unc_vtx_z"] + 0.004176);
        }
    };
    functions_["unc_vtx_pos_track_zbravo"] = calculateZbravo_pos;
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

void SimpAnaTTree::addVariableZalpha(double slope){
    std::cout << "[MutableTTree]::addVariableZalpha with slope " << slope << std::endl;

    //Create branch to hold new variable
    double* ele_zalpha = new double{999.9};
    tuple_["unc_vtx_ele_track_zalpha"] = ele_zalpha;
    newtree_->Branch("unc_vtx_ele_track_zalpha",tuple_["unc_vtx_ele_track_zalpha"],"unc_vtx_ele_track_zalpha/D");  
    new_variables_["unc_vtx_ele_track_zalpha"] = ele_zalpha;

    //Define lambda function to calculate ele zalpha
    std::function<double()> calculateZalpha_ele = [&, slope]()->double{
        if(*tuple_["unc_vtx_ele_track_z0"] > 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/slope)) );
        else
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_ele_track_z0"])/(-1*slope)) ));
    };
    functions_["unc_vtx_ele_track_zalpha"] = calculateZalpha_ele;

    //Create branch to hold new variable
    double* pos_zalpha = new double{999.9};
    tuple_["unc_vtx_pos_track_zalpha"] = pos_zalpha;
    newtree_->Branch("unc_vtx_pos_track_zalpha",tuple_["unc_vtx_pos_track_zalpha"],"unc_vtx_pos_track_zalpha/D");  
    new_variables_["unc_vtx_pos_track_zalpha"] = pos_zalpha;

    //Define lambda function to calculate pos zalpha
    std::function<double()> calculateZalpha_pos = [&,slope]()->double{
        if(*tuple_["unc_vtx_pos_track_z0"] > 0.0)
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"])/slope)) );
        else
            return ( *tuple_["unc_vtx_z"] - (((*tuple_["unc_vtx_pos_track_z0"]))/(-1*slope)) );
    };
    functions_["unc_vtx_pos_track_zalpha"] = calculateZalpha_pos;
}

void SimpAnaTTree::addVariableIsolationZ0Error(){
    double* ele_iso_z0err = new double{9999.9};
    tuple_["unc_vtx_ele_iso_z0err"] = ele_iso_z0err;
    newtree_->Branch("unc_vtx_ele_iso_z0err", tuple_["unc_vtx_ele_iso_z0err"], "unc_vtx_ele_iso_z0err/D");
    new_variables_["unc_vtx_ele_iso_z0err"] = ele_iso_z0err;

    std::function<double()> calculateIsolationZ0Error_ele = [&]()->double{
        return 2.0* *tuple_["unc_vtx_ele_track_L1_isolation"] / *tuple_["unc_vtx_ele_track_z0Err"];
    };
    functions_["unc_vtx_ele_iso_z0err"] = calculateIsolationZ0Error_ele;

    double* pos_iso_z0err = new double{9999.9};
    tuple_["unc_vtx_pos_iso_z0err"] = pos_iso_z0err;
    newtree_->Branch("unc_vtx_pos_iso_z0err", tuple_["unc_vtx_pos_iso_z0err"], "unc_vtx_pos_iso_z0err/D");
    new_variables_["unc_vtx_pos_iso_z0err"] = pos_iso_z0err;

    std::function<double()> calculateIsolationZ0Error_pos = [&]()->double{
        return 2.0* *tuple_["unc_vtx_pos_track_L1_isolation"] / *tuple_["unc_vtx_pos_track_z0Err"];
    };
    functions_["unc_vtx_pos_iso_z0err"] = calculateIsolationZ0Error_pos;
}

void SimpAnaTTree::addVariableZ0vsZ0Error(){
    double* ele_z0_z0err = new double{9999.9};
    tuple_["unc_vtx_ele_z0_z0err"] = ele_z0_z0err;
    newtree_->Branch("unc_vtx_ele_z0_z0err", tuple_["unc_vtx_ele_z0_z0err"], "unc_vtx_ele_z0_z0err/D");
    new_variables_["unc_vtx_ele_z0_z0err"] = ele_z0_z0err;

    std::function<double()> calculate_ratio_Z0_Z0Error_ele = [&]()->double{
        return std::abs(*tuple_["unc_vtx_ele_track_z0"])/ *tuple_["unc_vtx_ele_track_z0Err"];
    };
    functions_["unc_vtx_ele_z0_z0err"] = calculate_ratio_Z0_Z0Error_ele;

    double* pos_z0_z0err = new double{9999.9};
    tuple_["unc_vtx_pos_z0_z0err"] = pos_z0_z0err;
    newtree_->Branch("unc_vtx_pos_z0_z0err", tuple_["unc_vtx_pos_z0_z0err"], "unc_vtx_pos_z0_z0err/D");
    new_variables_["unc_vtx_pos_z0_z0err"] = pos_z0_z0err;

    std::function<double()> calculate_ratio_Z0_Z0Error_pos = [&]()->double{
        return std::abs(*tuple_["unc_vtx_pos_track_z0"])/ *tuple_["unc_vtx_pos_track_z0Err"];
    };
    functions_["unc_vtx_pos_z0_z0err"] = calculate_ratio_Z0_Z0Error_pos;
}

void SimpAnaTTree::addVariableIsolationCut(){
    std::cout << "[SimpAnaTTree]::adding variable Isolation Cut " << std::endl;
    double* ele_isolation = new double{9999.9};
    tuple_["unc_vtx_ele_isolation_cut"] = ele_isolation;
    newtree_->Branch("unc_vtx_ele_isolation_cut", tuple_["unc_vtx_ele_isolation_cut"], "unc_vtx_ele_isolation_cut/D");
    new_variables_["unc_vtx_ele_isolation_cut"] = ele_isolation;

    std::function<double()> calculateIsolation_ele = [&]()->double{

        return ( (2.0* *tuple_["unc_vtx_ele_track_L1_isolation"] / *tuple_["unc_vtx_ele_track_z0Err"]) - (std::abs(*tuple_["unc_vtx_ele_track_z0"])/ *tuple_["unc_vtx_ele_track_z0Err"]) );
    };
    functions_["unc_vtx_ele_isolation_cut"] = calculateIsolation_ele;

    double* pos_isolation = new double{9999.9};
    tuple_["unc_vtx_pos_isolation_cut"] = pos_isolation;
    newtree_->Branch("unc_vtx_pos_isolation_cut", tuple_["unc_vtx_pos_isolation_cut"], "unc_vtx_pos_isolation_cut/D");
    new_variables_["unc_vtx_pos_isolation_cut"] = pos_isolation;

    std::function<double()> calculateIsolation_pos = [&]()->double{

        return ( (2.0* *tuple_["unc_vtx_pos_track_L1_isolation"] / *tuple_["unc_vtx_pos_track_z0Err"]) - (std::abs(*tuple_["unc_vtx_pos_track_z0"])/ *tuple_["unc_vtx_pos_track_z0Err"]) );
    };
    functions_["unc_vtx_pos_isolation_cut"] = calculateIsolation_pos;
}

