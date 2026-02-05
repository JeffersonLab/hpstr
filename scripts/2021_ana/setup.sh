
__conda_setup="$('/sdf/group/hps/sw2/conda/bin/conda' 'shell.bash' 'hook' 2> /dev/null)"
if [ $? -eq 0 ]; then
    eval "$__conda_setup"
else
    if [ -f "/sdf/group/hps/sw2/conda/etc/profile.d/conda.sh" ]; then
        . "/sdf/group/hps/sw2/conda/etc/profile.d/conda.sh"
    else
        export PATH="/sdf/group/hps/sw2/conda/bin:$PATH"
    fi
fi
unset __conda_setup

conda activate hps-2021

source /sdf/group/hps/sw2/root/bin/thisroot.sh
source /sdf/data/hps/users/mgignac/software/dev/hps-mc/install/bin/hps-mc-env.sh
source /sdf/data/hps/users/mgignac/software/2021-ana/hpstr/install/bin/hpstr-env.sh
export LD_LIBRARY_PATH=/sdf/home/m/mgignac/.conda/envs/hps-2021/lib:$LD_LIBRARY_PATH
