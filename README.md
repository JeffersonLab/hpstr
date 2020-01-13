# Heavy Photon Search Toolkit for Reconstruction

The Heavy Photon Search Toolkit for Reconstruction (hpstr) provides an interface to physics data from the HPS experiment saved in the LCIO format and converts it into an ROOT based format. It also provides tools which can be used to analyze the ROOT format of the data.

Please change HPSTR_BASE in setup.sh for your system.


Checkout:

mkdir hpstr
cd hpstr
mkdir src build install
cd src
git clone git@github.com:JeffersonLab/hpstr.git
cd ..


Compilation:

cd build
cmake3 -DCMAKE_INSTALL_PREFIX=../install/ -DLCIO_DIR=$LCIO_DIR  ../src/hpstr/
make -j4 install

