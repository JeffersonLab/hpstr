import HpstrConf
import sys,os
import baseConfig as base

options = base.parser.parse_args()

# Use the input file to set the output file name
root1_file = options.inFilename[0]
root2_file = options.outFilename

print('Root file Input: %s' % root1_file)
print('Root file Output: %s' % root2_file)

p = HpstrConf.Process()

p.run_mode = 1
p.skip_events = 0
p.max_events = 100000000000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

rawAnaSvt = HpstrConf.Processor('svtana','SvtRawDataAnaProcessor')

###############################
#   Processor Configuration   #
###############################

#SvtRawAnaData

rawAnaSvt.parameters["debug"] = 1
rawAnaSvt.parameters["anaName"] = 'rawSvtHitAna'
rawAnaSvt.parameters["trkrHitColl"] = 'SVTRawTrackerHits'
rawAnaSvt.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/svt/rawSvtAnaHits3.json'
rawAnaSvt.parameters["sample"] = 1

RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/svtHit/"

rawAnaSvt.parameters["regionDefinitions"] = [RegionPath+'OneFit.json',
                                            RegionPath+'FirstFit.json',
                                            RegionPath+'SecondFit.json',
                                            RegionPath+'BothFit.json',
                                            RegionPath+'CTFit.json',
                                            RegionPath+'FTFit.json',
                                            RegionPath+'TimeResolution.json'
                                            ]
                                                            
rawAnaSvt.parameters["timeref"]=0.0
rawAnaSvt.parameters["ampref"]=0.0

#os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/basicRecoHit.json'
#os.environ['HPSTR_BASE']+'/analysis/plotconfigs/svt/Svt2DBl.json'

# Sequence which the processors will run.
p.sequence = [rawAnaSvt]

p.input_files=[root1_file]
p.output_files = [root2_file]

p.printProcess()
