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

void Redirect(int oldfd, int newfd);

void ReadLine(char *buf);
void Write(int fd, char *buf, size_t count);
void Read(int fd, char *buf);

void Close(int fd);
void TruncateFile(int fd);
void WriteLine(int fd, char *line, int count);
/*
void SendMessage(struct pollfd fdinfo, char *message);
void ReceiveMessage(struct pollfd fdinfo, char *bf);
*/

#endif
