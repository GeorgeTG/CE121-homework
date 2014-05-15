
/*  Programming II - CE121
 *  Homework 1
 *  ____________________________
 *  Giorgos Gougkoudis
 *  Konstantinos Kanellis
 *  ____________________________
 *
 *  User entries are saved in a sorted format, each line for one entry.
 *  Line size is fixed for easy access and parsing.
 *  First sorting key is Course name, and the second one is date.
 *  Ex. Format :
 *  this is a course name 1 <fill with whitespaces> ?MM/YY?Grade
 *  Since entries are sorted and the line size is fixed, we can
 *  use Binary Search to perform all searching queries.
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>

/* Usefull info about file formation */
#define DELIMITER '?'
#define COURSE_MAX_LENGTH 64
#define LINE_SIZE 76

/* Line size of courses file */
#define COURSES_LINE_SIZE 67

/* User interaction modes */
#define GET_NAME 1
#define GET_DATE 2
#define GET_GRADE 4
#define GET_ALL 7

/* Comparison modes */
#define CMP_NAME  1
#define CMP_DATE  2
#define CMP_GRADE 4
#define CMP_ALL   7

/* Core types */
typedef struct date {
   int month;
   int year;
} Date;

typedef struct course{
    char name[COURSE_MAX_LENGTH + 1];
    Date date;
    float grade;
} Course;

/* Core functions */
void AddCourse(Course course, int fd);
void DeleteCourse(Course course, int gradeFd);
void PrintAllCourses(Course course, int fd);
void CalculateAverage(int gradesFd, int coursesFd);
char SearchCourse(Course course, int fd, Course * lcourse, const int compareMode);

/* General helpers */
ssize_t GetLine(char *str, size_t size);
char IsCourseAvailable(int coursesFd,char *courseName);
void GetCourseInfo(Course *course, int gradesFd, int coursesFd, const int mode);
void RemoveTrailingSpaces(char *str);

/* Course driven converters - helpers */
void PrintCourse(const Course course);
void CourseToString(char *str, Course course);
void StringToCourse(Course *course, char *str);

/* Comparators */
int CourseCmp(const Course course1, const Course course2,const int mode);
int DateCmp(const Date date1, const Date date2);

/* Date functions */
char IsValidDate(const Date date);
void GetDateNow(Date *date);

/* File operation wrappers */
int GetLineCount(int fd);
void FileTruncate(int fd, int length);
void FileSync(int fd);
void FileSeek(int fd, int offset, int whence);
void WriteLine(int fd, char *line);
void sWrite(int fd, void *buf, size_t count);
ssize_t CoursesReadLine(int fd, char *line); /* Read line from courses file */
ssize_t ReadLine(int fd,char *buffer); /* Read line from grades file */
ssize_t sRead(int fd, void *buf, size_t count);

int main(int argc, char *argv[]){

    /******************************************************************/
    int coursesFd, gradesFd;
    if( argc == 3 ) {
        coursesFd = open(argv[1] , O_RDONLY, S_IRUSR);
        if( coursesFd == -1) {
            printf("Cannot open \"%s\".\n", argv[1]);
            perror("Error");
            exit(EXIT_FAILURE);
        }

        gradesFd = open( argv[2], O_RDWR | O_CREAT, S_IRWXU);
        if( gradesFd == -1) {
            printf("Cannot open \"%s\".\n", argv[2]);
            perror("Error");
            close(coursesFd); /*this is open by now */
            exit(EXIT_FAILURE);
        }
    }
    else{
        printf("Invalid number of arguments (%d)!\n\n", argc);
        printf("Example: ./hw1 courses.txt grades.txt\n\n");
        exit(EXIT_FAILURE);
    }

    /******************************************************************/

    int selection;
    Course course;

    do{
        printf("\n********** Menu ***********\n");
        printf("1. Add\n2. Delete\n3. Search\n4. Average\n5. Exit\n");
        printf("***************************\n\n");
        do{
            printf("Enter selection: ");
            scanf("%d",&selection);

            if ( (selection < 1 || selection > 5) ){
                printf("Invalid selection! Try 1-5.\n");
           }
        } while(selection < 1 || selection > 5);

        FileSeek(gradesFd, 0, SEEK_SET);

        switch(selection){
            case 1:
                GetCourseInfo(&course, gradesFd, coursesFd, GET_ALL);
                AddCourse(course, gradesFd);
                break;
            case 2:
                GetCourseInfo(&course, gradesFd, coursesFd, GET_NAME | GET_DATE);
                DeleteCourse(course, gradesFd);
                break;
            case 3:
                GetCourseInfo(&course, gradesFd, coursesFd, GET_NAME);
                PrintAllCourses(course, gradesFd);
                break;
            case 4:
                CalculateAverage(gradesFd, coursesFd);
                break;
            case 5:
                printf("Bye :)\n");
                break;
            default:
                break;
        }
    } while( selection != 5 );

    close(gradesFd);
    close(coursesFd);

    return 0;
}

/*****************************************************************************/

void AddCourse(Course course, int fd){
    Course lcourse, scourse;
    char line[LINE_SIZE + 1], sline[LINE_SIZE + 1], nline[LINE_SIZE + 1];
    ssize_t bytesRead;

    strcpy(scourse.name, course.name);

    /* Find if there is already a record of that course at the same date */
    SearchCourse(course, fd, &lcourse, CMP_NAME | CMP_DATE);

    if ( CourseCmp(lcourse, course, CMP_NAME | CMP_DATE) == 0 ) {
        printf("\nYou have already an entry with date: %02d/20%02d!\n",
                lcourse.date.month,
                lcourse.date.year
              );
        return;
    }

    /* Search the for maximum Date in this course
     * Actually find the lower bound!
     */
    GetDateNow(&scourse.date);
    scourse.date.month ++;
    scourse.date.year ++;

    SearchCourse(scourse, fd, &lcourse, CMP_ALL);

    if( CourseCmp(course, lcourse, CMP_NAME) == 0 ){
        if (lcourse.grade >= 5.0 ){
            printf("\n%s already PASSED with grade: %04.1f/10.0, at %02d/20%02d!\n",
                    lcourse.name,
                    lcourse.grade,
                    lcourse.date.month,
                    lcourse.date.year
                    );

            return;
        }
        else if ( course.grade >= 5.0 &&
                ( DateCmp(course.date, lcourse.date) > 0) ){
            printf("\nYou are attempting to add an entry with grade %04.1f/10.0,\n",
                    course.grade);
            printf("and date %02d/20%02d, while there is at least one entry\n",
                    course.date.month,
                    course.date.year
                  );
            printf("with less than required to pass grade, at a later date!\n");
            return;
        }

    }

    /* Search by the closest course by this course */
    if ( SearchCourse(course, fd, &lcourse, CMP_ALL) != 0 ){
        /*File is not empty */

        /* Course should be inserted before */
        if( CourseCmp(course, lcourse, CMP_ALL) < 0  ){
            /* Push all lines by a single line below */
            bytesRead = ReadLine(fd, sline);
            if( bytesRead > 0 ){

                FileSeek(fd, 0, SEEK_END);
                do{
                    FileSeek(fd, -LINE_SIZE, SEEK_CUR);

                    ReadLine(fd, line);
                    WriteLine(fd, line);

                    FileSeek(fd, -2*LINE_SIZE, SEEK_CUR);

                } while( strcmp(line, sline) != 0 );
            }
        }
        else{
            FileSeek(fd, LINE_SIZE, SEEK_CUR);
        }
    }

    /*Convert course to human-readable form */
    CourseToString(nline, course);

    WriteLine(fd, nline);
    FileSync(fd);

    printf("\nCourse added successfully: \n\n");
    PrintCourse(course);
}

/*****************************************************************************/

void DeleteCourse(Course course, int fd){
    char line[LINE_SIZE + 1];
    Course lcourse;

    /* Empty file check */
    if( GetLineCount(fd) == 0 ) {
        printf("\nThere are no grade records in file!\n\n");
        return;
    }

    SearchCourse(course, fd, &lcourse, CMP_NAME | CMP_DATE );
    PrintCourse(lcourse);

    /* If the entry exists */
    if ( CourseCmp(course, lcourse, CMP_NAME | CMP_DATE) == 0 ){

        ssize_t ReadBytes;
        int LineCount = GetLineCount(fd);

        /* Pull all lines below, a single line */
        FileSeek(fd, LINE_SIZE, SEEK_CUR);
        do {
            ReadBytes= ReadLine(fd, line);
            if (ReadBytes == 0) {
                /*if not bytes were read, we have to go back less bytes */
                FileSeek(fd, -LINE_SIZE, SEEK_CUR);
            }
            else {
                FileSeek(fd, -2*LINE_SIZE, SEEK_CUR);
            }

            WriteLine(fd, line);
            FileSeek(fd, LINE_SIZE, SEEK_CUR);

        } while( ReadBytes > 0);

        /* Now the entry we want to remove is the last line */
        FileTruncate(fd, (LineCount - 1) * LINE_SIZE);
        FileSync(fd);

        printf("\nEntry removed successfully!\n");
    }
    else {
        printf("\nEntry not found!\n");
    }
}

/*****************************************************************************/

void PrintAllCourses(Course course, int fd ){
    /* Set the lower bound to maximum date
     * so we get the first entry of many (if exist)
     */
    GetDateNow(&course.date);
    course.date.month ++;
    course.date.year ++;

    Course lcourse;
    SearchCourse(course, fd, &lcourse, CMP_ALL);

    if( CourseCmp(course, lcourse, CMP_NAME) != 0 ){
        printf("\nCan't find \"%s\" in the grades file.\n", course.name);
    }
    else {
        char line[LINE_SIZE + 1];
        ssize_t bytesRead;

        printf("\n");
        /* While we get entries for the same course */
        while( 1 ) {
            bytesRead = ReadLine(fd, line);
            if( bytesRead == 0 )
                break;

            StringToCourse(&lcourse, line);

            if( CourseCmp(course, lcourse, CMP_NAME) != 0 )
                break;

            PrintCourse(lcourse);
        }
    }
}

/*****************************************************************************/

void CalculateAverage(int gradesFd, int coursesFd){
    char line[COURSES_LINE_SIZE + 1];
    Course scourse, lcourse;
    int len, weight;

    /* Empty file check */
    if( GetLineCount(gradesFd) == 0 ){
        printf("\nThere are no grade records in file!\n\n");
        return;
    }

    /* We always search for the greatest grade */
    scourse.grade = 10.0;

    int totalCredits = 0;
    int coursesPassed = 0;
    double sum = 0.0;

    FileSeek(coursesFd, 0, SEEK_SET);
    while( CoursesReadLine(coursesFd, line) > 0 ){
        /* Parse the course name & course weight */
        len = strlen(line);
        weight = (line[len - 2] - '0');

        line[len - 3] = '\0';
        strcpy(scourse.name, line);
        RemoveTrailingSpaces(scourse.name);

        /* Perform the search in grades file */
        SearchCourse(scourse, gradesFd, &lcourse, CMP_NAME | CMP_GRADE);

        if( CourseCmp(scourse, lcourse, CMP_NAME) == 0 && lcourse.grade >= 5.0 ) {
            coursesPassed++;
            totalCredits += weight;
            sum += weight * lcourse.grade;
        }
    }

    if( sum > 0.0 ){
        printf("\nTotal of %d courses passed\n", coursesPassed);
        printf("\nAverage grade: %5.2lf/10.0\n\n", sum / ((double)totalCredits));
    }
    else{
        printf("\nNo passed courses found!\n\n");
    }

}

/*****************************************************************************/

/* Returns 0 if the file is empty, 1 otherwise */
char SearchCourse(Course course, int fd, Course * lcourse, const int compareMode){
    char line[LINE_SIZE + 1];
    int nLines = GetLineCount(fd);

    /* File is empty */
    if( nLines <= 0 ){
        FileSeek(fd, 0, SEEK_SET);
        return 0;
    }
    /***************/

    /* Binary search bounds */
    int mid, compare;
    int low = 0;
    int high = nLines - 1;
    /*--------------------*/

    while( low < high ){
        FileSeek(fd, 0, SEEK_SET);

        mid = (low + high) / 2;
        FileSeek(fd, mid*LINE_SIZE, SEEK_CUR);

        ReadLine(fd, line);
        StringToCourse(lcourse, line);

        compare = CourseCmp( *lcourse, course, compareMode);
        if( compare == 0 ){
            break;
        }
        else if( compare < 0 ){
            low = mid + 1;
        }
        else{
            high = mid ;
        }
    }

    /* Read mid line one more time to get course */
    mid = (low + high) / 2;

    FileSeek(fd, 0, SEEK_SET);
    FileSeek(fd, mid*LINE_SIZE, SEEK_CUR);

    ReadLine(fd, line);
    StringToCourse(lcourse, line);
    RemoveTrailingSpaces(lcourse->name);

    /* Rewind to the start of that line */
    FileSeek(fd, -LINE_SIZE, SEEK_CUR);
    return 1;
}

/*****************************************************************************/

ssize_t GetLine(char *str, size_t size){
    /* Reads a line from stdin */
    char *currentPos = str;

    /* used to throw away the last line feed, if any */
    if( (*currentPos = getchar() ) != '\n'){
        currentPos++;
        /* if it is a character move the  buffer, otherwise overwrite */
    }
    while( (*currentPos = getchar() ) != '\n' && ( currentPos-str < size )){
        currentPos ++;
    }
    *currentPos = '\0';
    return currentPos - str;
}

void GetCourseInfo(Course *course, int gradesFd, int coursesFd, const int mode){
    /* function used to get course infofrom user input
     * Flags defined at the start of the file are used to determine what
     * info we want to get.*/

    if( (mode & GET_NAME) != 0 ) {
        printf("Enter course name: ");
        GetLine(course->name, COURSE_MAX_LENGTH);
        RemoveTrailingSpaces( course->name );

        while (!IsCourseAvailable(coursesFd, course->name)){
            printf("Seems like this course doesn't exist!\nTry a different one: ");
            GetLine(course->name, COURSE_MAX_LENGTH);
        }
    }

    if ( (mode & GET_DATE) != 0 ) {
        printf("Enter date (mm/yy): ");
        scanf("%d/%d", &course->date.month, &course->date.year);
        while ( !IsValidDate(course->date ) ){
            printf("Invalid date!\n");

            printf("\nEnter date (mm/yy): ");
            scanf("%d/%d",  &course->date.month, &course->date.year);
        }
    }
    if ( (mode & GET_GRADE) != 0 ) {
        printf("Enter course grade: ");
        scanf("%f",&(course->grade));
        while ( (course->grade) < 0.0 || (course->grade) > 10.0 ){
            printf("Invalid grade!\n");

            printf("\nEnter course grade: ");
            scanf("%f", &(course->grade));
        }
    }
}

char IsCourseAvailable(int coursesFd, char *courseName) {
    /* Searches the courses file to check if a course is available */
    /* We have to remove padding spaces and weight so we can compare*/
    char line[COURSES_LINE_SIZE + 1];
    char *weightPos;

    RemoveTrailingSpaces(courseName);

    FileSeek(coursesFd, 0, SEEK_SET);
    while( CoursesReadLine(coursesFd, line) > 0 ) {
        weightPos = line + strlen(line) - 2; /*ignore line feed and null-term.*/
        *weightPos = '\0'; /*remove the weight, and line feed*/

        RemoveTrailingSpaces(line);
        if(strcmp(line,courseName) == 0){
            return 1;
        }
    }
    return 0;
}

void RemoveTrailingSpaces(char *str){
    int i = strlen(str) - 1;
    while( i >= 0 && str[i] == ' ') {
        i--;
    }
    str[i+1] = '\0';
}

/*****************************************************************************/

void PrintCourse(const Course course){
     printf("%s\tDate: %02d/20%02d\tGrade: %4.1f\n",
             course.name,
             course.date.month,
             course.date.year,
             course.grade
             );
}

void StringToCourse(Course *course, char *str){
    char formatStr[15];
    /*base that on delimiter instead of fixed length */
    strncpy(course->name, str, COURSE_MAX_LENGTH);
    course->name[COURSE_MAX_LENGTH]='\0';

    sprintf(formatStr, "%%d/%%d%c%%f", DELIMITER);
    sscanf(str + COURSE_MAX_LENGTH + 1, formatStr,
            &(course->date.month),
            &(course->date.year),
            &(course->grade)
          );

    RemoveTrailingSpaces(course->name);
}

void CourseToString(char *str, Course course){
    int i;
    for(i = strlen(course.name); i < COURSE_MAX_LENGTH; i++ ){
        course.name[i] = ' ';
    }
    course.name[COURSE_MAX_LENGTH] = '\0';

    sprintf(str, "%s%c%02d/%02d%c%04.1f\n",
            course.name,
            DELIMITER,
            course.date.month,
            course.date.year,
            DELIMITER,
            course.grade
           );
}

/*****************************************************************************/

/* Because we want to compare Course objects in many ways during the execution
 * of the program we use the mode flag. It uses an integer to encapsulate what
 * fields of the struct we want to compare each time and performs bitwise
 * operators to extract the mode. The valid modes are defined at the
 * beggining of the file
 */

int CourseCmp(const Course course1, const Course course2, const int mode){
    int compare;

    if( (mode & CMP_NAME) != 0) {
        compare = strcmp(course1.name, course2.name);
        if( compare != 0 )
            return compare;
    }

    if( (mode & CMP_DATE) != 0 ){
        compare = DateCmp(course1.date, course2.date);
        if (compare != 0)
            return compare;
    }

    if( (mode & CMP_GRADE) != 0 ){
        compare = -(course1.grade - course2.grade);
        if( compare != 0 ){
            return compare;
        }
    }

    return 0;
}

int DateCmp(const Date date1, const Date date2){
    if( date1.year != date2.year )
        return (date1.year > date2.year) ? -1 : 1;
    if( date1.month != date2.month )
        return (date1.month > date2.month) ? -1 : 1;

    return 0;
}

/*****************************************************************************/

void GetDateNow(Date *date) {
    struct tm *current;
    time_t timenow;

    time(&timenow); /*get time from epoch */
    current = localtime(&timenow);
    /*convert from time_t(calendar time) to tm struct */
    date->month = current->tm_mon + 1;
    date->year  = (current->tm_year + 1900) - 2000;
}

char IsValidDate(const Date date){
    Date dateNow;

    GetDateNow(&dateNow);
    /* time traveller detection logic */
    if( (dateNow.year < date.year) ||
           ((dateNow.year == date.year) && (dateNow.month < date.month)) ) {
        printf( "Sorry this application doesn't support time travelling\n");
        return 0;
    }
    else if( (date.month < 1) || (date.month > 12) || (date.year < 00) ) {
        return 0;
    }
    return 1;
}

/*****************************************************************************/

int GetLineCount(int fd){
    struct stat bf;

    /* get file info so we can divide size in bytes with line size to get
     * line count */
    if ( fstat(fd, &bf) != 0 ) {
        perror("Error reading file properties");
        exit(EXIT_FAILURE);
    }

    return (int)(bf.st_size/LINE_SIZE);
}

void FileTruncate(int fd, int length) {
    int success = ftruncate(fd, (off_t)length);
    if( success == -1 ) {
        perror("Error while truncating file. Terminating... \n");
        exit(EXIT_FAILURE);
    }
}

void FileSync(int fd) {
    int success = fsync(fd);
    if( success == -1 ) {
        perror("Error while syncing file. Terminating... \n");
        exit(EXIT_FAILURE);
    }
}

void FileSeek(int fd, int offset, int whence) {
    off_t currOffset = lseek(fd, (off_t)offset, whence);
    if( currOffset == -1 ){
        perror("Error while seeking in file. Terminating... \n");
        exit(EXIT_FAILURE);
    }
}

void WriteLine(int fd, char *line){
    sWrite(fd, line, LINE_SIZE);
}

void sWrite(int fd, void *buf, size_t count){
    ssize_t bytesWritten;
    ssize_t bytesLeft = count;

    do {
        bytesWritten = write(fd, buf, bytesLeft);
        if( bytesWritten == -1 ){
            perror("Error while writting. Terminating... \n");
            exit(EXIT_FAILURE);
        }
        else{
            bytesLeft -= bytesWritten;
            buf += bytesWritten;
        }
    } while( bytesLeft > 0 );
}

ssize_t CoursesReadLine(int fd, char *line){
    /*Function to read line from courses file.*/
    ssize_t bytesRead = sRead(fd, line, COURSES_LINE_SIZE);
    if(bytesRead == COURSES_LINE_SIZE){
        line[COURSES_LINE_SIZE] = '\0';
    }
    return bytesRead;
}

ssize_t ReadLine(int fd, char *line) {
    ssize_t bytesRead = sRead(fd, line, LINE_SIZE);
    if(bytesRead == LINE_SIZE){
        line[LINE_SIZE] = '\0';
    }
    return bytesRead;
}

ssize_t sRead(int fd, void *buf, size_t count){
/* reads until requested bytes are read, or until EOF, also prints errors */
    ssize_t bytesRead;/*# of bytes currently read*/
    size_t bytesLeft = count;

    do {
        bytesRead = read(fd, buf, bytesLeft);
        if (bytesRead == -1){
            /*we got error */
            perror("Error while reading. Terminating... \n");
            exit(EXIT_FAILURE);
        }
        else if ( bytesRead == 0){
            /* we got no more bytes to read, bailout */
            break;
        }
        else {
            /* we read something */
            bytesLeft -= bytesRead;
            buf += bytesRead; /*move our buffer for next read(if any)*/
        }
     } while(bytesLeft > 0);

    /* everything was OK so we have read our -count- requested bytes*/
    return count - bytesLeft;
}

/*****************************************************************************/
/*                                                                  -The end */
