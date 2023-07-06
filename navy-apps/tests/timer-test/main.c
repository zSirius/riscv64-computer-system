#include <unistd.h>
#include <stdio.h>
#include <sys/_timeval.h>
#include "../../libs/libndl/include/NDL.h"

int main(){
    NDL_Init(1);
    uint32_t oldTime, curTime;
    oldTime = NDL_GetTicks();
    while(1){
        curTime = NDL_GetTicks();
        if(curTime - oldTime >=500){
            printf("msec = %d. Time has gone 5 seconds.\n", oldTime);
            oldTime = curTime;
        }
    }

    return 0;
}