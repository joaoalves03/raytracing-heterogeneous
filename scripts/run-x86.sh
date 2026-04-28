#!/bin/bash
#SBATCH --job-name=raytracing-x86
#SBATCH --account=f202500001hpcvlabepicurex
#SBATCH --partition=normal-x86
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=128
#SBATCH --time=01:00:00

ml CMake/3.29.3-GCCcore-13.3.0
ml LLVM/19.1.7-GCCcore-13.3.0

mkdir -p ./out

echo "Job  : $SLURM_JOB_ID"
echo "Host : $(hostname)"
echo "Dir  : $(pwd)"
date

cmake -B build
cmake --build build

./build/inOneWeekend > ./out/$SLURM_JOB_ID

date