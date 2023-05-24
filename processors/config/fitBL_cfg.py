import HpstrConf
import baseConfig as base
import os


def timeSample_callback(options, opt, value, parser):
    setattr(parser.values, options.dest, value.split(','))


#To fit 2d histograms from file, provide a list of strings that match histograms of interest
#If attempting to run over all layers, aka by not specifying the Layer number, RAM requirements may crash the program.
base.parser.add_argument('-l', '--layer', type=str, dest="layer", default="",
                         help="To run on all layers, leave default. To select specific layer: L<n><T/B>")

base.parser.add_argument('-thresh', '--thresh', type=str, dest="thresholdsFileIn",
                         help="Load online thresholds file used to set apv channel threshold. Required for fitting!")

#Choose the RMS value that indicates a "dead" channel. This is a channel with low RMS compared to other channels, and varies based on Run
base.parser.add_argument("-deadRMS", '--deadRMS', type=int, dest="deadRMS",
                         help="Define dead channel by setting low RMS threshold", metavar="deadRMS", default=150)

base.parser.add_argument("-b", "--rebin", type=int, dest="rebin",
                         help="rebin factor.", metavar="rebin", default=1)

base.parser.add_argument("-minStats", '--minStats', type=int, dest="minStats",
                         help="Offline fitting requires a minimum number of stats to fit channel", metavar="minStats", default=1200)

options = base.parser.parse_args()

# Use the input file to set the output file name
lcio_file = options.inFilename[0]
root_file = options.outFilename

print('LCIO file: %s' % lcio_file)
print('Root file: %s' % root_file)

# Use the input file to set the output file name
histo_file = options.inFilename[0]
layer = options.layer

p = HpstrConf.Process()

p.run_mode = 2
p.skip_events = options.skip_events
p.max_events = options.nevents

# Library containing processors
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################

fitBL = HpstrConf.Processor('fitBL', 'SvtBlFitHistoProcessor')

###############################
#   Processor Configuration   #
###############################
fitBL.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/svt/SvtBlFits.json'
fitBL.parameters["rawhitsHistCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/svt/baselinefits/rawSvtHits.json'
fitBL.parameters["layer"] = options.layer
fitBL.parameters["rebin"] = options.rebin
fitBL.parameters["minStats"] = options.minStats
fitBL.parameters["deadRMS"] = options.deadRMS
fitBL.parameters["thresholdsFileIn"] = options.thresholdsFileIn
fitBL.parameters["debug"] = options.debug

# Sequence which the processors will run.
p.sequence = [fitBL]

p.input_files = [histo_file]
p.output_files = [root_file]

p.printProcess()
