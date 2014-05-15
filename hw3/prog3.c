#include <stdio.h>
#include <unistd.h>

int main(int argc, char * argv[]) {
    while(1) {
        printf("%s, reporting alive and happy!!!\n", argv[0]);
        sleep(2);
    }
    return 0;
}
