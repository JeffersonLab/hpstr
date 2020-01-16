import HpstrConf
import sys
import baseConfig

parser = baseConfig.parser

parser.add_option("-m", "--mass", type="int", dest="mass_hypo",
        help="Mass hypothesis in MeV.", metavar="mass_hypo", default=145)
parser.add_option("-p", "--poly", type="int", dest="poly_order",
        help="Polynomial order of background model.", metavar="poly_order", default=3)
parser.add_option("-w", "--win", type="int", dest="win_factor",
        help="Window factor for determining fit window size.", metavar="win_factor", default=11)
parser.add_option("-t", "--toys", type="int", dest="nToys",
        help="Number of toy spectra to throw.", metavar="nToys", default=100)
parser.add_option("-s", "--spec", type="string", dest="mass_spec",
        help="Name of mass spectrum histogram.", metavar="mass_spec", 
        default="mass_tweak__p_tot_min_cut")

(options, args) = parser.parse_args()

# Use the input file to set the output file name
histo_file = options.inFilename
mass_hypo = options.mass_hypo/1000.0
poly_order = options.poly_order
win_factor = options.win_factor
toy_file = '%s/bhToys_m%iw%ip%i.root'%(options.outDir, options.mass_hypo, win_factor, poly_order)

print('Histo file: %s' % histo_file)
print('Toy file: %s' % toy_file)

p = HpstrConf.Process()

p.run_mode = 2
#p.max_events = 1000

# Library containing processors
p.libraries.append("libprocessors.so")

###############################
#          Processors         #
###############################

bhtoys = HpstrConf.Processor('bhtoys', 'BhToysHistoProcessor')

###############################
#   Processor Configuration   #
###############################
#MCParticles
bhtoys.parameters["debug"] = 1 
bhtoys.parameters["massSpectrum"] = options.mass_spec
bhtoys.parameters["mass_hypo"] = mass_hypo
bhtoys.parameters["poly_order"] = poly_order
bhtoys.parameters["win_factor"] = win_factor
bhtoys.parameters["seed"] = 0
bhtoys.parameters["nToys"] = options.nToys

# Sequence which the processors will run.
p.sequence = [bhtoys]

p.input_files=[histo_file]
p.output_files = [toy_file]

p.printProcess()
