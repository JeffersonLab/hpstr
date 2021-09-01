#ifndef  _MODULE_MAPPER_H_
#define  _MODULE_MAPPER_H_

#include <map>
#include <vector>
#include <string>


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
    for (strmap_it it = hw_to_sw.begin(); it!= hw_to_sw.end(); ++it)
      hws.push_back(it->first);
  }

  //get list of sw names
  void getSws (std::vector<std::string>& sws) { 
    for (strmap_it it = sw_to_hw.begin(); it!= sw_to_hw.end(); ++it)
      sws.push_back(it->first);
  }

  std::map<std::string, std::map<int,int>> buildChannelSvtIDMap();

  int getSvtIDFromHWChannel(int channel, std::string hwTag, std::map<std::string,std::map<int,int>> svtid_map);  
  
  
  
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
    


};

#endif //_MODULE_MAPPER_H_
