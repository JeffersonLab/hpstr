import HpstrConf
import sys
import os
import baseConfig as base


options = base.parser.parse_args()


# Use the input file to set the output file name
infile = options.inFilename
outfile = options.outFilename

print('Input file: %s' % infile)
print('Output file: %s' % outfile)

p = HpstrConf.Process()

p.run_mode = 1
#p.max_events = 1000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

recoana = HpstrConf.Processor('recoana', 'SvtTimingProcessor')

###############################
#   Processor Configuration   #
###############################
#SvtTiming
recoana.parameters["debug"] = 0
recoana.parameters["anaName"] = "svtHitTimingAna"
recoana.parameters["trkColl"] = "KalmanFullTracks"
#recoana.parameters["trkrHitColl"] = "RotatedHelicalTrackHits"
recoana.parameters["rawHitColl"] = "SVTRawHitsOnTrack_KF"
recoana.parameters["trkrHitColl"] = "SiClustersOnTrack"
recoana.parameters["fspColl"] = "FinalStateParticles_KF"
recoana.parameters["ecalHitColl"] = "RecoEcalHits"
recoana.parameters["ecalClusColl"] = "RecoEcalClusters"
recoana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/svtTiming-ecalRef.json'
recoana.parameters["mcHistoCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/mc/basicMC.json'
#recoana.parameters["timingCalibFile"]="/sdf/group/hps/users/mgraham/TimingCorrections/physrun2021/run14507_calib.txt"
#recoana.parameters["timingCalibFile"]="/sdf/group/hps/users/mgraham/TimingCorrections/physrun2021/svtTiming_ecalRef_run_14495_preCal_calib.txt"
recoana.parameters["timingCalibDir"]="/sdf/group/hps/users/mgraham/TimingCorrections/physrun2021/perRunTiming"
recoana.parameters["postfixTiming"]="_calib_constants_final.txt"
recoana.parameters["analysis"] = options.analysis
recoana.parameters["selectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/svttiming_10hit.json'
print("anaSvtTiming_cfg.py::Setting runNumber to "+str(options.runNumber))
recoana.parameters["runNumber"]=options.runNumber

RegionPath=os.environ['HPSTR_BASE']+"/analysis/selections/svtTiming/"
recoana.parameters["regionDefinitions"] = [RegionPath+'evtPhase0.json',
                                           RegionPath+'evtPhase1.json',
                                           RegionPath+'evtPhase2.json',
                                           RegionPath+'evtPhase3.json',
                                           RegionPath+'evtPhase4.json',
                                           RegionPath+'evtPhase5.json']
#                                           RegionPath+'evtPhase0FEE.json',
#                                           RegionPath+'evtPhase1FEE.json',
#                                           RegionPath+'evtPhase2FEE.json',
#                                           RegionPath+'evtPhase3FEE.json',
#                                           RegionPath+'evtPhase4FEE.json',
#                                           RegionPath+'evtPhase5FEE.json']

# Sequence which the processors will run.
p.sequence = [recoana]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
