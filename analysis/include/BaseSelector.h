#ifndef BASESELECTOR_H
#define BASESELECTOR_H

#include <string>
#include <iostream>
#include <map>
#include <memory>

#include "TH1F.h"
#include "json.hpp"


// for convenience
using json = nlohmann::json;

/**
 * @brief brief description
 * 
 * more details
 */
class BaseSelector { 
    public: 
        BaseSelector();
        BaseSelector(const std::string& inputName);
        BaseSelector(const std::string& inputName, const std::string& cfgFile);
        virtual ~BaseSelector();

        /**
         * @brief Set the Cfg File object
         * 
         * @param cfgFile 
         */
        void setCfgFile(const std::string& cfgFile);

        /**
         * @brief Set Debug
         * 
         * @param val 
         */
        void setDebug(bool val);

        /**
         * @brief description
         * 
         * @param cutname 
         * @return true 
         * @return false 
         */
        bool hasCut(const std::string& cutname) {
            if (cuts.find(cutname) != cuts.end()) return true;
            else return false;
        }

        /**
         * @brief description
         * 
         * @return true 
         * @return false 
         */
        bool LoadSelection();

        /**
         * @brief Get cut values ?
         * 
         * @param cutname 
         * @return float 
         */
        float getCut(const std::string& cutname) {
            if (!hasCut(cutname)) {
                std::cout << "ERROR " << cutname << " cut not implemented" << std::endl;
                return -999;
            } 
            else return cuts[cutname].first;
        }

        /**
         * @brief description
         * 
         * @return std::map<std::string, std::pair<double,int> > 
         */
        std::map<std::string, std::pair<double,int> > getCuts(){ return cuts; }

        /**
         * @brief description
         * 
         */
        void makeCutFlowHisto();
        
        /**
         * @brief Get the Cut Flow Histo
         * 
         * @return std::shared_ptr<TH1F> 
         */
        std::shared_ptr<TH1F> getCutFlowHisto(){ return h_cf_; };

        /**
         * @brief description
         * 
         * @param cutname 
         * @param val 
         * @param weight 
         * @return true 
         * @return false 
         */
        bool passCut(const std::string& cutname, double val, double weight){ return true; };

        /**
         * @brief description
         * 
         * @param cutname 
         * @param val 
         * @param weight 
         * @return true 
         * @return false 
         */
        bool passCutEq(const std::string& cutname, double val, double weight);

        /**
         * @brief description
         * 
         * @param cutname 
         * @param val 
         * @param weight 
         * @return true 
         * @return false 
         */
        bool passCutLt(const std::string& cutname, double val, double weight);

        /**
         * @brief description
         * 
         * @param cutname 
         * @param val 
         * @param weight 
         * @return true 
         * @return false 
         */
        bool passCutGt(const std::string& cutname, double val, double weight);

        /**
         * @brief description
         * 
         */
        void clearSelector() { passSelection = true; }


    private:
        json _h_selections; //!< description
        std::string m_name{""}; //!< description
        std::string m_cfgFile{""}; //!< description
        //string: cutname
        //double: cutvalue
        //int   : cut id (for cut flow book-keeping)
        std::map<std::string,std::pair<double, int>> cuts; //!< description
        std::map<std::string,std::string> labels; //!< description

        bool debug_{false}; //!< description
        int ncuts_{0}; //!< description
        std::shared_ptr<TH1F> h_cf_; //!< description

        bool passSelection{false}; //!< description

        typedef std::map<std::string, std::pair<double, int>>::iterator cut_it; //!< description
};

#endif
