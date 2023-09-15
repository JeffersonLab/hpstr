#ifndef  _MODULE_MAPPER_H_
#define  _MODULE_MAPPER_H_

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>


class ModuleMapper {

    public:
        ModuleMapper(const int year = 2019);

        /** \todo, clean up? */
        ~ModuleMapper();

        /**
         * @brief Get the Hw From String
         *
         * @param key
         * @return std::string
         */
        std::string getHwFromString(const std::string& key)  {return string_to_hw[key];};

        /**
         * @brief Get the Sw From String
         *
         * @param key
         * @return std::string
         */
        std::string getSwFromString(const std::string& key)  {return string_to_sw[key];}; 

        /**
         * @brief Get the Hw From Sw
         *
         * @param key
         * @return std::string
         */
        std::string getHwFromSw    (const std::string& key)  {return sw_to_hw[key];};

        /**
         * @brief Get the Sw From Hw
         *
         * @param key
         * @return std::string
         */
        std::string getSwFromHw    (const std::string& key)  {return hw_to_sw[key];};

        /**
         * @brief Get the String From Hw
         *
         * @param key
         * @return std::string
         */
        std::string getStringFromHw(const std::string& key)  {return hw_to_string[key];};

        /**
         * @brief Get the String From Sw
         *
         * @param key
         * @return std::string
         */
        std::string getStringFromSw(const std::string& key)  {return sw_to_string[key];};

        /**
         * @brief Get the Hybrid Strings
         *
         * @return std::vector<std::string>
         */
        std::vector<std::string> getHybridStrings();

        /**
         * @brief Get list of string modules
         *
         * @return strings
         */
        void getStrings (std::vector<std::string>& strings) {
            for (strmap_it it = string_to_hw.begin(); it!= string_to_hw.end(); ++it)
                strings.push_back(it->first);
        }

        /**
         * @brief Get list of hw names
         *
         * @return hws
         */
        void getHws (std::vector<std::string>& hws) {
            for (strmap_it it = hw_to_string.begin(); it!= hw_to_string.end(); ++it)
                hws.push_back(it->first);
        }

        /**
         * @brief Get list of sw names
         *
         * @return sws
         */
        void getSws (std::vector<std::string>& sws) { 
            for (strmap_it it = hw_to_string.begin(); it!= hw_to_string.end(); ++it)
                sws.push_back(it->first);
        }

        /**
         * @brief Build global svt id map for all FebHybrid combinations
         *
         * @return std::map<std::string, std::map<int,int>>
         */
        std::map<std::string, std::map<int,int>> buildChannelSvtIDMap();

        /**
         * @brief Return global svt id for channel by providing local channel number and F<n>H<m> of channel
         * 
         * @param channel 
         * @param hwTag 
         * @param svtid_map 
         * @return int 
         */
        int getSvtIDFromHWChannel(int channel, std::string hwTag, std::map<std::string,std::map<int,int>> svtid_map);  

        /**
         * @brief Used to generate apv channel map and read in thresholds from
         * database
         */
        void buildApvChannelMap();

        /**
         * @brief Used to read svt channel DAQ threshold values from daq thresholds
         * file
         *
         * @param filename
         */
        void ReadThresholdsFile(std::string filename);


        /** 
         * @brief get APV channel number given feb, hybrid, and channel number
         *
         * @param feb
         * @param hybrid
         * @param channel
         * @return std::pair<std::string,int>
         */
        std::pair<std::string,int> findApvChannel(std::string feb, std::string hybrid, int channel);

        /** 
         * @brief Get channel DAQ threshold
         *
         * @param feb
         * @param hybrid
         * @param channel
         * @return std::pair<std::string,int>
         */
        int getThresholdValue(std::string feb, std::string hybrid, int channel);

        //TODO Bidirectional maps could be used

    private:

        int year_{2019}; //!< description

        std::map<std::string, std::string> hw_to_sw; //!< description
        std::map<std::string, std::string> sw_to_hw; //!< description

        std::map<std::string,std::string>  hw_to_string; //!< description
        std::map<std::string,std::string>  string_to_hw; //!< description

        std::map<std::string,std::string>  sw_to_string; //!< description
        std::map<std::string,std::string>  string_to_sw; //!< description

        typedef std::map<std::string,std::string>::iterator strmap_it; //!< description

        std::map<std::string,std::map<std::string,std::vector<int>>> apvChannelMap_; //!< description
        std::map<std::string, std::vector<int>> thresholdsIn_; //!< description
};

#endif //_MODULE_MAPPER_H_
