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

export PARA=40
export NUMA_AFFINITY=0
#Numa binding 
export APP_PREFIX="numactl --membind=$NUMA_AFFINITY --cpunodebind=$NUMA_AFFINITY"
#export APP_PREFIX=""
