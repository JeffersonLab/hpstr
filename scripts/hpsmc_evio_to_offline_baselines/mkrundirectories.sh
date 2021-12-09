#!/usr/bin/bash
while getopts r: flag
do
    case "${flag}" in
        r) run=${OPTARG};;
    esac

done

if [ -z "$run" ]
then
    echo "ERROR! MUST SPECIFY RUN NUMBER USING FLAG -r <run_number>. EXITING PROCESS!"
    exit 1
fi

mkdir -vp ./${run} 
mkdir -vp ./${run}/lcio
mkdir -vp ./${run}/ntuples
mkdir -vp ./${run}/2dhistos
mkdir -vp ./${run}/fits
