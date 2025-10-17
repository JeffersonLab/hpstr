#include "TreeCutSelector.h"

TreeCutSelector::TreeCutSelector() : BaseSelector() {}

TreeCutSelector::TreeCutSelector(const std::string& inputName) {
    m_name = inputName;
    m_cfgFile = "";
    debug_ = false;
}

TreeCutSelector::TreeCutSelector(const std::string& inputName, const std::string& cfgFile) {
    m_name = inputName;
    m_cfgFile = cfgFile;
    debug_ = false;
}

TreeCutSelector::~TreeCutSelector() {}

bool TreeCutSelector::LoadSelection() {
    if (m_cfgFile.empty()) {
        std::cout << "ERROR TreeCutSelector::Configuration file not specified" << std::endl;
        return false;
    }

    std::ifstream i_file(m_cfgFile);
    i_file >> _h_selections;
    if (debug_) {
        for (auto& el : _h_selections.items()) std::cout << el.key() << " : " << el.value() << "\n";
    }

    for (auto cut : _h_selections.items()) {
        range_cuts[cut.key()].first.first = cut.value().at("min");
        range_cuts[cut.key()].first.second = cut.value().at("max");
        range_cuts[cut.key()].second = cut.value().at("id");
        labels[cut.key()] = cut.value().at("info");
        ncuts_++;
    }

    if (debug_) {
        for (range_cut_it it = range_cuts.begin(); it != range_cuts.end(); ++it) {
            std::cout << it->first << " [min:] =" << it->second.first.first << " [max:] =" << it->second.first.second
                      << " [id:] =" << it->second.second << std::endl;
        }
    }

    makeCutFlowHisto();
    return true;
}

void TreeCutSelector::makeCutFlowHisto() {
    h_cf_ =
        std::make_shared<TH1F>((m_name + "_cutflow").c_str(), (m_name + "_cutflow").c_str(), ncuts_ + 1, 0, ncuts_ + 1);
    h_cf_->GetXaxis()->SetCanExtend(true);
    h_cf_->Sumw2();
    h_cf_->GetXaxis()->SetBinLabel(1, "no-cuts");

    for (range_cut_it it = range_cuts.begin(); it != range_cuts.end(); ++it) {
        h_cf_->GetXaxis()->SetBinLabel((it->second.second + 2), labels[(it->first)].c_str());
    }
}

std::string TreeCutSelector::getCutVar(std::string cutname, bool forTree) {
    std::string var = "";
    if (forTree) {
        var = _h_selections.at(cutname).at("var");
    } else {
        var = cutname;
    }
    return var;
}

std::string TreeCutSelector::getCutString(std::string cutname) {
    std::string cutstring = "";
    if (!hasCut(cutname)) {
        std::cout << "ERROR " << cutname << " cut not implemented" << std::endl;
        return cutstring;
    }
    std::string var = getCutVar(cutname);
    if (range_cuts[cutname].first.first > -999.) {
        cutstring = var + " >= " + std::to_string(range_cuts[cutname].first.first);
    }
    if (range_cuts[cutname].first.second < 999.) {
        if (!cutstring.empty()) cutstring += " && ";
        cutstring += var + " <= " + std::to_string(range_cuts[cutname].first.second);
    }

    return cutstring;
}

void TreeCutSelector::filterCuts(std::vector<std::string> cut_variable_list) {
    // Loop over Test Cuts loaded in from json configuration
    // for(std::map<std::string, std::pair<double,int>>::iterator it = cuts_ptr->begin(); it != cuts_ptr->end(); it++){
    for (std::map<std::string, std::pair<std::pair<double, double>, int>>::iterator it = getPointerToCuts()->begin();
         it != getPointerToCuts()->end();) {
        std::string cutname = it->first;
        bool found = false;

        // Confirm that cut variable is in list of configurable cut variables
        for (std::vector<std::string>::iterator iit = cut_variable_list.begin(); iit != cut_variable_list.end();
             iit++) {
            if ((std::string)*iit == cutname) {
                found = true;
                break;
            }
        }

        // If Test Cut Variable does not exist, remove the Test Cut from the list of cuts
        if (!found) {
            it = getPointerToCuts()->erase(it);
        } else
            ++it;
    }
}