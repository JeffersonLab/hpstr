/**
 * @mainpage Heavy Photon Search Toolkit for Reconstruction
 * 
 * The Heavy Photon Search Toolkit for Reconstruction (hpstr) provides an
 * interface to physics data from the HPS experiment saved in the LCIO
 * format and converts it into an ROOT based format. It also provides tools
 * which can be used to analyze the ROOT format of the data.
 * 
 * The code base can be found at <a href="https://github.com/JeffersonLab/hpstr">https://github.com/JeffersonLab/hpstr</a>.
 * 
 * @par 
 * Any questions can be directed to <a href="mailto:bravo@slac.stanford.edu">Cameron Bravo</a>.
 * 
 * @section install Installation
 * 
 * Hpstr can be installed on the following operating systems with some
 * adjustments depending on the tool set available for compilation. 
 * - MacOSX
 * - Ubuntu 20.4
 * - Centos7 
 * 
 * @subsection prerec Prerequisites
 * - <a href="https://github.com/JeffersonLab/hps-lcio">LCIO</a>
 * - <a href="https://root.cern.ch/">ROOT</a>
 * - Python 3 (Python 2 is no longer supported)
 * 
 * @subsection steps Installation Steps
 * After installing the prerequisites, the steps for installing hpstr are as follows:
 * - Checkout hpstr
 *   ```bash
 *   mkdir hpstr
 *   cd hpstr
 *   mkdir src build install
 *   cd src
 *   git clone git@github.com:JeffersonLab/hpstr.git
 *   cd ..
 *   ```
 * - Compilation
 *   ```bash
 *   cd build
 *   cmake3 -DCMAKE_INSTALL_PREFIX=../install/ -DLCIO_DIR=$LCIO_DIR  ../src/hpstr/
 *   make -j4 install
 *   ```
 *   NOTE: On SLAC machines ```cmake3``` is needed to call cmake version 3+, you might just need to call ```cmake``` to call the right version on your machine. 
 *   - If you do not want hpstr to use the default python3 executable which it finds in your environment, then supply the full path to the alternate Python installation.
 *     ```bash
 *     -DPython3_Executable=/path/to/some/python3 
 *     ```
 *   - To compile with debug information, just add -DCMAKE_BUILD_TYPE=Debug to the cmake3 command. 
 * - Setup
 *   ```bash
 *   source install/bin/setup.sh 
 *   ```
 * 
 * @par
 * For more information on how to install hpstr, please refer to the <a href="https://github.com/JeffersonLab/hpstr/blob/master/README.md">hpstr README</a>.
 * 
 * @section use Usage
 * The basic command string to run hpstr is
 * ```bash
 * Usage: hpstr [application arguments] {configuration_script.py} [arguments to configuration script]
 * Options:
 * -h, --help            show this help message and exit
 * -i inFilename, --inFile=inFilename
                      Input filename.
 * -d outDir, --outDir=outDir
                        Specify the output directory.
 * -o outFilename, --outFile=outFilename
                        Output filename.
 * -t, --isData          Type of lcio ntuple: 1=data, 0=MC
 * -y, --year            Select year of the data
 * -n, --nevents         Number of events to process
 * ```
 * where ```<config.py>``` is a config file (which are stored in ```hpstr/processors/config/```), followed by various command line options.
 * Different configuration files, might have specific command line options
 * Please check each configuration file to check which options are available on top of the common ones. 
 * 
 * Hpstr can both run on LCIO files to produce ROOT ntuples, producing the hpstr event with all the objects needed for analysis, and on ROOT ntuples to produce histograms.
 * This can be setup by using the appropriate configuration file. 
 * 
 * @par
 * For more information on how to run hpstr, please refer to the <a href="https://github.com/JeffersonLab/hpstr/blob/master/README.md">hpstr README</a>.
 */