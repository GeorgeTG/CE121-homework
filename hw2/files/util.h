#ifndef UTIL_H
#define UTIL_H

typedef int (*operation_f)(int, int);

void ParseExpression(char *expr,
        char *parsed,
        char *operations[],
        operation_f do_operation[],
        int count);

int FileSeek(int fd, int offset, int whence);
char FileEmpty(int fd);

void TruncateFile(int fd);
void ReadLine(int fd, char *line);
void WriteLine(int fd, char *line, int count);
void Close(int fd);

void SendMessage(int fd, char *message);
void ReceiveMessage(int fd, char *bf);
#endif
