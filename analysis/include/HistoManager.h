#ifndef HISTOMANAGER_H
#define HISTOMANAGER_H

#include "TH3.h"
#include "TH2.h"
#include "TH1.h"
#include "TFile.h"
#include "TDirectoryFile.h"
#include <string>
#include <map>
#include <vector>

class HistoManager {

    public:
        HistoManager();
        HistoManager(const std::string& inputName);


        virtual ~HistoManager();

        TH3F* get3dHisto(const std::string& str) {
            return histos3d[str];
        }

        TH2F* get2dHisto(const std::string& str) {
            return histos2d[str];
        }

        TH1F* get1dHisto(const std::string& str) {
            return histos1d[str];
        }

        TH1F*  plot1D(const std::string& name,const std::string& xtitle, int nbinsX, float xmin, float xmax);

        TH1F*  plot1D(const std::string& name,const std::string& xtitle, int nbinsX, double* axisX);

        TH2F*  plot2D(std::string name,
                std::string xtitle, int nbinsX, float xmin, float xmax,
                std::string ytitle, int nbinsY, float ymin, float ymax);

        TH2F*  plot2D(std::string name,
                std::string xtitle, int nbinsX, double* axisX,
                std::string ytitle, int nbinsY, double* axisY);

        TH2F*  plot2D(std::string name,
                std::string xtitle, int nbinsX, const double* axisX,
                std::string ytitle, int nbinsY, const double* axisY);

        TH2F*  plot2D(std::string name,
                std::string xtitle, int nbinsX, double* axisX,
                std::string ytitle, int nbinsY, float ymin, float ymax);

        TH3F*  plot3D(std::string name,
                std::string xtitle, int nbinsX, float xmin, float xmax,
                std::string ytitle, int nbinsY, float ymin, float ymax,
                std::string ztitle, int nbinsZ, float zmin, float zmax);


        TH3F*  plot3D(std::string name,
                std::string xtitle, int nbinsX, double* axisX,
                std::string ytitle, int nbinsY, double* axisY,
                std::string ztitle, int nbinsZ, double* axisZ);


        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos(){};

        virtual void GetHistosFromFile(TFile* inFile, const std::string& name,const std::string& folder = "");

        virtual void saveHistos(TFile* outF = nullptr,std::string folder = "");

        virtual void sumw2();

        void debugMode(bool debug) {debug_ = debug;}

    protected:

        std::string m_name;

        std::map<std::string, std::vector<double> > Axes;

        std::map<std::string, TH1F*> histos1d;
        typedef std::map<std::string, TH1F*>::iterator it1d;

        std::map<std::string, TH2F*> histos2d;
        typedef std::map<std::string, TH2F*>::iterator it2d;

        std::map<std::string, TH3F*> histos3d;
        typedef std::map<std::string, TH3F*>::iterator it3d;

        bool debug_{false};

};


#endif
