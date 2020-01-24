#include "AnaHelpers.h"



std::string AnaHelpers::getFileName(std::string filePath, bool withExtension)
{
    char sep = '/';
    
    size_t i = filePath.rfind(sep, filePath.length());
    std::string baseFile="";

    if (i != std::string::npos) {
        baseFile = filePath.substr(i+1, filePath.length() - i);
    }
    
    if (withExtension)
        return baseFile;
    else {
        i = baseFile.find_last_of('.');
        return baseFile.substr(0,i);
    }
    
    return "";
}



AnaHelpers::AnaHelpers() {
    rotSvt.RotateY(SVT_ANGLE);
    
    rotSvt_sym =  new TMatrixDSym(3);
    
    (*rotSvt_sym)(0,0) = cos(SVT_ANGLE);
    (*rotSvt_sym)(0,1) = 0.;
    (*rotSvt_sym)(0,2) = sin(SVT_ANGLE);
    
    (*rotSvt_sym)(1,0) = 0.;
    (*rotSvt_sym)(1,1) = 1.;
    (*rotSvt_sym)(1,2) = 0.;
    
    (*rotSvt_sym)(1,0) = -sin(SVT_ANGLE);
    (*rotSvt_sym)(1,1) = 0.;
    (*rotSvt_sym)(1,2) = cos(SVT_ANGLE);
    
    
        
}

TVector3 AnaHelpers::rotateToSvtFrame(TVector3 v) {
    
    v.RotateY(SVT_ANGLE);
    return v;   
}

TVector3 AnaHelpers::rotationToSvtFrame(const TVector3&  v) {
    return rotSvt*v;
}
