Given the American Grade System - _A_, _B_, _C_, _D_, _F_, this console application receives as an input X different files as arguments.

All files contain students' details. Each line in a file is structured in the same way as other lines of other files.

Both codes are a part of assignments in **Operating Systems course**.


## General Information

The `Makefile` enables automating the testing process running the `make` command in `Linux`.

1. Of course I made sure to separate header and code files, sticking to C convention.
2. Output is being printed to the `stdout`, errors to `stdout`.
3. Ex2 uses file redirection (file descriptors).
4. Ex3 uses threads and condition variables, for multithreading.
5. Both exercises use `fork()`, `wait()`, and other functions for the Unix API for **multiprocessing**.

## Ex2
Given a certain structure that each file upholds, the program first uses `children process` for that file. The code of that file is on a different C file.

Later on, each child process uses `file redirection`, because it is on a different file.

The code eventually creates new files, each file has its meaning in terms of the assignment.

## Ex3
Given a certain structure that each file upholds, the program first uses `X threads` for printing the grades of all students, and then `condition variables` and `5 threads` for printing it again.