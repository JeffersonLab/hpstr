updated: 3 Feb 2020

# Heavy Photon Search Toolkit for Reconstruction

The Heavy Photon Search Toolkit for Reconstruction (hpstr) provides an interface to physics data from the HPS experiment saved in the LCIO format and converts it into an ROOT based format. It also provides tools which can be used to analyze the ROOT format of the data.

Please change HPSTR_BASE in setup.sh for your system.


## Checkout

```bash
mkdir hpstr
cd hpstr
mkdir src build install
cd src
git clone git@github.com:JeffersonLab/hpstr.git
cd ..
```

## Compilation

```bash
cd build
cmake3 -DCMAKE_INSTALL_PREFIX=../install/ -DLCIO_DIR=$LCIO_DIR  ../src/hpstr/
make -j4 install
```
To compile with debug information, just add -DCMAKE_BUILD_TYPE=Debug to the cmake3 command

## Usage

The basic command string to run hpstr is

```
Usage: hpstr <config.py> [options]

Options:
  -h, --help            show this help message and exit
  -i inFilename, --inFile=inFilename
                        Input filename.
  -d outDir, --outDir=outDir
                        Specify the output directory.
  -o outFilename, --outFile=outFilename
                        Output filename.
```


where ```<config.py>``` is a config file (which are stored in ```hpstr/processors/config/```), followed by various command line options. Different configuration files, might have specific command line options. Please check each configuration file to check which options are available on top of the common ones. 

Hpstr can both run on LCIO files to produce ROOT ntuples, producing the hpstr event with all the objects needed for analysis, and on ROOT ntuples to produce histograms. This can be setup by using the appropriate configuration file. 

### Ntuples production

The configuration to produce ntuples from LCIO files is ```recoTuple_cfg.py```. Typical usage is:
```bash
hpstr recoTuple_cfg.py -i <inLcioFile> -o <outROOTFile>
```

### Making Plots

A working example on how to make some plots out of hpstr ntuple is 

```bash
hpstr anaVtxTuple_cfg.py -i /nfs/slac/g/hps3/users/bravo/data/physrun2016/7800/hps_007800.123_v0_4.2_4.4-SNAPSHOT_rereco.root -o hps_007800.123.root -t 1 
```
This example will run the standard vertex selection on a data file (to specify that this file is data one has to use the ```-t``` flag and passing 0 will tell hpstr that we are processing MonteCarlo. Plots will be produced according to the selections specified. 


### Processing multiple files

The script ```scripts/run_jobPool.py``` provides a way to process multiple files with hpstr in parallel in parallel threads. 
Here is an example on how to run it

```
python run_jobPool.py -t hpstr -c <configFile.py>  -i <inDir> -z <isData> -o <outDir> -r
```

where ```-c``` is used to specify the configurationFile for hpstr, ```-i``` and ```-o``` are for specifying the input and output directory respectively, ```-z``` is to choose between data (=1) and MC simulation (=0) input type, and finally ```-r``` is a switch to tell hpstr to run on root files instead of slcio. 

## Contributing to Hpstr

Fork the repository first. Open an issue to first discuss what needs to be changed and then open a pull request using the issue number. 
