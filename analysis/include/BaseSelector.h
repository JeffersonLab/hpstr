#ifndef BASESELECTOR_H
#define BASESELECTOR_H

#include <string>
#include <iostream>
#include <map>
#include <memory>

#include "TH1F.h"
#include "json.hpp"


//for convenience
using json = nlohmann::json;

class BaseSelector { 
    public: 
        BaseSelector();
        BaseSelector(const std::string& inputName);
        BaseSelector(const std::string& inputName, const std::string& cfgFile);
        virtual ~BaseSelector();

        void setCfgFile(const std::string& cfgFile);
        void setDebug(bool val);
        bool hasCut(const std::string&  cutname) { if (cuts.find(cutname) != cuts.end()) return true; else return false;}  
        bool LoadSelection();
        float getCut(const std::string& cutname) {if (!hasCut(cutname)) {std::cout<<"ERROR "<<cutname<<" cut not implemented"<<std::endl; return -999;} else return cuts[cutname].first;}
        std::map<std::string, std::pair<double,int> > getCuts(){return cuts;}
        void makeCutFlowHisto();
        std::shared_ptr<TH1F> getCutFlowHisto(){return h_cf_;};

        bool passCut(const std::string& cutname,double val,double weight){return true;};
        bool passCutEq(const std::string& cutname,double val,double weight);
        bool passCutLt(const std::string& cutname,double val,double weight);
        bool passCutGt(const std::string& cutname,double val,double weight);

        void clearSelector() { passSelection = true; }


        void clearSelector(){passSelection = true;}


    private:
        json _h_selections;
        std::string m_name{""};
        std::string m_cfgFile{""};
        //string: cutname
        //double: cutvalue
        //int   : cut id (for cut flow book-keeping)
        std::map<std::string,std::pair<double,int> > cuts;
        std::map<std::string,std::string > labels;

        bool debug_{false};
        int ncuts_{0};
        std::shared_ptr<TH1F> h_cf_;

        bool passSelection{false};

        typedef std::map<std::string,std::pair<double,int> >::iterator cut_it;
};

#endif
