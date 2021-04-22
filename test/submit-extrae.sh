#!/bin/bash

#SBATCH --job-name=submit-extrae.sh
#SBATCH -D .
#SBATCH --output=submit-extrae.sh.o%j
#SBATCH --error=submit-extrae.sh.e%j

USAGE="\n USAGE: ./submit-extrae.sh prog num_threads\n
        prog        -> omp program name\n
        num_threads -> number of threads\n"

if (test $# -lt 2 || test $# -gt 2)
then
	echo -e $USAGE
        exit 0
fi

make $1

HOST=$(echo $HOSTNAME | cut -f 1 -d'.')
if (test "${HOST}" = "boada-1")
then
    echo "Use sbatch to execute this script"
    exit 0
fi

export OMP_NUM_THREADS=$2

export LD_PRELOAD=${EXTRAE_HOME}/lib/libomptrace.so

./$1 
unset LD_PRELOAD
mpi2prv -f TRACE.mpits -o $1-$2-${HOST}.prv -e $1 -paraver
