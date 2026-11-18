# FEE Analysis Batch Processing (1.92 + 3.74 GeV)

Scripts for running the **FEE (Full Energy Electron) analysis** (`FeeAnaProcessor`,
via `anaFeeSmearing_*_cfg.py`) on hpstr ntuples using SLURM batch jobs.

This directory is **completely self-contained** and independent of
`scripts/2021_ana`. It carries its own copies of the hpstr config scripts and its
own job definitions, organized by the two 2021 beam energies:

| Energy   | hpstr config                  | track-selection JSON                         | FEE cluster E min |
|----------|-------------------------------|----------------------------------------------|-------------------|
| 3.74 GeV | `anaFeeSmearing_3p74_cfg.py`  | `analysis/selections/trackHit/trackHitAna.json`      | 2.5 GeV   |
| 1.92 GeV | `anaFeeSmearing_1p92_cfg.py`  | `analysis/selections/trackHit/trackHitAna_1p92.json` | 1.2 GeV   |

## Directory structure

```
fee_ana/
├── README.md                    # this file
├── config.sh                    # system settings (SLURM, base output dir, list dirs)
├── setup.sh                     # environment setup (conda / ROOT / hpstr)  [copied from 2021_ana]
├── job_params.sh                # per-energy job definitions
├── submit-jobs.sh               # submit SLURM jobs (--job / --all / --list / --dry-run)
├── batch-script.sh              # SLURM payload: runs hpstr FeeAnaProcessor
├── make_file_list.sh            # enumerate input ntuples into a .list
├── copy_summary.sh              # link MC summary.json into output dirs  [copied from 2021_ana]
├── hadd_fee_files.sh            # merge a job's output ROOT files
├── anaFeeSmearing_3p74_cfg.py   # local hpstr config, 3.74 GeV
├── anaFeeSmearing_1p92_cfg.py   # local hpstr config, 1.92 GeV
├── data_lists/{1p92,3p74}/      # input file lists for data
├── mc_lists/{1p92,3p74}/        # input file lists for MC (added later)
└── log/                         # SLURM job logs
```

## How it works

`FeeAnaProcessor` runs in hpstr **run_mode 1** (ntuple → histos/tree); it reads the
`FinalStateParticles_KF` branch and selects full-energy electrons (electron + matched
full-energy ECal cluster + track-quality cuts), then fills smearing-calibration
histograms and an output `tracks` tree.

`batch-script.sh` translates the `--sample` value into the hpstr `-t` (isData) flag and
**strips `--sample`** before invoking hpstr, because the FEE configs key off `-t` and do
not define a `--sample` argument. SLURM runs each job from this directory, so hpstr
imports the local `anaFeeSmearing_*_cfg.py` by name.

## Quick start

1. **Build input file lists** (you stage the inputs; this just enumerates them):
   ```bash
   ./make_file_list.sh --input /path/to/3p74/ntuples --output data_lists/3p74/data.list
   ./make_file_list.sh --input /path/to/1p92/ntuples --output data_lists/1p92/data.list
   ```

2. **Preview** what would be submitted:
   ```bash
   ./submit-jobs.sh --list
   ./submit-jobs.sh --job data_3p74 --dry-run
   ```

3. **Submit**:
   ```bash
   ./submit-jobs.sh --job data_3p74      # 3.74 GeV data
   ./submit-jobs.sh --job data_1p92      # 1.92 GeV data
   ./submit-jobs.sh --all                # everything defined in job_params.sh
   ```

4. **Monitor**: `squeue -u $USER`; logs land in `log/job_<jobid>.log`.

5. **Merge**: `./hadd_fee_files.sh --job data_3p74`

## Sample types

`--sample` is mapped to the hpstr `-t` flag in `batch-script.sh`:

| Sample      | `-t` | Meaning                  |
|-------------|------|--------------------------|
| `data`      | 1    | Real data                |
| `sim_bkgd`  | 0    | Simulated background MC  |

## Adding MC (later)

1. Drop MC ntuple lists into `mc_lists/<energy>/`.
2. Uncomment the `mc_1p92` / `mc_3p74` blocks in `job_params.sh` and point them at
   those lists (sample `sim_bkgd`).
3. `copy_summary.sh` will link each MC input dir's `summary.json` into the output dir
   (invoked automatically by `submit-jobs.sh --all`).

## Notes / open items

- **Inputs** are provided/staged separately by the user; the `*.list` files start empty.
- The two configs were copied from `processors/config/anaFeeSmearing_1p92_cfg.py` and
  `anaFeeSmearing_2021_cfg.py` (renamed `_3p74`). Edits here stay local to this directory.
- `BASE_OUTPUT_DIR` in `config.sh` defaults to `/sdf/data/hps/physics2021/fee_ana/v1`;
  adjust as needed. Output is organized as `<base>/<energy>/<data|mc>/`.
- **Timing:** both configs use `CalTimeOffset = CalTimeOffsetMC = 37.3 ns`, matching the
  2021 preselection (`PreselectAndCategorize2021`, `calTimeOffset = 37.3`). The cluster-time
  window is `±6 ns` applied to the offset-corrected time (`cluster.getTime() - CalTimeOffset`).
```
