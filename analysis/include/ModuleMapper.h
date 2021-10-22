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
  
  //TODO, clean up?
  ~ModuleMapper();
  
  std::string getHwFromString(const std::string& key)  {return string_to_hw[key];};
  std::string getSwFromString(const std::string& key)  {return string_to_sw[key];}; 
  std::string getHwFromSw    (const std::string& key)  {return sw_to_hw[key];};
  std::string getSwFromHw    (const std::string& key)  {return hw_to_sw[key];};
  std::string getStringFromHw(const std::string& key)  {return hw_to_string[key];};
  std::string getStringFromSw(const std::string& key)  {return sw_to_string[key];};
  

  std::vector<std::string> getHybridStrings();
  //get list of string modules
  void getStrings (std::vector<std::string>& strings) {
    for (strmap_it it = string_to_hw.begin(); it!= string_to_hw.end(); ++it)
      strings.push_back(it->first);
  }
  
  //get list of hw names
  void getHws (std::vector<std::string>& hws) {
    for (strmap_it it = hw_to_string.begin(); it!= hw_to_string.end(); ++it)
      hws.push_back(it->first);
  }

  //get list of sw names
  void getSws (std::vector<std::string>& sws) { 
    for (strmap_it it = hw_to_string.begin(); it!= hw_to_string.end(); ++it)
      sws.push_back(it->first);
  }

  //Build global svt id map for all FebHybrid combinations
  std::map<std::string, std::map<int,int>> buildChannelSvtIDMap();

  //Return global svt id for channel by providing local channel number and F<n>H<m> of channel
  int getSvtIDFromHWChannel(int channel, std::string hwTag, std::map<std::string,std::map<int,int>> svtid_map);  

  //Used to generate apv channel map and read in thresholds from database
  //formatted file
  void buildApvChannelMap();
  void ReadThresholdsFile(std::string filename);
  std::pair<std::string,int> findApvChannel(std::string feb, std::string hybrid, int channel);
  int getThresholdValue(std::string feb, std::string hybrid, int channel);
  
  //TODO Bidirectional maps could be used

 private:

  int year_{2019};
  
  std::map<std::string, std::string> hw_to_sw;
  std::map<std::string, std::string> sw_to_hw;
  
  std::map<std::string,std::string>  hw_to_string;
  std::map<std::string,std::string>  string_to_hw;
  
  std::map<std::string,std::string>  sw_to_string;
  std::map<std::string,std::string>  string_to_sw;
  
  typedef std::map<std::string,std::string>::iterator strmap_it;

  std::map<std::string,std::map<std::string,std::vector<int>>> apvChannelMap_;
  std::map<std::string, std::vector<int>> thresholdsIn_;
};

#endif //_MODULE_MAPPER_H_
