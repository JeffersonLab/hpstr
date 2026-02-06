#ifndef HISTOMANAGER_H
#define HISTOMANAGER_H

#include <map>
#include <string>
#include <vector>

#include "TDirectoryFile.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "json.hpp"

// for convenience
using json = nlohmann::json;

/**
 * @brief description
 *
 * details
 */
class HistoManager {
  public:
    /**
     * @brief default constructor
     *
     */
    HistoManager();

    /**
     * @brief Constructor
     *
     * @param inputName
     */
    HistoManager(const std::string& inputName);

    /** desfault destructor */
    virtual ~HistoManager();

    /**
     * @brief description
     *
     * @param str
     * @return TH3F*
     */
    TH3* get3dHisto(const std::string& str) { return histos3d[str]; }

    /**
     * @brief description
     *
     * @param str
     * @return TH2F*
     */
    TH2* get2dHisto(const std::string& str) { return histos2d[str]; }

    /**
     * @brief description
     *
     * @param str
     * @return TH1F*
     */
    TH1* get1dHisto(const std::string& str) { return histos1d[str]; }

    /**
     * @brief description
     *
     * @param name
     * @param xtitle
     * @param nbinsX
     * @param xmin
     * @param xmax
     * @return TH1F*
     */
    TH1F* plot1D(const std::string& name, const std::string& xtitle, int nbinsX, float xmin, float xmax);

    /**
     * @brief description
     *
     * @param name
     * @param xtitle
     * @param nbinsX
     * @param axisX
     * @return TH1F*
     */
    TH1F* plot1D(const std::string& name, const std::string& xtitle, int nbinsX, double* axisX);

    /**
     * @brief description
     *
     * @param name
     * @param xtitle
     * @param nbinsX
     * @param xmin
     * @param xmax
     * @param ytitle
     * @param nbinsY
     * @param ymin
     * @param ymax
     * @return TH2F*
     */
    TH2F* plot2D(std::string name, std::string xtitle, int nbinsX, float xmin, float xmax, std::string ytitle,
                 int nbinsY, float ymin, float ymax);

    /**
     * @brief description
     *
     * @param name
     * @param xtitle
     * @param nbinsX
     * @param axisX
     * @param ytitle
     * @param nbinsY
     * @param axisY
     * @return TH2F*
     */
    TH2F* plot2D(std::string name, std::string xtitle, int nbinsX, double* axisX, std::string ytitle, int nbinsY,
                 double* axisY);

    /**
     * @brief description
     *
     * @param name
     * @param xtitle
     * @param nbinsX
     * @param axisX
     * @param ytitle
     * @param nbinsY
     * @param axisY
     * @return TH2F*
     */
    TH2F* plot2D(std::string name, std::string xtitle, int nbinsX, const double* axisX, std::string ytitle, int nbinsY,
                 const double* axisY);

    /**
     * @brief description
     *
     * @param name
     * @param xtitle
     * @param nbinsX
     * @param axisX
     * @param ytitle
     * @param nbinsY
     * @param ymin
     * @param ymax
     * @return TH2F*
     */
    TH2F* plot2D(std::string name, std::string xtitle, int nbinsX, double* axisX, std::string ytitle, int nbinsY,
                 float ymin, float ymax);

    /**
     * @brief description
     *
     * @param name
     * @param xtitle
     * @param nbinsX
     * @param xmin
     * @param xmax
     * @param ytitle
     * @param nbinsY
     * @param ymin
     * @param ymax
     * @param ztitle
     * @param nbinsZ
     * @param zmin
     * @param zmax
     * @return TH3F*
     */
    TH3F* plot3D(std::string name, std::string xtitle, int nbinsX, float xmin, float xmax, std::string ytitle,
                 int nbinsY, float ymin, float ymax, std::string ztitle, int nbinsZ, float zmin, float zmax);

    /**
     * @brief description
     *
     * @param name
     * @param xtitle
     * @param nbinsX
     * @param axisX
     * @param ytitle
     * @param nbinsY
     * @param axisY
     * @param ztitle
     * @param nbinsZ
     * @param axisZ
     * @return TH3F*
     */
    TH3F* plot3D(std::string name, std::string xtitle, int nbinsX, double* axisX, std::string ytitle, int nbinsY,
                 double* axisY, std::string ztitle, int nbinsZ, double* axisZ);

    // Additional histograms that might be defined by hand
    /**
     * @brief define additional 3D histo by hand
     *
     */
    virtual void Define3DHistos() {};

    /**
     * @brief define additional 2D histo by hand
     *
     */
    virtual void Define2DHistos() {};

    /**
     * @brief define additional 1D histo by hand
     *
     */
    virtual void Define1DHistos() {};

    // Definition of histograms from json config
    /**
     * @brief Definition of histograms from json config
     *
     */
    virtual void DefineHistos();

    /**
     * @brief Definition of histograms from json config
     *
     * @param histoCopyNames
     * @param makeCopyJsonTag
     */
    virtual void DefineHistos(std::vector<std::string> histoCopyNames,
                              std::string makeCopyJsonTag = "default=single_copy");

    /**
     * @brief description
     *
     * @param histoName
     * @param value
     * @param weight
     */
    void Fill1DHisto(const std::string& histoName, float value, float weight = 1.);

    /**
     * @brief description
     *
     * @param histoName
     * @param valuex
     * @param valuey
     * @param weight
     */
    void Fill2DHisto(const std::string& histoName, float valuex, float valuey, float weight = 1.);

    /**
     * @brief description
     *
     * @param histoName
     * @param valuex
     * @param valuey
     * @param valuez
     * @param weight
     */
    void Fill3DHisto(const std::string& histoName, float valuex, float valuey, float valuez, float weight = 1.);

    /**
     * @brief Get histograms from input file
     *
     * @param inFile
     * @param name
     * @param folder
     */
    virtual void GetHistosFromFile(TFile* inFile, const std::string& name, const std::string& folder = "");

    /**
     * @brief save histograms
     *
     * @param outF
     * @param folder
     */
    virtual void saveHistos(TFile* outF = nullptr, std::string folder = "");

    /**
     * @brief load histogram config
     *
     * @param histoConfigFile
     */
    virtual void loadHistoConfig(const std::string histoConfigFile);

    /**
     * @brief description
     *
     */
    virtual void sumw2();

    /**
     * @brief description
     *
     */
    virtual void Clear();

    /**
     * @brief get name
     *
     * @return std::string
     */
    virtual std::string getName() { return m_name; }

    /**
     * @brief set debug
     *
     * @param debug
     */
    void debugMode(bool debug) { debug_ = debug; }

    std::vector<std::string> histos1dNamesfromTFile;  //!< description
    std::vector<std::string> histos2dNamesfromTFile;  //!< description
    std::vector<std::string> histos1dNamesfromJson;   //!< description

  protected:
    std::string m_name;                                  //!< description
    std::map<std::string, std::vector<double>> Axes;     //!< description
    std::map<std::string, TH1*> histos1d;                //!< description
    typedef std::map<std::string, TH1*>::iterator it1d;  //!< description
    std::map<std::string, TH2*> histos2d;                //!< description
    typedef std::map<std::string, TH2*>::iterator it2d;  //!< description
    std::map<std::string, TH3*> histos3d;                //!< description
    typedef std::map<std::string, TH3*>::iterator it3d;  //!< description

    bool debug_{false};           //!< description
    json _h_configs;              //!< description
    int maxWarnings_{10};         //!< description
    int printWarnings_{0};        //!< description
    bool doPrintWarnings_{true};  //!< description
};

#endif
