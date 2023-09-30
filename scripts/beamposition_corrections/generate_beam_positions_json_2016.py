#!/usr/bin/python3
import pandas as pd
import numpy as np
import json

beam_pos_file = '/sdf/group/hps/users/alspellm/projects/THESIS/ana/beamspot_conditions/beam_position_db_values_2016.txt'
beam_coll_file = '/sdf/group/hps/users/alspellm/projects/THESIS/ana/beamspot_conditions/beam_position_db_collections_2016.txt'

pos_df = pd.read_csv(beam_pos_file, sep="|")
pos_df = pos_df.rename(columns=lambda x: x.strip())
beamspot_positions = {}
for index, row in pos_df.iterrows():
    beamspot_positions[int(row['collection_id'])] = {'beamspot_x':row['x'],'beamspot_y':row['y'],'beamspot_z':row['z']}

coll_df = pd.read_csv(beam_coll_file, sep="|") 
coll_df = coll_df.rename(columns=lambda x: x.strip())
coll_runs = {}
run_collections = {}
for index, row in coll_df.iterrows():
    if 'Beam position for run' not in row['description']:
        continue
    run = int(row['description'].replace('Beam position for run ',''))
    coll_id = int(row['id'])
    run_collections[run] = coll_id

run_beam_pos = {}
for run,coll in run_collections.items():
    beam_pos = (beamspot_positions[coll])
    run_beam_pos[run] = beam_pos

json_string = json.dumps(run_beam_pos,sort_keys=True, indent=4)
with open("beamspot_positions_2016.json","w") as outfile:
    outfile.write(json_string)
print(json_string)







