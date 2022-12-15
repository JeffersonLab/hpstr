### Origin of MC used in reach estimate ###
All MC samples listed below are re-reconstructed using PhysicsRun2021MCRecon.lcsim, Run 14166, and detector 'HPS_Run2021Pass2FEE' using hps-java-v5.1

Reach Lumi: 168.1 pb-1

1) Tritrig+Beam Original Recon: https://confluence.slac.stanford.edu/display/hpsg/pass0+for+3pt7+GeV 
    1a) xsection: 6.853e8 pb
    1b) NFiles: 993
    1c) N MadGraph Events per file: 10000

2) WAB+Beam Original Recon: https://confluence.slac.stanford.edu/display/hpsg/pass0+for+3pt7+GeV (3333 files from this page) 
    2a) xsection: 7.557e10 pb
    2b) NFiles: (3333 files with 30k events/file + 1405 files with 40k events/file + 907 files with 40k events/file)
    2c) The 1405 and 907 file sets were generated and mixed with beam, but are not saved anywhere...(*CHECK ./gen_mc/gen_wab_mc FOR DETAILS) 

3) Rad+Beam Original Recon: (*CHECK ./gen_mc/gen_rad_mc FOR DETAILS)
    2a) xsection: 3.480e7 pb
    2b) NFiles: 1000
    2c) N MadGraph Events per file: 10000

4) SIMPs: Generated using hps-mc, readout steering-file: PhysicsRun2021TrigSingles.lcsim
   *CHECK ./gen_mc/gen_simp_mc FOR DETAILS ON SIMP MC GEN
    4a) mA' = (3/1.8)*mVD
    4b) mpi_D = mVD/1.8
    4c) Target pos = 0.0 mm
    4d) 10k events, 100 files per VD mass point {25..200..5} MeV
    4e) Beam Energy: 3.7
    4f) 8 um target with 50 nA beam current

### HOW ANALYSIS FILES ARE PRODUCED ###
1.) Tupilize Re-reconstructed files
    1a) Command: hpstr ./hpstr_config/kalSimpTuple_cfg.py -w KF -i <input.slcio> -o <tuple.root> -y 2021 -t 0
2.) Run vertex analysis in Signal Region (SR) and Control Region (CR)  
    2a) Command: hpstr ./hpstr_config/anaKalSimpTuple_cfg.py -w KF -i <tuple.root> -o <ana.root> -y 2-16 -t 0 -R <CR or SR>
    2b) Histo config: 'histoCfg' = hpstr/analysis/plotconfigs/tracking/simps/vtxAnalysis_2021_simp_reach.json
    2c) mc histo config: 'mcHistoCfg' = hpstr/analysis/plotconfigs/mc/basicMC.json
    2d) Pre-selection: 'vtxSelectionjson' = hpstr/analysis/selections/simps/vertexSelection_2021_simp_reach.json
    2e) Tight Selection Control Regions: recoana_kf.parameters["regionDefinitions"] = ['hpstr/analysis/selections/simps/Tight_2021_simp_reach_CR.json','hpstr/analysis/selections/simps/radMatchTight_2021_simp_reach_CR.json']
    2f) Tight Selection Signal Regions: recoana_kf.parameters["regionDefinitions"] = ['hpstr/analysis/selections/simps/Tight_2021_simp_reach_SR.json','hpstr/analysis/selections/simps/radMatchTight_2021_simp_reach_SR.json']

### HOW TO RUN REACH ESTIMATE ###
1.) Run makeComponents/makeRadFrac.py to get a polynomial expression for the Radiative Fraction
2.) Run makeComponents/makeTotRadAcc.py to get a polynomial expression for the Total Radiative Acceptance
3.) Run makeComponents/makeMassRes.py to get polynomial expression for mass resolution
4.) Run makeComponents/makeZcuts.py to get polynomial expression for Zcut
5.) Run hpstr vtxhProcess.py. Then run makeVtxResolution.py using the output file of vtxhProcess.py to get polynomial for Vertex Resolution
6.) Perform reach estimate using 'makeExpSignalSimps2021.py'. 
    6a) You must add the polynomial definitions calculated in steps 1-5 to this script.
    6b) You must define the proper MC scaling factors for 'tritrig' and 'wab' inside this script.
    6c) You must specify the dark_pion_decay_constant_ratio value (3 or 4pi) using flag -p
    6e) Example: python3 makeExpSignalSimps2021.py -o expSigRate_4Pi.root -x exContour_4Pi.txt -p 4pi

