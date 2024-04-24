# Utils data area

## TrackSmearingTool

The smearing terms from the fee analysis are stored in .root files.
The basic smearing terms are the stored in 1D histograms as function of the number of hits on tracks, separated for top and bottom SVT halves.

### Files naming convention:
`feeSmearing_<year>_<iov>_<productionDate>.root`
- _year_: is for which dataset to be used
- _iov_: the interval of validity of the smearing terms within a certain dataset. "all" for the full dataset.
- _productionDate_: when the file has been done.

### Histograms naming convention:
The histogram storing the smearing terms as function of the number of hits is named:
```
<Tracks>_p_vs_nHits_hh_smearing
```
- _Tracks_: name of the tracks used for evaluating the smearing terms. "KalmanFullTracks" is the default.

### Generate smeering terms
```
python3.9 smearingPlots.py -i fee_2pt3_recon.root -m fee_2pt3_recon_mc.root
```





## TrackBiasingTool

The track biasing tool can be used to bias some of the track fit bias parameters or to evaluating tracking systematics
on the analysis.

### Workflow

Currently the EoP average biases are stored in two histograms for top and bottom volumes separated by charge (e-/e+).
They can be loaded in the trackBiasing tool that is designed to take in input a modifiable track and will perform the
bias of (for the moment) the momentum.

The TrackBiasingTool can be loaded in the anaProcessors and used to bias tracks that will then be used to form the flat ntuples
or final plots.

### Histograms

The biasing tool for the moment needs two histograms:
<Tracks>_eop_vs_charge_<volume>
Where <Tracks> can be for example the "KalmanFullTracks" and <volume> can be "top" or "bot"