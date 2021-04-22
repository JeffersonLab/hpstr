#include "TSAnaHistos.h"
#include <math.h>
#include "TMath.h"

void TSAnaHistos::Define1DHistos() {

}

void TSAnaHistos::FillTSData(TSData* tsData, float weight ){
	int numSingles2 = 0;
	int numSingles3 = 0;

	if(tsData->prescaled.Single_0_Top == true) Fill1DHisto("ts_h", 0, weight);
	if(tsData->prescaled.Single_1_Top == true) Fill1DHisto("ts_h", 1, weight);
	if(tsData->prescaled.Single_2_Top == true) {
		Fill1DHisto("ts_h", 2, weight);
		numSingles2++;
	}
	if(tsData->prescaled.Single_3_Top == true) {
		Fill1DHisto("ts_h", 3, weight);
		numSingles3++;
	}

	if(tsData->prescaled.Single_0_Bot == true) Fill1DHisto("ts_h", 4, weight);
	if(tsData->prescaled.Single_1_Bot == true) Fill1DHisto("ts_h", 5, weight);
	if(tsData->prescaled.Single_2_Bot == true) {
		Fill1DHisto("ts_h", 6, weight);
		numSingles2++;
	}
	if(tsData->prescaled.Single_3_Bot == true) {
		Fill1DHisto("ts_h", 7, weight);
		numSingles3++;
	}

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

	Fill2DHisto("ts_num_singles_hh", (double)numSingles3, (double)numSingles2, weight);
}
