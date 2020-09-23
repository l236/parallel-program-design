CC = gcc
MPICC = mpicc
CFLAGS = -O3 -pthread -lm -lpng -fopenmp -std=c99

all: mpi_static mpi_dynamic omp hybrid

mpi_static: mpi_static.c
	$(MPICC) mpi_static.c $(CFLAGS) -o mpi_static

mpi_dynamic: mpi_dynamic.c
	$(MPICC) mpi_dynamic.c $(CFLAGS) -o mpi_dynamic

omp: omp.c
	$(MPICC) omp.c $(CFLAGS) -o omp

hybrid: hybrid.c 
	$(MPICC) hybrid.c $(CFLAGS) -o hybrid

sequential: sequential.c
	$(CC) sequential.c $(CFLAGS) -o sequential

hybrid2: hybrid2.c
	$(MPICC) hybrid2.c $(CFLAGS) -o hybrid2

clean:
	rm mpi_static hybrid omp mpi_dynamic