# 2021 Analysis Batch Processing

Scripts for running hpstr preselection on 2021 HPS data using SLURM batch jobs.

## Directory Structure

```
2021_ana/
├── config.sh          # Central configuration (paths, SLURM settings)
├── config_2021.py     # hpstr processor configuration
├── make_file_list.sh  # Generate input file lists
├── submit-jobs.sh     # Submit SLURM jobs
├── batch-script.sh    # SLURM batch script (runs hpstr)
├── hadd_data_files.sh # Merge output ROOT files
├── data_lists/        # Generated file lists
└── log/               # Job logs
```

## Quick Start

1. **Edit configuration** in `config.sh`:
   - `INPUT_DATA_DIR` - path to input ROOT files
   - `OUTPUT_DIR` - where output files will be written
   - `SLURM_PARTITION` and `SLURM_ACCOUNT` - SLURM settings

2. **Generate file list**:
   ```bash
   ./make_file_list.sh
   ```
   Or limit to N files for testing:
   ```bash
   ./make_file_list.sh 10
   ```

3. **Submit jobs**:
   ```bash
   ./submit-jobs.sh --sample data
   ```

4. **Monitor jobs**:
   ```bash
   squeue -u $USER
   ```
   Check logs in `log/job_<jobid>.log`

## Sample Types

The `--sample` argument is required and must be one of:

| Sample | Description |
|--------|-------------|
| `data` | Real data (requires `-D` flag internally) |
| `sim_bkgd` | Simulated background (tritrig, etc.) |
| `ap_signal` | A' signal MC |
| `simp_signal` | SIMP signal MC |

## Usage Examples

**Process all data files:**
```bash
./submit-jobs.sh --sample data
```

**Process with a custom file list:**
```bash
./submit-jobs.sh --sample data my_files.list
```

**Pass extra arguments to hpstr:**
```bash
./submit-jobs.sh --sample data -- -n 1000
```

**Output to a subdirectory (for systematics):**
```bash
PRESELECT_SUBDIR=systematic_v1 ./submit-jobs.sh --sample data
```

## Merging Output

After jobs complete, use `hadd` to merge output files:
```bash
hadd merged_output.root /path/to/output/*.root
```

## Configuration Details

### config.sh
Central bash configuration sourced by all scripts:
- `INPUT_DATA_DIR` - Input data location
- `OUTPUT_DIR` - Output directory
- `FILE_LIST` - Default file list path
- `HPSTR_CONFIG` - Python config file name

### config_2021.py
hpstr processor configuration that sets up the `PreselectAndCategorize2021` processor with appropriate parameters based on sample type.

## SLURM Resources

Default job resources (in `batch-script.sh`):
- CPUs: 1
- Memory: 4 GB
- Time limit: 4 hours
