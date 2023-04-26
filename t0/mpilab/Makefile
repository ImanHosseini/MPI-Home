
all: mpi_0 poisson_0 poisson_1 poisson_1_bad mult

mpi_0: mpi_0.cpp
	mpic++ -std=c++17 $< -o $@

poisson_0: poisson_0.cpp
	mpic++ -std=c++17 $< -o $@

poisson_1: poisson_1.cpp
	mpic++ -std=c++17 $< -o $@

poisson_1_bad: poisson_1_bad.cpp
	mpic++ -g -std=c++17 $< -o $@

mult: mult.cpp
	mpic++ -std=c++17 $< -o $@

clear:
	rm mpi_0 mult poisson_0 poisson_1 poisson_1_bad