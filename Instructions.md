## Installation instructions (for Jess)

### Make a directory for all hps-related things
```bash
mkdir hps
cd hps
```

### Source an LCG environment which will automatically setup Python3 & the appropriate version of ROOT for you!
```bash
source /cvmfs/sft.cern.ch/lcg/views/LCG_97python3/x86_64-centos7-gcc8-opt/setup.sh
```
### Install LCIO (something hpstr depends on)
```bash
mkdir LCIO
cd LCIO
mkdir src build install
cd src
git clone https://github.com/iLCSoft/LCIO.git
cd LCIO
git checkout -b v02-12-01 tags/v02-12-01
cd ../../build
cmake -DCMAKE_INSTALL_PREFIX=../install/ ../src/LCIO/
make install
cd ../../
```

### Install hpstr
```bash
mkdir hpstr
cd hpstr
mkdir src build install
cd src
git clone https://github.com/ssevova/hpstr.git
(or if you use SSH: git clone git@github.com:ssevova/hpstr.git)
cd ..
```

### Compile hpstr
```bash
cd build
cmake3 -DCMAKE_INSTALL_PREFIX=../install/ -DLCIO_DIR=$LCIO_DIR  ../src/hpstr/ -DPYTHON3=True
make -j4 install
cd ..
source install/bin/setup.sh
```

### For the future you can write a hpstr_setup.sh script and put it inside /hps/hpstr that looks something like:
```bash
#!bin/bash

source /cvmfs/sft.cern.ch/lcg/views/LCG_97python3/x86_64-centos7-gcc8-opt/setup.sh

export LCIO_DIR=<path to where you installed LCIO>/LCIO/install
export LCIO_INCLUDE_DIRS=<path to where you installed LCIO>/LCIO/install/include
export IO_LCIO_LIBRARY=<path to where you installed LCIO>/LCIO/install/lib/liblcio.so
export LD_LIBRARY_PATH=$LCIO_DIR/lib:$LD_LIBRARY_PATH

export PATH=$LCIO_DIR/bin:$PATH

cd build

cmake3 -DCMAKE_INSTALL_PREFIX=../install/ -DLCIO_DIR=$LCIO_DIR  ../src/hpstr/ -DPYTHON3=True
make -j4 install
cd ..
source install/bin/setup.sh
```