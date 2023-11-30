#include "one_student.h"

int main(int argc, char* argv[])
{
	// We did not get any arguments, so we cannot return student's details
    if (argc <= 1)
    {
        fprintf(stderr, "Program %s did not receive any arguments\n", argv[0]);
        return 1;
    }
	// We only have a student's name as argument, we don't know his grades (or average)
    else if (argc == 2)
    {
        fprintf(stdout, "%s NaN\n", argv[1]);
        return 0;
    }
	else
	{
		// Retrieving (student name, student average grade)
		double average_grade = 0;
		int total_grades = 0;

		// Iterating over student's grades
		for (int i = 2; 0 < argc && i < argc; i++)
		{
			average_grade += strtol(argv[i], NULL, 10);
			total_grades++;
		}
		// Prints student name and his average grade (second argument is student's name)
		if (total_grades == 0)
			fprintf(stdout, "%s NaN\n", argv[1]);
		else
		{
			average_grade = average_grade / total_grades;
			fprintf(stdout, "%s %.1f\n", argv[1], average_grade);
		}

		return 0;
	}
}