#include<mpi.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<stdio.h>

int main(int argc ,char* argv[]) {
	int comm_sz, rank;
	long long int count=0, rank_count=0;
	long long int i;
	double x, y;
	double pi;
	long long int pointers=0,rank_pointers;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if (rank == 0) {
		printf("请输入投掷次数：\n");
		fflush(stdout);
		scanf_s("%lld", &pointers);
	}
	MPI_Bcast(&pointers, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
	rank_pointers=pointers / comm_sz;
	srand((time(NULL)));
	for (i = 0; i < rank_pointers; i++) {
		x = (double)rand() / (double)RAND_MAX;
		x = 2 * x - 1;
		y = (double)rand() / (double)RAND_MAX;
		y = 2 * y - 1;
		if (x * x + y * y <= 1) {
			rank_count=rank_count+1;
		}
	}
	MPI_Reduce(&rank_count, &count , 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
	if (rank == 0) {
		pi = 4*(double)count / (double)pointers;
		printf("the estimate value of pi is %lf\n", pi);
	}
	MPI_Finalize();
	return 0;
}