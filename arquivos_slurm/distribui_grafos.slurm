#!/bin/bash  
#SBATCH -N 2
#SBATCH -n 6     

mpirun  -np 6 ./distribui