inputPath=$1
out_histoFilename=$2

if [ "$#" -ne 2 ]; then
    echo "Illegal number of parameters"
    echo "lcio_to_plots_align <inputPath> <out_histoFilename>"
    exit 1
    fi

python run_jobPool.py -t hpstr -c /home/pbutti/run/kalTuple_cfg_2019.py  -i ${inputPath}/outputFiles/ -z 1 -o ${inputPath}/hpstr_ntuples/ -r slcio -p 10

python run_jobPool.py -t hpstr -c /home/pbutti/run/anaKalVtxTuple_cfg.py  -i ${inputPath}/hpstr_ntuples/ -z 1 -o ${inputPath}/hpstr_histos_GBL/ -r root -p 10 -e "-wGBL"

python run_jobPool.py -t hpstr -c /home/pbutti/run/anaKalVtxTuple_cfg.py  -i ${inputPath}/hpstr_ntuples/ -z 1 -o ${inputPath}/hpstr_histos_KF/ -r root -p 10 -e "-wKF"


#Hadd the histograms
echo "hadding the outputs..."
hadd ${inputPath}/hpstr_histos_GBL/${out_histoFilename}.root ${inputPath}/hpstr_histos_GBL/*.root
hadd ${inputPath}/hpstr_histos_KF/${out_histoFilename}.root ${inputPath}/hpstr_histos_KF/*.root

#Make the projections
echo "Making the projections...."
hpstr /home/pbutti/run/vtxhProcess.py -i ${inputPath}/hpstr_histos_GBL/${out_histoFilename}.root -o ${out_histoFilename}_projections.root -w GBL
hpstr /home/pbutti/run/vtxhProcess.py -i ${inputPath}/hpstr_histos_KF/${out_histoFilename}.root -o ${out_histoFilename}_projections.root -w KF

