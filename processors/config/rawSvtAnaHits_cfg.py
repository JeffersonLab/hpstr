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
#p.max_events = 1000

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

RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/svtHit/"

rawAnaSvt.parameters["regionDefinitions"] = [RegionPath+'OneFit.json',
                                            RegionPath+'FirstFit.json',
                                            RegionPath+'SecondFit.json',
                                            RegionPath+'BothFit.json',
                                            RegionPath+'CTFit.json',
                                            RegionPath+'FTFit.json',
                                            RegionPath+'LAmp.json',
                                            RegionPath+'OneFitHighChi.json',
                                            RegionPath+'OneFitLowChi.json',
                                            RegionPath+'FirstFitLowChi.json',
                                            RegionPath+'FirstFitHighChi.json',
                                            RegionPath+'SecondFitLowChi.json', 
                                            RegionPath+'SecondFitHighChi.json',
                                            RegionPath+'CTFitLowChi.json',
                                            RegionPath+'CTFitHighChi.json',
                                            RegionPath+'FirstFitCut.json',
                                            RegionPath+'SecondFitCut.json',
                                            RegionPath+'FirstFitCut2.json',
                                            RegionPath+'SecondFitCut2.json',
                                            RegionPath+'FirstFitChannel0.json',
                                            RegionPath+'FirstFitChannel1.json',
                                            RegionPath+'FirstFitChannel2.json',
                                            RegionPath+'FirstFitChannel3.json',
                                            RegionPath+'FirstFitChannel4.json',
                                            RegionPath+'FirstFitChannel5.json',
                                            RegionPath+'FirstFitChannel6.json',
                                            RegionPath+'FirstFitChannel7.json',
                                            RegionPath+'SecondFitChannel0.json',
                                            RegionPath+'SecondFitChannel1.json',
                                            RegionPath+'SecondFitChannel2.json',
                                            RegionPath+'SecondFitChannel3.json',
                                            RegionPath+'SecondFitChannel4.json',
                                            RegionPath+'SecondFitChannel5.json',
                                            RegionPath+'SecondFitChannel6.json',
                                            RegionPath+'SecondFitChannel7.json',
                                            RegionPath+'OneFitChannel0.json',
                                            RegionPath+'OneFitChannel1.json',
                                            RegionPath+'OneFitChannel2.json',
                                            RegionPath+'OneFitChannel3.json',
                                            RegionPath+'OneFitChannel4.json',
                                            RegionPath+'OneFitChannel5.json',
                                            RegionPath+'OneFitChannel6.json',
                                            RegionPath+'OneFitChannel7.json'

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
