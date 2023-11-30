#define _GNU_SOURCE
#define GRADES_FILE "all_std.log"
#define MAX_SIZE 256
#define PROG_NAME "read_grades"
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

int create_input_file_descriptor(char* file_name);
int create_output_file_descriptor(char* file_name);
int* reallocate_array(int** pid_arr, int* max_files, int size, int type);
int call_execv(char* file_name, int output_fd);
int fork_and_exec_child_process(int* pid_arr, char* file_name, int output_fd, int* num_files);
void report_data_summary(int num_stud);
int write_to_output_file(int* pid_arr, int size);