import HpstrConf
import baseConfig as base
import os

options = base.parser.parse_args()

p = HpstrConf.Process()

#lcio_file = options.inFilename[0]
in_file = 'dummy.root'
out_file = options.outFilename

p.run_mode = 2
p.skip_events = options.skip_events
p.max_events = options.nevents

p.libraries.append("libprocessors.so")

###############################
#          Processors         #
################################

zbi = HpstrConf.Processor('zbi','ZBiCutflowProcessor')
zbi.parameters['debug'] = options.debug
zbi.parameters['signalFilename'] = '/sdf/group/hps/users/alspellm/projects/THESIS/ZBi/signal/hadd_mass_55_simp_recon_KF_ana.root'
zbi.parameters['tritrigFilename'] = '/sdf/group/hps/users/alspellm/projects/THESIS/ZBi/tritrig_beam/full_hadd_tritrigv2-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_KF_ana_nvtx1.root'
#zbi.parameters['radSlicFilename'] = ''
zbi.parameters['vdSimFilename'] = '/sdf/group/hps/users/alspellm/projects/THESIS/mc/2016/simps/slic_ana/hadd_mass_55_simp_mcAna.root'
zbi.parameters['vdMassMeV'] = 55.0
zbi.parameters['signalHistCfgFilename'] = '/sdf/group/hps/users/alspellm/src/test/hpstr/analysis/plotconfigs/tracking/zbiCutVariables.json'
zbi.parameters['outFileName'] = options.outFilename
zbi.parameters['cuts_cfgFile'] = '/sdf/group/hps/users/alspellm/src/test/hpstr/analysis/selections/simps/iterativeCuts.json'
zbi.parameters['ztail_events'] = 0.5
zbi.parameters['cutVariables'] = ["unc_vtx_psum", "unc_vtx_ele_track_p", "unc_vtx_pos_track_p", "unc_vtx_chi2", "unc_vtx_ele_track_t", "unc_vtx_pos_track_t", "unc_vtx_ele_track_clust_dt","unc_vtx_pos_track_clust_dt", "unc_vtx_x","unc_vtx_y","unc_vtx_x","unc_vtx_y", "unc_vtx_ele_clust_E","unc_vtx_pos_clust_E"]
#zbi.parameters['ApMassMeV'] = 

# Sequence which the processors will run.
p.sequence = [zbi]

p.input_files=[in_file]
p.output_files = [out_file]
p.printProcess()

