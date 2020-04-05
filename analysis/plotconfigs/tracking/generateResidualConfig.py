import os,sys

outFile = open("residuals.json","w")

lyrs = 14
vols = ["","_top","_bot"]
residuals = ["u_res_"]

outFile.write("{\n")

total=lyrs*len(vols)*len(residuals)
i_entries = 0;
for res in residuals:
    for ly in xrange(lyrs):
        for vol in vols:
            i_entries+=1
            name = '"'+res+'ly_'+str(ly)+vol+'_h"'
            outFile.write('    '+name + ' : { \n')
            outFile.write('        "bins" : 200,\n')
            outFile.write('        "minX" : -1,\n')
            outFile.write('        "maxX" : 1,\n')
            outFile.write('        "xtitle" : "Unbias res [mm]",\n')
            outFile.write('        "ytitle" : "Tracks"\n')
            #print i_entries, total
            
            if (i_entries!=total):
                outFile.write('    },\n')
            else:
                outFile.write('    }\n')
                        
outFile.write("}\n")
outFile.close()

