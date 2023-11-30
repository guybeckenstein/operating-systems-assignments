#include "ex3_q1.h"
//-----------------------------------------------
int main(int argc, char* argv[]) 
{
	// Locks and CV initialization
	pthread_mutex_init(&read_mutex, NULL);
	pthread_mutex_init(&write_mutex, NULL);
	pthread_mutex_init(&print_grade_mutex, NULL);
    pthread_cond_init(&initial_message_cv, NULL);
	for(int i = 0; i < NUM_WRITER_THREADS; i++)
		pthread_cond_init(&(writer_thread_cv[i]), NULL);
	
	// Main thread creates (argc - 1) reader threads, for reading files that are given as arguments
	for(int i = 1; i < argc; i++)
	{
		if (i == argc - 1)
		{
			pthread_mutex_lock(&read_mutex);
			all_files_read = 1;
			pthread_mutex_unlock(&read_mutex);
		}
		get_input_from_file(argv[i]);
	}
	// Main thread prints message that reader threads are finished
	reader_threads_finished_msg(argc - 1);
	
	pthread_t writer_threads[NUM_WRITER_THREADS];
	int thread_num[NUM_WRITER_THREADS];
	for(int i = 0; i < NUM_WRITER_THREADS; i++)
	{
		thread_num[i] = i;
		if (pthread_create(&writer_threads[i], NULL, write_grades, &(thread_num[i])) != 0)
		{
			fprintf(stderr, "Creating new thread attempt failed, in %s\n", __func__);
			exit(1);
		}
	}
	// Waiting for five children threads
	for(int i = 0; i < NUM_WRITER_THREADS; i++)
		pthread_join(writer_threads[i], NULL);
    
	// Main thread prints message that writer threads are finished
	writer_threads_finished_msg();
	// Clean up and exit
	pthread_mutex_destroy(&read_mutex);
	pthread_mutex_destroy(&write_mutex);
	pthread_mutex_destroy(&print_grade_mutex);
    pthread_cond_destroy(&initial_message_cv);
	for(int i = 0; i < NUM_WRITER_THREADS; i++)
		pthread_cond_destroy(&(writer_thread_cv[i]));
	pthread_exit(NULL);
}
//-----------------------------------------------
void get_input_from_file(char* file_name) 
{
	pthread_t reader_thread;
	if (pthread_create(&reader_thread, NULL, read_student_from_file, file_name) != 0)
	{
        fprintf(stderr, "Creating new thread attempt failed, in %s\n", __func__);
        exit(1);
	}
	else if (pthread_join(reader_thread, NULL) != 0)
	{
        fprintf(stderr, "Main thread wait attempt to new thread failed, in %s\n", __func__);
        exit(1);
	}
}
//-----------------------------------------------
void* read_student_from_file(void* ptr) 
{
	char* file_name = (char*)ptr;
	char* line, * tmp;
    size_t len;
	// Open file then read data of all students
    FILE* fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Opening file for reading failed, in child process %d\n", getpid());
        exit(1);
    }
	while(getline(&line, &len, fp) != -1)
	{
		// Read data of each student
		struct student curr_student = get_student(line, &tmp);
		add_to_student_arr(&curr_student);
	}
	// Close file
	if (fclose(fp) == EOF)
	{
		fprintf(stderr, "Closing '%s' file failed, in %s\n", file_name, __func__);
		exit(1);
	}
	pthread_mutex_lock(&read_mutex);
	// Last reader thread calls `print_student_arr` function
	if (all_files_read == 1)
		print_student_arr();
	pthread_mutex_unlock(&read_mutex);
	// Exit thread
	pthread_exit(NULL);
}
//-----------------------------------------------
struct student get_student(char* student_details_line, char** tmp) 
{
	struct student new_student;
	new_student.avg_grade = 0;
	int total_grades = 0;
    // Get the first token
    char* token = strtok_r(student_details_line, DELIM, tmp);
	if (token == NULL)
	{
        fprintf(stderr, "Empty line given, in %s\n", __func__);
        exit(1);
	}
	else
		strncpy(new_student.name, token, strlen(token) + 1); // Allocates memory for student's name
    // Walk through over other tokens, i.e., the grades
    while(1) 
    {
    	// Get new token, add it to sum of grades
    	if ((token = strtok_r(NULL, DELIM, tmp)) != NULL)
        {
			new_student.avg_grade += strtol(token, NULL, 10);
			total_grades++;
		}
		else // No more tokens
			break;
    }
	
	new_student.avg_grade /= total_grades; // Get average grade of student
	
    return new_student;
}
//-----------------------------------------------
void reader_threads_finished_msg(int num_threads) 
{
	fprintf(stderr, "all %d reader-threads terminated\n", num_threads);
}
//-----------------------------------------------
void* write_grades(void* ptr) 
{
	// Printing messages
	int thread_num = *((int*)ptr);
	declare_writer_thread_grades_responsibility(thread_num);
	// Print grades
	while (INT < all_stud.count)
		switch_case(thread_num);
	for (int i = 0; i < NUM_WRITER_THREADS; i++)
		pthread_cond_signal(&(writer_thread_cv[i]));
	// Exit thread
	pthread_exit(NULL);
}
void declare_writer_thread_grades_responsibility(int thread_num)
{
	if (thread_num < 4)
		printer_thread_msg('A' + thread_num);
	else
		printer_thread_msg('F');
	// Update number of writer threads that wrote initial message
	pthread_mutex_lock(&write_mutex);
	num_thread_exec++;
	pthread_mutex_unlock(&write_mutex);
	if (num_thread_exec == NUM_WRITER_THREADS)
	{
		// If NUM_WRITER_THREADS threads wrote the message, we continue
		pthread_mutex_lock(&print_grade_mutex);
        pthread_cond_broadcast(&initial_message_cv); // Wake up all sleeping threads
		pthread_mutex_unlock(&print_grade_mutex);
	}
	else
	{
		// Wait until num_thread_exec reaches NUM_WRITER_THREADS
		pthread_mutex_lock(&print_grade_mutex);
		while (num_thread_exec < NUM_WRITER_THREADS)
			pthread_cond_wait(&initial_message_cv, &print_grade_mutex);
		pthread_mutex_unlock(&print_grade_mutex);
	}
}
//-----------------------------------------------
void switch_case(int thread_num)
{
	int changed_value = 0;
	int wake_up_writer_thread_idx;
	
	pthread_mutex_lock(&write_mutex);
	while(changed_value == 0 && INT < all_stud.count)
	{
		changed_value = print_grade_using_writer_thread(thread_num);
		if (INT < all_stud.count)
		{
			double avg_grade = all_stud.stud_arr[INT].avg_grade;
			if (90.0 <= avg_grade)
				wake_up_writer_thread_idx = 0;
			else if (80.0 <= avg_grade && avg_grade < 90.0)
				wake_up_writer_thread_idx = 1;
			else if (70.0 <= avg_grade && avg_grade < 80.0)
				wake_up_writer_thread_idx = 2;
			else if (60.0 <= avg_grade && avg_grade < 70.0)
				wake_up_writer_thread_idx = 3;
			else if (avg_grade < 60.0)
				wake_up_writer_thread_idx = 4;
			else
			{
				fprintf(stderr, "Wrong thread_num value: %d, in %s\n", thread_num, __func__);
				exit(1);
			}
			if (thread_num == wake_up_writer_thread_idx)
				changed_value = 0;
			else
				pthread_cond_signal(&(writer_thread_cv[wake_up_writer_thread_idx]));
			random_sleep();
		}
	}
	pthread_mutex_unlock(&write_mutex);
}
//-----------------------------------------------
int print_grade_using_writer_thread(int thread_num)
{
	int changed_value = 0;
	double avg_grade = all_stud.stud_arr[INT].avg_grade;
	switch (thread_num)
	{
		case 0:
			if (90.0 <= avg_grade)
			{
				print_grade_A(INT);
				changed_value = 1;
			}
			break;
		case 1:
			if (80.0 <= avg_grade && avg_grade < 90.0)
			{
				print_grade_B(INT);
				changed_value = 1;
			}
			break;
		case 2:
			if (70.0 <= avg_grade && avg_grade < 80.0)
			{
				print_grade_C(INT);
				changed_value = 1;
			}
			break;
		case 3:
			if (60.0 <= avg_grade && avg_grade < 70.0)
			{
				print_grade_D(INT);
				changed_value = 1;
			}
			break;
		case 4:
			if (avg_grade < 60.0)
			{
				print_grade_F(INT);
				changed_value = 1;
			}
			break;
		default:
			fprintf(stderr, "Wrong thread_num value: %d, in %s\n", thread_num, __func__);
			exit(1);
	}
	if (changed_value == 0)
		pthread_cond_wait(&writer_thread_cv[thread_num], &write_mutex);
	else
	{
		INT++;
	}
	return changed_value;
}
//-----------------------------------------------
void writer_threads_finished_msg() 
{
	fprintf(stderr, "all printer-threads terminated\n");
}