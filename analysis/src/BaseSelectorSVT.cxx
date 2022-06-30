#include "BaseSelectorSVT.h"
#include <fstream>
#include <iostream>

BaseSelectorSVT::BaseSelectorSVT() { 
    BaseSelectorSVT("default");
    m_cfgFile = "";
    debug_ = false;
    passSelection = true;
}

BaseSelectorSVT::BaseSelectorSVT(const std::string& inputName) { 
    m_name = inputName;
    m_cfgFile = "";
    debug_ = false;
    passSelection = true;
}

BaseSelectorSVT::BaseSelectorSVT(const std::string& inputName, const std::string& cfgFile) { 
    m_name = inputName;
    m_cfgFile = cfgFile;
    debug_ = false;
    passSelection=true;
}

void BaseSelectorSVT::setCfgFile(const std::string& cfgFile) {
    m_cfgFile = cfgFile;
}

void BaseSelectorSVT::setDebug(bool val) { 
    debug_ = val;
}

BaseSelectorSVT::~BaseSelectorSVT(){}

bool BaseSelectorSVT::LoadSelection(){
    if (m_cfgFile.empty()) {
        std::cout<<"ERROR BaseSelector::Configuration File not specified";
        return false;
    }
    //os.environ['HPSTR_BASE']+"/analysis/selections/svtHit/"
    //std::cout<<"/sdf/home/r/rodwyer1/sw/hpstr/src/analysis/selections/svtHit/"+m_cfgFile<<std::endl;
    std::ifstream i_file("/sdf/home/r/rodwyer1/sw/hpstr/src/hpstr/analysis/selections/svtHit/"+m_cfgFile);
    //std::cout<<"Okay a little weird"<<std::endl;
    i_file >> _h_selections;
    if (debug_) {
        for (auto& el : _h_selections.items())
            std::cout<<el.key() << " : " << el.value() << "\n";
    }
    
    for (auto cut : _h_selections.items()) {
        cuts[cut.key()].first  = cut.value().at("cut");
        cuts[cut.key()].second = cut.value().at("id");
        labels[cut.key()] = cut.value().at("info");
        ncuts_++;
    }

    if (debug_) {
        for (cut_it it = cuts.begin(); it != cuts.end(); ++it) {
            std::cout<<it->first<<" [value:]=" <<it->second.first<<" [id:] ="<<it->second.second<<std::endl;
        }
    }

    makeCutFlowHisto();
    return true;
}
    
void BaseSelectorSVT::makeCutFlowHisto() {

    h_cf_ = std::make_shared<TH1F>((m_name+"_cutflow").c_str(),(m_name+"_cutflow").c_str(),ncuts_+1,0,ncuts_+1);
    h_cf_->GetXaxis()->SetCanExtend(true);
    h_cf_->Sumw2();
    h_cf_->GetXaxis()->SetBinLabel(1,"no-cuts");
    
    for (cut_it it = cuts.begin(); it != cuts.end(); ++it) {
        h_cf_->GetXaxis()->SetBinLabel((it->second.second+2),labels[(it->first)].c_str());
    }
}

//TODO Clean up logic
bool BaseSelectorSVT::passCutEq(const std::string& cutname, double val, double w) {
    
    if (hasCut(cutname)) {
        if (val != cuts[cutname].first) { 
            passSelection = passSelection && false;
            return false;
        }
        else {
            h_cf_->Fill((double)(cuts[cutname].second + 1), w);
            passSelection = passSelection && true;
        }
    }
    return true;
}


bool BaseSelectorSVT::passCutLt(const std::string& cutname, double val, double w) {
    
    if (hasCut(cutname)) {
        if (val > cuts[cutname].first) {
            passSelection = passSelection && false;
            return false;
        }
        else {
            h_cf_->Fill((double)(cuts[cutname].second + 1), w);
            passSelection = passSelection && true;
        }
    }
    return true;
}

bool BaseSelectorSVT::passCutGt(const std::string& cutname, double val, double w) {
    
    if (hasCut(cutname)) {
        if (val < cuts[cutname].first) {
            passSelection = passSelection && false;
            return false;
        }
        else {
            h_cf_->Fill((double)(cuts[cutname].second + 1), w);
            passSelection = passSelection && true;
        }
    }
    return true;
}


