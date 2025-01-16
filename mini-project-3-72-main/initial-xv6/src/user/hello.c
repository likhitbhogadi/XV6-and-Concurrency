#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
 
int 
main(void) {
    printf("no. of COW page faults are %d\n", hello());
    // printf("return val of system call is %d\n", hello());
    // printf("Congrats !! You have successfully added new system  call in xv6 OS :) \n");
    exit(0);
}
