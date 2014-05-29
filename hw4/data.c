#include <stdio.h>

#define BYTES 1024*100

int main(int argc, char * argv[] )
{
    freopen("data.txt", "w", stdout);

    int i;
    char c = 'A';
    for(i=0; i < BYTES; i++ ) {
        if(c == 'q') c++;
        putchar(c++);
        if( c == 'z' ) c = 'A';
    }

    putchar('q');
    return 0;
}
