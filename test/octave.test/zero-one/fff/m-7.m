prefer_zero_one_indexing = 0;
prefer_column_vectors = 0;
do_fortran_indexing = 0;
a = [9,8;7,6];
all (a(0:1,[2,1]) == [6,7])
