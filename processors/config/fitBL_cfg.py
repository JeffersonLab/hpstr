import HpstrConf
import baseConfig as base
import os



def timeSample_callback(options, opt, value, parser):
        setattr(parser.values, options.dest, value.split(','))


#To fit 2d histograms from file, provide a list of strings that match histograms of interest
#If attempting to run over all layers, aka by not specifying the Layer number, RAM requirements may crash the program.
base.parser.add_argument('-l', '--layer', type=str, dest="layer",default="", 
        help="To run on all layers, leave default. To select specific layer: L<n><T/B>")

#Choose the RMS value that indicates a "dead" channel. This is a channel with low RMS compared to other channels, and varies based on Run
base.parser.add_argument("-deadRMS", '--deadRMS', type=int, dest="deadRMS", 
        help="Define dead channel by setting low RMS threshold", metavar="deadRMS", default="150")

#Set simpleGausFit to True if fitting a baseline file that does not have any background
base.parser.add_argument('-simpleGausFit', '--simpleGausFit',type=str, dest="simpleGausFit",default="false", 
        help="To fit baselines with simple gaussian fit, set to True")

#Set the minimum number of statistics in a channel required to attempt to fit the channel. Low stats channels often have trouble being fit
base.parser.add_argument("-m", "--minStats", type=int, dest="minStats", 
        help="Minimum Statistics required per bin to perform fit", metavar="minStats", default="8500")

#Choose the RMS value that indicates a "noisy" channel. 
base.parser.add_argument("-noisy", '--noisy', type=int, dest="noisy", 
        help="Define noisy channel by RMS threshold", metavar="noisy", default="400")

#The following 3 options are advanced settings that should not be adjusted without consultation
base.parser.add_argument("-x", '--xmin', type=int, dest="xmin", 
        help="Set threshold for xmin of iterative fit range", metavar="xmin", default="50")

base.parser.add_argument("-p", "--nPoints", type=int, dest="nPoints", 
        help="Select number of points for second derivative.", metavar="nPoints", default="3")

base.parser.add_argument("-b", "--rebin", type=int, dest="rebin",
                help="rebin factor.", metavar="rebin", default="1")


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
fitBL.parameters["nPoints"] = options.nPoints
fitBL.parameters["xmin"] = options.xmin
fitBL.parameters["minStats"] = options.minStats
fitBL.parameters["noisy"] = options.noisy
fitBL.parameters["deadRMS"] = options.deadRMS
fitBL.parameters["simpleGausFit"] = options.simpleGausFit

# Sequence which the processors will run.
p.sequence = [fitBL]

p.input_files=[histo_file]
p.output_files = [root_file]

p.printProcess()
