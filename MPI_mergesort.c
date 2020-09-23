#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<math.h>
#include<string.h>
#include<mpi.h>

#define RMAX 1000

void Print_list(int local_A[], int local_n, int rank);
void Print_local_lists(int local_A[], int local_n,
    int my_rank, int p, MPI_Comm comm);
//读取输入的要排序的数的个数，并计算每个进程需要排序的个数
void read_n(int* n, int my_rank, MPI_Comm comm, int comm_sz, int* size);
//根据节点在通信树中的最高度为数组分配空间
void allocate_arrays(int** left, int** right, int** merge, int size, int height, int* myHeight, int my_rank);
//在left数组中生成size个随机数
void randnum(int* left, int size, int my_rank);
//以下三个函数时用于在通信树的叶节点对生成在left中的size个随机数进行快速排序
void swap(int* a, int* b);
int partition(int* left, int lo, int hi);
void QuickSort(int* left, int lo, int hi);
//父节点将接收子节点的数据与自己节点的数据进行归并
void merge_sort(int* left, int* right, int* merge, int size);
//控制通信，确定从哪个子节点中接收数据或向哪个父节点发送数据
void communicate(int Height, int my_rank, int size, int* right, int* left, int* merge, MPI_Comm comm);

int main(int argc, char* argv[]) {
    int n, comm_sz, my_rank;
    int* left;
    int* right;
    int* merge;
    int size;
    int myHeight = 0;
    MPI_Comm comm;
    int height = 0;

    MPI_Init(NULL, NULL);
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &comm_sz);
    MPI_Comm_rank(comm, &my_rank);
    read_n(&n, my_rank, comm, comm_sz, &size);
    int tt = comm_sz;
    for (int i = 0; i < comm_sz; i++) {
        tt = tt / 2;
        if (tt == 0)
            break;
        height++;
    }
    allocate_arrays(&left, &right, &merge, size, height, &myHeight, my_rank);

    randnum(left, size, my_rank);
    Print_local_lists(left, size, my_rank, comm_sz, comm);
    QuickSort(left, 0, size - 1);

    communicate(height, my_rank, size, right, left, merge, comm);
    if (my_rank == 0) {
        printf("The global list is:\n");
        if (height % 2 != 0)
            for (int i = 0; i < n; i++)
                printf("%d    ", merge[i]);
        else
            for (int i = 0; i < n; i++)
                printf("%d    ", left[i]);
        printf("\n");
    }
    MPI_Finalize();
    free(left);
    free(right);
    free(merge);

}
void read_n(int* n, int my_rank, MPI_Comm comm, int comm_sz, int* size) {
    if (my_rank == 0) {
        printf("Input the value of n:\n");
        fflush(stdout);
        scanf_s("%d", n);
    }
    MPI_Bcast(n, 1, MPI_INT, 0, comm);
    *size = *n / comm_sz;
}
void allocate_arrays(int** left, int** right, int** merge, int size, int height, int* myHeight, int my_rank) {
    for (int i = 0; i < height; i++) {
        int parent = my_rank & ~(1 << i);
        if (parent != my_rank)
            break;
        (*myHeight)++;
    }
    *left = malloc((1 << (*myHeight)) * size * sizeof(int));

    *right = malloc((1 << (*myHeight - 1)) * size * sizeof(int));
    *merge = malloc((1 << (*myHeight)) * size * sizeof(int));

}
void randnum(int* left, int size, int my_rank) {
    srand(my_rank+1);
    for (int i = 0; i < size; i++)
        left[i] = rand()%RMAX;
}
void swap(int* a, int* b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}
int partition(int* left, int lo, int hi)
{
    int key = left[hi];
    int i = lo - 1;
    for (int j = lo; j < hi; j++)
    {
        if (left[j] <= key)
        {
            i = i + 1;
            swap(&left[i], &left[j]);
        }
    }
    swap(&left[i + 1], &left[hi]);
    return i + 1;
}
void QuickSort(int* left, int lo, int hi)
{
    if (lo < hi)
    {
        int k = partition(left, lo, hi);
        QuickSort(left, lo, k - 1);
        QuickSort(left, k + 1, hi);
    }
}
void merge_sort(int* left, int* right, int* merge, int size) {

    int lp = 0, rp = 0, mp = 0;
    for (int i = 0; i < 2 * size; i++) {
        if (left[lp] <= right[rp]) {
            merge[mp] = left[lp];
            lp++;
            mp++;
        }
        else {
            merge[mp] = right[rp];
            rp++;
            mp++;
        }
        if (lp == size || rp == size)
            break;
    }
    if (lp == size) {
        memcpy(&merge[mp], &right[rp], (size - rp) * sizeof(int));
    }
    else {
        memcpy(&merge[mp], &left[lp], (size - lp) * sizeof(int));
    }
}
void communicate(int Height, int my_rank, int size, int* right, int* left, int* merge, MPI_Comm comm)
{
    for (int i = 0; i < Height; i++) {
        int parent = my_rank & ~(1 << i);
        if (parent == my_rank) {
            int rtChild = my_rank | (1 << i);

            MPI_Recv(right, size, MPI_INT, rtChild, 0, comm, MPI_STATUS_IGNORE);

            merge_sort(left, right, merge, size);

            int* temp;
            temp = left;
            left = merge;
            merge = temp;
            size *= 2;

        }
        else {
            MPI_Send(left, size, MPI_INT, parent, 0, comm);
            break;

        }
    }

}
/*-------------------------------------------------------------------
 * Only called by process 0
 */
void Print_list(int local_A[], int local_n, int rank) {
    int i;
    printf("%d: ", rank);
    for (i = 0; i < local_n; i++)
        printf("%d ", local_A[i]);
    printf("\n");
}  /* Print_list */

/*-------------------------------------------------------------------
 * 函数名：   Print_local_lists
 * 功能：     打印输出每个进程当前数列内容
 * 输入参数：  全部
 * 注释：
 * 1.  假定所有参与运算的进程都贡献了local_n个元素

 */
void Print_local_lists(int local_A[], int local_n,
    int my_rank, int p, MPI_Comm comm) {
    int* A;
    int        q;
    MPI_Status status;

    if (my_rank == 0) {
        A = (int*)malloc(local_n * sizeof(int));
        Print_list(local_A, local_n, my_rank);
        for (q = 1; q < p; q++) {
            MPI_Recv(A, local_n, MPI_INT, q, 0, comm, &status);
            Print_list(A, local_n, q);
        }
        printf("\n");
        free(A);
    }
    else {
        MPI_Send(local_A, local_n, MPI_INT, 0, 0, comm);
    }
}  /* Print_local_lists */