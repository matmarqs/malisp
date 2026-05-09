# mal in C

A fast way to test the current code is:
```
cat tests.txt | valgrind ./stepX_current  # actual tests
cat tests.txt | valgrind --leak-check=full ./stepX_current  # test for memory leaks
```
