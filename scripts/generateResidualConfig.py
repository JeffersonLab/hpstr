import os
import sys

outFile = open(os.environ['HPSTR_BASE']+"/analysis/plotconfigs/residuals.json", "w")

lyrs = 14
vols = ["", "_top", "_bot"]
residuals = ["u_res_"]


outFile.write("{\n")

total = lyrs*len(vols)*len(residuals)
i_entries = 0

#1D
for res in residuals:
    for ly in xrange(lyrs):
        for vol in vols:
            i_entries += 1
            name = '"'+res+'ly_'+str(ly)+vol+'_h"'
            outFile.write('    '+name + ' : { \n')
            outFile.write('        "bins" : 200,\n')
            outFile.write('        "minX" : -1,\n')
            outFile.write('        "maxX" : 1,\n')
            outFile.write('        "xtitle" : "Unbias res [mm]",\n')
            outFile.write('        "ytitle" : "Tracks"\n')
            #print i_entries, total
            outFile.write('    },\n')


variables = ["_vsp"]

for res in residuals:
    for ly in xrange(lyrs):
        for vol in vols:
            for var in variables:
                i_entries += 1
                name = '"'+res+'ly_'+str(ly)+vol+var+'_hh"'
                minX = "0"
                maxX = "10"
                outFile.write('    '+name + ' : { \n')
                outFile.write('        "binsX" : 200,\n')
                outFile.write('        "minX" : '+minX+',\n')
                outFile.write('        "maxX" : '+maxX+',\n')
                outFile.write('        "binsY" : 200,\n')
                outFile.write('        "minY" : -1,\n')
                outFile.write('        "maxY" : 1,\n')
                outFile.write('        "xtitle" : "'+var+'",\n')
                outFile.write('        "ytitle" : "u res [mm]"\n')
                outFile.write('    },\n')


variables = ["_vsy"]
vols = [""]

#2D vsp
i_entries = 0
total = len(residuals)*lyrs*len(vols)*len(variables)
for res in residuals:
    for ly in xrange(lyrs):
        for vol in vols:
            for var in variables:
                i_entries += 1
                name = '"'+res+'ly_'+str(ly)+vol+var+'_hh"'
                minX = "-50"
                maxX = "50"
                outFile.write('    '+name + ' : { \n')
                outFile.write('        "binsX" : 200,\n')
                outFile.write('        "minX" : '+minX+',\n')
                outFile.write('        "maxX" : '+maxX+',\n')
                outFile.write('        "binsY" : 200,\n')
                outFile.write('        "minY" : -1,\n')
                outFile.write('        "maxY" : 1,\n')
                outFile.write('        "xtitle" : "'+var+'",\n')
                outFile.write('        "ytitle" : "u res [mm]"\n')
                if (i_entries != total):
                    outFile.write('    },\n')
                else:
                    outFile.write('    }\n')


outFile.write("}\n")
outFile.close()
