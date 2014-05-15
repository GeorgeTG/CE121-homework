#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

#define DELIMITER '|'

#define LINE_SIZE 76
#define COURSE_NAME_LEN 64

typedef struct date{
    int month;
    int year;

} Date;


char cmp(const Date d1, const Date d2){ 
    return (d1.year < d2.year) || (d1.year == d2.year && d1.month < d2.month);
}

char less(Date d1, float g1, Date d2, float g2){
    if( fabs(g1 - g2) > 1e-3 ){
        return (g1 < g2);
    }
    return cmp(d1, d2);
}

void ParseLine(char *line, char *name, Date *date, float *grade){
    char formatStr[20];
    sprintf(formatStr, "%%%dc%c%%2d/%%2d%c%%4.1f", COURSE_NAME_LEN, DELIMITER, DELIMITER);

    sscanf(line, formatStr, name, &(date->month), &(date->year), grade);
}

ssize_t sRead(int fd, void *buf, size_t count){
    ssize_t bytesRead;/*# of bytes currently read*/
    size_t bytesLeft = count;
    do {
        bytesRead = read(fd, buf, bytesLeft);
        if (bytesRead == -1){
            /*we got error */
            perror("File read error");
            return(-1);
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
    return count-bytesLeft;
}

ssize_t ReadLine(int fd,char *buffer) {
    /* TODO: change with bufferful read, or fixed byte size read*/
    /* reads a line from fd to buffer. Line feed included. */
    int chrCount=0;
    ssize_t byteRead;
    char buf;
    do{
        if( (byteRead = sRead(fd, &buf, 1)) > 0) {
            buffer[chrCount] = buf;
            chrCount ++;
            if(buf == '\n'){
                /*we got new line */
                buffer[chrCount] = '\0'; /*terminate the string*/
                return chrCount;
            }
        }
    } while(chrCount <= strlen(buffer) &&  byteRead !=0);
    return 0;/*no line feed was found */
}

ssize_t WriteLine(int fd, char * line){
    int code = write(fd, line, LINE_SIZE);
    if( code == -1 ){
        perror("Error while writting\n");
        exit(EXIT_FAILURE);
    }
}


void AddCourse(int fd, char* name, Date date, float grade){
    char lname[COURSE_NAME_LEN + 1];
    Date ldate;
    float lgrade;

    char _EOF = 0;
    char line[LINE_SIZE], sline[LINE_SIZE], nline[LINE_SIZE];
    while( 1 ){ 
        
        int code = ReadLine(fd, line); 
        if( code == 0 ){
            _EOF = 1;
            break;
        }
        else if( code == -1 ){
            printf("Failure\n");
            exit(EXIT_FAILURE);
        }

        ParseLine(line, lname, &ldate, &lgrade);
        
        int cmpResult = strcmp(lname, name);
        if( cmpResult == 0 ){
            if( lgrade >= 5.0 ){ // Doesnt work yet!
                printf("Lesson PASSED with %f. Cannot add this one!\n", lgrade);
                break;
            }

            
            while( less(date, grade, ldate, lgrade) && ReadLine(fd, line) ) {
                ParseLine(line, lname, &ldate, &lgrade);
                if( strcmp(lname, name) != 0) break;
            }
        }
        else if ( cmpResult > 0 ) 
            break;
    }

    if (!_EOF){
        strcpy(sline, line);

        lseek(fd, 0, SEEK_END);
        while( 1 ){
            lseek(fd, -LINE_SIZE, SEEK_CUR);
            
            ReadLine(fd, line);
            WriteLine(fd, line);

            lseek(fd, -2*LINE_SIZE, SEEK_CUR);

            if(strcmp(line, sline) == 0){
                break;
            }
        }
    }
    
    strcpy(nline, name);
    int i = strlen(name);
    for( ; i < COURSE_NAME_LEN; i++){
        nline[i] = ' ';
    }

    sprintf(nline + COURSE_NAME_LEN, "%c%2d/%2d%c%4.1f\n", DELIMITER,
            date.month, date.year, DELIMITER, grade);

    WriteLine(fd, nline);
}


int main(int argc, char * argv[])
{
    int i,fd = open("courses.txt", O_RDWR | O_CREAT, S_IRWXU);
    if( fd == -1 ){
        return EXIT_FAILURE;
    }
    
    char name[COURSE_NAME_LEN];
    Date date;
    float grade;
    for(i=0; i<5;i++){
        lseek(fd, 0, SEEK_SET);
        scanf("%s %d/%d %f", name, &date.month, &date.year, &grade);
        printf("%s\n%d/%d\n%f\n", name, date.month, date.year, grade);

        AddCourse(fd, name, date, grade);
    }


    close(fd);
    return 0;
}
