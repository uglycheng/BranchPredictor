make
bunzip2 -kc ../traces/fp_1.bz2| ./predictor --custom6>>../../report/fp1.txt
bunzip2 -kc ../traces/fp_2.bz2| ./predictor --custom6>>../../report/fp2.txt
bunzip2 -kc ../traces/int_1.bz2| ./predictor --custom6>>../../report/int1.txt
bunzip2 -kc ../traces/int_2.bz2| ./predictor --custom6>>../../report/int2.txt
bunzip2 -kc ../traces/mm_1.bz2| ./predictor --custom6>>../../report/mm1.txt
bunzip2 -kc ../traces/mm_2.bz2| ./predictor --custom6>>../../report/mm2.txt
