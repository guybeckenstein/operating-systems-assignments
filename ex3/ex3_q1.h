#include "ex3_q1_given.h"

// Constants
#define DELIM "\t\b\n "
#define NUM_WRITER_THREADS 5

// Functions
void get_input_from_file(char* file_name);
void* read_student_from_file(void* ptr);
struct student get_student(char* student_details_line, char** tmp);
void reader_threads_finished_msg(int num_threads);
void* write_grades(void* ptr);
int print_grade_using_writer_thread(int thread_num);
void switch_case(int thread_num);
void declare_writer_thread_grades_responsibility(int thread_num);
void writer_threads_finished_msg();

// Global variables
struct all_students all_stud;
int all_files_read = 0; // For reader threads (1 if all input files were read)
int num_thread_exec = 0; // For the writer threads (printing in the beginning)
int INT = 0; // For the writer threads (when printing student's details)

pthread_mutex_t read_mutex; // Read lock
pthread_mutex_t write_mutex; // Write lock
pthread_mutex_t print_grade_mutex; // Print grade lock
pthread_cond_t initial_message_cv; // Condition Variable

pthread_cond_t writer_thread_cv[NUM_WRITER_THREADS]; // Writer threads Condition Variables