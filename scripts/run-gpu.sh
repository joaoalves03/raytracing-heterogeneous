#!/bin/bash
#SBATCH --job-name=raytracing-gpu
#SBATCH --account=f202500001hpcvlabepicureg
#SBATCH --partition=normal-a100-40
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --time=01:00:00
#SBATCH --gpus=1

ml CMake/3.29.3-GCCcore-13.3.0
ml LLVM/19.1.7-GCCcore-13.3.0

echo "Job  : $SLURM_JOB_ID"
echo "Host : $(hostname)"
echo "Dir  : $(pwd)"
date

cmake -B build
cmake --build build

./build/inOneWeekend > ./out/$SLURM_JOB_ID

date