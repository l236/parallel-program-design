#!/bin/bash
# 课程中心上传sh脚本会出现问题，遂改后缀为txt了 
# 使用时将后缀改回sh
# usage: 
# chmod +x run.sh
# ./run.sh <二进制文件>
file=$1
proc=4
thd_per_proc=16 
real_lower=-2
real_upper=2
imag_lower=-2 
imag_upper=2
w=400
h=400
output_path="./${file}.png"
if [ "$file" == "sequential" ]; then
    ./${file} ${thd_per_proc} ${real_lower} ${real_upper} ${imag_lower} ${imag_upper} ${w} ${h} ${output_path}
else
    /data/software/intel/compilers_and_libraries_2016.3.210/linux/mpi/intel64/bin/mpirun -n ${proc} ./${file} ${thd_per_proc} ${real_lower} ${real_upper} ${imag_lower} ${imag_upper} ${w} ${h} ${output_path}
fi
