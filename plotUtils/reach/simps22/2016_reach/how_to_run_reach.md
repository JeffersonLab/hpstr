### Origin of MC used in reach estimate ###
All MC samples listed below are re-reconstructed using PhysicsRun2016FullReconMC_KF_TrackClusterMatcher.lcsim, Run 7800, and detector 'HPS-PhysicsRun2016-Pass2' using hps-java-v5.1

Reach Lumi: 10.7 pb-1

1) Tritrig+Beam Original Recon: /mss/hallb/hps/production/PhysicsRun2016/pass4/npt224npt08n4pt3_npt33/recon/tritrig-beam_2500kBunches/ 2pt3/PhysRun2016-
Pass2/v4_5_0 (https://confluence.slac.stanford.edu/display/hpsg/pass4+for+2016+MC)
    1a) xsection: 1.416e9 pb
    1b) NFiles: 9853
    1c) N MadGraph Events per file: 50000

2) WAB+Beam Original Recon: /mss/hallb/hps/production/PhysicsRun2016/pass4/npt224npt08n4pt3_npt33/recon/wab-beam_2500kBunches/2pt3/PhysRun2016-
Pass2/v4_5_0/ (https://confluence.slac.stanford.edu/display/hpsg/pass4+for+2016+MC)
    2a) xsection: 0.1985e12 pb
    2b) NFiles: 9966
    2c) N MadGraph Events per file: 100000

3) Rad+Beam Original Recon: /mss/hallb/hps/production/PhysicsRun2016/pass4/npt224npt08n4pt3_npt33/recon/RAD_egs5-truth-beam_2500kBunches/2pt3/
PhysRun2016-Pass2/v4_5_0
    2a) xsection: 66.36e6 pb
    2b) NFiles: 9959
    2c) N MadGraph Events per file: 10000

4) SIMPs: /sdf/group/hps/users/bravo/run/simpPrep/output/recon/2pt3/
    4a) mA' = (3/1.8)*mVD
        4b) mpi_D = mVD/1.8
            4c) Target pos = -4.3 mm


### HOW ANALYSIS FILES ARE PRODUCED ###
1.) Tupilize Re-reconstructed files
    1a) Command: hpstr ./hpstr_config/kalSimpTuple_cfg.py -w KF -i <input.slcio> -o <tuple.root> -y 2016 -t 0
2.) Run vertex analysis in Signal Region (SR) and Control Region (CR)  
    2a) Command: hpstr ./hpstr_config/anaKalSimpTuple_cfg.py -w KF -i <tuple.root> -o <ana.root> -y 2-16 -t 0 -R <CR or SR>
    2b) Histo config: 'histoCfg' = hpstr/analysis/plotconfigs/tracking/vtxAnalysis_2016_simp_reach.json
    2c) mc histo config: 'mcHistoCfg' = hpstr/analysis/plotconfigs/mc/basicMC.json
    2d) Pre-selection: 'vtxSelectionjson' = hpstr/analysis/selections/vertexSelection_2016_simp_reach.json
    2e) Tight Selection Control Regions: recoana_kf.parameters["regionDefinitions"] = ['hpstr/analysis/selections/Tight_2016_simp_reach_CR.json','hpstr/analysis/selections/radMatchTight_2016_simp_reach_CR.json']
    2f) Tight Selection Signal Regions: recoana_kf.parameters["regionDefinitions"] = ['hpstr/analysis/selections/Tight_2016_simp_reach_SR.json','hpstr/analysis/selections/radMatchTight_2016_simp_reach_SR.json']

### HOW TO RUN REACH ESTIMATE ###
1.) Run makeComponents/makeRadFrac.py to get a polynomial expression for the Radiative Fraction
2.) Run makeComponents/makeTotRadAcc.py to get a polynomial expression for the Total Radiative Acceptance
3.) Run makeComponents/makeMassRes.py to get polynomial expression for mass resolution
4.) Run makeComponents/makeZcuts.py to get polynomial expression for Zcut
5.) Run hpstr vtxhProcess.py. Then run makeVtxResolution.py using the output file of vtxhProcess.py to get polynomial for Vertex Resolution
6.) Perform reach estimate using 'makeExpSignalSimps2016.py'. 
    6a) You must add the polynomial definitions calculated in steps 1-5 to this script.
    6b) You must define the proper MC scaling factors for 'tritrig' and 'wab' inside this script.
    6c) You must specify the dark_pion_decay_constant_ratio value (3 or 4pi) using flag -p
    6e) Example: python3 makeExpSignalSimps2016.py -o expSigRate_4Pi.root -x exContour_4Pi.txt -p 4pi

