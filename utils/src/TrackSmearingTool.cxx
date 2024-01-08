#include "TrackSmearingTool.h"
#include "TFile.h"

#include <stdexcept>

TrackSmearingTool::TrackSmearingTool(const std::string& smearingfile){
  smearingfile_ = std::make_shared<TFile>(smearingfile.c_str());
  if (!smearingfile_)
    throw std::invalid_argument( "Provided input smearing file does not exists");
};
