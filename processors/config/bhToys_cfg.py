import HpstrConf
import sys

# Use the input file to set the output file name
histo_file = sys.argv[1].strip()
toy_file = '%s_bhToys.root' % histo_file[:-5]

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
bhtoys.parameters["massSpectrum"] = 'mass_tweak__p_tot_min_cut'
bhtoys.parameters["mass_hypo"] = 95.0/1000.0
bhtoys.parameters["poly_order"] = 3
bhtoys.parameters["win_factor"] = 12
bhtoys.parameters["seed"] = 0
bhtoys.parameters["nToys"] = 100

# Sequence which the processors will run.
p.sequence = [bhtoys]

p.input_files=[histo_file]
p.output_files = [toy_file]

p.printProcess()
