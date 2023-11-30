#include "ex2_q1.h"
//-----------------------------------------------
int create_input_file_descriptor(char* file_name)
{
	// This function is enabling input file redirection for 'file_name' by creating a new file descriptor
	int input_fd = open(file_name, O_RDONLY);
    if (input_fd < 0) {
        fprintf(stderr, "Failed to create input file descriptor for '%s'\n", PROG_NAME);
        exit(1);
    }
    return input_fd;
}
//-----------------------------------------------
int create_output_file_descriptor(char* file_name)
{
	// This function is enabling output file redirection for 'pid.temp' by creating a new file descriptor
	// O_WRONLY == write only | O_CREATE == create new file | O_TRUNC == if a file already exists, overwrite it
	int output_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd < 0) {
        fprintf(stderr, "Failed to create output file '%s'\n", file_name);
        exit(1);
    }
    return output_fd;
}
//-----------------------------------------------
int* reallocate_array(int** pid_arr, int* max_files, int size, int type)
{
	// This function reallocates 'arguments_arr' to a new size, depends of the value of 'type'
	if (*max_files == size)
	{
		if (type == 0)
			*max_files = *max_files * 2;
		else
			*max_files = size;
		int* new_pid_arr = (int*)realloc(*pid_arr, *max_files * sizeof(int));
		if (new_pid_arr == NULL)
		{
			fprintf(stderr, "Failed to reallocate memory for arguments vector in child process %d\n", getpid());
			exit(1);
		}
		return new_pid_arr;
	}
	return *pid_arr;
}
//-----------------------------------------------
int call_execv(char* file_name, int output_fd)
{
	// This function is performing file redirection, and then calling exec (on a fork process) to execute 'PROG_NAME'
	int input_fd = create_input_file_descriptor(file_name);
	dup2(input_fd, STDIN_FILENO); // Redirects stdin to input file descriptor
	dup2(output_fd, STDOUT_FILENO); // Redirects stdout to output file descriptor
	char* arg_vec[2] = { PROG_NAME, NULL };
	execv(arg_vec[0], arg_vec);
	// Error
	fprintf(stderr, "*** ERROR: *** exec() of %s in child process %d failed!\n", arg_vec[0], getpid());
	if (close(input_fd) == -1)
		fprintf(stderr, "*** ERROR: *** close(input_fd) in child process %d failed!\n", getpid());
	if (close(output_fd) == -1)
		fprintf(stderr, "*** ERROR: *** close(output_fd) in child process %d failed!\n", getpid());
	return 1;
}
//-----------------------------------------------
int fork_and_exec_child_process(int* pid_arr, char* file_name, int output_fd, int* curr_file)
{	
	// This function is creating a child process using fork() system call, and later calling exec() system call
	// on child process. Parent process increases curr_file for each fork it does
	int pid = fork();
	if (pid == 0)
	{
		if (call_execv(file_name, output_fd) == 1)
		{
			fprintf(stderr, "Failed to execute exec() in child process %d\n", getpid());
			return -1;
		}
	}
	else if (pid < 0)
	{
		fprintf(stderr, "Failed to execute fork() in parent process %d\n", getpid());
		return -1;
	}
	else
		pid_arr[(*curr_file)++] = pid;
	return 0;
}
//--------------- --------------------------------
void report_data_summary(int num_stud)
{
	// This function is mandatory as written in ex2 file
	fprintf(stderr, "grade calculation for %d students is done \n", num_stud);
}
//-----------------------------------------------
int write_to_output_file(int* pid_arr, int size)
{
	// This function creates metadata file for each file read. The new file is called 'pid.temp'.
	// Then it prints the total students read by other programs to STDERR
	FILE* fp = fopen(GRADES_FILE, "w");
    if (fp == NULL)
    {
		fprintf(stderr, "Opening %s failed, in child process %d\n", GRADES_FILE, getpid());
        return 1;
    }
	
	int num_stud = 0;
	for (int i = 0; i < size; i++)
	{
		char tmp_file_name[64];
		sprintf(tmp_file_name, "%d.temp", pid_arr[i]);
		FILE* tmp_fp = fopen(tmp_file_name, "r");
		if (tmp_fp == NULL)
		{
		    fprintf(stderr, "Opening temp file %s failed, in child process %d\n", tmp_file_name, getpid());
			if (fclose(fp) == EOF)
				fprintf(stderr, "Closing %s failed, in child process %d\n", GRADES_FILE, getpid());
			return 1;
		}
		
		char* line = NULL;
		size_t len = 0;
		while (getline(&line, &len, tmp_fp) != -1)
		{
			fputs(line, fp);
			num_stud++;
		}
		
		if (fclose(tmp_fp) == EOF)
		{
			fprintf(stderr, "Closing %s failed, in child process %d\n", tmp_file_name, getpid());
			if (fclose(fp) == EOF)
				fprintf(stderr, "Closing %s failed, in child process %d\n", GRADES_FILE, getpid());
			return 1;
		}
	}
	
	if (fclose(fp) == EOF)
	{
		fprintf(stderr, "Closing %s failed, in child process %d\n", GRADES_FILE, getpid());
		return 1;
	}
	report_data_summary(num_stud);
	return 0;
}
//-----------------------------------------------
int main(int argc, char* argv[])
{
	int curr_file = 0, output_fd = -1;
	int* pid_arr;
	
	if (argc >= 2)
		output_fd = create_output_file_descriptor(argv[1]);
	
	// Detects input form file redirection
	if (isatty(0) == 0)
	{
		char curr_file_name[MAX_SIZE];
		int max_files = 2;
		pid_arr = (int*)malloc(max_files * sizeof(int));
        while (scanf("%255s", curr_file_name) == 1)
        {
			if (output_fd == -1)
				output_fd =  create_output_file_descriptor(curr_file_name);
			else
			{
				pid_arr = reallocate_array(&pid_arr, &max_files, curr_file, 0);
				int res = fork_and_exec_child_process(pid_arr, curr_file_name, output_fd, &curr_file);
				if (res == -1)
					return -1;
			}
		}
		pid_arr = reallocate_array(&pid_arr, &max_files, curr_file, 1);
	}
	// In case no input form file redirection detected and no output file given as argument
	else if (output_fd == -1)
	{
		fprintf(stderr, "Program did not get sufficient arguments, in parent process %d\n", getpid());
		return 1;
	}
	// Uses arguments as input
	else
	{
		pid_arr = (int*)malloc((argc - 2) * sizeof(int));
		for(int i = 2; i < argc; i++)
		{
			int res = fork_and_exec_child_process(pid_arr, argv[i], output_fd, &curr_file);
			if (res == -1)
				return -1;
		}
	}
	while((wait(NULL) != -1) || errno != ECHILD)
	{
	}
	
	int pid = fork();
	if (pid == 0)
	{
		int res = write_to_output_file(pid_arr, curr_file);
		return res;
	}
	else if (pid < 0)
	{
		fprintf(stderr, "Failed to execute fork() in parent process %d\n", pid);
		return 1;
	}
	else
	{
		wait(NULL);
		if (close(output_fd) == -1)
			fprintf(stderr, "*** ERROR: *** close(output_fd) in child process %d failed!\n", getpid());
	}
	
	free(pid_arr);
    return 0;
}