make
echo "PREDICT fp_1:"
bunzip2 -kc ../traces/fp_1.bz2 | ./predictor --$1
echo "PREDICT fp_2:"
bunzip2 -kc ../traces/fp_2.bz2 | ./predictor --$1
echo "PREDICT int_1:"
bunzip2 -kc ../traces/int_1.bz2 | ./predictor --$1
echo "PREDICT int_2:"
bunzip2 -kc ../traces/int_2.bz2 | ./predictor --$1
echo "PREDICT mm_1:"
bunzip2 -kc ../traces/mm_1.bz2 | ./predictor --$1
echo "PREDICT mm_2:"
bunzip2 -kc ../traces/mm_2.bz2 | ./predictor --$1
make clean
