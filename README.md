updated: 15 Jan 2020

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

### Running Examples -- ntuples production

The configuration to produce ntuples from LCIO files is ```recoTuple_cfg.py```. Typical usage is:
```bash
hpstr recoTuple_cfg.py -i <inLcioFile> -o <outROOTFile>
```

## Contributing to Hpstr

Fork the repository first. Open an issue to first discuss what needs to be changed and then open a pull request using the issue number. 
