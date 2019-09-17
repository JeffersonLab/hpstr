#ifndef  _MODULE_MAPPER_H_
#define  _MODULE_MAPPER_H_
#include <map>
#include <string>


class ModuleMapper {

 public:
  ModuleMapper();
  ModuleMapper(const int year) {year_ = year;};
  
  //TODO, clean up?
  ~ModuleMapper();

  
  //Bidirectional maps could be used

 private:

  int year_{2019};
  
  std::map<std::string, std::string> hw_to_sw;
  std::map<std::string, std::string> sw_to_hw;
  
  std::map<std::string,std::string>  hw_to_string;
  std::map<std::string,std::string>  string_to_hw;
  
  std::map<std::string,std::string>  sw_to_string;
  std::map<std::string,std::string>  string_to_sw;
  

};

#endif //_MODULE_MAPPER_H_
