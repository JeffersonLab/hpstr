#!/bin/bash 

export HPSTR_BASE=/data/src/hpstr
export PYTHONPATH=$HPSTR_BASE/pyplot:$HPSTR_BASE/install/lib/python:$PYTHONPATH

export PATH=$HPSTR_BASE/install/bin:$PATH
export LD_LIBRARY_PATH=$HPSTR_BASE/install/lib:$LD_LIBRARY_PATH

