#include "read_grades.h"
//-----------------------------------------------
int create_input_file_descriptor()
{
	// This function is enabling input file redirection for 'PROG_NAME' by creating a new file descriptor
    int input_fd = open(PROG_NAME, O_RDONLY);
    if (input_fd < 0) {
        fprintf(stderr, "Failed to create input file descriptor for '%s'\n", PROG_NAME);
        exit(1);
    }
    return input_fd;
}
//-----------------------------------------------
int create_output_file_descriptor()
{
	// This function is enabling output file redirection for 'pid.temp' by creating a new file descriptor
    char file_name[64];
    snprintf(file_name, sizeof(file_name), "%d.temp", getpid());
	// O_WRONLY == write only | O_CREATE == create new file | O_TRUNC == if a file already exists, overwrite it
    int output_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd < 0) {
        fprintf(stderr, "Failed to create output file '%s'\n", file_name);
        exit(1);
    }
    return output_fd;
}
//-----------------------------------------------
char** reallocate_array(char*** arguments_arr, int* max_arguments, int size, int type)
{
	// This function reallocates 'arguments_arr' to a new size, depends of the value of 'type'
	if (*max_arguments == size)
	{
		if (type == 0)
			*max_arguments = *max_arguments * 2;
		else
			*max_arguments = size + 1;
		char** new_arguments_arr = (char**)realloc(*arguments_arr, *max_arguments * sizeof(char*));
		if (new_arguments_arr == NULL)
		{
			fprintf(stderr, "Failed to reallocate memory for arguments vector in child process %d\n", getpid());
			exit(1);
		}
		return new_arguments_arr;
	}
	return *arguments_arr;
}
//-----------------------------------------------
char** create_argv(char* student_details_line)
{
	// This function creates an argument vector (argv), i.e., a char** array for calling exec system call
    int size = 0, max_arguments = 2;

    char** arguments_arr = (char**)malloc(max_arguments * sizeof(char*));
    if (arguments_arr == NULL)
    {
        fprintf(stderr, "Arguments vector memory allocation failed, in child process %d\n", getpid());
        exit(1);
    }
    arguments_arr[size++] = strdup(PROG_NAME); // Allocates new memory + copies program name
    // Get the first token
    char* token = strtok(student_details_line, DELIM);
    arguments_arr[size++] = strdup(token);
    // Walk through other tokens
    while (1) 
    {
		arguments_arr = reallocate_array(&arguments_arr, &max_arguments, size, 0);
    	// Get new token, add it to arguments vector and update the amount of arguments
    	if ((token = strtok(NULL, DELIM)) != NULL)
            arguments_arr[size++] = strdup(token);
		else // No more tokens
			break;
    }
	// Function's output
    return reallocate_array(&arguments_arr, &max_arguments, size, 1);
}
//-----------------------------------------------
void free_arg_vec(char** arg_vec)
{
	// This function frees the allocated memory we don't need anymore
	for(int i = 0; arg_vec[i] != NULL; i++)
		free(arg_vec[i]);
	free(arg_vec);
}
//-----------------------------------------------
int call_execv(char* student_details_line, int input_fd, int output_fd)
{
	// This function is performing file redirection, and then calling exec (on a fork process) to execute 'PROG_NAME'
	dup2(input_fd, STDIN_FILENO); // Redirects stdin to input file descriptor
	dup2(output_fd, STDOUT_FILENO); // Redirects stdout to output file descriptor
    char** arg_vec = create_argv(student_details_line);
    execv(arg_vec[0], arg_vec);
	free_arg_vec(arg_vec);
    // Error
    fprintf(stderr, "*** ERROR: *** exec() of %s in child process %d failed!\n", arg_vec[0], getpid());
	if (close(input_fd) == -1)
		fprintf(stderr, "*** ERROR: *** close(input_fd) in child process %d failed!\n", getpid());
	if (close(output_fd) == -1)
		fprintf(stderr, "*** ERROR: *** close(output_fd) in child process %d failed!\n", getpid());
    return 1;
}
//-----------------------------------------------
int fork_and_exec_child_process(char* student_details_line, int input_fd, int output_fd, int* num_stud)
{
	// This function is creating a child process using fork() system call, and later calling exec() system call
	// on child process. Parent process increases num_stud for each fork it does
	int pid = fork();
	if (pid < 0)
	{
		fprintf(stderr, "Failed to execute fork() in parent process %d\n", getpid());
		return -1;
	}
	else if (pid == 0)
	{
		if (call_execv(student_details_line, input_fd, output_fd) == 1)
		{
			fprintf(stderr, "Failed to execute exec() in child process %d\n", getpid());
			return -1;
		}
	}
	else
		(*num_stud)++;
	return 0;
}
//-----------------------------------------------
void report_data_summary(int num_stud)
{
	// This function is mandatory as written in ex2 file
    fprintf(stdout, "process: %d: Completed grade calculation for %d students.\n", getpid(), num_stud);
}
//-----------------------------------------------
int main(int argc, char* argv[])
{
	// This main function creates file descriptors (input and output), and get students' information from STDIN
	// For each student, it calls fork_and_exec_child_process(...) function, ultimately it prints info to STDOUT
	char* student_details_line = NULL;
    size_t len;
	int num_stud = 0;
	// Creating file descriptors for file redirections
	int input_fd = create_input_file_descriptor(), output_fd = create_output_file_descriptor();
	while (getline(&student_details_line, &len, stdin) != -1)
	{
		int res = fork_and_exec_child_process(student_details_line, input_fd, output_fd, &num_stud);
		if (res == -1)
			return 1;
	}
	while((wait(NULL) != -1) || errno != ECHILD)
	{
	}
	// Closing file descriptors, including edge cases check
	if (close(input_fd) == -1)
	{
		if (close(output_fd) == -1)
			fprintf(stderr, "*** ERROR: *** close(output_fd) in child process %d failed!\n", getpid());
		fprintf(stderr, "*** ERROR: *** close(input_fd) in child process %d failed!\n", getpid());
		return 1;
	}
	if (close(output_fd) == -1)
	{
		fprintf(stderr, "*** ERROR: *** close(output_fd) in child process %d failed!\n", getpid());
		return 1;
	}
	// Printing information to STDOUT
	report_data_summary(num_stud);
	return 0;
}