#define PNG_NO_SETJMP

#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>


const int MAX_ITER = 10000;
void write_png(const char* filename, const int width, const int height, const int* buffer) {
    FILE* fp = fopen(filename, "wb");
    assert(fp);
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(png_ptr);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert(info_ptr);
    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);
    size_t row_size = 3 * width * sizeof(png_byte);
    png_bytep row = (png_bytep)malloc(row_size);
    for (int y = 0; y < height; ++y) {
        memset(row, 0, row_size);
        for (int x = 0; x < width; ++x) {
            int p = buffer[(height - 1 - y) * width + x];
            png_bytep color = row + x * 3;
            if (p != MAX_ITER) {
                if (p & 16) {
                    color[0] = 240;
                    color[1] = color[2] = p % 16 * 16;
                } else {
                    color[0] = p % 16 * 16;
                }
            }
        }
        png_write_row(png_ptr, row);
    }
    free(row);
    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

int calculate(double x0,double y0){
	int repeats = 0;
	double x = 0;
	double y = 0;
	double length_squared = 0;
	while (repeats < MAX_ITER && length_squared < 4) {
		double temp = x * x - y * y + x0;
		y = 2 * x * y + y0;
		x = temp;
		length_squared = x * x + y * y;
		++repeats;
	}
	return repeats;
}

void calRow(int rowStart,int rowEnd, double left,double right,double lower,double upper,int width,int height,int* image){
	double real,imag;
	double real_scale=(right-left)/width;
	double imag_scale=(upper-lower)/height;
	for(int i=rowStart;i<rowEnd;i++){
		imag=lower+i*imag_scale;
		# pragma omp parallel for schedule(dynamic)
		for(int j=0;j<width;j++){
			real=left+j*real_scale;
			image[(i-rowStart)*width+j]=calculate(real,imag);
		}
	}
}

int main(int argc, char** argv) {
    /* argument parsing */
    assert(argc == 9);
    int num_threads = strtol(argv[1], 0, 10);
    double left = strtod(argv[2], 0);
    double right = strtod(argv[3], 0);
    double lower = strtod(argv[4], 0);
    double upper = strtod(argv[5], 0);
    int width = strtol(argv[6], 0, 10);
    int height = strtol(argv[7], 0, 10);
    const char* filename = argv[8];

	int comm_sz;
	int my_rank;
	int rank;
	double start,finish;
	
    /* allocate memory for image */
    int* image = (int*)malloc(width * height * sizeof(int));
	
    assert(image);

	MPI_Init(&argc,NULL);
	MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	MPI_Status status;
	
	start=MPI_Wtime();
	
	int* imageP=(int*)malloc(width*sizeof(int));
	int row=0;
	int activeP=comm_sz;
	int tmpRow;
	
	calRow(my_rank,my_rank+1,left,right,lower,upper,width,height,imageP);

	if(my_rank==0){
		for(int i=1;i<comm_sz;i++){
			MPI_Send(&row,1,MPI_INT,i,0,MPI_COMM_WORLD);
			row++;
		}
		while(row<height+comm_sz-1){
			MPI_Recv(&tmpRow,1,MPI_INT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&status);
			rank=status.MPI_SOURCE;
			MPI_Recv(&image[tmpRow*width],width,MPI_INT,rank,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			MPI_Send(&row,1,MPI_INT,rank,0,MPI_COMM_WORLD);
			row++;	
		}
		write_png(filename, width, height, image);
		free(image);
		finish=MPI_Wtime();
		printf("Elapsed time = %lf seconds\n", finish-start);
	}else{
		MPI_Recv(&row,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
		while(row<height){
			calRow(row,row+1,left,right,lower,upper,width,height,imageP);
			MPI_Send(&row,1,MPI_INT,0,1,MPI_COMM_WORLD);
			MPI_Send(imageP,width,MPI_INT,0,2,MPI_COMM_WORLD);	
			MPI_Recv(&row,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		}
	}	
	MPI_Finalize();
	return 0;
}
