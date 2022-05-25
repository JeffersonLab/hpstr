#ifndef __ECALTIMING_ANAPROCESSOR_H__
#define __ECALTIMING_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "TSData.h"
#include "VTPData.h"
#include "CalCluster.h"
#include "Track.h"
#include "Vertex.h"
#include "HistoManager.h"
#include "EcalTimingAnaHistos.h"


//ROOT
#include "Processor.h"
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"

//C++
#include <memory>

class EcalTimingAnaProcessor : public Processor {

public:
	EcalTimingAnaProcessor(const std::string& name, Process& process);
    ~EcalTimingAnaProcessor();
    virtual bool process(IEvent* ievent);

    virtual void initialize(TTree* tree);

    virtual void finalize();

    virtual void configure(const ParameterSet& parameters);

private:
    //Containers to hold histogrammer info
    EcalTimingAnaHistos* histos{nullptr};
    std::string  histCfgFilename_;

    //TODO Change this to be held from HPSEvent
    TTree* tree_{nullptr};
    TBranch* bvtpData_{nullptr};
    TBranch* btsData_{nullptr};
    TBranch* btrks_{nullptr};
    TBranch* becalClusters_{nullptr};
    TBranch* bvtxs_{nullptr};

    VTPData* vtpData_{};
    TSData* tsData_{};
    std::vector<Track*>  * trks_{};
    std::vector<CalCluster*> * ecalClusters_{};
    std::vector<Vertex*> * vtxs_{};

    std::string anaName_{"ecalTimingAna"};
    std::string vtpColl_{"VTPBank"};
    std::string tsColl_{"TSBank"};
    std::string trkColl_{"GBLTracks"};
    std::string ecalClusColl_{"EcalClustersCorr"};
    std::string vtxColl_{"Vertices"};

    //Debug level
    int debug_{0};

    double beamE_{4.55};

    TH1D* histDT;
    TH1D* histDTESumMinCut;
    TH1D* histDTOnePosNegTrack;

    TH1D* histDTCorr;
    TH1D* histDTESumMinCutCorr;
    TH1D* histDTOnePosNegTrackCorr;

    TH1D* histDTESumMinCutCorrGlobal;

    TH2D* histDTMean;
    TH2D* histDTCount;
    std::map< std::pair<int, int>, TH1D* > histMapDTMean;
    std::map< std::pair<int, int>, int > dtCountMap;

    TH2D* histDTMeanCorr;
    TH2D* histDTCountCorr;
    std::map< std::pair<int, int>, TH1D* > histMapDTMeanCorr;
    std::map< std::pair<int, int>, int > dtCountMapCorr;

    TF1 *ecalClusterTimeCutTop;
    TF1 *ecalClusterTimeCutBot;
    TH2D *histEcalClusterTimeCorection;
    const double ecalClusterTimeCorrectionArray[47][11] = {
    		{-1.09066, -0.970334, -1.05506, -1.13044, -0.487152, 0, -0.837955, 0.145682, -0.35811, -0.0431728, -0.150026},
    		{-1.21937, -1.05756, -1.12247, -0.956992, -1.00755, 0, -0.709516, -0.459094, 0.0298515, -0.206663, -0.193488},
    		{-1.13901, -1.24962, -1.25293, -1.09923, -1.06537, 0, -0.360048, -0.280961, -0.107345, -0.295748, -0.211943},
    		{-1.36864, -1.05825, -0.949506, -1.05523, -1.03211, 0, -0.394909, -0.089573, 0.0786045, -0.10277, -0.0104624},
    		{-1.01074, -1.02923, -0.881051, -0.994233, -0.979549, 0, -0.188704, 0.0800577, 0.112149, 0.108774, -0.016552},
    		{-1.21975, -1.00774, -1.05417, -0.904809, -2.8051, 0, -0.389807, 0.0808634, 0.109685, 0.227554, 0.15322},
    		{-0.891317, -1.07995, -1.10191, -0.848959, -1.02845, 0, -0.272871, 0.0718894, 0.0149332, 0.0626781, 0.162928},
    		{2.88921, -1.12878, -1.07344, -1.04204, -0.751673, 0, -0.196197, 0.191277, 0.0827836, 0.0989397, 0.0657134},
    		{-1.08986, -0.711483, -0.75313, 0, -0.810774, 0, -0.0433727, 0.129785, -0.0918517, 0.147716, 0.110351},
    		{-0.911442, -1.00728, -1.02656, -0.71958, -0.928958, 0, -0.0749954, 0.151433, 0.192447, 0.189893, 0.118486},
    		{-0.852022, -0.686833, -0.855167, -0.647203, -0.73114, 0, -0.163644, 0.243087, 0.255451, 0.158473, 0.203382},
    		{-0.734988, -0.549145, -0.589233, -0.686024, -0.805875, 0, 0.185282, 0.374012, 0.37952, 0.226183, 0.352746},
    		{-0.757174, -0.535768, -0.744713, -0.605716, -0.576215, 0, 0.284271, 0.492565, 0.465353, 0.198867, 0.38012},
    		{-0.442613, -0.464595, -0.712608, -0.518248, 0, 0, 0, 0.335605, 0.464888, 0.341146, 0.319195},
    		{-0.383155, -0.404719, -0.534829, -0.469441, 0, 0, 0, 0, 0.64755, 0.389309, 0.56343},
    		{-0.415687, -0.561202, -0.286004, -0.269523, 0, 0, 0, 0.468616, 0.568921, 0.435681, 0.32554},
    		{-0.498942, -0.360537, -0.374227, 0, 0, 0, 0, 0.376145, 0.609784, 0.601857, 0.367749},
    		{-0.126899, -0.198889, -2.14178, -0.180991, 0, 0, 0, 0.471769, 0.52529, 0.333599, 0.821118},
    		{-0.310626, -0.203272, -0.388405, 0.0738902, 0, 0, 0, 0.464376, 0.510446, 0.33651, 0.262677},
    		{-0.355125, -0.61117, -0.241119, 0.0849778, 0, 0, 0, 0.294194, -0.0152563, 0.133715, 0.517118},
    		{-0.726604, -0.659977, -0.287314, -0.442049, 0, 0, 0, 0.30941, 0.0975479, 0.219382, 0.525138},
    		{0.728535, 0.0714534, -0.505667, -0.248322, 0, 0, 0, -0.0244938, 0.30671, 0.129655, 0.130588},
    		{-0.544479, -0.395735, -0.0339376, -0.115949, -0.371984, 0, 0, 0.235343, 0.235519, 0.278519, 0.285632},
    		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    		{-0.560893, -0.565814, -0.816968, -0.390486, -0.273167, 0, 0.136198, 0.312664, 0.129043, 0.0265825, -0.210692},
    		{0.0632518, -1.79106, -0.425555, -0.285075, -0.307605, 0, 0.419778, 0.264138, 0.353268, 0.304361, -0.123938},
    		{-0.422913, -0.62886, -0.536426, 0.594766, -0.350165, 0, 0.334996, 0.014972, 0.334675, 0.791336, 0.469718},
    		{0.0399691, -0.994101, -0.676277, -0.154618, -0.157419, 0, 0.421418, 0.224204, 0.0571832, -0.00149841, 1.25386},
    		{-0.486649, -0.665337, -0.320019, 0, -0.312458, 0, 0.779009, 0.92438, 0.829694, 0.578834, 0.348822},
    		{-0.518174, -0.418915, -0.402935, -0.0783439, -0.0533507, 0, 0.69536, 0.946009, 0.607614, 0.631285, 0.432207},
    		{-0.535066, -0.517943, 0, -0.19659, -0.0499346, 0, 0.996294, 0.829907, 0.645982, 0.533458, 0.350898},
    		{-0.582412, -0.386486, -0.500583, -0.175097, 0.135081, 0, 0.988288, 0.887547, 0.690437, 0.439674, 0.289733},
    		{-0.607369, -0.656167, -0.356941, -0.115133, -0.111953, 0, 0.78082, 0.794674, 0.586217, 0.624473, 0.385247},
    		{-0.613844, -0.435476, -0.391244, -0.209419, 0.0243872, 0, 1.08353, 0.518795, 0.651421, 0.572915, 0.441314},
    		{-0.606243, -0.719387, -0.432793, -0.1817, -0.0483881, 0, 0.658207, 0.838074, 0.762824, 0.497, 0.517901},
    		{-0.675073, -0.373433, -0.292134, -0.230314, -0.134154, 0, 0.750077, 0.727624, 0.617309, 0.740206, 0.456196},
    		{-0.448351, -0.455739, -0.519575, -0.252334, -0.327719, 0, 0.711966, 0.774394, 0.851415, 0.544888, 0.704055},
    		{-0.66741, -0.566768, -0.392245, -0.319939, -0.353086, 0, 0.549666, 0.750973, 0.516427, 0.438487, 0.351892},
    		{-0.815464, -0.393753, -0.696219, -0.442839, -0.236547, 0, 0.701132, 0.636066, 0.50918, 0.459785, 0.470805},
    		{-0.65845, -0.577667, -0.472438, -0.355643, -0.175059, 0, 0.403427, 0.325825, 0.619052, 0.495058, 0.42785},
    		{-0.532989, -0.474055, -0.596887, -0.28132, -0.444292, 0, 0.245256, 0.689143, 0.559157, 0.473381, 0.359702},
    		{-0.475103, -0.844122, -0.26685, -0.528838, -0.228631, 0, -0.146541, 0.304914, 0.228263, 0.393334, 0.206889},
    		{-0.971546, -0.85168, -0.39128, -0.3318, -0.466535, 0, -0.0494039, 0.5153, 0.504734, 0.322953, 0.169487},
    		{-0.664231, -0.557598, -0.647641, -0.573209, -0.1914, 0, -0.486739, 0.113172, 0.189132, 0.235152, 0.376988},
    		{-0.862777, -0.415862, -0.313904, -0.711039, -0.309446, 0, -0.180307, 0.0868965, 0.263169, 0.248215, 0.193564},
    		{-0.344658, -0.126505, -1.05684, -0.423784, -0.782504, 0, 0.00014376, -0.133169, 0.251427, 0.399125, 0.213795},
    		{-0.450936, -0.219791, -0.176433, -0.676508, -0.328823, 0, -0.102569, -0.134226, 0.429199, 0.371737, -0.419751}
    };
};

#endif