[b, r] = deconv ([3, 6], [1, 2, 3]);
b == 0 && all (all (r == [0, 3, 6]))
