(vander (1) == 1 && vander ([1, 2, 3]) == vander ([1; 2; 3])
 && vander ([1, 2, 3]) == [1, 1, 1; 4, 2, 1; 9, 3, 1]
 && vander ([1, 2, 3]*i) == [-1, i, 1; -4, 2i, 1; -9, 3i, 1])
