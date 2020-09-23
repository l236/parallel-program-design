#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>



void Usage(char* program){
	fprintf(stderr,"usage: %s ",program);
	fprintf(stderr,"<桶数 b> <线程数> <区间左侧值(double)> <区间右侧值(double)>");
	exit(0);
}

void Get_args(
		char* argv[],
		int* bin_count,
		int* thread_count,
		double* left,
		double* right){
			*bin_count=atoi(argv[1]);
			*thread_count=atoi(argv[2]);
			*left=atof(argv[3]);
			*right=atof(argv[4]);
}

int main(int argc,char* argv[]){
	int i;
	double left=0.0,right=0.0;
	int bin_count=0,thread_count=0;
	if(argc!=5) Usage(argv[0]);
	Get_args(argv,&bin_count,&thread_count,&left,&right);
	int data_count;
	scanf("%d",&data_count);
	double* data=(double*)malloc(data_count*sizeof(double));
	for(i=0;i<data_count;i++)
		scanf("%lf",&data[i]);
	int* bin_counts=malloc(bin_count*sizeof(int));
	for(i=0;i<bin_count;i++)
		bin_counts[i]=0;
	# pragma omp parallel num_threads(thread_count)
	{
		int bin;
		int my_rank=omp_get_thread_num();
		int data_left=data_count/thread_count*(my_rank);
		int data_right=data_count/thread_count*(my_rank+1);
		int* local_bin_counts=malloc(bin_count*sizeof(int));
		for(int i=0;i<bin_count;i++)
			local_bin_counts[i]=0;
		for(i=data_left;i<data_right;i++){
			bin=(data[i]-left)/(right-left)*bin_count;
			local_bin_counts[bin]++;
		}
		# pragma omp critical
		{
			for(int i=0;i<bin_count;i++)
				bin_counts[i]+=local_bin_counts[i];
		}
	}
	double tmp=(right-left)/bin_count;
	for(i=0;i<bin_count;i++){
		printf("From %.3lf to %.3lf: %d\n", left+i*tmp, left+(i+1)*tmp,bin_counts[i]);
	}
}
