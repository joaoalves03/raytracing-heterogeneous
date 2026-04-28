#!/bin/bash
#SBATCH --job-name=raytracing-gpu
#SBATCH --account=f202500001hpcvlabepicureg
#SBATCH --partition=normal-a100-40
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --time=01:00:00
#SBATCH --gpus=1

ml purge
ml CMake/3.29.3-GCCcore-13.3.0
ml NVHPC/25.9-CUDA-12.9.1
ml CUDA/12.9.1

mkdir -p ./out
mkdir -p ./perf
rm -rf build

cmake -B build -DCMAKE_CXX_COMPILER=nvc++ -DCMAKE_BUILD_TYPE=Release
cmake --build build

export PGI_ACC_TIME=1

echo "Starting Profiling with Nsight Systems..."


nsys profile \
    -o ./perf/report_$SLURM_JOB_ID \
    --stats=true \
    -t openacc,cuda,nvtx \
    ./build/inOneWeekend > ./out/$SLURM_JOB_ID.ppm

date
