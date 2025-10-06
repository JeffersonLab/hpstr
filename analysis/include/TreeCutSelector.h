#ifndef TREECUTSELECTOR_H
#define TREECUTSELECTOR_H

#include <fstream>
#include <iostream>

#include "BaseSelector.h"
#include "TTree.h"

class TreeCutSelector : public BaseSelector {
  public:
    TreeCutSelector();
    TreeCutSelector(const std::string& inputName);
    TreeCutSelector(const std::string& inputName, const std::string& cfgFile);

    virtual ~TreeCutSelector();

    bool LoadSelection();
    std::string getCutString(std::string cutname);
    std::string getCutVar(std::string cutname, bool forTree = false);

    /**
     * @brief description
     *
     */
    void makeCutFlowHisto();

    /**
     * @brief get pointer to the base class cuts
     *
     */
    std::map<std::string, std::pair<std::pair<double, double>, int>>* getPointerToCuts() { return &range_cuts; }

    std::pair<double, double> getCutRange(std::string cutname) {
        if (!hasCut(cutname)) {
            std::cout << "ERROR " << cutname << " cut not implemented" << std::endl;
            return std::make_pair(-999.9, 999.9);
        } else
            return range_cuts[cutname].first;
    }

    void printCuts() {
        for (range_cut_it it = range_cuts.begin(); it != range_cuts.end(); ++it) {
            std::cout << it->first << " [min:] =" << it->second.first.first << " [max:] =" << it->second.first.second
                      << " [id:] =" << it->second.second << std::endl;
        }
    }

    void setCutValue(std::string cutname, std::pair<double, double> value) {
        int id = range_cuts[cutname].second;
        range_cuts[cutname] = std::make_pair(value, id);
    }

    bool hasCut(const std::string& cutname) {
        if (range_cuts.find(cutname) != range_cuts.end())
            return true;
        else
            return false;
    }

  protected:
    typedef std::map<std::string, std::pair<std::pair<double, double>, int>>::iterator range_cut_it;  //!< description
    std::map<std::string, std::pair<std::pair<double, double>, int>> range_cuts;

  private:
    json _h_selections;                         //!< description
    std::string m_name{""};                     //!< description
    std::string m_cfgFile{""};                  //!< description
    std::map<std::string, std::string> labels;  //!< description
    int ncuts_{0};                              //!< description
    std::shared_ptr<TH1F> h_cf_;                //!< description
};

#endif  // TREECUTSELECTOR_H