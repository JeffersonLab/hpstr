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
