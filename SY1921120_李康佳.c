#include<stdlib.h>
#include<pthread.h>
#include<stdio.h>
#include<semaphore.h>

double a;
double b;
int n;
double sum=0.0;
int thread_count;
int flag=0;
pthread_mutex_t MUTEX;
sem_t sem;

double f(double x){
	return x*(x+1);
}

void* busy_waiting(void* rank){
	long my_rank=(long)rank;
	double interval=(b-a)/thread_count;
	double h=(b-a)/n;
	int local_n=n/thread_count;
	double local_a=a+my_rank*interval;
	double local_b=local_a+interval;
	double local_sum=0.0;
	local_sum+=(f(local_a)+f(local_b))/2.0;
	for(int i=1;i<local_n;i++)
		local_sum+=f(local_a+i*h);
	local_sum*=h;
	while(flag!=my_rank);
	sum+=local_sum;
	flag+=1;
	return NULL;
}

void* mutex(void* rank){
	long my_rank=(long)rank;
	double interval=(b-a)/thread_count;
	double h=(b-a)/n;
	int local_n=n/thread_count;
	double local_a=a+my_rank*interval;
	double local_b=local_a+interval;
	double local_sum=0.0;
	local_sum+=(f(local_a)+f(local_b))/2.0;
	for(int i=1;i<local_n;i++)
		local_sum+=f(local_a+i*h);
	local_sum*=h;
	
	pthread_mutex_lock(&MUTEX);
	sum+=local_sum;
	pthread_mutex_unlock(&MUTEX);
	
	return NULL;
}

void* semophore(void* rank){
	long my_rank=(long)rank;
	double interval=(b-a)/thread_count;
	double h=(b-a)/n;
	int local_n=n/thread_count;
	double local_a=a+my_rank*interval;
	double local_b=local_a+interval;
	double local_sum=0.0;
	local_sum+=(f(local_a)+f(local_b))/2.0;
	for(int i=1;i<local_n;i++)
		local_sum+=f(local_a+i*h);
	local_sum*=h;
	sem_wait(&sem);
	sum+=local_sum;
	sem_post(&sem);
	return NULL;
}

int main(int argc,char* argv[]){
	int method;                       //当 method 值为 1 时使用忙等待， method 值为 2 时使用互斥量， method 值为3 时使用信号量。
	thread_count=atoi(argv[1]);
	method=atoi(argv[2]);
	pthread_t* thread_handles;
	thread_handles=(pthread_t*)malloc(thread_count*sizeof(pthread_t));
	printf("Input the value of a,b,n:\n");
	scanf("%lf%lf%d",&a,&b,&n);
	if(method==1){
		for(long thread=0;thread<thread_count;thread++)
			pthread_create(&thread_handles[thread],NULL,busy_waiting,(void*)thread);
		for(long thread=0;thread<thread_count;thread++)
			pthread_join(thread_handles[thread],NULL);
	}else if(method==2){
		pthread_mutex_init(&MUTEX,NULL);
		for(long thread=0;thread<thread_count;thread++)
			pthread_create(&thread_handles[thread],NULL,mutex,(void*)thread);
		for(long thread=0;thread<thread_count;thread++)
			pthread_join(thread_handles[thread],NULL);
		pthread_mutex_destroy(&MUTEX);
	}else{
		sem_init(&sem,0,1);
		for(long thread=0;thread<thread_count;thread++)
			pthread_create(&thread_handles[thread],NULL,semophore,(void*)thread);
		for(long thread=0;thread<thread_count;thread++)
			pthread_join(thread_handles[thread],NULL);
		sem_destroy(&sem);
	}
	printf("Estimate of the integral:%lf\n",sum);
	free(thread_handles);
	return 0;
}