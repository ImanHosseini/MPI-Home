## Debugging
mpirun -n 2 xterm -hold -e gdb -ex run --args ./poisson_1

## Run
mpirun -host s0,s1 ./mult

## Copy To Remote
scp mult s1:/mpilab