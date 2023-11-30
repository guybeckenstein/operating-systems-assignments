#define _GNU_SOURCE
#define PROG_NAME "one_student"
#define DELIM "\t\b\n "

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <signal.h>

int create_input_file_descriptor();
int create_output_file_descriptor();
char** reallocate_array(char*** arguments_arr, int* max_arguments, int size, int type);
char** create_argv(char* student_details_line);
void free_arg_vec(char** arg_vec);
int call_execv(char* student_details_line, int input_fd, int output_fd);
int fork_and_exec_child_process(char* student_details_line, int input_fd, int output_fd, int* num_stud);
void report_data_summary(int num_stud);