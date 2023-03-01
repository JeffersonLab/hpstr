import HpstrConf
import sys
import os
import baseConfig as base

options = base.parser.parse_args()

# Use the input file to set the output file name
root1_file = options.inFilename[0]
root2_file = options.outFilename

print('Root file Input: %s' % root1_file)
print('Root file Output: %s' % root2_file)

p = HpstrConf.Process()

p.run_mode = 1
p.skip_events = options.skip_events
p.max_events = options.nevents

#Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

rawAnaSvt = HpstrConf.Processor('svtana', 'SvtRawDataAnaProcessor')

###############################
#   Processor Configuration   #
###############################

#SvtRawAnaData

rawAnaSvt.parameters["debug"] = 0
rawAnaSvt.parameters["anaName"] = 'rawSvtHitAna'
rawAnaSvt.parameters["trkrHitColl"] = 'SVTRawTrackerHits'
rawAnaSvt.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/svt/rawSvtAnaHits.json'

rawAnaSvt.parameters["sample"] = 1.0

RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/svtHit/"

rawAnaSvt.parameters["baselineFile"] = os.environ['HPSTR_BASE']+"/processors/dat/hps_14552_offline_baselines.dat"
rawAnaSvt.parameters["timeProfiles"] = os.environ['HPSTR_BASE'] + "/processors/dat/hpssvt_014393_database_svt_pulse_shapes_final.dat"

rawAnaSvt.parameters["regionDefinitions"] =

[RegionPath+'OneFit.json',
 #RegionPath+'FirstFit.json',
 #RegionPath+'SecondFit.json',
 #RegionPath+'BothFit.json',
 #RegionPath+'CTFit.json',
 RegionPath+'FTFit.json',
 #RegionPath+'CTFitErr.json',
 #RegionPath+'FTFitErr.json',
 RegionPath+'SecondFitTimeCT.json',
 #RegionPath+'TimeResolution.json',
 #RegionPath+'OneFitHitChi.json',
 #RegionPath+'FirstFitHitChi.json',
 #RegionPath+'SecondFitHitChi.json',
 #RegionPath+'FirstFitChannel0.json',
 #RegionPath+'FirstFitChannel1.json',
 #RegionPath+'FirstFitChannel2.json',
 #RegionPath+'FirstFitChannel3.json',
 #RegionPath+'FirstFitChannel4.json',
 #RegionPath+'FirstFitChannel5.json',
 #RegionPath+'FirstFitChannel6.json',
 #RegionPath+'FirstFitChannel7.json',
 #RegionPath+'SecondFitChannel0.json',
 #RegionPath+'SecondFitChannel1.json',
 #RegionPath+'SecondFitChannel2.json',
 #RegionPath+'SecondFitChannel3.json',
 #RegionPath+'SecondFitChannel4.json',
 #RegionPath+'SecondFitChannel5.json',
 #RegionPath+'SecondFitChannel6.json',
 #RegionPath+'SecondFitChannel7.json',
 #RegionPath+'OneFitChannel0.json',
 #RegionPath+'OneFitChannel1.json',
 #RegionPath+'OneFitChannel2.json',
 #RegionPath+'OneFitChannel3.json',
 #RegionPath+'OneFitChannel4.json',
 #RegionPath+'OneFitChannel5.json',
 #RegionPath+'OneFitChannel6.json',
 #RegionPath+'OneFitChannel7.json',
 #RegionPath+'TimeResolution.json']
 RegionPath+'LowTimeDiff.json']

rawAnaSvt.parameters["MatchList"] = ['OneFit', 'CTFit', 'SecondFitTimeCT']
rawAnaSvt.parameters["timeref"] = 0.0
rawAnaSvt.parameters["ampref"] = 0.0
rawAnaSvt.parameters["sample"] = 1

p.sequence = [rawAnaSvt]

p.input_files = [root1_file]
p.output_files = [root2_file]

p.printProcess()
