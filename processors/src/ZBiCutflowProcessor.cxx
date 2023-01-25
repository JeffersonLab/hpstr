#include "ZBiCutflowProcessor.h"
//#include <FlatTupleMaker.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>

ZBiCutflowProcessor::ZBiCutflowProcessor(const std::string& name, Process& process) : Processor(name,process) {
    std::cout << "[ZBiCutflowPRocessor] Constructor()" << std::endl;

}

ZBiCutflowProcessor::~ZBiCutflowProcessor(){}

void ZBiCutflowProcessor::configure(const ParameterSet& parameters) {
    std::cout << "[ZBiCutflowProcessor] configure()" << std::endl;
    try
    {
        debug_ = parameters.getInteger("debug",debug_);
        cuts_cfgFile_ = parameters.getString("cuts_cfgFile",cuts_cfgFile_);
        signalFilename_ = parameters.getString("signalFilename", signalFilename_);
        tritrigFilename_ = parameters.getString("tritrigFilename", tritrigFilename_);
        vdSimFilename_ = parameters.getString("vdSimFilename", vdSimFilename_);
        vdMassMeV_ = parameters.getDouble("vdMassMeV", vdMassMeV_);
        //ApMassMeV_ = parameters.getDouble("ApMassMeV", ApMassMeV_);
        signalHistCfgFilename_ = parameters.getString("signalHistCfgFilename",signalHistCfgFilename_);
        outFileName_ = parameters.getString("outFileName",outFileName_);
        cutVariables_ = parameters.getVString("cutVariables", cutVariables_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void ZBiCutflowProcessor::initializeFlatTuple(TTree* tree, std::map<std::string, double*> &tuple_map){
    int nBr = tree->GetListOfBranches()->GetEntries();
    for(int iBr = 0; iBr < nBr; iBr++){
        TBranch *br = dynamic_cast<TBranch*>(tree->GetListOfBranches()->At(iBr)); 
        if(debug_)
            std::cout << "Reading variable: " << br->GetFullName() << std::endl;
        double* value = new double;
        std::string varname = (std::string)br->GetFullName();
        if(debug_)
            std::cout << "Storing varname " << varname << " in tuple map" << std::endl;
        tuple_map[varname] = value;
        tree->SetBranchAddress(varname.c_str(),tuple_map[varname]);
    }
}

void ZBiCutflowProcessor::initialize(std::string inFilename, std::string outFilename){
    std::cout << "[ZBiCutflowProcessor] Initialize" << std::endl;

    //init cut selector and load cuts from configuration file
    cutSelector_ = new IterativeCutSelector("iterativeCuts",cuts_cfgFile_);
    cutSelector_->setDebug(debug_);
    cutSelector_->LoadSelection();
    //Get pointer to map of cuts...allows cuts to be updated in this processor
    cuts_ = cutSelector_->getPointerToCuts();
    if(debug_){ 
        std::cout << "[ZBiCutflowProcessor] Cuts loaded:" << std::endl;
        cutSelector_->printCuts();
    }

    //Only allow cut to persist if the variable it cuts on is in the list of cutvariables
    for(cut_iter_ it=cuts_->begin(); it!=cuts_->end();){
        std::string cutname = it->first;
        std::string cutvariable = cutSelector_->getCutVar(cutname);
        if(debug_) std::cout << "Checking that variable " << cutvariable << " exists for cut " << cutname << std::endl;
        bool found = false;
        for(std::vector<std::string>::iterator iit=cutVariables_.begin(); iit !=cutVariables_.end(); iit++){
            if((std::string)*iit == cutvariable){
                if(debug_) std::cout << "Found " << *iit << " in list of cut variables" << std::endl;
                found = true;
                break;
            }
        }
        if(!found){
            std::cout << "[ZBiCutflowProcessor] WARNING::Cut variable corresponding to " << cutname << " does not exist!" << std::endl;
            std::cout << "Removing " << cutname << " from list of cuts." << std::endl;
            it = cuts_->erase(it);
        }
        else
            ++it;
    }

    if(debug_){ 
        std::cout << "[ZBiCutflowProcessor] Cuts loaded after filtering:" << std::endl;
        cutSelector_->printCuts();
    }

    //mc scaling... TODO: make configurable 
    double Lumi =10.7; //pb-1
    mcScale_["tritrig"] = 1.416e9*Lumi/(50000*9853);
    mcScale_["wab"] = 0.1985e12*Lumi/(100000*9966);
    mcScale_["rad"] = 66.36e6*Lumi/(10000*9959);
    mcScale_["rad_slic"] = 66.36e6*Lumi/(10000*9959);

    //Initialize output file
    outFile_ = new TFile(outFileName_.c_str(),"RECREATE");

    //Initialize Signal tuple histograms for each variable
    signalHistos_ = new ZBiHistos("signal");
    signalHistos_->debugMode(debug_);
    signalHistos_->loadHistoConfig(signalHistCfgFilename_);
    signalHistos_->DefineHistos();
    if(debug_)
        signalHistos_->printHistos1d();

    //Initialize input signal tuple
    if(debug_) std::cout << "Initializing input tuple from signal file" << std::endl;
    TFile* signalFile = new TFile(signalFilename_.c_str(),"READ");
    //TODO make this selection configurable
    signalTree_ = (TTree*)signalFile->Get("vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree");
    initializeFlatTuple(signalTree_, signal_tuple_);

    //Initialize input tritrig tuple
    if(debug_) std::cout << "Initializing input tuple from tritrig file" << std::endl;
    TFile* tritrigFile = new TFile(tritrigFilename_.c_str(),"READ");
    //TODO make this selection configurable
    tritrigTree_ = (TTree*)tritrigFile->Get("vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree");
    initializeFlatTuple(tritrigTree_, tritrig_tuple_);

    //Get the pretrigger vtx distribution ONLY NEED TO DO THIS ONCE!
    std::cout << "Getting pretrigger signal vtx distribution" << std::endl;
    vdSimZ_h_ = new TH1F("signal_vdSimZ_h_","signal_vdSimZ;true z_{vtx} [mm];events", 200, -50.3, 149.7);
    TFile* vdSimFile = new TFile(vdSimFilename_.c_str(), "READ");
    TH1F* vdSimZ_h = (TH1F*)vdSimFile->Get("mcAna/mcAna_mc625Z_h");
    for(int i=0; i < 201; i++){
        vdSimZ_h_->SetBinContent(i,vdSimZ_h->GetBinContent(i));
    }
    vdSimFile->Close();
    delete vdSimFile;

    //Initialize z vertex distribution histograms corresponding to each cut. These are used to calculate
    //expected signal and number of background events for each cut. 
    std::cout << "Initializing z vertex distributions for selected signal and tritrig" << std::endl;
    cutHistos_ = new ZBiHistos("cutHistos");
    cutHistos_->debugMode(debug_);
    for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
        std::string cutname = it->first;
        cutHistos_->addHisto1d("signal_vdSelZ_"+cutname+"_h","true z_{vtx} [mm]",200, -50.3, 149.7);
        cutHistos_->addHisto1d("tritrig_zVtx_"+cutname+"_h","unc z_{vtx} [mm]",150, -50.0, 100.0);
    }
    if(debug_){
        std::cout << "[ZBiCutflowProcessor] Cut Histos initialized:" << std::endl;
        cutHistos_->printHistos1d();
    }

    //Initialize various values (NEED TO BE CONFIGURABLE THROUGH SINGLE JSON)
    double mV_MeV = 55.0;
    double mAp_MeV = mV_MeV*(3/1.8);
    double radFrac = 0.07;
    double radAcc = 0.125;
    double massRes_MeV = 3.0;
    double dNdm = 372000.0;

    double m_pi = mAp_MeV/3.0;
    double alpha_D = 0.01;
    double m_l = 0.511;
    double f_pi = m_pi/(4.*M_PI);

    double lowMass = mV_MeV - 2.0*massRes_MeV/2.0;
    double highMass = mV_MeV + 2.0*massRes_MeV/2.0;

    //Fill signal variable histograms with initial values for each variable
    std::cout << "Filling initial signal histograms for each variable corresponding to cut selection" << std::endl;
    for(int e=0;  e < signalTree_->GetEntries(); e++){
        signalTree_->GetEntry(e);
        if(*signal_tuple_["unc_vtx_mass"]*1000.0 > highMass) continue;
        if(*signal_tuple_["unc_vtx_mass"]*1000.0 < lowMass) continue;
        //Loop over each signal variable
        //if cut exists for variable, fill histogram for that variable
        for(std::vector<std::string>::iterator it=cutVariables_.begin(); it !=cutVariables_.end(); it++){
            std::string cutvar = (std::string)*it;
            if(signalHistos_->get1dHistos().count("signal_"+cutvar+"_h")){
                signalHistos_->Fill1DHisto(cutvar+"_h",*signal_tuple_[cutvar]);
            }
            else{
                std::cout << "No Histogram named " << cutvar+"_h" << " exists for variable " << cutvar << std::endl; 
                abort();
            }
        }
    }

    //Get intial integral values for each signal cut variable histogram
    std::cout << "Getting intial signal histogram integrals to be used as reference for signal cutting" << std::endl;
    for(std::vector<std::string>::iterator it=cutVariables_.begin(); it !=cutVariables_.end(); it++){
        std::string cutvar = (std::string)*it;
        initialIntegrals_[cutvar] = signalHistos_->getIntegral("signal_"+cutvar+"_h");
    }
}

bool ZBiCutflowProcessor::process(){
    std::cout << "[ZBiCutflowProcessor] process()" << std::endl;

    //Initialize various values (NEED TO BE CONFIGURABLE THROUGH SINGLE JSON)
    double mV_MeV = 55.0;
    double mAp_MeV = mV_MeV*(3/1.8);
    double radFrac = 0.07;
    double radAcc = 0.125;
    double massRes_MeV = 3.0;
    double dNdm = 372000.0;

    double m_pi = mAp_MeV/3.0;
    double alpha_D = 0.01;
    double m_l = 0.511;
    double f_pi = m_pi/(4.*M_PI);

    double lowMass = mV_MeV - 2.0*massRes_MeV/2.0;
    double highMass = mV_MeV + 2.0*massRes_MeV/2.0;

    //Each iteration loops over the set of cuts being tested. The expected signal and background
    //rate is calculated given each cut independently. 
    //We calculate ZBi, the chosen metric of signal vs background, for each cut.
    //Within an iteration, the cut the gives the largest ZBi value is selected as the best_cut.
    //On the next iteration, the best_cut is applied to all events, then the process repeats.
    
    std::string iter_best_cut;
    std::string iter_best_cut_variable;
    double cutFraction = 0.01; //<- TODO: Make configurable
    for(int iteration = 1; iteration < 4; iteration ++){
        if(debug_) std::cout << "############### ITERATION " 
            << iteration << " #####################" << std::endl;

        //Reset histograms at the start of each iteration
        signalHistos_->resetHistograms1d();
        cutHistos_->resetHistograms1d();

        //Fill histograms for each signal variable being analyzed.
        for(int e=0;  e < signalTree_->GetEntries(); e++){
            signalTree_->GetEntry(e);
            //If event fails best cut from previous iteration, skip event
            if(iteration > 1){
                if(!cutSelector_->passCutGTorLT(iter_best_cut, *signal_tuple_[iter_best_cut_variable]))
                    continue;
            }
            //Cut events outside the chosen mass window
            if(*signal_tuple_["unc_vtx_mass"]*1000.0 > highMass) continue;
            if(*signal_tuple_["unc_vtx_mass"]*1000.0 < lowMass) continue;

            //Fill histogram for each cut variable
            for(std::vector<std::string>::iterator it=cutVariables_.begin(); it !=cutVariables_.end(); it++){
                std::string cutvar = (std::string)*it;
                signalHistos_->Fill1DHisto(cutvar+"_h",*signal_tuple_[cutvar]);
            }
        }

        //For each cut, find the variable value that cuts the configured fraction of
        //the original signal.
        for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
            std::string cutname = it->first;
            std::string cutvar = cutSelector_->getCutVar(cutname);
            if(debug_){
                std::cout << "Cutname: " << cutname << " | Cutvar: " << cutvar << std::endl;
            }
            bool isCutGT = cutSelector_->isCutGreaterThan(cutname);
            double cutvalue = signalHistos_->cutFractionOfIntegral("signal_"+cutvar+"_h", isCutGT, iteration*cutFraction, initialIntegrals_[cutvar]);
            cutSelector_->setCutValue(cutname, cutvalue);
            if(debug_) std::cout << "Cut " << cutname << " updated to value " << cutSelector_->getCut(cutname) << std::endl;
        }

        //Loop over each cut and build the background zvtx disribution. 
        //Fit zvtx with Gaus+Tail to determine zcut 
        for(int e=0;  e < tritrigTree_->GetEntries(); e++){
            tritrigTree_->GetEntry(e);
            //Only consider vertices within defined mass window centered on Vd mass
            if(*tritrig_tuple_["unc_vtx_mass"]*1000.0 > highMass) continue;
            if(*tritrig_tuple_["unc_vtx_mass"]*1000.0 < lowMass) continue;
            //Apply best cut from previous iteration to all events
            if(iteration > 1){
                if(!cutSelector_->passCutGTorLT(iter_best_cut, *tritrig_tuple_[iter_best_cut_variable]))
                    continue;
            }

            for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
                std::string cutname = it->first;
                std::string cutvar = cutSelector_->getCutVar(cutname);
                if(!cutSelector_->passCutGTorLT(cutname, *tritrig_tuple_[cutvar]))
                    continue;
                //If event passes, fill zVtx distribution
                cutHistos_->Fill1DHisto("tritrig_zVtx_"+cutname+"_h",*tritrig_tuple_["unc_vtx_z"],1.0);
            }
        }
        
        //Find the zcut value for each cut by fitting the tail of the bkg zvtx distribution
        std::map<std::string, double> zcuts;
        for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
            std::string cutname = it->first;
            std::cout << cutname << std::endl;
            double zcut = cutHistos_->fitZTail("tritrig_zVtx_"+cutname+"_h",100.0); //<- TO DO: make
            //100.0 configurable
            zcuts[cutname] = zcut;
            if(debug_) std::cout << "Zcut for cut " << cutname << ": " << zcut << "[mm]" << std::endl;
        }
        
        //At this stage, for each cut, the zcut value has been found. 
        //Using this zcut, we count the background events, measure the signal zvt selection 
        //efficiency, and then calculate the expected signal
        std::map<std::string,double> scaled_backgrounds;
        std::map<std::string,double> n_offs;

        //Initialize map to hold the background rate
        for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
            std::string cutname = it->first;
            scaled_backgrounds[cutname] = 0.0;
            n_offs[cutname] = 0.0;
        }

        //Count the background rate for each cut
        for(int e=0;  e < tritrigTree_->GetEntries(); e++){
            tritrigTree_->GetEntry(e);
            if(*tritrig_tuple_["unc_vtx_mass"]*1000.0 > highMass) continue;
            if(*tritrig_tuple_["unc_vtx_mass"]*1000.0 < lowMass) continue;
            //Apply best cut from previous iteration to all events
            if(iteration > 1){
                if(!cutSelector_->passCutGTorLT(iter_best_cut, *tritrig_tuple_[iter_best_cut_variable]))
                    continue;
            }
            for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
                std::string cutname = it->first;
                std::string cutvar = cutSelector_->getCutVar(cutname);
                if(!cutSelector_->passCutGTorLT(cutname, *tritrig_tuple_[cutvar]))
                    continue;
                if(*tritrig_tuple_["unc_vtx_z"] < zcuts[cutname]) continue;
                scaled_backgrounds[cutname] += mcScale_["tritrig"];
                n_offs[cutname] += 1.0;
            }
        }

        //Make signal selected vtxZ distribution for each cut...
        //these selections are used to calculated NSig for each cut
        for(int e=0;  e < signalTree_->GetEntries(); e++){
            signalTree_->GetEntry(e);
            //Apply best cut from previous iteration
            if(iteration > 1){
                if(!cutSelector_->passCutGTorLT(iter_best_cut, *signal_tuple_[iter_best_cut_variable]))
                    continue;
            }
            //Cut events outside the chosen mass window
            if(*signal_tuple_["unc_vtx_mass"]*1000.0 > highMass) continue;
            if(*signal_tuple_["unc_vtx_mass"]*1000.0 < lowMass) continue;

            for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
                std::string cutname = it->first;
                std::string cutvar = cutSelector_->getCutVar(cutname);
                if(!cutSelector_->passCutGTorLT(cutname, *signal_tuple_[cutvar]))
                    continue;
                if(*signal_tuple_["unc_vtx_z"] < zcuts[cutname]) continue;
                cutHistos_->Fill1DHisto("signal_vdSelZ_"+cutname+"_h",*signal_tuple_["true_vtx_z"],1.0);
            }
        }

        //Store the best cut for this iteration
        std::map<std::string,std::pair<double,double>> iter_zbi_map;
        for(cut_iter_ it=cuts_->begin(); it!=cuts_->end(); it++){
            //Get the selected vd vtxz distribution
            //In order to make TEfficiency with vdSimZ_h and vdSelZ_h, 
            //vdSelZ_h must be cloned from vdSimZ_h
            std::string cutname = it->first;
            TH1F* vdSelZ_h = (TH1F*)cutHistos_->get1dHisto("cutHistos_signal_vdSelZ_"+cutname+"_h");
            TH1F* vdSelZ_h_clone = (TH1F*)vdSimZ_h_->Clone(("clone_cutHistos_signal_vdSelZ_"+cutname+"_h").c_str());
            for(int i=0; i<201; i++){
                vdSelZ_h_clone->SetBinContent(i,vdSelZ_h->GetBinContent(i));
            }
            //make efficiencies to get F(z)
            TEfficiency* effCalc_h = new TEfficiency(*vdSelZ_h_clone, *vdSimZ_h_);

            //perform NSig calculation for each of the two possible mesons and combine rates
            double logEps2 = -7.5;
            double eps2 = std::pow(10, logEps2);
            double eps = std::sqrt(eps2);
            double Nsig = 0.0;
            std::string mesons[2] = {"rho","phi"};
            for(int i =0; i < 2; i++){
                bool rho = false;
                bool phi = false;
                if(mesons[i] == "rho") rho = true;
                else phi = true;

                if(debug_) std::cout << "Calculating " << mesons[i] << " NSig for cut " 
                    << cutname << std::endl;
                
                double ctau = simpEqs_->getCtau(mAp_MeV,m_pi,mV_MeV,eps,alpha_D,f_pi,m_l,rho);
                double E_V = 1.35; //GeV
                double gcTau = ctau * simpEqs_->gamma(mV_MeV/1000.0,E_V);

                double effVtx = 0.0;
                for(int zbin =0; zbin < 201; zbin++){
                    double zz = vdSelZ_h_clone->GetBinLowEdge(zbin);
                    if(zz < zcuts[cutname]) continue;
                    effVtx += (TMath::Exp((-4.3-zz)/gcTau)/gcTau)*(effCalc_h->GetEfficiency(zbin) - effCalc_h->GetEfficiencyErrorLow(zbin))*vdSelZ_h_clone->GetBinWidth(zbin);
                }

                double tot_apProd = (3.*137/2.)*3.14159*(mAp_MeV*eps2*radFrac*dNdm)/radAcc;
                double br_Vpi_val = simpEqs_->br_Vpi(mAp_MeV,m_pi,mV_MeV,alpha_D,f_pi,rho,phi);
                double br_V_to_ee = 1.0;

                Nsig = Nsig + tot_apProd*effVtx*br_V_to_ee*br_Vpi_val;
            }

           double n_on = Nsig + scaled_backgrounds[cutname];
           double tau = 1./mcScale_["tritrig"];
           double n_off = n_offs[cutname];

           //calculate ZBi value
           double ZBi = calculateZBi(n_on, n_off, tau);
           if(debug_){
               std::cout << "Values for cut " << cutname << std::endl;
               std::cout << "Nsig: " << Nsig << std::endl;
               std::cout << "Scaled tritrig: " << scaled_backgrounds[cutname];
               std::cout << "n_on: " << n_on << std::endl;
               std::cout << "n_off: " << n_off << std::endl;
               std::cout << "tau: " << tau << std::endl;
               std::cout << "ZBi: " << ZBi << std::endl;
           }
           std::pair<double, double> value_pair;
           value_pair.first = (double)cutSelector_->getCut(cutname);
           value_pair.second = ZBi;
           iter_zbi_map[cutname] = value_pair;
           global_ZBi_map_[cutname].push_back(value_pair);

           //CLEAR POINTERS
           delete effCalc_h;
        }

        //Find the cut that results in the largest ZBi value
        double best_ZBi = -9999.9;
        std::string best_cutname;
        double best_cutvalue;
        for(std::map<std::string,std::pair<double,double>>::iterator it=iter_zbi_map.begin(); it!=iter_zbi_map.end(); it++){
            std::string cutname = it->first; 
            double cutvalue = it->second.first;
            double zbi = it->second.second;
            if(zbi > best_ZBi){
                best_ZBi = zbi;
                best_cutname = cutname;
                best_cutvalue = cutvalue;
            }
        }
        //Keep the cut that results in the largest ZBi value and apply that cut
        //to all events in the next iteration
        iter_best_cut = best_cutname;
        iter_best_cut_variable = cutSelector_->getCutVar(iter_best_cut);

        signalHistos_->writeHistos1d(outFile_,"signal_iter_"+std::to_string(iteration));
        cutHistos_->writeHistos1d(outFile_, "cutHistos_iter_"+std::to_string(iteration));

        if(debug_) std::cout << "Iteration " << iteration << ": Best cut " << best_cutname << " has value: " 
            << best_cutvalue << " with ZBi: " << best_ZBi << std::endl;
    }

    //After all iterations, find the single best cut (resulting in largest overall ZBi value)
    double final_best_ZBi = -9999.9;
    std::string final_best_cutname;
    double final_best_cutvalue;

    //Find the largest ZBI value for each cut
    std::map<std::string,std::pair<double,double>> cut_best_zbi_map;
    for(std::map<std::string,std::vector<std::pair<double, double>>>::iterator it=global_ZBi_map_.begin(); it != global_ZBi_map_.end(); it++){
        std::string cutname = it->first; 
        std::vector<std::pair<double,double>> cut_pairs = it->second;

        double cut_best_zbi = -9999.9;
        std::pair<double,double> cut_best_pair;
        for(std::vector<std::pair<double,double>>::iterator itt=cut_pairs.begin(); itt!=cut_pairs.end(); itt++){
            if(itt->second > cut_best_zbi){
                cut_best_pair = *itt;
            }
        }
        cut_best_zbi_map[cutname] = cut_best_pair;
        if(debug_) std::cout << "Largest ZBi for cut " << cutname << " is " << cut_best_pair.second << " with cutvalue " << cut_best_pair.first << std::endl;

        if(cut_best_pair.second > final_best_ZBi){
            final_best_ZBi = cut_best_pair.second;
            final_best_cutvalue = cut_best_pair.first;
            final_best_cutname = cutname;
        }
    }

    if(debug_) std::cout << "Final best cut " << final_best_cutname << " has value: " 
        << final_best_cutvalue << " with ZBi: " << final_best_ZBi << std::endl;
}

void ZBiCutflowProcessor::printZBiMatrix(){
    typedef std::map<std::string,std::vector<std::pair<double,double>>>::iterator iter;
    for(iter it = global_ZBi_map_.begin(); it != global_ZBi_map_.end(); it++){
        std::cout << '\n';
        std::cout << it->first << ": ";
        for(std::vector<std::pair<double,double>>::iterator vec_it = it->second.begin(); vec_it != it->second.end(); vec_it++){
            double cutvalue = vec_it->first;
            double zbi = vec_it->second;
            std::cout << " |" << cutvalue << "," << zbi << "| ";
        }
        std::cout << '\n';
    }
}

void ZBiCutflowProcessor::finalize() {
    std::cout << "[ZBiCutflowProcessor] finalize()" << std::endl;
    printZBiMatrix();
    //outFile_->cd();
    //signalHistos_->saveHistos(outFile_,"signal");
    //outFile_->Close();
}

double ZBiCutflowProcessor::calculateZBi(double n_on, double n_off, double tau){
    double P_Bi = TMath::BetaIncomplete(1./(1.+tau),n_on,n_off+1);
    double Z_Bi = std::pow(2,0.5)*TMath::ErfInverse(1-2*P_Bi);
    return Z_Bi;
}

bool ZBiCutflowProcessor::doesCutVariableExist(std::string cutvariable){
    //Check that variable being used to cut actually exists in the signal data...
    //for(std::map<std::string,double*>::iterator it=signal_tuple_.begin(); it != signal_tuple.end(); it++){
    bool exists = true;

    if(signal_tuple_.find(cutvariable) == signal_tuple_.end()){
        std::cout << "[ZBiCutflowProcessor] WARNING::Cut variable " << cutvariable << " does not exist in the signal data" << std::endl;
        std::cout << "Removing " << cutvariable << " from list of cut variables." << std::endl;
        bool exists = false;
    }
    if(tritrig_tuple_.find(cutvariable) == tritrig_tuple_.end()){
        std::cout << "[ZBiCutflowProcessor] WARNING::Cut variable " << cutvariable << " does not exist in the tritrig data" << std::endl;
        std::cout << "Removing " << cutvariable << " from list of cut variables." << std::endl;
        bool exists = false;
    }
    if(exists == false){
        auto itr = std::find(cutVariables_.begin(), cutVariables_.end(), cutvariable);
        if (itr != cutVariables_.end()) cutVariables_.erase(itr);
    }
    return exists;
}

DECLARE_PROCESSOR(ZBiCutflowProcessor);
