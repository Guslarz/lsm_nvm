#!/bin/bash
export NOVELSMSRC=$PWD
export NOVELSMSCRIPT=$NOVELSMSRC/scripts
export DBBENCH=$NOVELSMSRC/out-static
export TEST_TMPDIR=/mnt/mem
#DRAM buffer size in MB
export DRAMBUFFSZ=64
#NVM buffer size in MB
export NVMBUFFSZ=4096
export INPUTXML=$NOVELSMSCRIPT/input.xml
#Vanilla LevelDB benchmark
export NOVELSM_VANILLA=$NOVELSMSRC/novelsm-1.20
export DBBENCH_VANLILLA=$NOVELSM_VANILLA/out-static
export PARA=40
export NUMA_AFFINITY=0
#Numa binding 
export APP_PREFIX="numactl --membind=$NUMA_AFFINITY --cpunodebind=$NUMA_AFFINITY"
#export APP_PREFIX=""
