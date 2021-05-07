#include "VtxAnaHistos.h"
#include <math.h>
#include "TMath.h"

void VtxAnaHistos::Define1DHistos() {

}

void VtxAnaHistos::FillTSData(TSData* tsData, float weight ){
	if(tsData->prescaled.Single_0_Top == true) Fill1DHisto("ts_h", 0, weight);
	if(tsData->prescaled.Single_1_Top == true) Fill1DHisto("ts_h", 1, weight);
	if(tsData->prescaled.Single_2_Top == true) Fill1DHisto("ts_h", 2, weight);
	if(tsData->prescaled.Single_3_Top == true) Fill1DHisto("ts_h", 3, weight);

	if(tsData->prescaled.Single_0_Bot == true) Fill1DHisto("ts_h", 4, weight);
	if(tsData->prescaled.Single_1_Bot == true) Fill1DHisto("ts_h", 5, weight);
	if(tsData->prescaled.Single_2_Bot == true) Fill1DHisto("ts_h", 6, weight);
	if(tsData->prescaled.Single_3_Bot == true) Fill1DHisto("ts_h", 7, weight);

	if(tsData->prescaled.Pair_0 == true) Fill1DHisto("ts_h", 8, weight);
	if(tsData->prescaled.Pair_1 == true) Fill1DHisto("ts_h", 9, weight);
	if(tsData->prescaled.Pair_2 == true) Fill1DHisto("ts_h", 10, weight);
	if(tsData->prescaled.Pair_3 == true) Fill1DHisto("ts_h", 11, weight);

	if(tsData->prescaled.LED == true) Fill1DHisto("ts_h", 12, weight);
	if(tsData->prescaled.Cosmic == true) Fill1DHisto("ts_h", 13, weight);
	if(tsData->prescaled.Hodoscope == true) Fill1DHisto("ts_h", 14, weight);
	if(tsData->prescaled.Pulser == true) Fill1DHisto("ts_h", 15, weight);

	if(tsData->prescaled.Mult_0 == true) Fill1DHisto("ts_h", 16, weight);
	if(tsData->prescaled.Mult_1 == true) Fill1DHisto("ts_h", 17, weight);

	if(tsData->prescaled.FEE_Top == true) Fill1DHisto("ts_h", 18, weight);
	if(tsData->prescaled.FEE_Bot == true) Fill1DHisto("ts_h", 19, weight);
}


void VtxAnaHistos::FillUnconstrainedV0s(std::vector<Vertex*> *vtxs, float weight ){
	std::shared_ptr<AnaHelpers> _ah =  std::make_shared<AnaHelpers>();

	int nVtxs = vtxs->size();
    Fill1DHisto("numUnconstrainedVertices_h", (float)nVtxs, weight);

    for(int i = 0; i < nVtxs; i++){
        Vertex* vtx = vtxs->at(i);

    	double x = vtx->getX();
    	double y = vtx->getY();
    	double z = vtx->getZ();

    	double chi2 = vtx->getChi2();
    	double ndf = 1.; // NDF = 2n - 3 for unconstrained vertices; for vertices constructed by two tracks, n = 2

        Particle* ele = nullptr;
        Particle* pos = nullptr;
        bool foundParts = _ah->GetParticlesFromVtx(vtx,ele,pos);
        if (!foundParts) {
            std::cout<<"VertexAnaProcessor::WARNING::Found vtx without ele/pos. Skip."<<std::endl;
        }

        Track trackEle = ele->getTrack();
        Track trackPos = pos->getTrack();

        double chi2Ele = trackEle.getChi2();
        double ndfEle = trackEle.getNdf();

        double chi2Pos = trackPos.getChi2();
        double ndfPos = trackPos.getNdf();

        Fill1DHisto("xUnconstrainedVertices_h", (float)x, weight);
        Fill1DHisto("yUnconstrainedVertices_h", (float)y, weight);
        Fill1DHisto("zUnconstrainedVertices_h", (float)z, weight);

        Fill2DHisto("xyUnconstrainedVertices_hh", (float)x, (float)y, weight);
        Fill2DHisto("yzUnconstrainedVertices_hh", (float)y, (float)z, weight);
        Fill2DHisto("zxUnconstrainedVertices_hh", (float)z, (float)x, weight);

        Fill1DHisto("chi2PerNdfUnconstrainedVertices_h", (float)chi2/ndf, weight);
        Fill1DHisto("chi2PerNdfEleUnconstrainedVertices_h", (float)chi2Ele/ndfEle, weight);
        Fill1DHisto("chi2PerNdfPosUnconstrainedVertices_h", (float)chi2Pos/ndfPos, weight);

        Fill1DHisto("logChi2PerNdfUnconstrainedVertices_h", (float)TMath::Log(chi2/ndf), weight);
        Fill1DHisto("logChi2PerNdfEleUnconstrainedVertices_h", (float)TMath::Log(chi2Ele/ndfEle), weight);
        Fill1DHisto("logChi2PerNdfPosUnconstrainedVertices_h", (float)TMath::Log(chi2Pos/ndfPos), weight);

        Fill2DHisto("logChi2PerNdfEleVsPosUnconstrainedVertices_hh", (float)TMath::Log(chi2Ele/ndfEle), (float)TMath::Log(chi2Pos/ndfPos), weight);
        Fill2DHisto("logChi2PerNdfVertexVsEleUnconstrainedVertices_hh", (float)TMath::Log(chi2/ndf), (float)TMath::Log(chi2Ele/ndfEle), weight);
        Fill2DHisto("logChi2PerNdfVertexVsPosUnconstrainedVertices_hh", (float)TMath::Log(chi2/ndf), (float)TMath::Log(chi2Pos/ndfPos), weight);
    }


}
