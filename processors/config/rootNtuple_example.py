import HpstrConf
import sys
p = HpstrConf.Process()

# Library containing processors
p.libraries.append("libprocessors.dylib")

#Processors

#event    = HpstrConf.Processor('event'   ,'HPSEventProcessor')
clusters = HpstrConf.Processor('clusters','ClusterOnTrackProcessor')

p.sequence = [clusters]
#p.input_files  = ["testRun.root"]
#p.input_files   = ["/Users/Pierfrancesco/HPS/dataFiles/MC19/tritrig/total_tritrig_hipsterNtuples_2019.root"]
p.input_files    = ["/Users/Pierfrancesco/HPS/sw/hipster/run/hps_10494.root"]
p.output_files  = ["testHistograms.root"]
#p.max_events   = 10
p.printProcess()

